/* MIT License

Copyright (c) 2020 Jason C. Fain

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


var userSettings = {};
var upDateTimeout;
var restartRequired = false;
var documentLoaded = false;
var newtoungeHatExists = false;
var infoNode;

document.addEventListener("DOMContentLoaded", function() {
	loadPage()
  });
  
function loadPage()
{
	infoNode = document.getElementById('info');
    onDocumentLoad();
}
function onDocumentLoad()
{
	var xhr = new XMLHttpRequest();
	xhr.open('GET', "/userSettings", true);
	xhr.responseType = 'json';
	xhr.onload = function() {
        var status = xhr.status;
        if (status !== 200) {
			showError("Error loading user settings!");
		} else {
            userSettings = xhr.response;
            getUserSettings()
		}
	};
	xhr.send();
}

function onDefaultClick() 
{		
	if (confirm("WARNING! Are you sure you wish to reset ALL settings?")) 
	{
		infoNode.hidden = true;
		infoNode.innerText = "Resetting...";
		infoNode.style.color = 'black';
		var xhr = new XMLHttpRequest();
		xhr.open("POST", "/default", true);
		xhr.onreadystatechange = function() 
		{
			if (xhr.readyState === 4) 
			{
				onDocumentLoad();
				infoNode.innerText = "Settings reset!";
                infoNode.style.color = 'green';
				document.getElementById('requiresRestart').hidden = false;
				document.getElementById('resetBtn').disabled = false ;
				setTimeout(() => 
				{
                    infoNode.hidden = true;
                    infoNode.innerText = "";
				}, 5000)
			}
		}
		xhr.send();
	}
}

function getUserSettings() 
{
    toggleNonTCodev3Options(userSettings["TCodeVersion"] == 1);
    toggleDeviceOptions(userSettings["sr6Mode"]);
    toggleStaticIPSettings(userSettings["staticIP"]);
    toggleDisplaySettings(userSettings["displayEnabled"]);
    togglePitchServoFrequency(userSettings["pitchFrequencyIsDifferent"]);
    document.getElementById("version").innerHTML = userSettings["esp32Version"];
    var xMin = userSettings["xMin"];
    var xMax = userSettings["xMax"];
    document.getElementById("xMin").value = xMin;
    calculateAndUpdateMinUI("x", xMin);
    document.getElementById("xMax").value = xMax;
    calculateAndUpdateMaxUI("x", xMax);
    updateRangePercentageLabel("x", tcodeToPercentage(xMin), tcodeToPercentage(xMax));

    var yRollMin = userSettings["yRollMin"];
    var yRollMax = userSettings["yRollMax"];
    document.getElementById("yRollMin").value = yRollMin;
    calculateAndUpdateMinUI("yRoll", yRollMin);
    document.getElementById("yRollMax").value = yRollMax;
    calculateAndUpdateMaxUI("yRoll", yRollMax);
    updateRangePercentageLabel("yRoll", tcodeToPercentage(yRollMin), tcodeToPercentage(yRollMax));

    var xRollMin = userSettings["xRollMin"];
    var xRollMax = userSettings["xRollMax"];
    document.getElementById("xRollMin").value =xRollMin;
    calculateAndUpdateMinUI("xRoll", xRollMin);
    document.getElementById("xRollMax").value =xRollMax;
    calculateAndUpdateMaxUI("xRoll", xRollMax);
    updateRangePercentageLabel("xRoll", tcodeToPercentage(xRollMin), tcodeToPercentage(xRollMax));

    updateSpeedUI(userSettings["speed"]);

    document.getElementById("udpServerPort").value = userSettings["udpServerPort"];
    document.getElementById("hostname").value = userSettings["hostname"];
    document.getElementById("friendlyName").value = userSettings["friendlyName"];
	document.getElementById("servoFrequency").value = userSettings["servoFrequency"];
	document.getElementById("pitchFrequency").value = userSettings["pitchFrequency"];
	document.getElementById("valveFrequency").value = userSettings["valveFrequency"];
	document.getElementById("twistFrequency").value = userSettings["twistFrequency"];
	
	document.getElementById("continuousTwist").checked = userSettings["continuousTwist"];
	document.getElementById("analogTwist").checked = userSettings["analogTwist"];
    
    document.getElementById("TwistFeedBack_PIN").value = userSettings["TwistFeedBack_PIN"];
    document.getElementById("RightServo_PIN").value = userSettings["RightServo_PIN"];
    document.getElementById("LeftServo_PIN").value = userSettings["LeftServo_PIN"];
    document.getElementById("RightUpperServo_PIN").value = userSettings["RightUpperServo_PIN"];
    document.getElementById("LeftUpperServo_PIN").value = userSettings["LeftUpperServo_PIN"];
    document.getElementById("PitchLeftServo_PIN").value = userSettings["PitchLeftServo_PIN"];
    document.getElementById("PitchRightServo_PIN").value = userSettings["PitchRightServo_PIN"];
    document.getElementById("ValveServo_PIN").value = userSettings["ValveServo_PIN"];
	document.getElementById("TwistServo_PIN").value = userSettings["TwistServo_PIN"];
    document.getElementById("Vibe0_PIN").value = userSettings["Vibe0_PIN"];
    document.getElementById("Vibe1_PIN").value = userSettings["Vibe1_PIN"];
	document.getElementById("LubeManual_PIN").value = userSettings["LubeManual_PIN"];
	
    document.getElementById("RightServo_ZERO").value = userSettings["RightServo_ZERO"];
    document.getElementById("LeftServo_ZERO").value = userSettings["LeftServo_ZERO"];
    document.getElementById("RightUpperServo_ZERO").value = userSettings["RightUpperServo_ZERO"];
    document.getElementById("LeftUpperServo_ZERO").value = userSettings["LeftUpperServo_ZERO"];
    document.getElementById("PitchLeftServo_ZERO").value = userSettings["PitchLeftServo_ZERO"];
    document.getElementById("PitchRightServo_ZERO").value = userSettings["PitchRightServo_ZERO"];
    document.getElementById("ValveServo_ZERO").value = userSettings["ValveServo_ZERO"];
	document.getElementById("TwistServo_ZERO").value = userSettings["TwistServo_ZERO"];
	document.getElementById("lubeEnabled").checked = userSettings["lubeEnabled"];
	document.getElementById("lubeAmount").value = userSettings["lubeAmount"];
	document.getElementById("sr6Mode").checked = userSettings["sr6Mode"];
	document.getElementById("autoValve").checked = userSettings["autoValve"];
	document.getElementById("inverseValve").checked = userSettings["inverseValve"];
	document.getElementById("valveServo90Degrees").checked = userSettings["valveServo90Degrees"];
	document.getElementById("inverseStroke").checked = userSettings["inverseStroke"];
	document.getElementById("inversePitch").checked = userSettings["inversePitch"];

	document.getElementById("displayEnabled").checked = userSettings["displayEnabled"];
	document.getElementById("sleeveTempEnabled").checked = userSettings["sleeveTempEnabled"];
	document.getElementById("tempControlEnabled").checked = userSettings["tempControlEnabled"];
	document.getElementById("pitchFrequencyIsDifferent").checked = userSettings["pitchFrequencyIsDifferent"];
	document.getElementById("Display_Screen_Width").value = userSettings["Display_Screen_Width"];
	document.getElementById("Display_Screen_Height").value = userSettings["Display_Screen_Height"];
	document.getElementById("TargetTemp").value = userSettings["TargetTemp"];
	document.getElementById("HeatPWM").value = userSettings["HeatPWM"];
	document.getElementById("HoldPWM").value = userSettings["HoldPWM"];
	document.getElementById("Display_I2C_Address").value = userSettings["Display_I2C_Address"];
	// document.getElementById("Display_Rst_PIN").value = userSettings["Display_Rst_PIN"];
	document.getElementById("Temp_PIN").value = userSettings["Temp_PIN"];
	document.getElementById("Heater_PIN").value = userSettings["Heater_PIN"];
	document.getElementById("WarmUpTime").value = userSettings["WarmUpTime"];
    document.getElementById('TCodeVersion').value = userSettings["TCodeVersion"];
	
    document.getElementById("ssid").value = userSettings["ssid"];
    document.getElementById("wifiPass").value = userSettings["wifiPass"];
    document.getElementById("staticIP").checked = userSettings["staticIP"];
    document.getElementById("localIP").value = userSettings["localIP"];
    document.getElementById("gateway").value = userSettings["gateway"];
    document.getElementById("subnet").value = userSettings["subnet"];
    document.getElementById("dns1").value = userSettings["dns1"];
    document.getElementById("dns2").value = userSettings["dns2"];
    
    newtoungeHatExists = userSettings["newtoungeHatExists"]

    document.getElementById("TwistFeedBack_PIN").readonly = newtoungeHatExists;
    document.getElementById("RightServo_PIN").readonly = newtoungeHatExists;
    document.getElementById("LeftServo_PIN").readonly = newtoungeHatExists;
    document.getElementById("RightUpperServo_PIN").readonly = newtoungeHatExists;
    document.getElementById("LeftUpperServo_PIN").readonly = newtoungeHatExists;
    document.getElementById("PitchLeftServo_PIN").readonly = newtoungeHatExists;
    document.getElementById("PitchRightServo_PIN").readonly = newtoungeHatExists;
    document.getElementById("ValveServo_PIN").readonly = newtoungeHatExists;
	document.getElementById("TwistServo_PIN").readonly = newtoungeHatExists;
    document.getElementById("Vibe0_PIN").readonly = newtoungeHatExists;
    document.getElementById("Vibe1_PIN").readonly = newtoungeHatExists;
    document.getElementById("LubeManual_PIN").readonly = newtoungeHatExists;
	document.getElementById("Temp_PIN").readonly = newtoungeHatExists;
	document.getElementById("Heater_PIN").readonly = newtoungeHatExists;
	// document.getElementById("Display_Rst_PIN").readonly = newtoungeHatExists;

	document.getElementById("Display_Screen_Width").readonly = true;
	document.getElementById("Display_Screen_Height").readonly = true;
	// document.getElementById("Display_Rst_PIN").readonly = true;

    documentLoaded = true;
}

function updateUserSettings() 
{
    if (documentLoaded) {
        if(upDateTimeout !== null) 
        {
            clearTimeout(upDateTimeout);
        }
        upDateTimeout = setTimeout(() => 
        {
            closeError();
            infoNode.hidden = false;
            infoNode.innerText = "Saving...";
            infoNode.style.color = 'black';
            var xhr = new XMLHttpRequest();
            var response = {};
            xhr.open("POST", "/settings", true);
            xhr.onreadystatechange = function() 
            {
                if (xhr.readyState === 4) 
				{
                    if(xhr.responseText === '')
                    {
                        response["msg"] = xhr.status + ': ' + xhr.statusText;
                    }
                    else 
                    {
                        response = JSON.parse(xhr.responseText);
                    }
                    if (response["msg"] !== "done") 
                    {
                        infoNode.hidden = true;
                        infoNode.innerText = "";
                        showError("Error saving: " + response["msg"]);
                        onDocumentLoad();
                    } 
                    else 
                    {
                        infoNode.visibility = "visible";
                        infoNode.innerText = "Settings saved!";
                        infoNode.style.color = 'green';
                        if (restartRequired) 
                        {
                            document.getElementById('requiresRestart').hidden = false;
                            document.getElementById('resetBtn').disabled = false;
                        }
                        setTimeout(() => 
                        {
                            infoNode.hidden = true;
                            infoNode.innerText = "";
                        }, 5000)
                    }
                }
            }
            xhr.setRequestHeader('Content-Type', 'application/json');
            var body = JSON.stringify(userSettings);
            xhr.send(body);
            upDateTimeout = null;
        }, 3000);
    }
}
function closeError() 
{
    document.getElementById("errorText").innerHTML = "";
    document.getElementById("errorMessage").hidden = true;
}
function showError(message) 
{
    document.getElementById("errorText").innerHTML += message;
    document.getElementById("errorMessage").hidden = false;
}
function onMinInput(axis) 
{
    var axisName = axis + "Min";
    var inputAxisMin = document.getElementById(axisName);
    var inputAxisMax = document.getElementById(axis + "Max"); 
    inputAxisMin.value = Math.min(inputAxisMin.value, inputAxisMax.value - 2);
    
    var value=(100/(parseInt(inputAxisMin.max)-parseInt(inputAxisMin.min)))*parseInt(inputAxisMin.value)-(100/(parseInt(inputAxisMin.max)-parseInt(inputAxisMin.min)))*parseInt(inputAxisMin.min);
    updateMinUI(axis, value);

    updateRangePercentageLabel(axis, inputAxisMin.value, document.getElementById(""+axis+"Max").value);

    var tcodeValue =  percentageToTcode(inputAxisMin.value);
    if (tcodeValue < 1) {
        tcodeValue = 1;
    }
    //console.log("Min tcode: " + tcodeValue);
    userSettings[axisName] = tcodeValue;
    updateUserSettings();
}

function onMaxInput(axis) 
{
    var axisName = axis + "Max";
    var inputAxisMax = document.getElementById(axisName);
    var inputAxisMin = document.getElementById(axis + "Min"); 
    inputAxisMax.value = Math.max(inputAxisMax.value, inputAxisMin.value - (-2));

    var value=(100/(parseInt(inputAxisMax.max)-parseInt(inputAxisMax.min)))*parseInt(inputAxisMax.value)-(100/(parseInt(inputAxisMax.max)-parseInt(inputAxisMax.min)))*parseInt(inputAxisMax.min);
    updateMaxUI(axis, value);

    updateRangePercentageLabel(axis, document.getElementById(""+axis+"Min").value, inputAxisMax.value);

    var tcodeValue =  percentageToTcode(inputAxisMax.value);
    //console.log("Max tcode: " + tcodeValue);
    userSettings[axisName] = tcodeValue;
    updateUserSettings();
}

function calculateAndUpdateMinUI(axis, tcodeValue) 
{
    updateMinUI(axis, tcodeToPercentage(tcodeValue));
}

function calculateAndUpdateMaxUI(axis, tcodeValue) 
{
    updateMaxUI(axis, tcodeToPercentage(tcodeValue));
}

function updateMinUI(axis, value) 
{
    document.getElementById("" + axis + "Min").value =value;
    document.getElementById("" + axis + "InverseMin").style.width = value + "%";
    document.getElementById("" + axis + "Range").style.left = value + "%";
    document.getElementById("" + axis + "ThumbMin").style.left = value + "%";
    document.getElementById("" + axis + "SignMin").style.left = value + "%";
    document.getElementById("" + axis + "ValueMin").innerText = value + '%';
}

function updateMaxUI(axis, value) 
{
    document.getElementById("" + axis + "Max").value = value;
    document.getElementById("" + axis + "InverseMax").style.width = (100-value) + "%";
    document.getElementById("" + axis + "Range").style.right = (100-value) + "%";
    document.getElementById("" + axis + "ThumbMax").style.left = value + "%";
    document.getElementById("" + axis + "SignMax").style.left = value + "%";
    document.getElementById("" + axis + "ValueMax").innerText = value + '%';
}

function updateSpeedUI(millisec) 
{
    var value = speedToPercentage(millisec);
    document.getElementById("speedSign").style.left = value + "%";
    document.getElementById("speedThumb").style.left = value + "%";
    document.getElementById("speedValue").style.left = value + "%";
    //document.getElementById("speedInverseMin").style.width = (value) + "%";
    document.getElementById("speedInverseMax").style.width =  (100-value) + "%";
    document.getElementById("speedRange").style.right = (100-value) + "%";
    //document.getElementById("speedRange").style.left = value + "%";
    if (millisec > 999) 
    {
        document.getElementById("speedValue").innerText = millisec;
        document.getElementById("speedLabel").innerText = millisec + "ms";
    } 
    else 
    {
        document.getElementById("speedValue").innerText = "off";
        document.getElementById("speedLabel").innerText = "off";
    }
}

function updateRangePercentageLabel(axis, minValue, maxValue) 
{
    document.getElementById("" + axis + "RangeLabel").innerText = maxValue - minValue + "%";
}

function tcodeToPercentage(tcodeValue) 
{
    return convertRange(1, 1000, 1, 100, tcodeValue);
}

function percentageToTcode(value) 
{
    return convertRange(1, 100, 1, 1000, value);
}

function speedToPercentage(tcodeValue) 
{
    return convertRange(999, 4000, 0, 100, tcodeValue);
}

function convertRange(input_start, input_end, output_start, output_end, value) 
{
    var slope = (output_end - output_start) / (input_end - input_start);
    return Math.round((output_start + slope * (value - input_start)));
}

function onSpeedInput() 
{
    var speedInMillisecs = parseInt(document.getElementById("speedInput").value);
    userSettings["speed"] = speedInMillisecs > 999 ? speedInMillisecs : 0;
    updateSpeedUI(speedInMillisecs);
    updateUserSettings();
}

function updateUdpPort() 
{
    userSettings["udpServerPort"] = parseInt(document.getElementById('udpServerPort').value);
    showRestartRequired();
    updateUserSettings();
}

function setPitchFrequencyIsDifferent() 
{
    var isChecked = document.getElementById('pitchFrequencyIsDifferent').checked;
    userSettings["pitchFrequencyIsDifferent"] = isChecked;
    togglePitchServoFrequency(isChecked);
}

function updateServoFrequency() 
{
    userSettings["servoFrequency"] = parseInt(document.getElementById('servoFrequency').value);
    userSettings["pitchFrequency"] = parseInt(document.getElementById('pitchFrequency').value);
    userSettings["valveFrequency"] = parseInt(document.getElementById('valveFrequency').value);
    userSettings["twistFrequency"] = parseInt(document.getElementById('twistFrequency').value);
    showRestartRequired();
    updateUserSettings();
}

function updateContinuousTwist()
{
	var checked = document.getElementById('continuousTwist').checked;
	if (checked) 
	{
		if (confirm("WARNING! If you enable continuous twist\nMAKE SURE THERE ARE NO WIRES CONNECTED TO YOUR FLESHLIGHT CASE!\nThis can twist the wires and possible injury can occur.\n CONFIRM THERE ARE NO WIRES CONNECTED?")) 
		{
			userSettings["continuousTwist"] = checked;
			updateUserSettings();
		} 
		else 
		{
			document.getElementById('continuousTwist').checked = false;
		} 
	}
	else
	{
		userSettings["continuousTwist"] = false;
		updateUserSettings();
	}
}
function updateAnalogTwist()
{
	var checked = document.getElementById('analogTwist').checked;
    userSettings["analogTwist"] = checked;
    updateUserSettings();
}

function updateHostName() 
{
    userSettings["hostname"] = document.getElementById('hostname').value;
    showRestartRequired();
    updateUserSettings();
}

function updateFriendlyName() 
{
    userSettings["friendlyName"] = document.getElementById('friendlyName').value;
    showRestartRequired();
    updateUserSettings();
}

function setSR6Mode() {
    userSettings["sr6Mode"] = document.getElementById('sr6Mode').checked;
    toggleDeviceOptions(userSettings["sr6Mode"]);
    showRestartRequired();
	updateUserSettings();
}

function setAutoValve() {
    userSettings["autoValve"] = document.getElementById('autoValve').checked;
	updateUserSettings();
}
function setInverseValve() {
    userSettings["inverseValve"] = document.getElementById('inverseValve').checked;
	updateUserSettings();
}
function setValveServo90Degrees() {
	var checked = document.getElementById('valveServo90Degrees').checked;
	if (checked) 
	{
		if (confirm("WARNING! If you 90 degree servo\nMAKE SURE YOU ARE NOT USING THE T-Valve LID!\nThe servo will stall hitting the wall and burn out!")) 
		{
			userSettings["valveServo90Degrees"] = checked;
			updateUserSettings();
		} 
		else 
		{
			document.getElementById('valveServo90Degrees').checked = false;
		} 
	}
	else
	{
		userSettings["valveServo90Degrees"] = false;
		updateUserSettings();
	}
}
function setInverseStroke() {
    userSettings["inverseStroke"] = document.getElementById('inverseStroke').checked;
	updateUserSettings();
}
function setInversePitch() {
    userSettings["inversePitch"] = document.getElementById('inversePitch').checked;
	updateUserSettings();
}

function updatePins() 
{
    if (!newtoungeHatExists) {
        if(upDateTimeout !== null) 
        {
            clearTimeout(upDateTimeout);
        }
        upDateTimeout = setTimeout(() => 
        {
            //PWM availible on: 2,4,5,12-19,21-23,25-27,32-33
            var validPWMpins = [2,4,5,12,13,14,15,16,17,18,19,21,23,25,26,27,32,33];
            var assignedPins = [];
            var errors = [];
            var pmwErrors = [];
            var twistFeedBack = parseInt(document.getElementById('TwistFeedBack_PIN').value);
            assignedPins.push(twistFeedBack);

            var twistServo = parseInt(document.getElementById('TwistServo_PIN').value);
            if(assignedPins.indexOf(twistServo) > -1)
                errors.push("Twist servo pin");
            if(validPWMpins.indexOf(twistServo) == -1)
                pmwErrors.push("Twist servo pin: "+twistServo);
            assignedPins.push(twistServo);

            var rightPin = parseInt(document.getElementById('RightServo_PIN').value);
            if(assignedPins.indexOf(rightPin) > -1)
                errors.push("Right servo pin");
            if(validPWMpins.indexOf(rightPin) == -1)
                pmwErrors.push("Right servo pin: "+rightPin);
            assignedPins.push(rightPin);

            var leftPin = parseInt(document.getElementById('LeftServo_PIN').value);
            if(assignedPins.indexOf(leftPin) > -1)
                errors.push("Left servo pin");
            if(validPWMpins.indexOf(leftPin) == -1)
                pmwErrors.push("Left servo pin: "+leftPin);
            assignedPins.push(leftPin);

            var rightUpper = parseInt(document.getElementById('RightUpperServo_PIN').value);
            if(assignedPins.indexOf(rightUpper) > -1)
                errors.push("Right upper servo pin");
            if(validPWMpins.indexOf(rightUpper) == -1)
                pmwErrors.push("Right upper servo pin: "+rightUpper);
            assignedPins.push(rightUpper);

            var leftUpper = parseInt(document.getElementById('LeftUpperServo_PIN').value);
            if(assignedPins.indexOf(leftUpper) > -1)
                errors.push("Left upper servo pin");
            if(validPWMpins.indexOf(leftUpper) == -1)
                pmwErrors.push("Left upper servo pin: "+leftUpper);
            assignedPins.push(leftUpper);

            var pitchLeft = parseInt(document.getElementById('PitchLeftServo_PIN').value);
            if(assignedPins.indexOf(pitchLeft) > -1)
                errors.push("Pitch left servo pin");
            if(validPWMpins.indexOf(pitchLeft) == -1)
                pmwErrors.push("Pitch left servo pin: "+pitchLeft);
            assignedPins.push(pitchLeft);

            var pitchRight = parseInt(document.getElementById('PitchRightServo_PIN').value);
            if(assignedPins.indexOf(pitchRight) > -1)
                errors.push("Pitch right servo pin");
            if(validPWMpins.indexOf(pitchRight) == -1)
                pmwErrors.push("Pitch right servo pin: "+pitchRight);
            assignedPins.push(pitchRight);

            var valveServo = parseInt(document.getElementById('ValveServo_PIN').value);
            if(assignedPins.indexOf(valveServo) > -1)
                errors.push("Valve servo pin");
            if(validPWMpins.indexOf(valveServo) == -1)
                pmwErrors.push("Valve servo pin: "+valveServo);
            assignedPins.push(valveServo);

            var vibe0 = parseInt(document.getElementById('Vibe0_PIN').value);
            if(assignedPins.indexOf(vibe0) > -1)
                errors.push("Vibe 0 pin");
            if(validPWMpins.indexOf(vibe0) == -1)
                pmwErrors.push("Vibe 0 pin: "+vibe0);
            assignedPins.push(vibe0);

            var vibe1 = parseInt(document.getElementById('Vibe1_PIN').value);
            if(assignedPins.indexOf(vibe1) > -1)
                errors.push("Lube/Vibe 1 pin");
            if(validPWMpins.indexOf(vibe1) == -1)
                pmwErrors.push("Lube/Vibe 1 pin: "+vibe1);
            assignedPins.push(vibe1);

            var temp = parseInt(document.getElementById('Temp_PIN').value);
            if(assignedPins.indexOf(temp) > -1)
                errors.push("Temp pin");
            if(validPWMpins.indexOf(temp) == -1)
                pmwErrors.push("Temp pin: "+temp);
            assignedPins.push(temp);

            var heat = parseInt(document.getElementById('Heater_PIN').value);
            if(assignedPins.indexOf(heat) > -1)
                errors.push("Heater pin");
            if(validPWMpins.indexOf(heat) == -1)
                pmwErrors.push("Heater pin: "+heat);
            assignedPins.push(heat);

            var lubeManual = parseInt(document.getElementById('LubeManual_PIN').value);
            if(assignedPins.indexOf(lubeManual) > -1)
                errors.push("Lube manual pin");
            if(validPWMpins.indexOf(lubeManual) == -1)
                pmwErrors.push("Lube manual pin: "+lubeManual);

            if (errors.length > 0 || pmwErrors.length > 0) {
                var errorString = "Pins NOT saved due to invalid input.<br>";
                if(errors.length > 0 )
                    errorString += "<div style='margin-left: 25px;'>The following pins are duplicated:<br><div style='color: white; margin-left: 25px;'>"+errors.join("<br>")+"</div></div>";
                if (pmwErrors.length > 0) {
                    if(errors.length > 0) {
                        errorString += "<br>";
                    } 
                    errorString += "<div style='margin-left: 25px;'>The following pins are invalid PWM pins:<br><div style='color: white; margin-left: 25px;'>"+pmwErrors.join("<br>")+"</div></div>";
                }
                showError(errorString);
            } else {
                closeError();
                userSettings["TwistFeedBack_PIN"] = twistFeedBack;
                userSettings["TwistServo_PIN"] = twistServo
                userSettings["RightServo_PIN"] = rightPin;
                userSettings["LeftServo_PIN"] = leftPin;
                userSettings["RightUpperServo_PIN"] = rightUpper;
                userSettings["LeftUpperServo_PIN"] = leftUpper;
                userSettings["PitchLeftServo_PIN"] = pitchLeft;
                userSettings["PitchRightServo_PIN"] = pitchRight;
                userSettings["ValveServo_PIN"] = valveServo;
                userSettings["Vibe0_PIN"] = vibe0;
                userSettings["Vibe1_PIN"] = vibe1;
                userSettings["Temp_PIN"] = temp;
                userSettings["Heater_PIN"] = heat;
                userSettings["LubeManual_PIN"] = lubeManual;;
                showRestartRequired();
                updateUserSettings();
            }
        }, 2000);
    }
}
function updateZeros() 
{
    if(upDateTimeout !== null) 
    {
        clearTimeout(upDateTimeout);
    }
    upDateTimeout = setTimeout(() => 
    {
        var validValue = true;
        var invalidValues = [];
        var RightServo_ZERO = parseInt(document.getElementById('RightServo_ZERO').value);
        if(!RightServo_ZERO || RightServo_ZERO > 1750 || RightServo_ZERO < 1250)
        {
            validValue = false;
            invalidValues.push("Right servo ZERO")
        }
        var LeftServo_ZERO = parseInt(document.getElementById('LeftServo_ZERO').value);
        if(!LeftServo_ZERO || LeftServo_ZERO > 1750 || LeftServo_ZERO < 1250)
        {
            validValue = false;
            invalidValues.push("Left servo ZERO")
        }
        var RightUpperServo_ZERO = parseInt(document.getElementById('RightUpperServo_ZERO').value);
        if(!RightUpperServo_ZERO || RightUpperServo_ZERO > 1750 || RightUpperServo_ZERO < 1250)
        {
            validValue = false;
            invalidValues.push("Right upper servo ZERO")
        }
        var LeftUpperServo_ZERO = parseInt(document.getElementById('LeftUpperServo_ZERO').value);
        if(!LeftUpperServo_ZERO || LeftUpperServo_ZERO > 1750 || LeftUpperServo_ZERO < 1250)
        {
            validValue = false;
            invalidValues.push("Left upper servo ZERO")
        }
        var PitchLeftServo_ZERO = parseInt(document.getElementById('PitchLeftServo_ZERO').value);
        if(!PitchLeftServo_ZERO || PitchLeftServo_ZERO > 1750 || PitchLeftServo_ZERO < 1250)
        {
            validValue = false;
            invalidValues.push("Pitch left servo ZERO")
        }
        var PitchRightServo_ZERO = parseInt(document.getElementById('PitchRightServo_ZERO').value);
        if(!PitchRightServo_ZERO || PitchRightServo_ZERO > 1750 || PitchRightServo_ZERO < 1250)
        {
            validValue = false;
            invalidValues.push("Pitch right servo ZERO")
        }
        var ValveServo_ZERO = parseInt(document.getElementById('ValveServo_ZERO').value);
        if(!ValveServo_ZERO || ValveServo_ZERO > 1750 || ValveServo_ZERO < 1250)
        {
            validValue = false;
            invalidValues.push("Valve servo ZERO")
        }
        var TwistServo_ZERO = parseInt(document.getElementById('TwistServo_ZERO').value);
        if(!TwistServo_ZERO || TwistServo_ZERO > 1750 || TwistServo_ZERO < 1250)
        {
            validValue = false;
            invalidValues.push("Twist servo ZERO")
        }

        if(validValue)
        {
            closeError();
            userSettings["RightServo_ZERO"] = document.getElementById('RightServo_ZERO').value;
            userSettings["LeftServo_ZERO"] = document.getElementById('LeftServo_ZERO').value;
            userSettings["RightUpperServo_ZERO"] = document.getElementById('RightUpperServo_ZERO').value;
            userSettings["LeftUpperServo_ZERO"] = document.getElementById('LeftUpperServo_ZERO').value;
            userSettings["PitchLeftServo_ZERO"] = document.getElementById('PitchLeftServo_ZERO').value;
            userSettings["PitchRightServo_ZERO"] = document.getElementById('PitchRightServo_ZERO').value;
            userSettings["ValveServo_ZERO"] = document.getElementById('ValveServo_ZERO').value;
            userSettings["TwistServo_ZERO"] = document.getElementById('TwistServo_ZERO').value;
            updateUserSettings();
        }
        else
        {
            showError("Zeros NOT saved due to invalid input.<br><div style='margin-left: 25px;'>The values should be between 1250 and 1750 for the following:<br><div style='color: white; margin-left: 25px;'>"+invalidValues.join("<br>")+"</div></div>");
        }
    }, 2000);
}
function updateLubeAmount()
{
    userSettings["lubeAmount"] = parseInt(document.getElementById('lubeAmount').value);
    updateUserSettings();
}
function toggleDisplaySettings(enabled) 
{
    if(!enabled) 
    {
        document.getElementById('deviceSettingsDisplayTable').hidden = true;
    }
    else
    {
        document.getElementById('deviceSettingsDisplayTable').hidden = false;
    }
}
function setDisplaySettings()
{
    userSettings["displayEnabled"] = document.getElementById('displayEnabled').checked;
    toggleDisplaySettings(userSettings["displayEnabled"]);
    userSettings["Display_Screen_Width"] = parseInt(document.getElementById('Display_Screen_Width').value);
    userSettings["Display_Screen_Height"] = parseInt(document.getElementById('Display_Screen_Height').value);

    userSettings["sleeveTempEnabled"] = document.getElementById('sleeveTempEnabled').checked;
    userSettings["tempControlEnabled"] = document.getElementById('tempControlEnabled').checked;
    userSettings["TargetTemp"] = parseInt(document.getElementById('TargetTemp').value);
    userSettings["HeatPWM"] = parseInt(document.getElementById('HeatPWM').value);
    userSettings["HoldPWM"] = parseInt(document.getElementById('HoldPWM').value);
    // userSettings["Display_Rst_PIN"] = parseInt(document.getElementById('Display_Rst_PIN').value);
    userSettings["Display_I2C_Address"] = document.getElementById('Display_I2C_Address').value;
    userSettings["WarmUpTime"] = parseInt(document.getElementById('WarmUpTime').value);
	
    showRestartRequired();
    updateUserSettings();
}

function connectWifi() {
    
  /*   var xhr = new XMLHttpRequest();
    xhr.open("POST", "/connectWifi", true);
    xhr.onreadystatechange = function() 
    {
        if (xhr.readyState === 4) {
            var response = JSON.parse(xhr.responseText);
            if (!response["connected"]) 
            {
                var x = document.getElementById("errorMessage");
                x.hidden = false;
                x.text = "Error connection to wifi access point";
            } 
            else 
            {
                infoNode.visibility = "visible";
                infoNode.innerText = "Wifi Connected! IP Address: " + response["IPAddress"] + " Keep this IP address and restart the device. After rebooting enter the IP address into your browsers address bar.");
                infoNode.style.color", 'green');
            }
        }
    }
    xhr.send(); */
}

