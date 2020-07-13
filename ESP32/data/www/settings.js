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
$(document).ready( 
    async function () 
    {
        await getUserSettings()
    }
);

async function getUserSettings() 
{
    let response = await fetch('/userSettings');
    userSettings = await response.json();

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
}

async function updateUserSettings() 
{
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
            if (xhr.readyState === 4) {
                var response = JSON.parse(xhr.responseText);
                if (response["msg"] !== "done") 
                {
                    $("#errorMessage").attr("hidden", false);
                    $("#errorMessage").text("Error saving: " + response["msg"]);
                } 
                else 
                {
                    $("#info").attr("hidden", false);
                    $("#info").text("Settings saved!");
                    $("#info").css("color", 'green');
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
    userSettings["speed"] = parseInt($('udpServerPort').text);
    $('#requiresRestart').show();
    updateUserSettings();
}

function updateHostName() 
{
    userSettings["hostname"] = $('hostname').text;
    $('#requiresRestart').show();
    updateUserSettings();
}

function updateFriendlyName() 
{
    userSettings["friendlyName"] = $('friendlyName').text;
    $('#requiresRestart').show();
    updateUserSettings();
}