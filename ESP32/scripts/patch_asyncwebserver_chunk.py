import os
from pathlib import Path

Import("env")

PIOENV = env["PIOENV"]
PROJECT_DIR = Path(env["PROJECT_DIR"])

webresponses = PROJECT_DIR / ".pio" / "libdeps" / PIOENV / "ESPAsyncWebServer" / "src" / "WebResponses.cpp"

if not webresponses.exists():
    print(f"[patch_asyncwebserver_chunk] Skipped: {webresponses} not found yet")
else:
    source = webresponses.read_text(encoding="utf-8")
    marker = "static constexpr size_t ASYNC_RESPONSE_MAX_ALLOC = 1460;"

    if marker in source:
        print("[patch_asyncwebserver_chunk] Already patched")
    else:
        needle = """    uint8_t *buf = (uint8_t *)malloc(outLen + headLen);"""
        replacement = """    // Cap per-ACK allocation to a single-MSS sized block to survive\n    // heavy heap fragmentation on ESP32 under WiFi/BLE coexistence.\n    static constexpr size_t ASYNC_RESPONSE_MAX_ALLOC = 1460;\n    if (outLen > ASYNC_RESPONSE_MAX_ALLOC) {\n      outLen = ASYNC_RESPONSE_MAX_ALLOC;\n    }\n\n    uint8_t *buf = (uint8_t *)malloc(outLen + headLen);"""

        if needle not in source:
            print("[patch_asyncwebserver_chunk] Failed: expected _ack() malloc pattern not found")
        else:
            source = source.replace(needle, replacement, 1)
            webresponses.write_text(source, encoding="utf-8")
            print(f"[patch_asyncwebserver_chunk] Patched {webresponses}")
