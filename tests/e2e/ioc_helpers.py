"""Helpers for starting and stopping the zaberMotion IOC subprocess.

The IOC is started with a custom startup script that sources the real
envPaths (so EPICS module paths resolve correctly) but replaces the
hard-coded Zaber TCP address with the mock zaber devices's port.
"""

import asyncio
import platform
import subprocess
from pathlib import Path
from typing import Any

from aioca import caget

_REPO_ROOT = Path(__file__).parent.parent.parent
_IOC_ROOT = _REPO_ROOT / "iocs/zaberMotionIOC"
_arch = "linux-x86_64" if platform.system() == "Linux" else "darwin-aarch64"
_IOC_BINARY = _IOC_ROOT / f"bin/{_arch}/zaberMotion"
_ENV_PATHS = _IOC_ROOT / "iocBoot/iocZaberMotion/envPaths"

# PV that becomes readable once iocInit and motorUtilInit have completed.
_READY_PV = "zaberMotion:alldone"

# One IOC with two controllers: an X-MCC with linear stages, and one with rotary stages.
_ST_CMD_TEMPLATE = """\
< {envpaths}
cd "${{TOP}}"
dbLoadDatabase "dbd/zaberMotion.dbd"
zaberMotion_registerRecordDeviceDriver pdbbase
dbLoadRecords("${{MOTOR}}/db/motorUtil.db", "P=zaberMotion:")
dbLoadTemplate("{linear_substitutions}")
# Fast idle poll (100 ms) so status/warning-flag tests don't wait on a slow poll cycle.
ZaberMotionCreateController( \
    "XMCC_LINEAR", 4, 50, 100, "tcp://127.0.0.1:{linear_port}", 1, 1.0 1.0 1.0 0.001 \
)
ZaberControllerCreateProfile("XMCC_LINEAR", 50)
dbLoadTemplate("{rotary_substitutions}")
ZaberMotionCreateController( \
    "XMCC_ROTARY", 4, 50, 100, "tcp://127.0.0.1:{rotary_port}", 1, 1e-6 1e-6 1e-6 1e-6 \
)
iocInit
motorUtilInit("zaberMotion:")
"""

_SUBSTITUTIONS_DIR = Path(__file__).parent / "substitutions"
_LINEAR_SUBSTITUTIONS = _SUBSTITUTIONS_DIR / "motor.substitutions.linear-test.zaber"
_ROTARY_SUBSTITUTIONS = _SUBSTITUTIONS_DIR / "motor.substitutions.rotary-test.zaber"


def render_st_cmd(tmp_path: Path, linear_port: int, rotary_port: int) -> Path:
    """Write a startup script (linear + rotary controllers) and return its path."""
    content = _ST_CMD_TEMPLATE.format(
        envpaths=_ENV_PATHS,
        linear_port=linear_port,
        rotary_port=rotary_port,
        linear_substitutions=_LINEAR_SUBSTITUTIONS,
        rotary_substitutions=_ROTARY_SUBSTITUTIONS,
    )
    st_cmd = tmp_path / "st.cmd"
    st_cmd.write_text(content)
    return st_cmd


async def _wait_for_pv(pv: str, timeout: float = 30.0) -> None:
    """Block until pv is accessible via channel access."""
    deadline = asyncio.get_running_loop().time() + timeout
    last_exc: Exception | None = None
    while True:
        try:
            result: Any = await caget(pv, timeout=2.0)
            if result is not None:
                return
        except Exception as exc:  # noqa: BLE001
            last_exc = exc
        if asyncio.get_running_loop().time() > deadline:
            raise TimeoutError(
                f"IOC did not initialize ({pv!r} not accessible). "
                f"Last error: {last_exc}"
            )
        await asyncio.sleep(0.5)


def start_ioc(st_cmd: Path) -> subprocess.Popen[bytes]:
    """Launch the IOC binary and return the process handle."""
    return subprocess.Popen([str(_IOC_BINARY), str(st_cmd)], stdin=subprocess.PIPE)


async def wait_for_ioc_ready(timeout: float = 30.0) -> None:
    """Block until the IOC's motorUtil ready PV is accessible."""
    await _wait_for_pv(_READY_PV, timeout=timeout)


def stop_ioc(proc: subprocess.Popen[bytes]) -> None:
    """Terminate the IOC process and wait for it to exit."""
    proc.terminate()
    try:
        proc.wait(timeout=10)
    except subprocess.TimeoutExpired:
        proc.kill()
        proc.wait()
