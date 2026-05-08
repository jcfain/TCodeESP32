"""Entry point for ``python -m tcode_flasher`` and the PyInstaller exe."""
from __future__ import annotations

import argparse
import sys
import tempfile
from pathlib import Path
from typing import Optional

from . import __version__, backup as backup_mod, detect, flash, wifi
from .manifest import find_bundle, load_bundles


def _build_arg_parser() -> argparse.ArgumentParser:
    p = argparse.ArgumentParser(prog="tcode-flasher",
        description="Cross-platform flasher for TCode ESP32 controllers.")
    p.add_argument("--version", action="version", version=f"tcode-flasher {__version__}")
    p.add_argument("--cli", action="store_true", help="Run the headless CLI flow instead of the GUI wizard.")
    p.add_argument("--port", help="Serial port (auto-detected if omitted)")
    p.add_argument("--board", help="Board id (auto-detected if omitted, e.g. sr6_pcb)")
    p.add_argument("--ssid", help="Wi-Fi SSID to push after flashing")
    p.add_argument("--password", help="Wi-Fi password (used with --ssid)")
    p.add_argument("--no-backup", action="store_true", help="Skip pre-flash settings backup")
    p.add_argument("--no-restore", action="store_true", help="Skip post-flash settings restore")
    p.add_argument("--no-wifi", action="store_true", help="Skip Wi-Fi credential push")
    p.add_argument("--list", action="store_true", help="List candidate ports and exit")
    return p


def _cli_log(line: str) -> None:
    print(line)


def _run_cli(args) -> int:
    bundles = load_bundles()
    if not bundles:
        print("No firmware bundles available; install firmware/<board>/ payloads next to the executable.",
              file=sys.stderr)
        return 2

    if args.list:
        for c in detect.list_candidate_ports(bundles):
            detect.identify(c, bundles)
            print(c.label)
        return 0

    cands = detect.list_candidate_ports(bundles)
    if args.port:
        cands = [c for c in cands if c.port == args.port]
    if not cands:
        print("No matching device found. Try --list.", file=sys.stderr)
        return 2

    candidate = cands[0]
    detect.identify(candidate, bundles)

    if args.board:
        candidate.matched_bundle = find_bundle(args.board, bundles)
    if not candidate.matched_bundle and len(bundles) == 1:
        candidate.matched_bundle = bundles[0]
    if not candidate.matched_bundle:
        print("Could not identify the board. Pass --board <id>; available:",
              ", ".join(b.board_id for b in bundles), file=sys.stderr)
        return 2

    bundle = candidate.matched_bundle
    port = candidate.port
    print(f"Target: {bundle.display_name} on {port}")
    backup: Optional[backup_mod.Backup] = None

    if not args.no_backup:
        print("--- backup ---")
        backup = backup_mod.backup_settings(port, bundle, Path(tempfile.gettempdir()) / "tcode_flasher_backups",
                                            cb=_cli_log)
    print("--- flash ---")
    flash.write_flash(port, bundle, cb=_cli_log)
    if backup and not args.no_restore and not backup.is_empty:
        print("--- restore ---")
        backup_mod.restore_settings(port, bundle, backup, cb=_cli_log)

    if args.ssid and not args.no_wifi:
        print("--- wifi ---")
        ip = wifi.push_credentials(port, args.ssid, args.password or "", cb=_cli_log)
        if ip:
            print(f"Device IP: {ip}")
        else:
            print("Wi-Fi credentials sent; device IP not captured before timeout.")

    print("Done.")
    return 0


def main() -> int:
    parser = _build_arg_parser()
    args = parser.parse_args()

    if args.cli or args.list:
        return _run_cli(args)

    # GUI path. Imported lazily so headless CI can run --list without tk.
    from .wizard import main as gui_main
    return gui_main()


if __name__ == "__main__":
    sys.exit(main())
