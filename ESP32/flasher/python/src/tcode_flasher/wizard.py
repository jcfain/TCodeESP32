"""Tkinter wizard UI.

Pages, in order:

1. **Detect** — list ports, identify the target. User picks one and clicks
   "Next".
2. **Plan** — show the running firmware version, the bundled firmware
   version, optionally check GitHub Releases. User clicks "Flash".
3. **Flash** — stream esptool output. Disables Back/Next while running.
4. **Wi-Fi** — collect SSID/password, push them, wait for the device's IP.
   Skippable.
5. **Done** — show summary + "Finish".

The wizard uses only stdlib widgets so the PyInstaller bundle stays small.
"""
from __future__ import annotations

import queue
import sys
import tempfile
import threading
import tkinter as tk
from pathlib import Path
from tkinter import messagebox, scrolledtext, ttk
from typing import List, Optional

from . import backup as backup_mod
from . import detect, flash, wifi
from .manifest import FirmwareBundle, load_bundles


_PAD = 10


class FlasherApp(tk.Tk):
    def __init__(self) -> None:
        super().__init__()
        self.title("TCode Flasher")
        self.geometry("720x520")
        self.minsize(640, 460)

        self.bundles: List[FirmwareBundle] = load_bundles()
        self.candidate: Optional[detect.CandidatePort] = None
        self.backup: Optional[backup_mod.Backup] = None
        self.log_queue: "queue.Queue[str]" = queue.Queue()
        self._worker: Optional[threading.Thread] = None

        self._build_layout()
        self._show_page("detect")
        self.after(100, self._drain_log_queue)

    # ------------------------------------------------------------------ layout
    def _build_layout(self) -> None:
        self.container = ttk.Frame(self, padding=_PAD)
        self.container.pack(fill="both", expand=True)

        # Page area
        self.page_holder = ttk.Frame(self.container)
        self.page_holder.pack(fill="both", expand=True)

        # Nav buttons
        nav = ttk.Frame(self.container)
        nav.pack(fill="x", pady=(_PAD, 0))
        self.back_btn = ttk.Button(nav, text="< Back", command=self._on_back, state="disabled")
        self.back_btn.pack(side="left")
        self.next_btn = ttk.Button(nav, text="Next >", command=self._on_next)
        self.next_btn.pack(side="right")
        self.cancel_btn = ttk.Button(nav, text="Quit", command=self.destroy)
        self.cancel_btn.pack(side="right", padx=(0, _PAD))

        self.pages = {
            "detect": _DetectPage(self.page_holder, self),
            "plan":   _PlanPage(self.page_holder, self),
            "flash":  _FlashPage(self.page_holder, self),
            "wifi":   _WifiPage(self.page_holder, self),
            "done":   _DonePage(self.page_holder, self),
        }
        self._page_order = ["detect", "plan", "flash", "wifi", "done"]
        self._current = "detect"

    def _show_page(self, name: str) -> None:
        for k, p in self.pages.items():
            p.pack_forget()
        self.pages[name].pack(fill="both", expand=True)
        self._current = name
        idx = self._page_order.index(name)
        self.back_btn.configure(state=("normal" if idx > 0 and name not in ("flash", "done") else "disabled"))
        self.next_btn.configure(text=("Finish" if name == "done" else "Next >"))
        self.pages[name].on_show()

    # ------------------------------------------------------------------ nav
    def _on_back(self) -> None:
        idx = self._page_order.index(self._current)
        if idx > 0:
            self._show_page(self._page_order[idx - 1])

    def _on_next(self) -> None:
        page = self.pages[self._current]
        if not page.validate():
            return
        if self._current == "done":
            self.destroy()
            return
        idx = self._page_order.index(self._current)
        nxt = self._page_order[idx + 1]
        # Detect -> Plan: kick off the flash step automatically when entering
        # the Flash page via Next.
        self._show_page(nxt)
        if nxt == "flash":
            self.pages["flash"].start()

    # ------------------------------------------------------------------ logging
    def log(self, line: str) -> None:
        self.log_queue.put(line)

    def _drain_log_queue(self) -> None:
        try:
            while True:
                line = self.log_queue.get_nowait()
                page = self.pages.get(self._current)
                if hasattr(page, "append_log"):
                    page.append_log(line + "\n")
        except queue.Empty:
            pass
        self.after(80, self._drain_log_queue)

    # ------------------------------------------------------------------ workers
    def run_in_worker(self, target, on_done) -> None:
        """Run *target()* on a background thread, then call ``on_done(error)``
        on the Tk thread (``error`` is None on success).
        """
        if self._worker and self._worker.is_alive():
            return

        def run():
            err: Optional[BaseException] = None
            try:
                target()
            except BaseException as exc:  # noqa: BLE001
                err = exc
            finally:
                self.after(0, lambda: on_done(err))

        self._worker = threading.Thread(target=run, daemon=True)
        self._worker.start()


