//! Locate firmware bundles on disk relative to the running executable.

use std::path::PathBuf;

/// Directory containing `firmware/<board>/manifest.json` payloads.
///
/// In a packaged release this lives next to the executable. During `cargo run`
/// from a checkout we also probe `flasher/millibyte-flasher/firmware/`.
pub fn firmware_root() -> PathBuf {
    if let Ok(exe) = std::env::current_exe() {
        if let Some(parent) = exe.parent() {
            let candidate = parent.join("firmware");
            if candidate.is_dir() {
                return candidate;
            }
        }
    }
    // Dev fallback: <crate>/firmware/
    PathBuf::from(env!("CARGO_MANIFEST_DIR")).join("firmware")
}
