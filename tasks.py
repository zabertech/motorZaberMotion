#!/usr/bin/env python3
"""Build and test tasks for the motorZaberMotion module.

Usage: uv run tasks.py <task>
"""

import argparse
import os
import re
import shutil
import subprocess
import sys
from collections.abc import Callable
from pathlib import Path

ZML_VERSION = "9.0.0"

MODULE_DIR = Path(__file__).resolve().parent


def _epics_base() -> Path:
    epics_base = os.environ.get("EPICS_BASE")
    if not epics_base:
        sys.exit("EPICS_BASE environment variable must be set")
    return Path(epics_base)


def _epics_support() -> Path:
    return Path(os.path.normpath(f"{_epics_base()}/../support"))


def _run(command: list[str], cwd: Path | None = None, env: dict[str, str] | None = None) -> None:
    print("+", " ".join(command))
    subprocess.run(command, cwd=cwd, check=True, env=env)


def _sub_file(path: Path, *subs: tuple[str, str], count: int = 1) -> None:
    contents = path.read_text()
    for pattern, replacement in subs:
        contents = re.sub(pattern, replacement, contents, count=count, flags=re.MULTILINE)
    path.write_text(contents)


def _update_release_for_module(module_path: Path) -> None:
    _sub_file(
        module_path / "configure" / "RELEASE",
        (r"^#?\s*SUPPORT\s*=.*$", f"SUPPORT = {_epics_support()}"),
        (r"^#?\s*EPICS_BASE\s*=.*$", f"EPICS_BASE = {os.environ['EPICS_BASE']}"),
    )


def _update_support_configs() -> None:
    support = _epics_support()
    for module in ("asyn", "sequencer", "autosave", "busy", "motor"):
        _update_release_for_module(support / module)

    # asyn-specific changes
    _sub_file(
        support / "asyn" / "configure" / "RELEASE",
        (r"^#?\s*SNCSEQ\s*=.*", f"SNCSEQ={support}/sequencer"),
    )
    if sys.platform == "linux":
        config_site = support / "asyn" / "configure" / "CONFIG_SITE"
        print("updating file with asyn path: ", config_site)
        _sub_file(config_site, (r"^#? *TIRPC *=.*", "TIRPC = YES"))

    # busy-specific changes
    _sub_file(
        support / "busy" / "configure" / "RELEASE",
        (r"^#?\s*ASYN\s*=.*", f"ASYN={support}/asyn"),
        (r"^#?\s*BUSY\s*=.*", f"BUSY={support}/busy"),
        (r"^#?\s*AUTOSAVE\s*=.*", f"AUTOSAVE={support}/autosave"),
    )

    # motor-specific changes
    _sub_file(
        support / "motor" / "configure" / "RELEASE",
        (r"^#?\s*ASYN\s*=.*", f"ASYN={support}/asyn"),
        (r"^#?\s*BUSY\s*=.*", f"BUSY={support}/busy"),
        (r"^#?\s*SNCSEQ\s*=.*", f"SNCSEQ={support}/sequencer"),
        (r"^#?\s*IPAC\s*=(.*)", r"#IPAC=\g<1>"),
    )

    modules_makefile = support / "motor" / "modules" / "Makefile"
    _sub_file(modules_makefile, (r"^SUBMODULES\s*\+=(.*)", r"#SUBMODULES=\g<1>"), count=0)
    _sub_file(modules_makefile, (r"^#SUBMODULES.*", "SUBMODULES += motorZaberMotion"))

    # changes specific to this module
    (MODULE_DIR / "configure" / "CONFIG_SITE.local").write_text("BUILD_IOCS=YES\n")


def update_support_package_version() -> None:
    """Set ZML_VERSION in zaberMotionSupport/Makefile to the version pinned in this script."""
    _sub_file(
        MODULE_DIR / "zaberMotionSupport" / "Makefile",
        (r"^(ZML_VERSION\s*=\s*).*$", rf"\g<1>{ZML_VERSION}"),
    )


def build() -> None:
    """Configure all EPICS support modules and build them along with motor."""
    _update_support_configs()
    support = _epics_support()
    for module in ("sequencer", "asyn", "autosave", "busy"):
        _run(["make", "-C", str(support / module)])
    build_motor()


def build_motor() -> None:
    """Build the motor module (including this driver)."""
    _run(["make", "-C", str(_epics_support() / "motor")])


def clean_motor() -> None:
    """Run make realclean in the motor module."""
    _run(["make", "realclean", "-C", str(_epics_support() / "motor")])


def run_ioc(st_cmd: str | None = None) -> None:
    """Start the example IOC via its start.sh script (defaults to st.cmd.linear-stage)."""
    ioc_boot = MODULE_DIR / "iocs" / "zaberMotionIOC" / "iocBoot" / "iocZaberMotion"
    env = os.environ.copy()
    if not env.get("EPICS_HOST_ARCH"):
        arch_script = _epics_base() / "startup" / "EpicsHostArch"
        arch = subprocess.run([str(arch_script)], check=True, capture_output=True, text=True)
        env["EPICS_HOST_ARCH"] = arch.stdout.strip()
    _run(["./start.sh", *([st_cmd] if st_cmd else [])], cwd=ioc_boot, env=env)


def test() -> None:
    """Build and run the C++ unit tests, then lint, type-check and run the Python e2e tests."""
    build_dir = MODULE_DIR / "zaberMotionApp" / "test" / "build"
    shutil.rmtree(build_dir, ignore_errors=True)
    build_dir.mkdir(parents=True)

    _run(["cmake", ".."], cwd=build_dir)
    _run(["make", f"-j{os.cpu_count() or 1}"], cwd=build_dir)
    _run(["ctest", "--verbose"], cwd=build_dir)

    _run(["uv", "run", "ruff", "check", "tests", "tasks.py"], cwd=MODULE_DIR)
    _run(["uv", "run", "pyright", "tests", "tasks.py"], cwd=MODULE_DIR)
    _run(["uv", "run", "pytest", "tests"], cwd=MODULE_DIR)


TASKS: dict[str, Callable[[], None]] = {
    "update_support_package_version": update_support_package_version,
    "build": build,
    "build_motor": build_motor,
    "clean_motor": clean_motor,
    "run_ioc": run_ioc,
    "test": test,
}


def main() -> None:
    """Run the task named on the command line."""
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("task", choices=TASKS)
    parser.add_argument("st_cmd", nargs="?", help="st.cmd file for the run_ioc task")
    args = parser.parse_args()
    if args.task == "run_ioc":
        run_ioc(args.st_cmd)
    elif args.st_cmd:
        parser.error(f"{args.task} does not take arguments")
    else:
        TASKS[args.task]()


if __name__ == "__main__":
    main()
