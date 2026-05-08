//! Serial port enumeration & ESP32 candidate detection.
//!
//! We try to be lenient: any port whose USB VID/PID matches a known
//! ESP32 USB-serial bridge (CP210x, CH34x, FTDI) **or** whose name contains
//! "USB"/"UART" is offered as a candidate.

use serialport::{SerialPortInfo, SerialPortType, UsbPortInfo};

use crate::manifest::Bundle;

#[derive(Debug, Clone)]
pub struct Candidate {
    pub port: String,
    pub label: String,
    pub usb: Option<UsbPortInfo>,
}

/// Known USB VID/PID combos for typical ESP32 dev boards / USB-serial bridges.
const KNOWN_VID_PID: &[(u16, u16)] = &[
    // Espressif native USB-serial-jtag
    (0x303a, 0x1001),
    (0x303a, 0x0002),
    // Silicon Labs CP210x family
    (0x10c4, 0xea60),
    (0x10c4, 0xea70),
    // WCH CH340/CH341
    (0x1a86, 0x7523),
    (0x1a86, 0x55d4),
    // FTDI
    (0x0403, 0x6001),
    (0x0403, 0x6010),
    (0x0403, 0x6014),
    (0x0403, 0x6015),
];

fn looks_like_esp(info: &SerialPortInfo) -> bool {
    match &info.port_type {
        SerialPortType::UsbPort(usb) => {
            if KNOWN_VID_PID
                .iter()
                .any(|(v, p)| *v == usb.vid && *p == usb.pid)
            {
                return true;
            }
            // Fall back to fuzzy product/manufacturer match.
            let haystack = format!(
                "{} {}",
                usb.manufacturer.as_deref().unwrap_or(""),
                usb.product.as_deref().unwrap_or("")
            )
            .to_ascii_lowercase();
            haystack.contains("esp")
                || haystack.contains("cp210")
                || haystack.contains("ch340")
                || haystack.contains("ch910")
                || haystack.contains("ftdi")
                || haystack.contains("silicon labs")
        }
        _ => false,
    }
}

pub fn list_candidates() -> Vec<Candidate> {
    list_candidates_inner(false)
}

/// List every serial port, even ones that don't match an ESP heuristic.
/// Useful as a GUI fallback when auto-detection misses an unusual USB bridge.
pub fn list_all_ports() -> Vec<Candidate> {
    list_candidates_inner(true)
}

fn list_candidates_inner(include_all: bool) -> Vec<Candidate> {
    let ports = match serialport::available_ports() {
        Ok(p) => p,
        Err(e) => {
            log::warn!("serialport enumeration failed: {e}");
            return Vec::new();
        }
    };

    ports
        .into_iter()
        .filter(|p| include_all || looks_like_esp(p))
        .map(|p| {
            let usb = match &p.port_type {
                SerialPortType::UsbPort(info) => Some(info.clone()),
                _ => None,
            };
            let label = match &usb {
                Some(u) => format!(
                    "{}  ({}{})",
                    p.port_name,
                    u.product.as_deref().unwrap_or("USB serial"),
                    u.serial_number
                        .as_ref()
                        .map(|s| format!(", SN {s}"))
                        .unwrap_or_default(),
                ),
                None => p.port_name.clone(),
            };
            Candidate {
                port: p.port_name,
                label,
                usb,
            }
        })
        .collect()
}

/// If a single bundle covers the candidate's USB VID:PID, return it.
pub fn match_bundle<'a>(candidate: &Candidate, bundles: &'a [Bundle]) -> Option<&'a Bundle> {
    let usb = candidate.usb.as_ref()?;
    let key = format!("{:04x}:{:04x}", usb.vid, usb.pid);
    bundles.iter().find(|b| {
        b.manifest
            .usb_vid_pid
            .iter()
            .any(|s| s.eq_ignore_ascii_case(&key))
    })
}
