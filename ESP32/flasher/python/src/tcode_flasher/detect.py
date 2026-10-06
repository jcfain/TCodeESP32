"""Detect and identify TCode controllers connected over USB."""
from __future__ import annotations

from dataclasses import dataclass
from typing import List, Optional

from serial.tools import list_ports  # type: ignore[import-untyped]

from .manifest import FirmwareBundle
from .serial_console import DeviceInfo, query_device_info


# USB IDs we recognise as "an ESP32-class chip the firmware might run on".
# Format: lowercase "vid:pid". A bundle's manifest can claim a subset of
# these; if no bundle matches by VID:PID we fall back to probing the boot
# banner for the board name.
_KNOWN_USB_IDS = {
    # Espressif native USB CDC (S2/S3)
    "303a:1001",
    "303a:0002",
    # CP210x (used on most dev boards)
    "10c4:ea60",
    "10c4:ea70",
    # CH340/CH341
    "1a86:7523",
    "1a86:55d4",
    # FTDI
    "0403:6001",
    "0403:6010",
    "0403:6014",
    "0403:6015",
}


@dataclass
class CandidatePort:
    port: str
    description: str
    vid_pid: Optional[str]
    matched_bundle: Optional[FirmwareBundle] = None
    info: Optional[DeviceInfo] = None

    @property
    def label(self) -> str:
        bits = [self.port]
        if self.matched_bundle:
            bits.append(f"[{self.matched_bundle.display_name}]")
        elif self.description:
            bits.append(f"({self.description})")
        if self.info and self.info.firmware_version:
            bits.append(f"fw={self.info.firmware_version}")
        return " ".join(bits)


def _format_vid_pid(p) -> Optional[str]:
    if p.vid is None or p.pid is None:
        return None
    return f"{p.vid:04x}:{p.pid:04x}"


def list_candidate_ports(bundles: List[FirmwareBundle]) -> List[CandidatePort]:
    """Enumerate serial ports plausibly hosting a TCode controller."""
    cands: List[CandidatePort] = []
    for p in list_ports.comports():
        vid_pid = _format_vid_pid(p)
        # Match against bundle-claimed IDs first.
        bundle = None
        if vid_pid:
            for b in bundles:
                if vid_pid in (s.lower() for s in b.usb_vid_pid):
                    bundle = b
                    break
        if bundle is None and vid_pid not in _KNOWN_USB_IDS:
            # Not a chip we recognise.
            continue
        cands.append(
            CandidatePort(
                port=p.device,
                description=p.description or "",
                vid_pid=vid_pid,
                matched_bundle=bundle,
            )
        )
    return cands


def identify(candidate: CandidatePort, bundles: List[FirmwareBundle]) -> CandidatePort:
    """Probe *candidate* for boot-banner identification.

    Mutates ``candidate.info`` and ``candidate.matched_bundle`` in place,
    then returns it.
    """
    candidate.info = query_device_info(candidate.port)
    if candidate.matched_bundle is None and candidate.info.board_type:
        bt = candidate.info.board_type.lower()
        for b in bundles:
            if b.board_id.lower() == bt or bt in b.board_id.lower():
                candidate.matched_bundle = b
                break
    return candidate


def auto_pick(bundles: List[FirmwareBundle]) -> Optional[CandidatePort]:
    """Return a single candidate if one is unambiguous, else ``None``."""
    cands = list_candidate_ports(bundles)
    if len(cands) == 1:
        return identify(cands[0], bundles)
    return None
