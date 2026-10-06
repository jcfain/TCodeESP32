# -*- mode: python ; coding: utf-8 -*-
"""PyInstaller spec for the TCode ESP32 flasher.

Builds a single-file executable per OS. The ``firmware`` folder next to this
spec is bundled inside the executable; users get one self-contained binary
with the latest firmware payload baked in.

Build:
    pyinstaller flasher.spec

Output: dist/TCodeFlasher (or .exe / .app)
"""
import sys
from pathlib import Path

block_cipher = None

# Bundle every firmware payload directory (firmware/<board>/*).
firmware_root = Path("firmware")
firmware_datas = []
if firmware_root.is_dir():
    for board in sorted(firmware_root.iterdir()):
        if board.is_dir():
            for entry in board.iterdir():
                if entry.is_file():
                    # ('source', 'destination relative to bundle root')
                    firmware_datas.append((str(entry), f"firmware/{board.name}"))

a = Analysis(
    ["src/tcode_flasher/__main__.py"],
    pathex=["src"],
    binaries=[],
    datas=firmware_datas,
    hiddenimports=[
        "esptool",
        "esptool.bin_image",
        "esptool.cmds",
        "esptool.loader",
        "esptool.targets",
        "esptool.util",
        "serial.tools.list_ports",
    ],
    hookspath=[],
    runtime_hooks=[],
    excludes=[],
    cipher=block_cipher,
    noarchive=False,
)
pyz = PYZ(a.pure, a.zipped_data, cipher=block_cipher)

exe = EXE(
    pyz,
    a.scripts,
    a.binaries,
    a.zipfiles,
    a.datas,
    [],
    name="TCodeFlasher",
    debug=False,
    bootloader_ignore_signals=False,
    strip=False,
    upx=False,
    upx_exclude=[],
    runtime_tmpdir=None,
    console=False if sys.platform == "win32" else True,
    icon=None,
)

# On macOS also wrap the binary in a .app bundle so Finder treats it as a
# first-class application (icon, double-click launch, right-click "Open"
# Gatekeeper bypass). The raw exe inside dist/ stays available for CLI use.
if sys.platform == "darwin":
    app = BUNDLE(
        exe,
        name="TCodeFlasher.app",
        icon=None,
        bundle_identifier="org.tcode.flasher",
        info_plist={
            "CFBundleName": "TCode Flasher",
            "CFBundleDisplayName": "TCode Flasher",
            "CFBundleShortVersionString": "0.1.0",
            "CFBundleVersion": "0.1.0",
            "NSHighResolutionCapable": True,
            # The flasher needs raw USB-serial access; declare the entitlement
            # description so macOS shows a sensible prompt.
            "NSAppleEventsUsageDescription": "TCode Flasher uses serial ports to talk to the controller.",
        },
    )
