/* MIT License

Copyright (c) 2024 Jason C. Fain

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
        for (let index = 0; index < availableTimers.length; index++) {
            const timerObj = availableTimers[index];
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
        }
    }
};