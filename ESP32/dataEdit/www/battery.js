
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

function batterySetup() {
    document.getElementById('batteryLevelEnabled').checked = userSettings["batteryLevelEnabled"];
    //document.getElementById('Battery_Voltage_PIN').value = userSettings["Battery_Voltage_PIN"];
    document.getElementById('batteryLevelNumeric').checked = userSettings["batteryLevelNumeric"];
    //document.getElementById('batteryVoltageMax').value = userSettings["batteryVoltageMax"];
    document.getElementById('batteryCapacityMax').value = userSettings["batteryCapacityMax"];

    document.getElementById('powerMonitor3v3DividerRatio').value = userSettings["powerMonitor3v3DividerRatio"];
    document.getElementById('powerMonitor5vDividerRatio').value = userSettings["powerMonitor5vDividerRatio"];
    document.getElementById('powerMonitorBatteryDividerRatio').value = userSettings["powerMonitorBatteryDividerRatio"];
    document.getElementById('powerMonitorMotorDividerRatio').value = userSettings["powerMonitorMotorDividerRatio"];
    document.getElementById('powerMonitorBusDividerRatio').value = userSettings["powerMonitorBusDividerRatio"];

    document.getElementById('powerMonitor3v3Offset').value = userSettings["powerMonitor3v3Offset"];
    document.getElementById('powerMonitor5vOffset').value = userSettings["powerMonitor5vOffset"];
    document.getElementById('powerMonitorBatteryOffset').value = userSettings["powerMonitorBatteryOffset"];
    document.getElementById('powerMonitorMotorOffset').value = userSettings["powerMonitorMotorOffset"];
    document.getElementById('powerMonitorBusOffset').value = userSettings["powerMonitorBusOffset"];

    document.getElementById('powerMonitorVBusNominal').value = userSettings["powerMonitorVBusNominal"];
    document.getElementById('powerMonitorVMotorNominal').value = userSettings["powerMonitorVMotorNominal"];
}

function wsBatteryStatus(data) {
    var status = data["message"];
    var batteryVoltage = status["batteryVoltage"];
    var batteryCapacityRemainingPercentage = status["batteryCapacityRemainingPercentage"];
    var batteryCapacityRemaining = status["batteryCapacityRemaining"];
    var batteryTemperature = status["batteryTemperature"];

    document.getElementById("batteryVoltage").value = batteryVoltage;
    document.getElementById("batteryCapacityRemaining").value = batteryCapacityRemaining;
    document.getElementById("batteryCapacityRemainingPercentage").value = batteryCapacityRemainingPercentage;
    document.getElementById("batteryTemperature").value = batteryTemperature;
}

// Timestamp of the most recent user toggle of the VMOTOR enable checkbox.
// While inside the hold-off window we ignore servoVoltageEnabled values from
// powerStatus broadcasts so the UI stays authoritative and a stale broadcast
// (emitted before the firmware processed the toggle) can't snap the checkbox
// back to the previous state.
var vmotorEnabledLastUserToggleMs = 0;
const VMOTOR_TOGGLE_HOLDOFF_MS = 2500;

function wsPowerStatus(data) {
    var status = data["message"] || {};

    setPowerVoltageField("powerVoltage3v3", status["Voltage_3V3"]);
    setPowerVoltageField("powerVoltage5v", status["Voltage_5V"]);
    setPowerVoltageField("powerVoltageBattery", status["Voltage_Battery"]);
    setPowerVoltageField("powerVoltageMotor", status["Voltage_Motor"]);
    setPowerVoltageField("powerVoltageBus", status["Voltage_Bus"]);

    // Update VMOTOR enable state if available, but defer to the user during
    // the post-click hold-off so an in-flight broadcast can't undo their toggle.
    if (status["servoVoltageEnabled"] !== undefined) {
        const vmotorEnabledElement = document.getElementById("vmotorEnabled");
        if (vmotorEnabledElement) {
            const sinceToggle = Date.now() - vmotorEnabledLastUserToggleMs;
            if (sinceToggle >= VMOTOR_TOGGLE_HOLDOFF_MS) {
                vmotorEnabledElement.checked = status["servoVoltageEnabled"];
            }
        }
    }
}