function showWifiPassword() {
    var x = document.getElementById('wifiPass');
    if (x.type === "password") {
      x.type = "text";
    } else {
      x.type = "password";
    }
}

function updateWifiSettings() {
    userSettings["ssid"] = document.getElementById('ssid').value;
    userSettings["wifiPass"] = document.getElementById('wifiPass').value;
	var staticIP = document.getElementById('staticIP').checked;
	var localIP = document.getElementById('localIP').value;
	var gateway = document.getElementById('gateway').value;
	var subnet = document.getElementById('subnet').value;
	var dns1 = document.getElementById('dns1').value;
	var dns2 = document.getElementById('dns2').value;
    userSettings["staticIP"] = staticIP;
    toggleStaticIPSettings(staticIP);
    userSettings["localIP"] = localIP;
    userSettings["gateway"] = gateway;
    userSettings["subnet"] = subnet;
    userSettings["dns1"] = dns1;
    userSettings["dns2"] = dns2;
	showRestartRequired();
	updateUserSettings();
	
}
function togglePitchServoFrequency(isChecked) 
{
    if(isChecked) 
    {
        document.getElementById('pitchFrequencyRow').hidden = false;
    } 
    else
    {
        document.getElementById('pitchFrequencyRow').hidden = true;
    }
}
function toggleStaticIPSettings(enabled)
{
    if(!enabled) 
    {
        document.getElementById('localIPLabel').hidden = true;
        document.getElementById('gatewayLabel').hidden = true;
        document.getElementById('subnetLabel').hidden = true;
        document.getElementById('dns1Label').hidden = true;
        document.getElementById('dns2Label').hidden = true;
        document.getElementById('localIP').hidden = true;
        document.getElementById('gateway').hidden = true;
        document.getElementById('subnet').hidden = true;
        document.getElementById('dns1').hidden = true;
        document.getElementById('dns2').hidden = true;
    } 
    else
    {
        document.getElementById('localIPLabel').hidden = false;
        document.getElementById('gatewayLabel').hidden = false;
        document.getElementById('subnetLabel').hidden = false;
        document.getElementById('dns1Label').hidden = false;
        document.getElementById('dns2Label').hidden = false;
        document.getElementById('localIP').hidden = false;
        document.getElementById('gateway').hidden = false;
        document.getElementById('subnet').hidden = false;
        document.getElementById('dns1').hidden = false;
        document.getElementById('dns2').hidden = false;
    }
}
function toggleDeviceOptions(sr6Mode)
{
    var osrOnly = document.getElementsByClassName('osrOnly');
    var sr6Only = document.getElementsByClassName('sr6Only');
    for(var i=0;i < sr6Only.length; i++)
        sr6Only[i].style.display = sr6Mode ? "revert" : "none";
    for(var i=0;i < osrOnly.length; i++)
        osrOnly[i].style.display = sr6Mode ? "none" : "revert";
}

function toggleNonTCodev3Options(v3)
{
    var v2Only = document.getElementsByClassName('v2Only');
    var v3Only = document.getElementsByClassName('v3Only');
    for(var i=0;i < v3Only.length; i++)
        v3Only[i].style.display = v3 ? "revert" : "none";
    for(var i=0;i < v2Only.length; i++)
        v2Only[i].style.display = v3 ? "none" : "revert";
}

function updateBlueToothSettings()
{
    userSettings["bluetoothEnabled"] = document.getElementById('bluetoothEnabled').checked;
	showRestartRequired();
	updateUserSettings();
}

function setTCodeVersion() 
{
    userSettings["TCodeVersion"] = parseInt(document.getElementById('TCodeVersion').value);
    toggleNonTCodev3Options(userSettings["TCodeVersion"] == 1)
	showRestartRequired();
	updateUserSettings();
}

function showRestartRequired() {
    if (documentLoaded) {
        restartRequired = true;
    }
}

function updateLubeEnabled() {
    userSettings["lubeEnabled"] = document.getElementById('lubeEnabled').checked;
	updateUserSettings();
}