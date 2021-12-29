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
var debugEnabled = true;
var playSounds = false;

document.addEventListener("DOMContentLoaded", function() {
    onDocumentLoad();
  });

function logdebug(message) {
    if(debugEnabled)
        console.log(message);
}
function onDocumentLoad() {
	infoNode = document.getElementById('info');
    getUserSettings();
    initWebSocket();
}

function getUserSettings() {
	var xhr = new XMLHttpRequest();
	xhr.open('GET', "/userSettings", true);
	xhr.responseType = 'json';
	xhr.onload = function() {
        var status = xhr.status;
        if (status !== 200) {
			showError("Error loading user settings!");
		} else {
            userSettings = xhr.response;
            setUserSettings()
		}
	};
	xhr.send();
}

function initWebSocket() {
	try {
		var wsUri = "ws://" + window.location.host + "/ws";
		if (typeof MozWebSocket == 'function')
			WebSocket = MozWebSocket;
		if ( websocket && websocket.readyState == 1 )
			websocket.close();
		var websocket = new WebSocket( wsUri );
		websocket.onopen = function (evt) {
			//xtpConnected = true;
			logdebug("CONNECTED");
			//updateSettingsUI();
		};
		websocket.onclose = function (evt) {
			logdebug("DISCONNECTED");
			//xtpConnected = false;
		};
		websocket.onmessage = function (evt) {
			wsCallBackFunction(evt);
			logdebug("MESSAGE RECIEVED: "+ evt.data);
		};
		websocket.onerror = function (evt) {
			alert('ERROR: ' + evt.data + ", Address: "+wsUri);
			//xtpConnected = false;
		};
	} catch (exception) {
		alert('ERROR: ' + exception + ", Address: "+wsUri);
		//xtpConnected = false;
	}
}

