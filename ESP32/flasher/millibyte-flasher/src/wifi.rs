//! Push Wi-Fi credentials to the device over the serial console.
//!
//! Mirrors `configure_wifi_and_reset.ps1` in the repo root: open the port,
//! send the controller's text-protocol commands, wait for an IP banner.

use std::io::{Read, Write};
use std::time::{Duration, Instant};

use anyhow::{anyhow, Context, Result};

pub fn push_credentials(
    port: &str,
    ssid: &str,
    password: &str,
    log: &mut dyn FnMut(&str),
) -> Result<Option<String>> {
    log(&format!("Opening {port} for Wi-Fi configuration..."));
    let mut serial = serialport::new(port, 115_200)
        .timeout(Duration::from_millis(300))
        .open_native()
        .with_context(|| format!("opening {port}"))?;

    // Drain the boot banner.
    let _ = read_for(&mut serial, Duration::from_secs(2), log);

    // Switch the device into Wi-Fi mode.
    send(&mut serial, "#wifi-mode:1\n", log)?;
    expect(&mut serial, &["wifi mode set", "Wifi mode set", "OK"], 3, log)?;

    send(&mut serial, &format!("#wifi-ssid:{ssid}\n"), log)?;
    expect(&mut serial, &["ssid", "OK"], 3, log)?;

    send(&mut serial, &format!("#wifi-pass:{password}\n"), log)?;
    expect(&mut serial, &["pass", "OK"], 3, log)?;

    // The firmware uses `$save` (system command, $-prefix) for persisting
    // settings, but `#restart` (value command, #-prefix) for reboot. Don't
    // unify these — the firmware will reject the wrong prefix silently.
    send(&mut serial, "$save\n", log)?;
    expect(&mut serial, &["Saved", "saved", "OK"], 3, log)?;

    send(&mut serial, "#restart\n", log)?;

    // After reboot, watch for the IP banner. Standard format on this firmware
    // is roughly: "WiFi connected. IP address: 192.168.x.y".
    let buf = read_for(&mut serial, Duration::from_secs(30), log).unwrap_or_default();
    Ok(extract_ip(&buf))
}

fn send(port: &mut impl Write, line: &str, log: &mut dyn FnMut(&str)) -> Result<()> {
    log(&format!("> {}", line.trim_end()));
    port.write_all(line.as_bytes())?;
    port.flush()?;
    Ok(())
}

fn read_for(
    port: &mut impl Read,
    duration: Duration,
    log: &mut dyn FnMut(&str),
) -> Result<String> {
    let deadline = Instant::now() + duration;
    let mut buf = [0u8; 256];
    let mut acc = String::new();
    while Instant::now() < deadline {
        match port.read(&mut buf) {
            Ok(0) => continue,
            Ok(n) => {
                let chunk = String::from_utf8_lossy(&buf[..n]);
                for line in chunk.lines() {
                    if !line.is_empty() {
                        log(&format!("< {line}"));
                    }
                }
                acc.push_str(&chunk);
            }
            Err(e) if e.kind() == std::io::ErrorKind::TimedOut => continue,
            Err(e) => return Err(anyhow!(e)),
        }
    }
    Ok(acc)
}

fn expect(
    port: &mut impl Read,
    needles: &[&str],
    seconds: u64,
    log: &mut dyn FnMut(&str),
) -> Result<()> {
    let deadline = Instant::now() + Duration::from_secs(seconds);
    let mut buf = [0u8; 256];
    let mut acc = String::new();
    while Instant::now() < deadline {
        match port.read(&mut buf) {
            Ok(0) => continue,
            Ok(n) => {
                let chunk = String::from_utf8_lossy(&buf[..n]);
                for line in chunk.lines() {
                    if !line.is_empty() {
                        log(&format!("< {line}"));
                    }
                }
                acc.push_str(&chunk);
                if needles.iter().any(|needle| acc.contains(needle)) {
                    return Ok(());
                }
            }
            Err(e) if e.kind() == std::io::ErrorKind::TimedOut => continue,
            Err(e) => return Err(anyhow!(e)),
        }
    }
    // Non-fatal: device firmware versions vary in reply text.
    log("(no explicit ack within timeout, continuing)");
    Ok(())
}

fn extract_ip(text: &str) -> Option<String> {
    for line in text.lines() {
        let lower = line.to_ascii_lowercase();
        if lower.contains("ip") {
            // Greedy match for first dotted-quad in the line.
            let mut current = String::new();
            for ch in line.chars() {
                if ch.is_ascii_digit() || ch == '.' {
                    current.push(ch);
                } else {
                    if is_ipv4(&current) {
                        return Some(current);
                    }
                    current.clear();
                }
            }
            if is_ipv4(&current) {
                return Some(current);
            }
        }
    }
    None
}

fn is_ipv4(s: &str) -> bool {
    let parts: Vec<&str> = s.split('.').collect();
    if parts.len() != 4 {
        return false;
    }
    parts.iter().all(|p| !p.is_empty() && p.parse::<u8>().is_ok())
}
