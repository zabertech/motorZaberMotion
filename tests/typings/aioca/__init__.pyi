from collections.abc import Iterator
from typing import Any

class AugmentedValue:
    name: str
    ok: bool
    severity: int
    status: int
    element_count: int

    def __int__(self) -> int: ...
    def __float__(self) -> float: ...
    def __index__(self) -> int: ...
    def __iter__(self) -> Iterator[Any]: ...
    def __len__(self) -> int: ...

async def caget(
    pv: str,
    *,
    count: int = ...,
    timeout: float = ...,
) -> AugmentedValue: ...
async def caput(
    pv: str,
    value: float | str | list[float],
    *,
    wait: bool = ...,
    timeout: float = ...,
) -> object: ...
def purge_channel_caches() -> None: ...
