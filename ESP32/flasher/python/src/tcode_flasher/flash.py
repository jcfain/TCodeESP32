"""Wrapper around the esptool Python module.

We invoke esptool in-process via ``esptool.main(args)`` so the flasher exe
stays a single binary with no spawned subprocess.

All functions accept an optional ``progress`` callback ``cb(line: str)`` so
the GUI can stream esptool output into a status pane.
"""
from __future__ import annotations

import io
import re
import sys
import threading
from contextlib import contextmanager
from pathlib import Path
from typing import Callable, Iterator, List, Optional

import esptool  # type: ignore[import-untyped]

from .manifest import FirmwareBundle


ProgressCb = Optional[Callable[[str], None]]


# ---------------------------------------------------------------------------
# Output capture
# ---------------------------------------------------------------------------
class _StreamTee(io.TextIOBase):
    """Forward ``write`` to both an underlying stream and a callback."""
    def __init__(self, underlying, cb: ProgressCb):
        self._u = underlying
        self._cb = cb
        self._buf = ""

    def write(self, s):  # type: ignore[override]
        if self._u is not None:
            try:
                self._u.write(s)
            except Exception:
                pass
        if self._cb is None:
            return len(s)
        self._buf += s
        while "\n" in self._buf:
            line, self._buf = self._buf.split("\n", 1)
            try:
                self._cb(line)
            except Exception:
                pass
        return len(s)

    def flush(self):  # type: ignore[override]
        if self._u is not None:
            try:
                self._u.flush()
            except Exception:
                pass


_capture_lock = threading.Lock()


@contextmanager
def _capture_stdio(cb: ProgressCb) -> Iterator[None]:
    if cb is None:
        yield
        return
    with _capture_lock:
        orig_out, orig_err = sys.stdout, sys.stderr
        sys.stdout = _StreamTee(orig_out, cb)
        sys.stderr = _StreamTee(orig_err, cb)
        try:
            yield
        finally:
            sys.stdout, sys.stderr = orig_out, orig_err


def _run(args: List[str], cb: ProgressCb) -> None:
    """Invoke ``esptool.main(args)`` and re-raise on failure."""
    if cb:
        cb("$ esptool " + " ".join(str(a) for a in args))
    with _capture_stdio(cb):
        esptool.main(args)


# ---------------------------------------------------------------------------
# Operations
# ---------------------------------------------------------------------------
def _common(port: str, chip: str, baud: int) -> List[str]:
    return ["--chip", chip, "--port", port, "--baud", str(baud)]


def chip_id(port: str, chip: str = "auto", baud: int = 460800,
            cb: ProgressCb = None) -> None:
    _run(_common(port, chip, baud) + ["chip_id"], cb)


def write_flash(port: str, bundle: FirmwareBundle, baud: int = 921600,
                cb: ProgressCb = None) -> None:
    args = _common(port, bundle.chip, baud) + [
        "write_flash",
        "--flash_mode", bundle.flash_mode,
        "--flash_freq", bundle.flash_freq,
        "--flash_size", bundle.flash_size,
    ]
    for f in bundle.files:
        args.extend([hex(f.offset), str(f.path)])
    _run(args, cb)


def write_partition(port: str, chip: str, offset: int, image: Path,
                    baud: int = 921600, cb: ProgressCb = None) -> None:
    args = _common(port, chip, baud) + [
        "write_flash", hex(offset), str(image),
    ]
    _run(args, cb)


def read_partition(port: str, chip: str, offset: int, size: int, dest: Path,
                   baud: int = 460800, cb: ProgressCb = None) -> None:
    args = _common(port, chip, baud) + [
        "read_flash", hex(offset), hex(size), str(dest),
    ]
    _run(args, cb)


# ---------------------------------------------------------------------------
# Partition table parsing
# ---------------------------------------------------------------------------
# Read the on-device partition table and return the (offset, size) of the
# filesystem partition (spiffs/littlefs). Used for backup/restore so we
# don't rely on the bundled CSV matching what's already on the chip.

PartitionEntry = tuple[str, str, int, int]


_PART_TYPE_DATA = 0x01
_PART_SUBTYPE_SPIFFS = 0x82
_PART_SUBTYPE_LITTLEFS = 0x83  # not used by esp-idf; some forks


def _parse_partition_blob(blob: bytes) -> List[PartitionEntry]:
    """Parse the binary partition table format (32 bytes per entry)."""
    entries: List[PartitionEntry] = []
    for i in range(0, len(blob), 32):
        e = blob[i : i + 32]
        if len(e) < 32:
            break
        magic = e[0:2]
        if magic != b"\xaa\x50":
            # MD5 entry or end of table.
            continue
        ptype = e[2]
        subtype = e[3]
        offset = int.from_bytes(e[4:8], "little")
        size = int.from_bytes(e[8:12], "little")
        name = e[12:28].rstrip(b"\x00").decode("ascii", "replace")
        # crude type label
        if ptype == _PART_TYPE_DATA and subtype in (_PART_SUBTYPE_SPIFFS, _PART_SUBTYPE_LITTLEFS):
            label = "fs"
        elif ptype == 0x00:
            label = "app"
        else:
            label = "data"
        entries.append((name, label, offset, size))
    return entries


def find_fs_partition(port: str, chip: str, cb: ProgressCb = None) -> Optional[tuple[int, int]]:
    """Read the partition table and return ``(offset, size)`` of the FS part."""
    import tempfile

    with tempfile.NamedTemporaryFile(delete=False, suffix=".bin") as tmp:
        tmp_path = Path(tmp.name)
    try:
        # Standard partition-table location is 0x8000, max size 0x1000.
        read_partition(port, chip, 0x8000, 0x1000, tmp_path, baud=460800, cb=cb)
        blob = tmp_path.read_bytes()
    finally:
        try:
            tmp_path.unlink()
        except OSError:
            pass

    for name, label, offset, size in _parse_partition_blob(blob):
        if label == "fs":
            if cb:
                cb(f"Filesystem partition '{name}' @ {hex(offset)} size={size}")
            return offset, size
    return None
