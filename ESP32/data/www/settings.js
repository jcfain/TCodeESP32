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
$(document).ready( 
	onDocumentLoad()
);

async function onDocumentLoad()
{
	fetch('/userSettings')
	.then(function(response) {
		if (!response.ok) {
			$("#errorMessage").attr("hidden", false);
			$("#errorMessage").text("Error loading user settings!");
		} else {
			response.json().then(data => {
				userSettings = data;
                getUserSettings()
			});
		}
	});
}

function onDefaultClick() 
{		
	if (confirm("WARNING! Are you sure you wish to reset ALL settings?")) 
	{
		$("#info").attr("hidden", false);
		$("#info").text("Resetting...");
		$("#info").css("color", 'black');
		var xhr = new XMLHttpRequest();
		xhr.open("POST", "/default", true);
		xhr.onreadystatechange = function() 
		{
			if (xhr.readyState === 4) 
			{
				onDocumentLoad();
				$("#info").text("Settings reset!");
				$("#info").css("color", 'green');
				$('#requiresRestart').show();
				$('#resetBtn').prop("disabled", false );
				setTimeout(() => 
				{
					$("#info").attr("hidden", true);
					$("#info").text("");
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
    $("#version").html(userSettings["esp32Version"]);
    var xMin = userSettings["xMin"];
    var xMax = userSettings["xMax"];
    $("#xMin").val(xMin);
    calculateAndUpdateMinUI("x", xMin);
    $("#xMax").val(xMax);
    calculateAndUpdateMaxUI("x", xMax);
    updateRangePercentageLabel("x", tcodeToPercentage(xMin), tcodeToPercentage(xMax));

    var yRollMin = userSettings["yRollMin"];
    var yRollMax = userSettings["yRollMax"];
    $("#yRollMin").val(yRollMin);
    calculateAndUpdateMinUI("yRoll", yRollMin);
    $("#yRollMax").val(yRollMax);
    calculateAndUpdateMaxUI("yRoll", yRollMax);
    updateRangePercentageLabel("yRoll", tcodeToPercentage(yRollMin), tcodeToPercentage(yRollMax));

    var xRollMin = userSettings["xRollMin"];
    var xRollMax = userSettings["xRollMax"];
    $("#xRollMin").val(xRollMin);
    calculateAndUpdateMinUI("xRoll", xRollMin);
    $("#xRollMax").val(xRollMax);
    calculateAndUpdateMaxUI("xRoll", xRollMax);
    updateRangePercentageLabel("xRoll", tcodeToPercentage(xRollMin), tcodeToPercentage(xRollMax));

    updateSpeedUI(userSettings["speed"]);

    $("#udpServerPort").val(userSettings["udpServerPort"]);
    $("#hostname").val(userSettings["hostname"]);
    $("#friendlyName").val(userSettings["friendlyName"]);
	$("#servoFrequency").val(userSettings["servoFrequency"]);
	
    $("#continousTwist").val(userSettings["continousTwist"]);
    $("#TwistFeedBack_PIN").val(userSettings["TwistFeedBack_PIN"]);
    $("#RightServo_PIN").val(userSettings["RightServo_PIN"]);
    $("#LeftServo_PIN").val(userSettings["LeftServo_PIN"]);
    $("#RightUpperServo_PIN").val(userSettings["RightUpperServo_PIN"]);
    $("#LeftUpperServo_PIN").val(userSettings["LeftUpperServo_PIN"]);
    $("#PitchLeftServo_PIN").val(userSettings["PitchLeftServo_PIN"]);
    $("#PitchRightServo_PIN").val(userSettings["PitchRightServo_PIN"]);
    $("#ValveServo_PIN").val(userSettings["ValveServo_PIN"]);
	$("#TwistServo_PIN").val(userSettings["TwistServo_PIN"]);
	
    $("#RightServo_ZERO").val(userSettings["RightServo_ZERO"]);
    $("#LeftServo_ZERO").val(userSettings["LeftServo_ZERO"]);
    $("#RightUpperServo_ZERO").val(userSettings["RightUpperServo_ZERO"]);
    $("#LeftUpperServo_ZERO").val(userSettings["LeftUpperServo_ZERO"]);
    $("#PitchLeftServo_ZERO").val(userSettings["PitchLeftServo_ZERO"]);
    $("#PitchRightServo_ZERO").val(userSettings["PitchRightServo_ZERO"]);
    $("#ValveServo_ZERO").val(userSettings["ValveServo_ZERO"]);
	$("#TwistServo_ZERO").val(userSettings["TwistServo_ZERO"]);
	
    $("#Vibe0_PIN").val(userSettings["Vibe0_PIN"]);
    $("#Vibe1_PIN").val(userSettings["Vibe1_PIN"]);
	$("#Lube_Pin").val(userSettings["Lube_Pin"]);
	$("#lubeAmount").val(userSettings["lubeAmount"]);
	$("#sr6Mode").prop('checked', userSettings["sr6Mode"]);
	$("#autoValve").prop('checked', userSettings["autoValve"]);
	$("#inverseValve").prop('checked', userSettings["inverseValve"]);
	$("#valveServo90Degrees").prop('checked', userSettings["valveServo90Degrees"]);
	$("#inverseStroke").prop('checked', userSettings["inverseStroke"]);
	$("#inversePitch").prop('checked', userSettings["inversePitch"]);

	$("#displayEnabled").prop('checked', userSettings["displayEnabled"]);
	$("#sleeveTempEnabled").prop('checked', userSettings["sleeveTempEnabled"]);
	$("#tempControlEnabled").prop('checked', userSettings["tempControlEnabled"]);
	$("#Display_Screen_Width").val(userSettings["Display_Screen_Width"]);
	$("#Display_Screen_Height").val(userSettings["Display_Screen_Height"]);
	$("#TargetTemp").val(userSettings["TargetTemp"]);
	$("#HeatPWM").val(userSettings["HeatPWM"]);
	$("#HoldPWM").val(userSettings["HoldPWM"]);
	$("#Display_I2C_Address").val(userSettings["Display_I2C_Address"]);
	$("#Display_Rst_PIN").val(userSettings["Display_Rst_PIN"]);
	$("#Temp_PIN").val(userSettings["Temp_PIN"]);
	$("#Heater_PIN").val(userSettings["Heater_PIN"]);
	$("#WarmUpTime").val(userSettings["WarmUpTime"]);
    $('#TCodeVersion').val(userSettings["TCodeVersion"]).prop('selected', true);
	
    $("#ssid").val(userSettings["ssid"]);
    $("#wifiPass").val(userSettings["wifiPass"]);
    $("#staticIP").prop('checked', userSettings["staticIP"]);
    $("#localIP").val(userSettings["localIP"]);
    $("#gateway").val(userSettings["gateway"]);
    $("#subnet").val(userSettings["subnet"]);
    $("#dns1").val(userSettings["dns1"]);
    $("#dns2").val(userSettings["dns2"]);

    documentLoaded = true;
}

async function updateUserSettings() 
{
    if (documentLoaded) {
        if(upDateTimeout !== null) 
        {
            clearTimeout(upDateTimeout);
        }
        upDateTimeout = setTimeout(() => 
        {
            $("#errorMessage").attr("hidden", true);
            $("#errorMessage").text("");
            $("#info").attr("hidden", false);
            $("#info").text("Saving...");
            $("#info").css("color", 'black');
            var xhr = new XMLHttpRequest();
            xhr.open("POST", "/settings", true);
            xhr.onreadystatechange = function() 
            {
                if (xhr.readyState === 4) 
				{
                    var response = JSON.parse(xhr.responseText);
                    if (response["msg"] !== "done") 
                    {
                        showError("Error saving: " + response["msg"]);
                    } 
                    else 
                    {
                        $("#info").attr("hidden", false);
                        $("#info").text("Settings saved!");
                        $("#info").css("color", 'green');
                        if (restartRequired) 
                        {
                            $('#requiresRestart').show();
                            $('#resetBtn').prop("disabled", false );
                        }
                        setTimeout(() => 
                        {
                            $("#info").attr("hidden", true);
                            $("#info").text("");
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
    $("#errorText").html("");
    $("#errorMessage").attr("hidden", true);
}
function showError(message) 
{
    $("#errorText").html(message);
    $("#errorMessage").attr("hidden", false);
}
function onMinInput(axis) 
{
    var axisName = axis + "Min";
    var inputAxisMin = document.getElementById(axisName);
    var inputAxisMax = document.getElementById(axis + "Max"); 
    inputAxisMin.value = Math.min(inputAxisMin.value, inputAxisMax.value - 2);
    
    var value=(100/(parseInt(inputAxisMin.max)-parseInt(inputAxisMin.min)))*parseInt(inputAxisMin.value)-(100/(parseInt(inputAxisMin.max)-parseInt(inputAxisMin.min)))*parseInt(inputAxisMin.min);
    updateMinUI(axis, value);

    updateRangePercentageLabel(axis, inputAxisMin.value, $("#"+axis+"Max").val());

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

    updateRangePercentageLabel(axis, $("#"+axis+"Min").val(), inputAxisMax.value);

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
    $("#" + axis + "Min").val(value);
    $("#" + axis + "InverseMin").css("width", value + "%");
    $("#" + axis + "Range").css("left", value + "%");
    $("#" + axis + "ThumbMin").css("left", value + "%");
    $("#" + axis + "SignMin").css("left", value + "%");
    $("#" + axis + "ValueMin").text(value + '%');
}

function updateMaxUI(axis, value) 
{
    $("#" + axis + "Max").val(value);
    $("#" + axis + "InverseMax").css("width", (100-value) + "%");
    $("#" + axis + "Range").css("right", (100-value) + "%");
    $("#" + axis + "ThumbMax").css("left", value + "%");
    $("#" + axis + "SignMax").css("left", value + "%");
    $("#" + axis + "ValueMax").text(value + '%');
}

function updateSpeedUI(millisec) 
{
    var value = speedToPercentage(millisec);
    $("#speedSign").css("left", value + "%");
    $("#speedThumb").css("left", value + "%");
    $("#speedValue").css("left", value + "%");
    //$("#speedInverseMin").css("width", (value) + "%");
    $("#speedInverseMax").css("width", (100-value) + "%");
    $("#speedRange").css("right", (100-value) + "%");
    //$("#speedRange").css("left", value + "%");
    if (millisec > 999) 
    {
        $("#speedValue").text(millisec);
        $("#speedLabel").text(millisec + "ms");
    } 
    else 
    {
        $("#speedValue").text("off");
        $("#speedLabel").text("off");
    }
}

function updateRangePercentageLabel(axis, minValue, maxValue) 
{
    $("#" + axis + "RangeLabel").text(maxValue - minValue + "%");
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
    var speedInMillisecs = parseInt($("#speedInput").val())
    userSettings["speed"] = speedInMillisecs > 999 ? speedInMillisecs : 0;
    updateSpeedUI(speedInMillisecs);
    updateUserSettings();
}

function updateUdpPort() 
{
    userSettings["udpServerPort"] = parseInt($('#udpServerPort').val());
    showRestartRequired();
    updateUserSettings();
}

function updateServoFrequency() 
{
    userSettings["servoFrequency"] = parseInt($('#servoFrequency').val());
    showRestartRequired();
    updateUserSettings();
}

function updateContinuousTwist()
{
	var checked = $('#continuousTwist').prop('checked');
	if (checked) 
	{
		if (confirm("WARNING! If you enable continuous twist\nMAKE SURE THERE ARE NO WIRES CONNECTED TO YOUR FLESHLIGHT CASE!\nThis can twist the wires and possible injury can occur.\n CONFIRM THERE ARE NO WIRES CONNECTED?")) 
		{
			userSettings["continousTwist"] = checked;
			updateUserSettings();
		} 
		else 
		{
			$('#continuousTwist').prop('checked', false);
		} 
	}
	else
	{
		userSettings["continousTwist"] = false;
		updateUserSettings();
	}
}

function updateHostName() 
{
    userSettings["hostname"] = $('#hostname').val();
    showRestartRequired();
    updateUserSettings();
}

function updateFriendlyName() 
{
    userSettings["friendlyName"] = $('#friendlyName').val();
    showRestartRequired();
    updateUserSettings();
}

function setSR6Mode() {
    userSettings["sr6Mode"] = $('#sr6Mode').prop('checked');
    toggleDeviceOptions(userSettings["sr6Mode"]);
    showRestartRequired();
	updateUserSettings();
}

function setAutoValve() {
    userSettings["autoValve"] = $('#autoValve').prop('checked');
	updateUserSettings();
}
function setInverseValve() {
    userSettings["inverseValve"] = $('#inverseValve').prop('checked');
	updateUserSettings();
}
function setValveServo90Degrees() {
	var checked = $('#valveServo90Degrees').prop('checked');
	if (checked) 
	{
		if (confirm("WARNING! If you 90 degree servo\nMAKE SURE YOU ARE NOT USING THE T-Valve LID!\nThe servo will stall hitting the wall and burn out!")) 
		{
			userSettings["valveServo90Degrees"] = checked;
			updateUserSettings();
		} 
		else 
		{
			$('#valveServo90Degrees').prop('checked', false);
		} 
	}
	else
	{
		userSettings["valveServo90Degrees"] = false;
		updateUserSettings();
	}
}
function setInverseStroke() {
    userSettings["inverseStroke"] = $('#inverseStroke').prop('checked');
	updateUserSettings();
}
function setInversePitch() {
    userSettings["inversePitch"] = $('#inversePitch').prop('checked');
	updateUserSettings();
}

function updatePins() 
{
    userSettings["TwistFeedBack_PIN"] = $('#TwistFeedBack_PIN').val();
    userSettings["RightServo_PIN"] = $('#RightServo_PIN').val();
    userSettings["LeftServo_PIN"] = $('#LeftServo_PIN').val();
    userSettings["RightUpperServo_PIN"] = $('#RightUpperServo_PIN').val();
    userSettings["LeftUpperServo_PIN"] = $('#LeftUpperServo_PIN').val();
    userSettings["PitchLeftServo_PIN"] = $('#PitchLeftServo_PIN').val();
    userSettings["PitchRightServo_PIN"] = $('#PitchRightServo_PIN').val();
    userSettings["ValveServo_PIN"] = $('#ValveServo_PIN').val();
    userSettings["TwistServo_PIN"] = $('#TwistServo_PIN').val();
    userSettings["Vibe0_PIN"] = $('#Vibe0_PIN').val();
	userSettings["Vibe1_PIN"] = $('#Vibe1_PIN').val();
	userSettings["Lube_Pin"] = $('#Lube_Pin').val();
    showRestartRequired();
    updateUserSettings();
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
        var RightServo_ZERO = parseInt($('#RightServo_ZERO').val());
        if(!RightServo_ZERO || RightServo_ZERO > 1750 || RightServo_ZERO < 1250)
        {
            validValue = false;
            invalidValues.push("Right servo ZERO")
        }
        var LeftServo_ZERO = parseInt($('#LeftServo_ZERO').val());
        if(!LeftServo_ZERO || LeftServo_ZERO > 1750 || LeftServo_ZERO < 1250)
        {
            validValue = false;
            invalidValues.push("Left servo ZERO")
        }
        var RightUpperServo_ZERO = parseInt($('#RightUpperServo_ZERO').val());
        if(!RightUpperServo_ZERO || RightUpperServo_ZERO > 1750 || RightUpperServo_ZERO < 1250)
        {
            validValue = false;
            invalidValues.push("Right upper servo ZERO")
        }
        var LeftUpperServo_ZERO = parseInt($('#LeftUpperServo_ZERO').val());
        if(!LeftUpperServo_ZERO || LeftUpperServo_ZERO > 1750 || LeftUpperServo_ZERO < 1250)
        {
            validValue = false;
            invalidValues.push("Left upper servo ZERO")
        }
        var PitchLeftServo_ZERO = parseInt($('#PitchLeftServo_ZERO').val());
        if(!PitchLeftServo_ZERO || PitchLeftServo_ZERO > 1750 || PitchLeftServo_ZERO < 1250)
        {
            validValue = false;
            invalidValues.push("Pitch left servo ZERO")
        }
        var PitchRightServo_ZERO = parseInt($('#PitchRightServo_ZERO').val());
        if(!PitchRightServo_ZERO || PitchRightServo_ZERO > 1750 || PitchRightServo_ZERO < 1250)
        {
            validValue = false;
            invalidValues.push("Pitch right servo ZERO")
        }
        var ValveServo_ZERO = parseInt($('#ValveServo_ZERO').val());
        if(!ValveServo_ZERO || ValveServo_ZERO > 1750 || ValveServo_ZERO < 1250)
        {
            validValue = false;
            invalidValues.push("Valve servo ZERO")
        }
        var TwistServo_ZERO = parseInt($('#TwistServo_ZERO').val());
        if(!TwistServo_ZERO || TwistServo_ZERO > 1750 || TwistServo_ZERO < 1250)
        {
            validValue = false;
            invalidValues.push("Twist servo ZERO")
        }

        if(validValue)
        {
            closeError();
            userSettings["RightServo_ZERO"] = $('#RightServo_ZERO').val();
            userSettings["LeftServo_ZERO"] = $('#LeftServo_ZERO').val();
            userSettings["RightUpperServo_ZERO"] = $('#RightUpperServo_ZERO').val();
            userSettings["LeftUpperServo_ZERO"] = $('#LeftUpperServo_ZERO').val();
            userSettings["PitchLeftServo_ZERO"] = $('#PitchLeftServo_ZERO').val();
            userSettings["PitchRightServo_ZERO"] = $('#PitchRightServo_ZERO').val();
            userSettings["ValveServo_ZERO"] = $('#ValveServo_ZERO').val();
            userSettings["TwistServo_ZERO"] = $('#TwistServo_ZERO').val();
            updateUserSettings();
        }
        else
        {
            showError("Settings NOT saved due to invalid servo ZERO input. The values should be between 1250 and 1750 for the following:<br>"+invalidValues.join("<br>"));
        }
    }, 2000);
}
function updateLubeAmount()
{
    userSettings["lubeAmount"] = parseInt($('#lubeAmount').val());
    updateUserSettings();
}
function toggleDisplaySettings(enabled) 
{
    if(!enabled) 
    {
        $('#deviceSettingsDisplayTable').hide();
    }
    else
    {
        $('#deviceSettingsDisplayTable').show();
    }
}
function setDisplaySettings()
{
    userSettings["displayEnabled"] = $('#displayEnabled').prop('checked');
    toggleDisplaySettings(userSettings["displayEnabled"]);
    userSettings["sleeveTempEnabled"] = $('#sleeveTempEnabled').prop('checked');
    userSettings["tempControlEnabled"] = $('#tempControlEnabled').prop('checked');
    userSettings["Temp_PIN"] = parseInt($('#Temp_PIN').val());
    userSettings["Heater_PIN"] = parseInt($('#Heater_PIN').val());
    userSettings["Display_Screen_Width"] = parseInt($('#Display_Screen_Width').val());
    userSettings["Display_Screen_Height"] = parseInt($('#Display_Screen_Height').val());
    userSettings["TargetTemp"] = parseInt($('#TargetTemp').val());
    userSettings["HeatPWM"] = parseInt($('#HeatPWM').val());
    userSettings["HoldPWM"] = parseInt($('#HoldPWM').val());
    userSettings["Display_Rst_PIN"] = parseInt($('#Display_Rst_PIN').val());
    userSettings["Display_I2C_Address"] = $('#Display_I2C_Address').val();
    userSettings["WarmUpTime"] = parseInt($('#WarmUpTime').val());
	
    showRestartRequired();
    updateUserSettings();
}

function connectWifi() {
    
    var xhr = new XMLHttpRequest();
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
                $("#info").attr("hidden", false);
                $("#info").text("Wifi Connected! IP Address: " + response["IPAddress"] + " Keep this IP address and restart the device. After rebooting enter the IP address into your browsers address bar.");
                $("#info").css("color", 'green');
            }
        }
    }
    xhr.send();
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
    userSettings["ssid"] = $('#ssid').val();
    userSettings["wifiPass"] = $('#wifiPass').val();
	var staticIP = $('#staticIP').prop('checked');
	var localIP = $('#localIP').val();
	var gateway = $('#gateway').val();
	var subnet = $('#subnet').val();
	var dns1 = $('#dns1').val();
	var dns2 = $('#dns2').val();
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
function toggleStaticIPSettings(enabled)
{
    if(!enabled) 
    {
        $('#localIPLabel').hide();
        $('#gatewayLabel').hide();
        $('#subnetLabel').hide();
        $('#dns1Label').hide();
        $('#dns2Label').hide();
        $('#localIP').hide();
        $('#gateway').hide();
        $('#subnet').hide();
        $('#dns1').hide();
        $('#dns2').hide();
    } 
    else
    {
        $('#localIPLabel').show();
        $('#gatewayLabel').show();
        $('#subnetLabel').show();
        $('#dns1Label').show();
        $('#dns2Label').show();
        $('#localIP').show();
        $('#gateway').show();
        $('#subnet').show();
        $('#dns1').show();
        $('#dns2').show();
    }
}
function toggleDeviceOptions(sr6Mode)
{
    if(sr6Mode) 
    {
        $('.osrOnly').hide();
        $('.sr6Only').show();
    } 
    else
    {
        $('.osrOnly').show();
        $('.sr6Only').hide();
    }
}

function toggleNonTCodev3Options(v3)
{
    if(v3) 
    {
        $('.v2Only').hide();
    } 
    else
    {
        $('.v2Only').show();
    }
}

function updateBlueToothSettings()
{
    userSettings["bluetoothEnabled"] = $('#bluetoothEnabled').prop('checked');
	showRestartRequired();
	updateUserSettings();
}

function setTCodeVersion() 
{
    userSettings["TCodeVersion"] = parseInt($('#TCodeVersion').val());
    toggleNonTCodev3Options(userSettings["TCodeVersion"] == 1)
	showRestartRequired();
	updateUserSettings();
}

function showRestartRequired() {
    if (documentLoaded) {
        restartRequired = true;
    }
}