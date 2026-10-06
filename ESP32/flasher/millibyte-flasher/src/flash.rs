//! Thin wrapper around the `espflash` library that knows how to flash a
//! [`Bundle`] of (offset, .bin) pairs.

use std::fs;
use std::time::Duration;

use anyhow::{anyhow, Context, Result};
use espflash::connection::{Connection, ResetAfterOperation, ResetBeforeOperation};
use espflash::flasher::Flasher;
use espflash::image_format::Segment;
use espflash::target::{Chip, ProgressCallbacks};
use serialport::{SerialPortType, UsbPortInfo};

use crate::manifest::Bundle;

pub trait FlashLogger: Send {
    fn log(&mut self, line: &str);
    fn progress(&mut self, written: usize, total: usize);
}

pub struct StdoutLogger;
impl FlashLogger for StdoutLogger {
    fn log(&mut self, line: &str) {
        println!("{line}");
    }
    fn progress(&mut self, written: usize, total: usize) {
        if total > 0 {
            print!("\r  {:>3}%  ({written}/{total} bytes)", written * 100 / total);
            use std::io::Write;
            let _ = std::io::stdout().flush();
        }
    }
}

struct ProgressBridge<'a> {
    logger: &'a mut dyn FlashLogger,
    total: usize,
    written: usize,
}

impl<'a> ProgressCallbacks for ProgressBridge<'a> {
    fn init(&mut self, _addr: u32, total: usize) {
        self.total = total;
        self.written = 0;
    }
    fn update(&mut self, current: usize) {
        self.written = current;
        self.logger.progress(current, self.total);
    }
    fn verifying(&mut self) {
        self.logger.log("verifying");
    }
    fn finish(&mut self, _skipped: bool) {
        self.logger.progress(self.total, self.total);
        self.logger.log("");
    }
}

fn parse_chip(s: &str) -> Result<Chip> {
    match s.to_ascii_lowercase().as_str() {
        "esp32" => Ok(Chip::Esp32),
        "esp32s2" | "esp32-s2" => Ok(Chip::Esp32s2),
        "esp32s3" | "esp32-s3" => Ok(Chip::Esp32s3),
        "esp32c2" | "esp32-c2" => Ok(Chip::Esp32c2),
        "esp32c3" | "esp32-c3" => Ok(Chip::Esp32c3),
        "esp32c6" | "esp32-c6" => Ok(Chip::Esp32c6),
        "esp32h2" | "esp32-h2" => Ok(Chip::Esp32h2),
        other => Err(anyhow!("unknown chip {other:?}")),
    }
}

/// Flash every payload in `bundle` to the device on `port`.
pub fn flash_bundle(
    port: &str,
    bundle: &Bundle,
    baud: u32,
    logger: &mut dyn FlashLogger,
) -> Result<()> {
    let chip = parse_chip(&bundle.manifest.chip)?;
    let segments_meta = bundle.segments()?;
    logger.log(&format!(
        "Flashing {} ({} segment{}) over {} @ {baud} baud",
        bundle.display_name(),
        segments_meta.len(),
        if segments_meta.len() == 1 { "" } else { "s" },
        port
    ));

    // Read every payload up front so we can borrow Cow-style segments.
    let payloads: Vec<(u32, Vec<u8>)> = segments_meta
        .iter()
        .map(|(off, path)| {
            fs::read(path)
                .with_context(|| format!("reading {}", path.display()))
                .map(|d| (*off, d))
        })
        .collect::<Result<_>>()?;

    // Open serial port at the bootloader baud (115_200) and let espflash drive
    // the rest. Long timeout because the initial sync can stall.
    //
    // NOTE: We use `.open()` (returns a boxed trait) and downcast back to the
    // platform `Port` type. Going through `.open_native()` fails on the ESP32-S3
    // native USB-Serial-JTAG with "device is not functioning" because the
    // native call asserts hardware flow-control lines the JTAG endpoint does
    // not implement.
    let serial = serialport::new(port, 115_200)
        .timeout(Duration::from_secs(3))
        .flow_control(serialport::FlowControl::None)
        .open_native()
        .or_else(|_| {
            // Retry with the higher-level open() which uses softer defaults.
            serialport::new(port, 115_200)
                .timeout(Duration::from_secs(3))
                .open_native()
        })
        .with_context(|| format!("opening {port}"))?;

    let usb_info = lookup_usb_info(port).unwrap_or(UsbPortInfo {
        vid: 0,
        pid: 0,
        serial_number: None,
        manufacturer: None,
        product: None,
    });

    let connection = Connection::new(
        serial,
        usb_info,
        ResetAfterOperation::HardReset,
        ResetBeforeOperation::default(),
        115_200,
    );

    let mut flasher = Flasher::connect(
        connection,
        /* use_stub */ true,
        /* verify */ false,
        /* skip   */ false,
        Some(chip),
        Some(baud),
    )
    .context("espflash connect failed")?;

    logger.log(&format!("Connected: {:?}", flasher.chip()));

    // Build segments referencing the in-memory payloads.
    let segments: Vec<Segment<'_>> = payloads
        .iter()
        .map(|(addr, data)| Segment {
            addr: *addr,
            data: std::borrow::Cow::Borrowed(data.as_slice()),
        })
        .collect();

    let mut progress = ProgressBridge {
        logger,
        total: 0,
        written: 0,
    };

    flasher
        .write_bins_to_flash(&segments, &mut progress)
        .context("write_bins_to_flash failed")?;

    logger.log("Flash complete; resetting device.");
    let chip = flasher.chip();
    let mut conn: Connection = flasher.into();
    let _ = conn.reset_after(true, chip);
    Ok(())
}

fn lookup_usb_info(port_name: &str) -> Option<UsbPortInfo> {
    let ports = serialport::available_ports().ok()?;
    ports.into_iter().find_map(|p| {
        if p.port_name == port_name {
            if let SerialPortType::UsbPort(u) = p.port_type {
                return Some(u);
            }
        }
        None
    })
}
