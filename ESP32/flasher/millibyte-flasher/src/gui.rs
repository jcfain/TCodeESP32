//! egui-based GUI wizard.
//!
//! Background thread does the actual flashing; the UI thread polls a
//! channel for log lines and progress updates.

use std::sync::mpsc::{channel, Receiver, Sender};
use std::thread;
use std::time::{Duration, Instant};

use anyhow::Result;
use eframe::egui;

use crate::detect::{self, Candidate};
use crate::flash::{self, FlashLogger};
use crate::manifest::{self, Bundle};
use crate::paths;
use crate::wifi;

pub fn run() -> Result<()> {
    let options = eframe::NativeOptions {
        viewport: egui::ViewportBuilder::default()
            .with_title("Millibyte Flasher")
            .with_inner_size([720.0, 520.0])
            .with_min_inner_size([560.0, 420.0]),
        ..Default::default()
    };
    eframe::run_native(
        "Millibyte Flasher",
        options,
        Box::new(|_cc| Ok(Box::<App>::default())),
    )
    .map_err(|e| anyhow::anyhow!("eframe error: {e}"))?;
    Ok(())
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
enum Status {
    Idle,
    Working,
    Done,
    Failed,
}

#[derive(Debug)]
enum WorkerMsg {
    Log(String),
    Progress { written: usize, total: usize },
    Finished(Result<Option<String>, String>),
}

struct App {
    bundles: Vec<Bundle>,
    candidates: Vec<Candidate>,
    selected_port: Option<String>,
    selected_board: Option<String>,
    ssid: String,
    password: String,
    do_wifi: bool,
    show_all_ports: bool,
    last_port_scan: Instant,
    baud: u32,
    log: Vec<String>,
    progress: f32,
    status: Status,
    rx: Option<Receiver<WorkerMsg>>,
}

impl Default for App {
    fn default() -> Self {
        let bundles = manifest::load_bundles(&paths::firmware_root());
        let candidates = detect::list_candidates();
        let selected_port = candidates.first().map(|c| c.port.clone());
        let selected_board = bundles.first().map(|b| b.manifest.board_id.clone());
        Self {
            bundles,
            candidates,
            selected_port,
            selected_board,
            ssid: String::new(),
            password: String::new(),
            do_wifi: true,
            show_all_ports: false,
            last_port_scan: Instant::now(),
            baud: 921_600,
            log: vec!["Ready. Plug in your controller via USB.".to_string()],
            progress: 0.0,
            status: Status::Idle,
            rx: None,
        }
    }
}

impl App {
    fn refresh_ports(&mut self) {
        self.candidates = if self.show_all_ports {
            detect::list_all_ports()
        } else {
            detect::list_candidates()
        };
        self.last_port_scan = Instant::now();
        if let Some(sel) = &self.selected_port {
            if !self.candidates.iter().any(|c| &c.port == sel) {
                self.selected_port = self.candidates.first().map(|c| c.port.clone());
            }
        } else {
            self.selected_port = self.candidates.first().map(|c| c.port.clone());
        }
    }

    fn drain_worker(&mut self, ctx: &egui::Context) {
        let mut keep = self.rx.is_some();
        if let Some(rx) = &self.rx {
            loop {
                match rx.try_recv() {
                    Ok(WorkerMsg::Log(line)) => {
                        self.log.push(line);
                        if self.log.len() > 1000 {
                            self.log.drain(..self.log.len() - 1000);
                        }
                    }
                    Ok(WorkerMsg::Progress { written, total }) => {
                        self.progress = if total > 0 {
                            written as f32 / total as f32
                        } else {
                            0.0
                        };
                    }
                    Ok(WorkerMsg::Finished(result)) => {
                        match result {
                            Ok(Some(ip)) => {
                                self.log.push(format!("Device IP: {ip}"));
                                self.status = Status::Done;
                            }
                            Ok(None) => {
                                self.log.push("Done.".to_string());
                                self.status = Status::Done;
                            }
                            Err(e) => {
                                self.log.push(format!("ERROR: {e}"));
                                self.status = Status::Failed;
                            }
                        }
                        keep = false;
                        break;
                    }
                    Err(std::sync::mpsc::TryRecvError::Empty) => break,
                    Err(std::sync::mpsc::TryRecvError::Disconnected) => {
                        keep = false;
                        break;
                    }
                }
            }
        }
        if !keep {
            self.rx = None;
        }
        ctx.request_repaint_after(Duration::from_millis(100));
    }

    fn start_flash(&mut self) {
        let port = match &self.selected_port {
            Some(p) => p.clone(),
            None => {
                self.log.push("No port selected.".to_string());
                return;
            }
        };
        let bundle = match self
            .selected_board
            .as_ref()
            .and_then(|id| self.bundles.iter().find(|b| &b.manifest.board_id == id))
            .cloned()
        {
            Some(b) => b,
            None => {
                self.log.push("No firmware bundle selected.".to_string());
                return;
            }
        };
        self.log.clear();
        self.progress = 0.0;
        self.status = Status::Working;

        let (tx, rx) = channel::<WorkerMsg>();
        self.rx = Some(rx);

        let baud = self.baud;
        let do_wifi = self.do_wifi;
        let ssid = self.ssid.clone();
        let password = self.password.clone();

        thread::spawn(move || {
            let mut logger = ChannelLogger { tx: tx.clone() };
            let flash_result = flash::flash_bundle(&port, &bundle, baud, &mut logger);
            if let Err(e) = flash_result {
                let _ = tx.send(WorkerMsg::Finished(Err(format!("{e:#}"))));
                return;
            }

            if do_wifi && !ssid.is_empty() {
                let _ = tx.send(WorkerMsg::Log("Configuring Wi-Fi...".to_string()));
                std::thread::sleep(Duration::from_secs(2));
                let tx_clone = tx.clone();
                let mut wifi_log = move |s: &str| {
                    let _ = tx_clone.send(WorkerMsg::Log(s.to_string()));
                };
                match wifi::push_credentials(&port, &ssid, &password, &mut wifi_log) {
                    Ok(ip) => {
                        let _ = tx.send(WorkerMsg::Finished(Ok(ip)));
                    }
                    Err(e) => {
                        let _ = tx.send(WorkerMsg::Finished(Err(format!("{e:#}"))));
                    }
                }
            } else {
                let _ = tx.send(WorkerMsg::Finished(Ok(None)));
            }
        });
    }
}

impl eframe::App for App {
    fn update(&mut self, ctx: &egui::Context, _frame: &mut eframe::Frame) {
        self.drain_worker(ctx);

        // Periodically rescan serial ports while idle so users plugging in a
        // device after launch don't have to hit Refresh manually.
        if self.status != Status::Working
            && self.last_port_scan.elapsed() >= Duration::from_millis(1500)
        {
            self.refresh_ports();
        }

        egui::TopBottomPanel::top("header").show(ctx, |ui| {
            ui.heading("Millibyte Flasher");
            ui.label("Plug in your controller and click Flash.");
            ui.add_space(4.0);
        });

        egui::SidePanel::left("config").min_width(280.0).show(ctx, |ui| {
            ui.heading("Target");

            ui.horizontal(|ui| {
                ui.label("Port:");
                let label = self
                    .selected_port
                    .clone()
                    .unwrap_or_else(|| "(scanning…)".to_string());
                egui::ComboBox::from_id_salt("port")
                    .selected_text(label)
                    .show_ui(ui, |ui| {
                        for c in &self.candidates {
                            ui.selectable_value(
                                &mut self.selected_port,
                                Some(c.port.clone()),
                                &c.label,
                            );
                        }
                    });
                if ui.button("⟳").on_hover_text("Refresh ports").clicked() {
                    self.refresh_ports();
                }
            });

            if self.candidates.is_empty() {
                ui.colored_label(
                    egui::Color32::YELLOW,
                    if self.show_all_ports {
                        "No serial ports detected. Plug in the device."
                    } else {
                        "No ESP32-like ports detected. Plug in the device, or enable\n\"Show all serial ports\" below if your USB bridge isn’t recognised."
                    },
                );
            }

            if ui
                .checkbox(&mut self.show_all_ports, "Show all serial ports")
                .changed()
            {
                self.refresh_ports();
            }

            ui.horizontal(|ui| {
                ui.label("Board:");
                let label = self
                    .selected_board
                    .as_ref()
                    .and_then(|id| self.bundles.iter().find(|b| &b.manifest.board_id == id))
                    .map(|b| b.display_name().to_string())
                    .unwrap_or_else(|| "(none)".to_string());
                egui::ComboBox::from_id_salt("board")
                    .selected_text(label)
                    .show_ui(ui, |ui| {
                        for b in &self.bundles {
                            ui.selectable_value(
                                &mut self.selected_board,
                                Some(b.manifest.board_id.clone()),
                                b.display_name(),
                            );
                        }
                    });
            });

            ui.add_space(8.0);
            ui.heading("Wi-Fi");
            ui.checkbox(&mut self.do_wifi, "Configure Wi-Fi after flashing");
            ui.add_enabled_ui(self.do_wifi, |ui| {
                ui.horizontal(|ui| {
                    ui.label("SSID:");
                    ui.text_edit_singleline(&mut self.ssid);
                });
                ui.horizontal(|ui| {
                    ui.label("Password:");
                    ui.add(egui::TextEdit::singleline(&mut self.password).password(true));
                });
            });

            ui.add_space(8.0);
            ui.collapsing("Advanced", |ui| {
                ui.horizontal(|ui| {
                    ui.label("Baud:");
                    ui.add(egui::DragValue::new(&mut self.baud).range(115_200..=921_600));
                });
                ui.label(format!(
                    "Firmware root: {}",
                    paths::firmware_root().display()
                ));
                if self.bundles.is_empty() {
                    ui.colored_label(egui::Color32::RED, "No firmware bundles found!");
                }
            });

            ui.add_space(12.0);
            ui.add_enabled_ui(
                self.status != Status::Working
                    && self.selected_port.is_some()
                    && self.selected_board.is_some(),
                |ui| {
                    if ui
                        .add_sized([200.0, 36.0], egui::Button::new("⚡ Flash"))
                        .clicked()
                    {
                        self.start_flash();
                    }
                },
            );

            match self.status {
                Status::Working => {
                    ui.add(egui::ProgressBar::new(self.progress).show_percentage());
                }
                Status::Done => {
                    ui.colored_label(egui::Color32::LIGHT_GREEN, "✔ Done");
                }
                Status::Failed => {
                    ui.colored_label(egui::Color32::LIGHT_RED, "✖ Failed");
                }
                Status::Idle => {}
            }
        });

        egui::CentralPanel::default().show(ctx, |ui| {
            ui.heading("Log");
            egui::ScrollArea::vertical()
                .auto_shrink([false; 2])
                .stick_to_bottom(true)
                .show(ui, |ui| {
                    for line in &self.log {
                        ui.monospace(line);
                    }
                });
        });
    }
}

struct ChannelLogger {
    tx: Sender<WorkerMsg>,
}

impl FlashLogger for ChannelLogger {
    fn log(&mut self, line: &str) {
        let _ = self.tx.send(WorkerMsg::Log(line.to_string()));
    }
    fn progress(&mut self, written: usize, total: usize) {
        let _ = self.tx.send(WorkerMsg::Progress { written, total });
    }
}
