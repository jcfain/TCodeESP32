//! Firmware bundle manifests.
//!
//! Each bundle is a directory under [`crate::paths::firmware_root`] containing
//! a `manifest.json` plus the binary payloads it references.

use std::fs;
use std::path::{Path, PathBuf};

use anyhow::{anyhow, Context, Result};
use serde::Deserialize;

#[derive(Debug, Clone, Deserialize)]
#[allow(dead_code)] // flash_mode/flash_freq/flash_size + fs_payload reserved for future use
pub struct Manifest {
    pub board_id: String,
    #[serde(default)]
    pub display_name: Option<String>,
    pub chip: String,
    #[serde(default)]
    pub firmware_version: Option<String>,
    #[serde(default = "default_flash_mode")]
    pub flash_mode: String,
    #[serde(default = "default_flash_freq")]
    pub flash_freq: String,
    #[serde(default = "default_flash_size")]
    pub flash_size: String,
    #[serde(default)]
    pub usb_vid_pid: Vec<String>,
    pub files: Vec<ManifestFile>,
}

#[derive(Debug, Clone, Deserialize)]
pub struct ManifestFile {
    pub offset: String,
    pub name: String,
}

fn default_flash_mode() -> String {
    "dio".to_string()
}
fn default_flash_freq() -> String {
    "40MHz".to_string()
}
fn default_flash_size() -> String {
    "4MB".to_string()
}

/// One firmware bundle resolved against disk.
#[derive(Debug, Clone)]
pub struct Bundle {
    pub manifest: Manifest,
    pub root: PathBuf,
}

impl Bundle {
    pub fn display_name(&self) -> &str {
        self.manifest
            .display_name
            .as_deref()
            .unwrap_or(&self.manifest.board_id)
    }

    /// Resolve every payload to (offset, absolute path), in manifest order.
    pub fn segments(&self) -> Result<Vec<(u32, PathBuf)>> {
        let mut out = Vec::with_capacity(self.manifest.files.len());
        for f in &self.manifest.files {
            let offset = parse_offset(&f.offset)
                .with_context(|| format!("bad offset {:?} in {}", f.offset, self.display_name()))?;
            let path = self.root.join(&f.name);
            if !path.is_file() {
                return Err(anyhow!(
                    "{}: missing payload {}",
                    self.display_name(),
                    path.display()
                ));
            }
            out.push((offset, path));
        }
        Ok(out)
    }

    /// Path of the filesystem image (`littlefs.bin` / `spiffs.bin`), if any.
    #[allow(dead_code)] // Reserved for future settings-backup/restore flow.
    pub fn fs_payload(&self) -> Option<PathBuf> {
        self.manifest.files.iter().find_map(|f| {
            let name = f.name.to_ascii_lowercase();
            if name == "littlefs.bin" || name == "spiffs.bin" {
                Some(self.root.join(&f.name))
            } else {
                None
            }
        })
    }
}

fn parse_offset(s: &str) -> Result<u32> {
    let trimmed = s.trim();
    let value = if let Some(rest) = trimmed
        .strip_prefix("0x")
        .or_else(|| trimmed.strip_prefix("0X"))
    {
        u32::from_str_radix(rest, 16)?
    } else {
        trimmed.parse::<u32>()?
    };
    Ok(value)
}

/// Load every `manifest.json` found one level below `root`.
pub fn load_bundles(root: &Path) -> Vec<Bundle> {
    let mut out = Vec::new();
    let entries = match fs::read_dir(root) {
        Ok(it) => it,
        Err(e) => {
            log::warn!("firmware root {} unreadable: {e}", root.display());
            return out;
        }
    };
    for entry in entries.flatten() {
        let dir = entry.path();
        if !dir.is_dir() {
            continue;
        }
        let manifest_path = dir.join("manifest.json");
        if !manifest_path.is_file() {
            continue;
        }
        match fs::read_to_string(&manifest_path)
            .map_err(anyhow::Error::from)
            .and_then(|s| serde_json::from_str::<Manifest>(&s).map_err(anyhow::Error::from))
        {
            Ok(manifest) => out.push(Bundle {
                manifest,
                root: dir,
            }),
            Err(e) => log::warn!(
                "skipping bad manifest {}: {e:#}",
                manifest_path.display()
            ),
        }
    }
    out.sort_by(|a, b| a.manifest.board_id.cmp(&b.manifest.board_id));
    out
}

pub fn find_bundle<'a>(bundles: &'a [Bundle], board_id: &str) -> Option<&'a Bundle> {
    bundles.iter().find(|b| b.manifest.board_id == board_id)
}
