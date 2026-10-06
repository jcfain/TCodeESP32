"""Push Wi-Fi credentials to the device via the serial console.

The firmware exposes ``#wifi-ssid:<ssid>`` and ``#wifi-pass:<pwd>`` plus
``$save`` and ``#restart`` commands (see ``SystemCommandHandler.h``).
"""
from __future__ import annotations

import time
from typing import Optional

from .serial_console import (
    DeviceInfo,
    open_console,
    query_device_info,
    read_for,
    write_line,
)


def push_credentials(port: str, ssid: str, password: str,
                     wait_for_ip: float = 25.0,
                     cb=None) -> Optional[str]:
    """Send Wi-Fi creds, save, restart, then wait for the new IP.

    Returns the IP address string the device prints, or ``None`` on timeout.
    """
    log = cb or (lambda *_: None)

    with open_console(port, timeout=0.2) as ser:
        # Drain any pending output.
        read_for(ser, 0.5)
        log(f"[wifi] setting SSID '{ssid}'")
        write_line(ser, f"#wifi-ssid:{ssid}")
        time.sleep(0.2)
        log("[wifi] setting password (redacted)")
        write_line(ser, f"#wifi-pass:{password}")
        time.sleep(0.2)
        log("[wifi] $save")
        write_line(ser, "$save")
        time.sleep(0.5)
        log("[wifi] #restart")
        write_line(ser, "#restart")
        # Capture banner + 'IP:' line over the wait window.
        banner = read_for(ser, wait_for_ip)

    log(banner.strip())
    # Try to extract an IP — firmware prints "IP: x.x.x.x" via #ip / banner.
    import re
    m = re.search(r"\b(\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3})\b", banner)
    if m and m.group(1) not in ("0.0.0.0", "127.0.0.1", "255.255.255.255"):
        return m.group(1)
    return None


def query(port: str) -> DeviceInfo:
    return query_device_info(port)