# =====================================================================
# Pages
# =====================================================================
class _Page(ttk.Frame):
    def __init__(self, parent, app: FlasherApp) -> None:
        super().__init__(parent, padding=_PAD)
        self.app = app

    def on_show(self) -> None:  # noqa: D401
        """Called when the page becomes visible."""

    def validate(self) -> bool:
        return True


class _DetectPage(_Page):
    def __init__(self, parent, app):
        super().__init__(parent, app)
        ttk.Label(self, text="1. Connect your controller", font=("", 14, "bold")).pack(anchor="w")
        ttk.Label(self, text="Plug the SR6PCB or SSR1PCB into USB and click Refresh.").pack(anchor="w", pady=(0, _PAD))
        bar = ttk.Frame(self); bar.pack(fill="x")
        ttk.Button(bar, text="Refresh", command=self.refresh).pack(side="left")
        self.identify_var = tk.BooleanVar(value=True)
        ttk.Checkbutton(bar, text="Probe boot banner (slower, more accurate)",
                        variable=self.identify_var).pack(side="left", padx=_PAD)
        self.tree = ttk.Treeview(self, columns=("desc", "vidpid", "fw"), show="headings", height=8)
        for col, label, w in (("desc", "Port / Description", 300), ("vidpid", "USB VID:PID", 110), ("fw", "Firmware", 200)):
            self.tree.heading(col, text=label)
            self.tree.column(col, width=w, anchor="w")
        self.tree.pack(fill="both", expand=True, pady=_PAD)
        self.tree.bind("<<TreeviewSelect>>", self._on_select)

    def on_show(self):
        self.refresh()

    def refresh(self):
        for i in self.tree.get_children():
            self.tree.delete(i)
        cands = detect.list_candidate_ports(self.app.bundles)
        if self.identify_var.get():
            for c in cands:
                detect.identify(c, self.app.bundles)
        self._cands = cands
        for c in cands:
            fw = (c.info.firmware_version if c.info and c.info.firmware_version else "?")
            self.tree.insert("", "end", values=(c.label, c.vid_pid or "?", fw))
        if cands:
            self.tree.selection_set(self.tree.get_children()[0])

    def _on_select(self, _evt):
        sel = self.tree.selection()
        if not sel:
            self.app.candidate = None
            return
        idx = self.tree.index(sel[0])
        self.app.candidate = self._cands[idx] if idx < len(self._cands) else None

    def validate(self) -> bool:
        if not self.app.candidate:
            messagebox.showwarning("Pick a port", "Select a controller from the list, then click Next.")
            return False
        if not self.app.candidate.matched_bundle:
            # Last-chance: if there's exactly one bundle in the manifest we
            # assume it's the right one.
            if len(self.app.bundles) == 1:
                self.app.candidate.matched_bundle = self.app.bundles[0]
            else:
                messagebox.showwarning("Unknown board",
                    "Couldn't identify the board. Make sure firmware bundles are installed "
                    "in the firmware/ directory next to the executable.")
                return False
        return True


