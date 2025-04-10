
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

function batterySetup() {
    document.getElementById('batteryLevelEnabled').checked = userSettings["batteryLevelEnabled"];
    //document.getElementById('Battery_Voltage_PIN').value = userSettings["Battery_Voltage_PIN"];
    document.getElementById('batteryLevelNumeric').checked = userSettings["batteryLevelNumeric"];
    //document.getElementById('batteryVoltageMax').value = userSettings["batteryVoltageMax"];
    document.getElementById('batteryCapacityMax').value = userSettings["batteryCapacityMax"];
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
function setBatteryFull() {
    sendWebsocketCommand("setBatteryFull");
}