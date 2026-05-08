"""Bundled firmware manifest.

Each board ships as a folder under ``firmware/<board_id>/`` containing the
four artefacts needed by ``esptool write_flash`` plus a small JSON manifest.

Schema (``firmware/<board>/manifest.json``)::

    {
        "board_id":      "sr6_pcb",
        "display_name":  "SR6 PCB (ESP32-S3)",
        "chip":          "esp32s3",
        "firmware_version": "0.483b",
        "flash_mode":    "dio",
        "flash_freq":    "80m",
        "flash_size":    "4MB",
        "usb_vid_pid":   ["303a:1001", "303a:0002"],
        "fs_partition":  "spiffs",
        "files": [
            {"offset": "0x0000",   "name": "bootloader.bin"},
            {"offset": "0x8000",   "name": "partitions.bin"},
            {"offset": "0xe000",   "name": "boot_app0.bin"},
            {"offset": "0x10000",  "name": "firmware.bin"},
            {"offset": "0x290000", "name": "littlefs.bin"}
        ]
    }

The flasher looks for manifests next to the executable (PyInstaller layout)
and inside the source tree (``flasher/python/firmware/``) when running from
checkout.
"""
from __future__ import annotations

import json
import os
import sys
from dataclasses import dataclass, field
from pathlib import Path
from typing import Iterable, List, Optional


@dataclass(frozen=True)
class FirmwareFile:
    offset: int
    name: str
    path: Path


@dataclass(frozen=True)
class FirmwareBundle:
    board_id: str
    display_name: str
    chip: str
    firmware_version: str
    flash_mode: str
    flash_freq: str
    flash_size: str
    usb_vid_pid: List[str]
    fs_partition: str
    files: List[FirmwareFile]
    root: Path

    @property
    def fs_file(self) -> Optional[FirmwareFile]:
        for f in self.files:
            if f.name in ("littlefs.bin", "spiffs.bin"):
                return f
        return None


def _bundled_root() -> Path:
    """Return the directory that holds bundled firmware payloads.

    When running under PyInstaller, payloads live next to the executable in
    ``firmware/``. In dev mode they live at ``flasher/python/firmware/``.
    """
    if getattr(sys, "frozen", False):
        # PyInstaller one-file: data is unpacked under sys._MEIPASS.
        meipass = getattr(sys, "_MEIPASS", None)
        if meipass:
            return Path(meipass) / "firmware"
        return Path(sys.executable).resolve().parent / "firmware"
    return Path(__file__).resolve().parents[2] / "firmware"


def _parse_offset(value: str | int) -> int:
    if isinstance(value, int):
        return value
    return int(value, 0)  # accepts "0x10000" or "65536"


def load_bundles(root: Optional[Path] = None) -> List[FirmwareBundle]:
    """Discover all firmware bundles under *root* (or the default location)."""
    base = root or _bundled_root()
    if not base.is_dir():
        return []
    bundles: List[FirmwareBundle] = []
    for child in sorted(base.iterdir()):
        manifest_path = child / "manifest.json"
        if not manifest_path.is_file():
            continue
        try:
            data = json.loads(manifest_path.read_text(encoding="utf-8"))
        except (OSError, json.JSONDecodeError) as exc:
            print(f"[flasher] skipping bad manifest {manifest_path}: {exc}", file=sys.stderr)
            continue
        files = [
            FirmwareFile(
                offset=_parse_offset(entry["offset"]),
                name=entry["name"],
                path=child / entry["name"],
            )
            for entry in data.get("files", [])
        ]
        # Make sure every referenced bin exists; otherwise the bundle is
        # unusable and we omit it from the menu.
        if not files or not all(f.path.is_file() for f in files):
            print(f"[flasher] bundle {child.name} missing payload files", file=sys.stderr)
            continue
        bundles.append(
            FirmwareBundle(
                board_id=data["board_id"],
                display_name=data.get("display_name", data["board_id"]),
                chip=data.get("chip", "esp32"),
                firmware_version=data.get("firmware_version", "?"),
                flash_mode=data.get("flash_mode", "dio"),
                flash_freq=data.get("flash_freq", "40m"),
                flash_size=data.get("flash_size", "4MB"),
                usb_vid_pid=list(data.get("usb_vid_pid", [])),
                fs_partition=data.get("fs_partition", "spiffs"),
                files=files,
                root=child,
            )
        )
    return bundles


def find_bundle(board_id: str, bundles: Iterable[FirmwareBundle]) -> Optional[FirmwareBundle]:
    for b in bundles:
        if b.board_id == board_id:
            return b
    return None