class _PlanPage(_Page):
    def __init__(self, parent, app):
        super().__init__(parent, app)
        ttk.Label(self, text="2. Review", font=("", 14, "bold")).pack(anchor="w")
        self.text = tk.Text(self, height=12, wrap="word", state="disabled")
        self.text.pack(fill="both", expand=True, pady=_PAD)
        self.skip_backup = tk.BooleanVar(value=False)
        ttk.Checkbutton(self, text="Skip settings backup/restore (clean install)",
                        variable=self.skip_backup).pack(anchor="w")

    def on_show(self):
        c = self.app.candidate
        b = c.matched_bundle if c else None
        lines = []
        if c:
            lines.append(f"Port:           {c.port}")
            lines.append(f"USB VID:PID:    {c.vid_pid or '?'}")
            if c.info and c.info.firmware_version:
                lines.append(f"Running fw:     {c.info.firmware_version}")
            else:
                lines.append("Running fw:     unknown (device blank or banner not captured)")
        if b:
            lines.append("")
            lines.append(f"Target board:   {b.display_name} ({b.board_id})")
            lines.append(f"Bundled fw:     {b.firmware_version}")
            lines.append(f"Chip:           {b.chip}    Flash: {b.flash_size} {b.flash_mode}@{b.flash_freq}")
            lines.append("")
            lines.append("Files to be written:")
            for f in b.files:
                lines.append(f"  {hex(f.offset):>10}  {f.name}")
        self.text.configure(state="normal")
        self.text.delete("1.0", "end")
        self.text.insert("end", "\n".join(lines))
        self.text.configure(state="disabled")


class _FlashPage(_Page):
    def __init__(self, parent, app):
        super().__init__(parent, app)
        ttk.Label(self, text="3. Flash", font=("", 14, "bold")).pack(anchor="w")
        self.status = ttk.Label(self, text="Idle.")
        self.status.pack(anchor="w", pady=(0, _PAD))
        self.log = scrolledtext.ScrolledText(self, height=18, wrap="none", state="disabled")
        self.log.pack(fill="both", expand=True)
        self._done = False

    def append_log(self, text: str) -> None:
        self.log.configure(state="normal")
        self.log.insert("end", text)
        self.log.see("end")
        self.log.configure(state="disabled")

    def on_show(self):
        if not self._done:
            self.app.next_btn.configure(state="disabled")

    def start(self) -> None:
        if not self.app.candidate or not self.app.candidate.matched_bundle:
            return
        self._done = False
        self.app.next_btn.configure(state="disabled")
        port = self.app.candidate.port
        bundle = self.app.candidate.matched_bundle
        skip_backup = self.app.pages["plan"].skip_backup.get()
        backup_dir = Path(tempfile.gettempdir()) / "tcode_flasher_backups"

        cb = self.app.log

        def work():
            self.status.configure(text="Backing up settings...")
            if not skip_backup:
                self.app.backup = backup_mod.backup_settings(port, bundle, backup_dir, cb=cb)
            self.status.configure(text="Flashing firmware + filesystem...")
            flash.write_flash(port, bundle, cb=cb)
            if self.app.backup and not self.app.backup.is_empty:
                self.status.configure(text="Restoring settings...")
                backup_mod.restore_settings(port, bundle, self.app.backup, cb=cb)

        def done(err):
            self._done = True
            if err is not None:
                self.status.configure(text=f"Failed: {err}")
                messagebox.showerror("Flash failed", str(err))
                self.app.back_btn.configure(state="normal")
                return
            self.status.configure(text="Done. Click Next to set up Wi-Fi.")
            self.app.next_btn.configure(state="normal")

        self.app.run_in_worker(work, done)