function wsCallBackFunction(evt) {
	try {
		var data = JSON.parse(evt.data);
		switch(data["command"]) {
			case "tempStatus":
				var status = data["message"];
				var tempStatus = status["status"];
				var temp = status["temp"];
                document.getElementById("currentTempStatus").innerText = tempStatus;
                document.getElementById("currentTemp").innerText = temp;
				break;
            case "tempReached":
                if(playSounds) {
                    var snd = new Audio("data:audio/mp3;base64,//SUQzAwAAAAAfdlRJVDIAAAAeAAAAbWVzc2FnZSBhbGVydCAgMSBzb3VuZCBlZmZlY3RUUEUxAAAAFAAAAGZyZWVzb3VuZGVmZmVjdC5uZXRUQUxCAAAAFAAAAGZyZWVzb3VuZGVmZmVjdC5uZXRUWUVSAAAABQAAADIwMTZUQ09OAAAABQAAAFJvY2tDT01NAAAADwAAAGVuZwBleGNlbGxlbnQhVFJDSwAAAAYAAAAwNC8xNgAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA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");
                    snd.play();
                }
				break;
            case "failSafeTriggered":
                if(playSounds) {
                    var snd = new Audio("data:audio/wav;base64,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qJrx4KOQjahZxWbcZgztj2c49nKmkId44S71j0c8eV9yDK6uPRzx5X18eDvjvQ6yKo9ZSS6l//8elePK/Lf//IInrOF/FvDoADYAGBMGb7FtErm5MXMlmPAJQVgWta7Zx2go+8xJ0UiCb8LHHdftWyLJE0QIAIsI+UbXu67dZMjmgDGCGl1H+vpF4NSDckSIkk7Vd+sxEhBQMRU8j/12UIRhzSaUdQ+rQU5kGeFxm+hb1oh6pWWmv3uvmReDl0UnvtapVaIzo1jZbf/pD6ElLqSX+rUmOQNpJFa/r+sa4e/pBlAABoAAAAA3CUgShLdGIxsY7AUABPRrgCABdDuQ5GC7DqPQCgbbJUAoRSUj+NIEig0YfyWUho1VBBBA//uQZB4ABZx5zfMakeAAAAmwAAAAF5F3P0w9GtAAACfAAAAAwLhMDmAYWMgVEG1U0FIGCBgXBXAtfMH10000EEEEEECUBYln03TTTdNBDZopopYvrTTdNa325mImNg3TTPV9q3pmY0xoO6bv3r00y+IDGid/9aaaZTGMuj9mpu9Mpio1dXrr5HERTZSmqU36A3CumzN/9Robv/Xx4v9ijkSRSNLQhAWumap82WRSBUqXStV/YcS+XVLnSS+WLDroqArFkMEsAS+eWmrUzrO0oEmE40RlMZ5+ODIkAyKAGUwZ3mVKmcamcJnMW26MRPgUw6j+LkhyHGVGYjSUUKNpuJUQoOIAyDvEyG8S5yfK6dhZc0Tx1KI/gviKL6qvvFs1+bWtaz58uUNnryq6kt5RzOCkPWlVqVX2a/EEBUdU1KrXLf40GoiiFXK///qpoiDXrOgqDR38JB0bw7SoL+ZB9o1RCkQjQ2CBYZKd/+VJxZRRZlqSkKiws0WFxUyCwsKiMy7hUVFhIaCrNQsKkTIsLivwKKigsj8XYlwt/WKi2N4d//uQRCSAAjURNIHpMZBGYiaQPSYyAAABLAAAAAAAACWAAAAApUF/Mg+0aohSIRobBAsMlO//Kk4soosy1JSFRYWaLC4qZBYWFRGZdwqKiwkNBVmoWFSJkWFxX4FFRQWR+LsS4W/rFRb/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////VEFHAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAU291bmRib3kuZGUAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAMjAwNGh0dHA6Ly93d3cuc291bmRib3kuZGUAAAAAAAAAACU=");  
                    snd.play();
                }
                break;
				
		}
	}
	catch(e) {
		console.error(e.toString());
	}
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

function setUserSettings() 
{
    toggleNonTCodev3Options(userSettings["TCodeVersion"] == 1);
    toggleDeviceOptions(userSettings["sr6Mode"]);
    toggleStaticIPSettings(userSettings["staticIP"]);
    toggleDisplaySettings(userSettings["displayEnabled"]);
    toggleTempSettings(userSettings["tempControlEnabled"]);
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
	document.getElementById("sleeveTempDisplayed").checked = userSettings["sleeveTempDisplayed"];
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
	document.getElementById("heaterFailsafeTime").value = userSettings["heaterFailsafeTime"];
	document.getElementById("heaterThreshold").value = userSettings["heaterThreshold"];
	document.getElementById("heaterResolution").value = userSettings["heaterResolution"];
	document.getElementById("heaterFrequency").value = userSettings["heaterFrequency"];
    
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
    
    if(checked ) {
        document.getElementById("TwistFeedBack_PIN").value = 32;
        userSettings["TwistFeedBack_PIN"] = 32;
        //if(!newtoungeHatExists)
        alert("Note, twist feedback pin has been changed to analog input pin 32.\nPlease adjust your hardware accordingly.");
    } else {
        document.getElementById("TwistFeedBack_PIN").value = 26;
        userSettings["TwistFeedBack_PIN"] = 26;
        alert("Note, twist feedback pin reset to 26.\nPlease adjust your hardware accordingly.");
    }
    showRestartRequired();
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
                userSettings["LubeManual_PIN"] = lubeManual;
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
function toggleTempSettings(enabled) 
{
    if(!enabled) 
    {
        document.getElementById('tempSettingsDisplayTable').hidden = true;
    }
    else
    {
        document.getElementById('tempSettingsDisplayTable').hidden = false;
    }
}
function setDisplaySettings()
{
    userSettings["displayEnabled"] = document.getElementById('displayEnabled').checked;
    toggleDisplaySettings(userSettings["displayEnabled"]);
    userSettings["Display_Screen_Width"] = parseInt(document.getElementById('Display_Screen_Width').value);
    userSettings["Display_Screen_Height"] = parseInt(document.getElementById('Display_Screen_Height').value);

    // userSettings["Display_Rst_PIN"] = parseInt(document.getElementById('Display_Rst_PIN').value);
    userSettings["Display_I2C_Address"] = document.getElementById('Display_I2C_Address').value;
    userSettings["sleeveTempDisplayed"] = document.getElementById('sleeveTempDisplayed').checked;
	
    showRestartRequired();
    updateUserSettings();
}
function setTempSettings() {
    userSettings["tempControlEnabled"] = document.getElementById('tempControlEnabled').checked;
    toggleTempSettings(userSettings["tempControlEnabled"]);
    userSettings["TargetTemp"] = parseInt(document.getElementById('TargetTemp').value);
    userSettings["HeatPWM"] = parseInt(document.getElementById('HeatPWM').value);
    userSettings["HoldPWM"] = parseInt(document.getElementById('HoldPWM').value);
    userSettings["WarmUpTime"] = parseInt(document.getElementById('WarmUpTime').value);
    userSettings["heaterFailsafeTime"] = parseInt(document.getElementById('heaterFailsafeTime').value);
    userSettings["heaterThreshold"] = parseInt(document.getElementById('heaterThreshold').value);
    userSettings["heaterResolution"] = parseInt(document.getElementById('heaterResolution').value);
    userSettings["heaterFrequency"] = parseInt(document.getElementById('heaterFrequency').value);
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

function toggleSounds() {
	playSounds = document.getElementById('soundsEnabled').checked;
}