//! Clap-driven CLI. Matches the GUI feature set so the same binary can be
//! scripted in CI or end-user flashing kiosks.

use anyhow::{anyhow, Context, Result};
use clap::{Parser, Subcommand};

use crate::detect;
use crate::flash::{self, FlashLogger, StdoutLogger};
use crate::manifest::{self, Bundle};
use crate::paths;
use crate::wifi;

#[derive(Parser, Debug)]
#[command(
    name = "millibyte-flasher",
    version,
    about = "One-click flasher for Millibyte TCode ESP32 controllers.",
    long_about = None
)]
pub struct Cli {
    #[command(subcommand)]
    pub command: Option<Command>,

    /// Serial port (auto-detect when omitted).
    #[arg(short, long, global = true)]
    pub port: Option<String>,

    /// Board id (auto-detect when omitted, e.g. "sr6_pcb").
    #[arg(short, long, global = true)]
    pub board: Option<String>,

    /// Wi-Fi SSID to push after flashing.
    #[arg(long, global = true)]
    pub ssid: Option<String>,

    /// Wi-Fi password (used with --ssid).
    #[arg(long, global = true)]
    pub password: Option<String>,

    /// Skip the filesystem image (firmware-only flash).
    #[arg(long, global = true)]
    pub no_fs: bool,

    /// Skip the post-flash Wi-Fi configuration step.
    #[arg(long, global = true)]
    pub no_wifi: bool,

    /// Flash baud rate.
    #[arg(long, default_value_t = 921_600, global = true)]
    pub baud: u32,

    /// Increase log verbosity (-v info, -vv debug, -vvv trace).
    #[arg(short, long, action = clap::ArgAction::Count, global = true)]
    pub verbose: u8,
}

#[derive(Subcommand, Debug, Clone)]
pub enum Command {
    /// Flash firmware (and filesystem) to a connected device. Default action.
    Flash,
    /// List serial ports that look like an ESP32.
    ListPorts,
    /// List every firmware bundle visible to the flasher.
    ListBoards,
    /// Push Wi-Fi credentials over serial without re-flashing.
    Configure,
}

pub fn run() -> Result<()> {
    let cli = Cli::parse();
    apply_verbosity(cli.verbose);

    match cli.command.clone().unwrap_or(Command::Flash) {
        Command::Flash => cmd_flash(&cli),
        Command::ListPorts => cmd_list_ports(),
        Command::ListBoards => cmd_list_boards(),
        Command::Configure => cmd_configure(&cli),
    }
}

fn apply_verbosity(level: u8) {
    if level == 0 {
        return;
    }
    let filter = match level {
        1 => "info",
        2 => "debug",
        _ => "trace",
    };
    std::env::set_var("RUST_LOG", filter);
    let _ = env_logger::Builder::from_env(env_logger::Env::default().default_filter_or(filter))
        .try_init();
}

fn cmd_list_ports() -> Result<()> {
    for c in detect::list_candidates() {
        println!("{}", c.label);
    }
    Ok(())
}

fn cmd_list_boards() -> Result<()> {
    for b in manifest::load_bundles(&paths::firmware_root()) {
        println!(
            "{}\t{}\t{}",
            b.manifest.board_id,
            b.display_name(),
            b.manifest.firmware_version.as_deref().unwrap_or("?")
        );
    }
    Ok(())
}

fn pick_bundle<'a>(cli: &Cli, bundles: &'a [Bundle]) -> Result<&'a Bundle> {
    if bundles.is_empty() {
        return Err(anyhow!(
            "No firmware bundles found under {}",
            paths::firmware_root().display()
        ));
    }
    if let Some(id) = &cli.board {
        return manifest::find_bundle(bundles, id)
            .ok_or_else(|| anyhow!("unknown board {id:?}; try `list-boards`"));
    }
    if bundles.len() == 1 {
        return Ok(&bundles[0]);
    }
    // Try VID:PID match against connected port.
    if let Some(port) = &cli.port {
        if let Some(c) = detect::list_candidates().into_iter().find(|c| &c.port == port) {
            if let Some(b) = detect::match_bundle(&c, bundles) {
                return Ok(b);
            }
        }
    } else {
        let candidates = detect::list_candidates();
        if candidates.len() == 1 {
            if let Some(b) = detect::match_bundle(&candidates[0], bundles) {
                return Ok(b);
            }
        }
    }
    Err(anyhow!(
        "Multiple boards available; pass --board <id>. Known: {}",
        bundles
            .iter()
            .map(|b| b.manifest.board_id.as_str())
            .collect::<Vec<_>>()
            .join(", ")
    ))
}

fn pick_port(cli: &Cli) -> Result<String> {
    if let Some(p) = &cli.port {
        return Ok(p.clone());
    }
    let mut candidates = detect::list_candidates();
    match candidates.len() {
        0 => Err(anyhow!("No serial ports found. Plug in the device or pass --port.")),
        1 => Ok(candidates.remove(0).port),
        _ => Err(anyhow!(
            "Multiple ports detected; pass --port. Found: {}",
            candidates
                .iter()
                .map(|c| c.port.as_str())
                .collect::<Vec<_>>()
                .join(", ")
        )),
    }
}

fn cmd_flash(cli: &Cli) -> Result<()> {
    let bundles = manifest::load_bundles(&paths::firmware_root());
    let bundle = pick_bundle(cli, &bundles)?;
    let port = pick_port(cli)?;
    let mut logger: StdoutLogger = StdoutLogger;

    flash::flash_bundle(&port, bundle, cli.baud, &mut logger as &mut dyn FlashLogger)
        .with_context(|| format!("flashing {} on {}", bundle.display_name(), port))?;

    if !cli.no_wifi {
        if let Some(ssid) = &cli.ssid {
            let pwd = cli.password.clone().unwrap_or_default();
            // Give the device a moment to come up after the flasher reset.
            std::thread::sleep(std::time::Duration::from_secs(2));
            let mut log = |s: &str| println!("{s}");
            match wifi::push_credentials(&port, ssid, &pwd, &mut log) {
                Ok(Some(ip)) => println!("Device IP: {ip}"),
                Ok(None) => println!("Wi-Fi credentials sent (no IP captured before timeout)."),
                Err(e) => eprintln!("Wi-Fi configuration failed: {e:#}"),
            }
        }
    }
    println!("Done.");
    Ok(())
}

fn cmd_configure(cli: &Cli) -> Result<()> {
    let port = pick_port(cli)?;
    let ssid = cli
        .ssid
        .as_deref()
        .ok_or_else(|| anyhow!("--ssid is required for `configure`"))?;
    let password = cli.password.clone().unwrap_or_default();
    let mut log = |s: &str| println!("{s}");
    match wifi::push_credentials(&port, ssid, &password, &mut log)? {
        Some(ip) => println!("Device IP: {ip}"),
        None => println!("Wi-Fi credentials sent (no IP captured before timeout)."),
    }
    Ok(())
}
