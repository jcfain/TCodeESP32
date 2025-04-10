
/* MIT License

Copyright (c) 2025 Austen Bartels

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

const PDLEVELS = {
    "5V": 0,
    "9V": 1,
    "12V": 2,
    "15V": 3,
    "20V": 4
};

function powerSetup() {
    if (isBoardType(BoardType.SR6PCB)) {
        togglePowerSettings(true);
    } else {
        togglePowerSettings(false);
    }
    
    document.getElementById('servoVoltage').value = userSettings["servoVoltage"];
    document.getElementById('servoVoltagePeek').innerHTML = parseFloat(userSettings["servoVoltage"]).toFixed(2);
    document.getElementById('servoVoltageEn').checked = userSettings["servoVoltageEn"];

    const pdElem = document.getElementById("PDVoltage");
    removeAllChildren(pdElem);
    for (const [key, value] of Object.entries(PDLEVELS)) {
        pdElem.appendChild((function () {
            var opt = document.createElement("option");
            opt.value = value;
            opt.innerText = key;
            return opt;
        })());
    }
    document.getElementById('PDVoltage').value = userSettings["PDVoltage"];
}

function wsPowerStatus(data) {
    var status = data["message"];
    var servoVoltage = status["servoVoltage"];
    var inputVoltage = status["inputVoltage"];

    document.getElementById("currentServoVoltage").value = servoVoltage;
    document.getElementById("currentInputVoltage").value = inputVoltage;
}

function togglePowerSettings(powerEnabled) {
    Utils.toggleControlVisibilityByClassName('powerOnly', powerEnabled);
}

function setPowerSettings() {
    userSettings["PDVoltage"] = document.getElementById('PDVoltage').value;
    userSettings["servoVoltage"] = document.getElementById('servoVoltage').value;
    userSettings["servoVoltageEn"] = document.getElementById('servoVoltageEn').checked;
    updateUserSettings();
}

function updateServoVoltage(voltage) {
    document.getElementById('servoVoltagePeek').innerHTML = parseFloat(voltage).toFixed(2);
    sendWebsocketCommandRaw("setServoVoltage", voltage);
    setPowerSettings();
}

function updateServoVoltageEn(enable) {
    sendWebsocketCommandRaw("enableServoVoltage", enable);
    setPowerSettings();
}

function updatePDLevel(level) {
    sendWebsocketCommandRaw("setPDLevel", level);
    setPowerSettings();
}
