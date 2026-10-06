"""Serial console helper.

Talks to the device's existing ``SerialHandler`` text protocol — newline-
terminated commands like ``#wifi-ssid:MyAp`` and ``$save``. Also captures
the boot banner (``Firmware version: X.Y.Z``) so the wizard can show the
running firmware before / after a flash.
"""
from __future__ import annotations

import re
import time
from contextlib import contextmanager
from dataclasses import dataclass
from typing import Iterator, Optional

import serial  # type: ignore[import-untyped]


_VERSION_RE = re.compile(r"Firmware version[:\s]+([\w.+-]+)")
_BOARD_RE = re.compile(r"Board ?type[:\s=]+(\w+)", re.IGNORECASE)
_DEVICE_RE = re.compile(r"Device ?type[:\s=]+(\w+)", re.IGNORECASE)


@dataclass
class DeviceInfo:
    firmware_version: Optional[str] = None
    board_type: Optional[str] = None
    device_type: Optional[str] = None


@contextmanager
def open_console(port: str, baud: int = 115200, timeout: float = 0.5) -> Iterator[serial.Serial]:
    """Open *port* with conservative settings.

    We deliberately do **not** toggle DTR/RTS on open — on ESP32-S3 boards
    the auto-reset circuit triggers a reboot when those lines toggle, which
    we'd then race against the boot banner read.
    """
    s = serial.Serial()
    s.port = port
    s.baudrate = baud
    s.timeout = timeout
    s.dtr = False
    s.rts = False
    s.open()
    try:
        yield s
    finally:
        try:
            s.close()
        except Exception:
            pass


def write_line(ser: serial.Serial, line: str) -> None:
    ser.write((line + "\n").encode("utf-8", errors="replace"))
    ser.flush()


def read_for(ser: serial.Serial, duration: float) -> str:
    """Read every byte the device emits over *duration* seconds."""
    deadline = time.monotonic() + duration
    chunks: list[bytes] = []
    while time.monotonic() < deadline:
        data = ser.read(4096)
        if data:
            chunks.append(data)
        else:
            time.sleep(0.05)
    return b"".join(chunks).decode("utf-8", errors="replace")


def reboot_into_app(port: str) -> None:
    """Pulse RTS to trigger the ESP32 auto-reset circuit (RTS = EN line).

    Skipped silently if the port can't be opened — some USB-CDC bridges
    don't expose RTS at all.
    """
    try:
        with serial.Serial(port) as s:
            s.dtr = False
            s.rts = True
            time.sleep(0.05)
            s.rts = False
            time.sleep(0.05)
    except Exception:
        pass


def query_device_info(port: str, banner_wait: float = 3.0) -> DeviceInfo:
    """Reset the device and capture the boot banner.

    Returns whatever fields could be parsed; callers should treat ``None``
    fields as "unknown".
    """
    info = DeviceInfo()
    try:
        with open_console(port, timeout=0.2) as ser:
            # Trigger a soft reset via RTS so we always see the banner, even
            # if the device has been running for hours.
            ser.dtr = False
            ser.rts = True
            time.sleep(0.05)
            ser.rts = False
            banner = read_for(ser, banner_wait)
    except (serial.SerialException, OSError):
        return info

    if m := _VERSION_RE.search(banner):
        info.firmware_version = m.group(1).strip()
    if m := _BOARD_RE.search(banner):
        info.board_type = m.group(1).strip()
    if m := _DEVICE_RE.search(banner):
        info.device_type = m.group(1).strip()
    return info
