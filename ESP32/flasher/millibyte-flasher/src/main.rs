//! Millibyte flasher entry point.
//!
//! Single binary that branches on argument count:
//! * `argc == 1` (no arguments) → launches the egui-based GUI wizard.
//! * `argc >= 2` (any arguments) → parses with clap and runs the CLI.
//!
//! On Windows we keep the **console subsystem** so CLI output works in
//! cmd/PowerShell. When invoked with no args (e.g. double-click from
//! Explorer) we call `FreeConsole` to hide the briefly-visible console
//! window before the egui window opens.

mod cli;
mod detect;
mod flash;
mod gui;
mod manifest;
mod paths;
mod wifi;

use std::process::ExitCode;

fn main() -> ExitCode {
    let _ = env_logger::Builder::from_env(env_logger::Env::default().default_filter_or("info"))
        .try_init();

    let args: Vec<String> = std::env::args().collect();
    if args.len() <= 1 {
        #[cfg(windows)]
        unsafe {
            extern "system" {
                fn FreeConsole() -> i32;
            }
            FreeConsole();
        }
        match gui::run() {
            Ok(()) => ExitCode::SUCCESS,
            Err(e) => {
                eprintln!("GUI error: {e:#}");
                ExitCode::FAILURE
            }
        }
    } else {
        match cli::run() {
            Ok(()) => ExitCode::SUCCESS,
            Err(e) => {
                eprintln!("Error: {e:#}");
                ExitCode::FAILURE
            }
        }
    }
}