function setPowerVoltageField(elementId, sourceStatus) {
    var element = document.getElementById(elementId);
    if(!element)
        return;
    if(!sourceStatus) {
        element.value = "Unset";
        return;
    }
    var adcVoltage = sourceStatus["adcVoltage"];
    var railVoltage = sourceStatus["railVoltage"];
    var pin = sourceStatus["pin"];
    var percentage = sourceStatus["percentage"];
    if(adcVoltage === undefined || railVoltage === undefined || pin === undefined) {
        element.value = "Unknown";
        return;
    }
    element.value = railVoltage.toFixed(2) + " V (ADC " + adcVoltage.toFixed(3) + "V @ pin " + pin + ")" +
        (percentage !== undefined ? " [" + percentage.toFixed(1) + "%]" : "");
}

function toggleBatterySettings(batteryEnabled) {
    var batteryOnly = document.getElementsByClassName('batteryOnly');
    for(var i=0;i < batteryOnly.length; i++)
        batteryOnly[i].style.display = batteryEnabled ? "flex" : "none";
}

function setBatterySettings() {
    userSettings["batteryLevelEnabled"] = document.getElementById('batteryLevelEnabled').checked;
    userSettings["batteryLevelNumeric"] = document.getElementById('batteryLevelNumeric').checked;
    //userSettings["batteryVoltageMax"] = parseFloat(document.getElementById('batteryVoltageMax').value);
    userSettings["batteryCapacityMax"] = parseFloat(document.getElementById('batteryCapacityMax').value);
    setRestartRequired();
    updateUserSettings();
}

function setPowerMonitorSettings() {
    userSettings["powerMonitor3v3DividerRatio"] = parseFloat(document.getElementById('powerMonitor3v3DividerRatio').value);
    userSettings["powerMonitor5vDividerRatio"] = parseFloat(document.getElementById('powerMonitor5vDividerRatio').value);
    userSettings["powerMonitorBatteryDividerRatio"] = parseFloat(document.getElementById('powerMonitorBatteryDividerRatio').value);
    userSettings["powerMonitorMotorDividerRatio"] = parseFloat(document.getElementById('powerMonitorMotorDividerRatio').value);
    userSettings["powerMonitorBusDividerRatio"] = parseFloat(document.getElementById('powerMonitorBusDividerRatio').value);

    userSettings["powerMonitor3v3Offset"] = parseFloat(document.getElementById('powerMonitor3v3Offset').value);
    userSettings["powerMonitor5vOffset"] = parseFloat(document.getElementById('powerMonitor5vOffset').value);
    userSettings["powerMonitorBatteryOffset"] = parseFloat(document.getElementById('powerMonitorBatteryOffset').value);
    userSettings["powerMonitorMotorOffset"] = parseFloat(document.getElementById('powerMonitorMotorOffset').value);
    userSettings["powerMonitorBusOffset"] = parseFloat(document.getElementById('powerMonitorBusOffset').value);

    userSettings["powerMonitorVBusNominal"] = parseFloat(document.getElementById('powerMonitorVBusNominal').value);
    userSettings["powerMonitorVMotorNominal"] = parseFloat(document.getElementById('powerMonitorVMotorNominal').value);

    setRestartRequired();
    updateUserSettings();
}
function setBatteryFull() {
    sendWebsocketCommand("setBatteryFull");
}

function setVmotorEnabled() {
    const enabled = document.getElementById('vmotorEnabled').checked;
    // Mark the click time so wsPowerStatus ignores the next few broadcasts and
    // the user's choice remains authoritative.
    vmotorEnabledLastUserToggleMs = Date.now();
    sendWebsocketCommand("setServoVoltageEnabled", enabled ? "true" : "false");
}