class _WifiPage(_Page):
    def __init__(self, parent, app):
        super().__init__(parent, app)
        ttk.Label(self, text="4. Wi-Fi (optional)", font=("", 14, "bold")).pack(anchor="w")
        ttk.Label(self, text="Push credentials to the device or skip and configure later via the AP portal.").pack(anchor="w", pady=(0, _PAD))
        form = ttk.Frame(self); form.pack(fill="x")
        ttk.Label(form, text="SSID:").grid(row=0, column=0, sticky="e", padx=(0, _PAD))
        self.ssid = ttk.Entry(form, width=40); self.ssid.grid(row=0, column=1, sticky="w", pady=2)
        ttk.Label(form, text="Password:").grid(row=1, column=0, sticky="e", padx=(0, _PAD))
        self.password = ttk.Entry(form, width=40, show="*"); self.password.grid(row=1, column=1, sticky="w", pady=2)
        bar = ttk.Frame(self); bar.pack(fill="x", pady=_PAD)
        self.push_btn = ttk.Button(bar, text="Push & restart", command=self._push)
        self.push_btn.pack(side="left")
        ttk.Button(bar, text="Skip", command=self._skip).pack(side="left", padx=_PAD)
        self.status = ttk.Label(self, text="")
        self.status.pack(anchor="w")
        self.log = scrolledtext.ScrolledText(self, height=10, wrap="none", state="disabled")
        self.log.pack(fill="both", expand=True, pady=_PAD)

    def append_log(self, text):
        self.log.configure(state="normal")
        self.log.insert("end", text); self.log.see("end")
        self.log.configure(state="disabled")

    def _skip(self):
        self.app._show_page("done")

    def _push(self):
        ssid = self.ssid.get().strip()
        pwd = self.password.get()
        if not ssid:
            messagebox.showwarning("SSID required", "Enter an SSID or click Skip.")
            return
        port = self.app.candidate.port
        self.push_btn.configure(state="disabled")
        self.status.configure(text="Sending credentials and rebooting...")

        def work():
            ip = wifi.push_credentials(port, ssid, pwd, cb=self.app.log)
            self.app._wifi_ip = ip

        def done(err):
            self.push_btn.configure(state="normal")
            if err is not None:
                self.status.configure(text=f"Failed: {err}")
                messagebox.showerror("Wi-Fi setup failed", str(err))
                return
            ip = getattr(self.app, "_wifi_ip", None)
            self.status.configure(text=f"Connected. Device IP: {ip or 'unknown'}")
            self.app._show_page("done")

        self.app.run_in_worker(work, done)


class _DonePage(_Page):
    def __init__(self, parent, app):
        super().__init__(parent, app)
        ttk.Label(self, text="All done", font=("", 16, "bold")).pack(anchor="w")
        self.summary = tk.Text(self, height=14, wrap="word", state="disabled")
        self.summary.pack(fill="both", expand=True, pady=_PAD)

    def on_show(self):
        c = self.app.candidate
        b = c.matched_bundle if c else None
        ip = getattr(self.app, "_wifi_ip", None)
        lines = ["Flash complete."]
        if b:
            lines.append(f"  Board:    {b.display_name} ({b.board_id})")
            lines.append(f"  Firmware: {b.firmware_version}")
        if c:
            lines.append(f"  Port:     {c.port}")
        if ip:
            lines.append("")
            lines.append(f"Device IP: {ip}")
            lines.append(f"Open http://{ip}/ in a browser to finish setup.")
        else:
            lines.append("")
            lines.append("If you didn't push Wi-Fi credentials, the device will boot into")
            lines.append("AP mode. Connect to the SSID it advertises and browse to 192.168.4.1.")
        self.summary.configure(state="normal")
        self.summary.delete("1.0", "end")
        self.summary.insert("end", "\n".join(lines))
        self.summary.configure(state="disabled")


def main() -> int:
    app = FlasherApp()
    app.mainloop()
    return 0


if __name__ == "__main__":
    sys.exit(main())
