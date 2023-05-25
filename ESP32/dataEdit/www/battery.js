
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