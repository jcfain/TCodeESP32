"""Backup / restore the on-device LittleFS partition.

We don't go through the firmware's settings JSON — that would require
either Wi-Fi (chicken-and-egg on a fresh device) or a custom serial dump
command. Instead we ``read_flash`` the entire FS partition before the
re-flash and ``write_flash`` it back afterwards. Survives any firmware
change that keeps the same FS layout.
"""
from __future__ import annotations

import time
from dataclasses import dataclass
from pathlib import Path
from typing import Optional

from . import flash
from .manifest import FirmwareBundle


@dataclass
class Backup:
    path: Path
    offset: int
    size: int

    @property
    def is_empty(self) -> bool:
        """True if the dumped image is all 0xFF (i.e. blank flash, no settings)."""
        try:
            with self.path.open("rb") as fp:
                while chunk := fp.read(64 * 1024):
                    if any(b != 0xFF for b in chunk):
                        return False
            return True
        except OSError:
            return True


def backup_settings(port: str, bundle: FirmwareBundle, dest_dir: Path,
                    cb: flash.ProgressCb = None) -> Optional[Backup]:
    """Read the FS partition off the device and stash it in *dest_dir*.

    Returns ``None`` if no FS partition was found (device blank / unflashed).
    """
    dest_dir.mkdir(parents=True, exist_ok=True)
    found = flash.find_fs_partition(port, bundle.chip, cb=cb)
    if not found:
        if cb:
            cb("[backup] no filesystem partition on device — skipping backup")
        return None
    offset, size = found
    ts = time.strftime("%Y%m%d-%H%M%S")
    dest = dest_dir / f"{bundle.board_id}-fs-{ts}.bin"
    flash.read_partition(port, bundle.chip, offset, size, dest, cb=cb)
    return Backup(path=dest, offset=offset, size=size)


def restore_settings(port: str, bundle: FirmwareBundle, backup: Backup,
                     cb: flash.ProgressCb = None) -> None:
    """Write a previously saved FS image back into the same partition.

    The post-flash partition table may have changed offsets — re-read it
    and refuse to restore if the size shrank.
    """
    found = flash.find_fs_partition(port, bundle.chip, cb=cb)
    if not found:
        if cb:
            cb("[restore] post-flash device has no FS partition — skipping restore")
        return
    offset, size = found
    if size < backup.size:
        if cb:
            cb(f"[restore] new FS partition smaller ({size} < {backup.size}); "
               f"restore skipped to avoid truncation")
        return
    flash.write_partition(port, bundle.chip, offset, backup.path, cb=cb)
