/* MIT License

Copyright (c) 2026 Jason C. Fain

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE. */

ESPTimer = {
    initialized: false,
    model: {},
    debounces: [],
    // PwmDriver enum values must match C++ enum class PwmDriver
    PwmDriver: { MCPWM: 0, LEDC: 1 },
    show() {
        this.modal.show();
    },
    setup() {
        if(this.initialized) {
            return;
        }
        this.initialized = true;
        this.modal = document.getElementById("espTimerSetupModal");
        let table = Utils.createModalTableSection(this.modal, "Timer setup");
        let availableTimers = systemInfo["availableTimers"];
        // We dont know what the resolution of the attached device at this point.
        // Maybe with a lookup and a bit of a redesign we can validate the frequency.
        // For now, the user needs to know what they are doing here.
        //const maxHz = Math.floor(80000000 / (2 ** systemInfo.servoPWMResolution));// 2^16bit = 65536. 80000000(80Mhz) ÷ 65536 = 1220.703125. floor = 1220
        for (let index = 0; index < availableTimers.length; index++) {
            const timerObj = availableTimers[index];

            // Frequency row
            let timerFrequencyRow = Utils.createNumericFormRow(0, timerObj.name + " (hz)", 'timerFrequency'+index, pinoutSettings[timerObj.id], 50, 80000000);
            timerFrequencyRow.title = `Set the frequency of this timer`;
            timerFrequencyRow.input.oninput = function(timerObj, timerFrequencyRow) {
                if(this.debounces[timerObj.id])
                    clearTimeout(this.debounces[timerObj.id]);
                this.debounces[timerObj.id] = setTimeout( function(){
                    if(validateIntControl(timerFrequencyRow.input, pinoutSettings, timerObj.id)) {
                        setRestartRequired();
                        postPinoutSettings(0);
                    }
                }, defaultDebounce);
            }.bind(this, timerObj, timerFrequencyRow);
            table.body.appendChild(timerFrequencyRow.row);

            // Driver row — only shown when the driverKey is present (ESP-IDF 5+ with PwmDriver support)
            if(timerObj.driverKey) {
                let driverRow = Utils.createFormRow(0);
                let driverLabel = Utils.createFormCell(0, timerObj.name + " driver");
                let driverCell = Utils.createFormCell();
                let driverSelect = document.createElement("select");
                driverSelect.id = 'timerDriver' + index;
                driverSelect.title = "PWM driver for outputs assigned to this timer. MCPWM gives servo-grade timing precision; LEDC is for motors/misc. The firmware falls back to LEDC automatically if MCPWM is full.";

                let mcpwmOption = document.createElement("option");
                mcpwmOption.value = this.PwmDriver.MCPWM;
                mcpwmOption.innerText = "MCPWM (servo)";
                let ledcOption = document.createElement("option");
                ledcOption.value = this.PwmDriver.LEDC;
                ledcOption.innerText = "LEDC (vibe/misc)";
                driverSelect.appendChild(mcpwmOption);
                driverSelect.appendChild(ledcOption);
                driverSelect.value = pinoutSettings[timerObj.driverKey] !== undefined
                    ? pinoutSettings[timerObj.driverKey]
                    : timerObj.pwmDriver;

                driverSelect.onchange = function(timerObj, driverSelect) {
                    pinoutSettings[timerObj.driverKey] = parseInt(driverSelect.value);
                    setRestartRequired();
                    postPinoutSettings(0);
                    validatePwmDriverContention();
                }.bind(this, timerObj, driverSelect);

                driverCell.appendChild(driverSelect);
                driverRow.appendChild(driverLabel);
                driverRow.appendChild(driverCell);
                table.body.appendChild(driverRow);
            }
        }
        const helpTextNodeDiv = document.createElement("div");
        helpTextNodeDiv.style = "font-size: 0.6em;"
        const freqMhz = systemInfo.apbClockFrequency / 1000000;
        helpTextNodeDiv.innerHTML =
`
To calculate the MAXIMUM frequency for your chip (Not the servo)
<br>use the formula:
<br>&nbsp&nbsp&nbsp&nbsp ${systemInfo.apbClockFrequency} ÷ (2^resolution)
<br>The max resolution for your chip is ${systemInfo.maxPWMResolution} bit
<br>The APB clock frequency is ${freqMhz} Mhz
`;
        table.body.appendChild(helpTextNodeDiv);
    },
    /**
     * Returns an object { mcpwm: N, ledc: N } counting how many LEDC-channel
     * outputs are assigned to MCPWM timers and vice-versa, based on the current
     * pinoutSettings and availableTimers driver config.
     */
    getDriverCounts() {
        let counts = { mcpwm: 0, ledc: 0 };
        let timers = systemInfo["availableTimers"];
        if(!timers) return counts;

        // Build a map from channel numeric value -> pwmDriver for quick lookup
        let channelDriverMap = {};
        timers.forEach(timer => {
            // Read the current (possibly unsaved) driver from pinoutSettings if available
            let driver = timer.driverKey && pinoutSettings[timer.driverKey] !== undefined
                ? parseInt(pinoutSettings[timer.driverKey])
                : timer.pwmDriver;
            if(timer.channels) {
                timer.channels.forEach(ch => {
                    channelDriverMap[ch.value] = driver;
                });
            }
        });

        // All channel select elements contribute an active output if their value != -1
        const timerSelects = document.getElementsByName('timerChannels');
        timerSelects.forEach(sel => {
            let val = parseInt(sel.value);
            if(val > -1 && channelDriverMap[val] !== undefined) {
                if(channelDriverMap[val] === ESPTimer.PwmDriver.MCPWM) counts.mcpwm++;
                else counts.ledc++;
            }
        });
        return counts;
    }
};