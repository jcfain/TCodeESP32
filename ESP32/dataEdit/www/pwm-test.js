/* MIT License

Copyright (c) 2024 Jason C. Fain

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions: */

/**
 * PwmTest — manual override panel for verifying PWM output on any
 * PWM-capable pin. Bypasses normal motor/vibe routing; bound directly via
 * PwmManager::attachExclusive on the firmware. Once a test write is sent
 * the pin stays bound to the chosen backend until /pwmTestStop or a
 * device reboot.
 *
 * Surfaced under the existing "Test" section, gated by Advanced settings.
 */
var PwmTest = {
    initialized: false,
    setup() {
        if (this.initialized) return;
        this.initialized = true;

        this.pinSelect = document.getElementById('pwmTestPin');
        this.driverSelect = document.getElementById('pwmTestDriver');
        this.timerSelect = document.getElementById('pwmTestTimer');
        this.freqInput = document.getElementById('pwmTestFreq');
        this.resInput = document.getElementById('pwmTestResolution');
        this.dutyInput = document.getElementById('pwmTestDuty');
        this.dutyOutput = document.getElementById('pwmTestDutyOutput');
        this.applyBtn = document.getElementById('pwmTestApplyBtn');
        this.stopBtn = document.getElementById('pwmTestStopBtn');
        this.statusOut = document.getElementById('pwmTestStatus');

        if (!this.pinSelect) return; // panel not in DOM

        // Populate pin dropdown from validPWMpins (set by setSystemInfo()
        // for the active platform — WROOM32 vs S3).
        this.populatePins();
        this.populateDrivers();
        this.populateTimers();

        // Reasonable defaults: 50 Hz / 14-bit (servo) is the common case
        // we want to verify on the SR6PCB vibe pins.
        if (!this.freqInput.value) this.freqInput.value = 50;
        if (!this.resInput.value) this.resInput.value = 14;
        if (!this.dutyInput.value) this.dutyInput.value = 0;
        this.updateDutyOutput();

        this.dutyInput.addEventListener('input', () => this.updateDutyOutput());
        this.applyBtn.addEventListener('click', () => this.apply());
        this.stopBtn.addEventListener('click', () => this.stop());
    },

    populatePins() {
        removeAllChildren(this.pinSelect);
        const opt = document.createElement('option');
        opt.value = '-1';
        opt.innerText = '-- select pin --';
        this.pinSelect.appendChild(opt);
        if (Array.isArray(validPWMpins)) {
            const sorted = validPWMpins.slice().sort((a, b) => a - b);
            sorted.forEach(p => {
                const o = document.createElement('option');
                o.value = p;
                o.innerText = `GPIO ${p}`;
                this.pinSelect.appendChild(o);
            });
        }
    },

    populateDrivers() {
        removeAllChildren(this.driverSelect);
        // PwmDriver enum: MCPWM=0, LEDC=1
        const optMcpwm = document.createElement('option');
        optMcpwm.value = '0';
        optMcpwm.innerText = 'MCPWM';
        const optLedc = document.createElement('option');
        optLedc.value = '1';
        optLedc.innerText = 'LEDC';
        this.driverSelect.appendChild(optLedc);
        this.driverSelect.appendChild(optMcpwm);
        this.driverSelect.value = '1'; // default LEDC
    },

    populateTimers() {
        removeAllChildren(this.timerSelect);
        const noneOpt = document.createElement('option');
        noneOpt.value = '-1';
        noneOpt.innerText = '(any free timer)';
        this.timerSelect.appendChild(noneOpt);
        const timers = (systemInfo && systemInfo.availableTimers) || [];
        timers.forEach(t => {
            const o = document.createElement('option');
            o.value = t.value;
            const driverLabel = (t.pwmDriver === ESPTimer.PwmDriver.MCPWM) ? 'MCPWM' : 'LEDC';
            o.innerText = `${driverLabel} timer ${t.value} (${t.name})`;
            this.timerSelect.appendChild(o);
        });
    },

    updateDutyOutput() {
        if (this.dutyOutput) this.dutyOutput.innerText = `${this.dutyInput.value}%`;
    },

    apply() {
        const pin = parseInt(this.pinSelect.value, 10);
        if (!(pin >= 0)) {
            this.setStatus('Pick a pin first', true);
            return;
        }
        const driver = parseInt(this.driverSelect.value, 10); // 0=MCPWM 1=LEDC
        const freq = parseInt(this.freqInput.value, 10);
        const resolution = parseInt(this.resInput.value, 10);
        const dutyPct = parseFloat(this.dutyInput.value);

        if (!(freq > 0)) { this.setStatus('Frequency must be > 0', true); return; }
        if (!(resolution >= 1 && resolution <= 20)) { this.setStatus('Resolution 1..20 bits', true); return; }
        if (!(dutyPct >= 0 && dutyPct <= 100)) { this.setStatus('Duty 0..100%', true); return; }

        this.setStatus('Applying…', false);
        this.applyBtn.disabled = true;
        const body = JSON.stringify({ pin, driver, freq, resolution, dutyPct });
        fetch('/pwmTest', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body
        }).then(r => r.json().then(j => ({ ok: r.ok, j })))
        .then(({ ok, j }) => {
            if (ok) {
                this.setStatus(
                    `OK: pin ${pin} -> ${j.backend}, duty ${j.duty}/${j.maxDuty}`,
                    false);
            } else {
                this.setStatus(`Failed: ${(j && j.msg) || 'unknown'}`, true);
            }
        })
        .catch(e => this.setStatus(`Error: ${e}`, true))
        .finally(() => { this.applyBtn.disabled = false; });
    },

    stop() {
        const pin = parseInt(this.pinSelect.value, 10);
        if (!(pin >= 0)) { this.setStatus('Pick a pin first', true); return; }
        this.setStatus('Stopping…', false);
        fetch('/pwmTestStop', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ pin })
        }).then(r => r.json())
        .then(j => this.setStatus(`Detached: pin ${pin}. Reboot to restore normal config.`, false))
        .catch(e => this.setStatus(`Error: ${e}`, true));
    },

    setStatus(msg, isError) {
        if (!this.statusOut) return;
        this.statusOut.innerText = msg;
        this.statusOut.style.color = isError ? '#ff5252' : '#9be59b';
    }
};
