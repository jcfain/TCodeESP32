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

var userSettings = {};
var wifiSettings = {};
var pinoutSettings = {};
var systemInfo = {};
var motionProviderSettings = {};
var buttonSettings = {};
var channelsProfileSettings = {};
var debounceTimeouts = {};
var upDateTimeout;
var upDatePinsTimeout;
var restartRequired = false;
var documentLoaded = false;
var debugEnabled = true;
var playSounds = false;
var importSettingsInputElement;
var websocket;
const EndPointType = {
    System: { uri: "/systemInfo"},
    Common: {uri: "/settings"},
    Pins: {uri: "/pins"},
    Wifi: {uri: "/wifiSettings"},
    MotionProfile: {uri: "/motionProfiles"},
    Buttons: {uri: "/buttonSettings"},
    ChannelProfiles: {uri: "/channelsProfile"},
}
const TCodeVersion = {
    V3: 0,
    V4: 1
}
const latestTCodeVersion = TCodeVersion.V3;
const MotorType = {
    Servo: 0,
    BLDC: 1
};
const ModuleType = {
    WROOM32: 0,
    S3: 1
};
const BuildFeature = {
    NONE: 0,
    DEBUG: 1,
    WIFI: 2,
    BLUETOOTH: 3,
    BLE: 4,
    DA: 5,
    DISPLAY_: 6,
    TEMP: 7,
    HTTPS: 8,
    COEXIST: 9,
    MAX_FEATURE: 10
};

let BoardType = {
    DEVKIT: 0,
    ZERO: 1,
    N8R8: 2,
    CRIMZZON: 3,
    ISAAC: 4,
    SSR1PCB: 5
};
let DeviceType = {
    OSR: 0,
    SR6: 1,
    SSR1: 2
};
let BLEDeviceType = {
    TCODE: 0,
    LOVE: 1,
    HC: 2
};
let BLELoveDeviceType = {
    EDGE: 0
};
let BLDCEncoderType = {
    MT6701: 0,
    SPI: 1,
    PWM: 2
};
const TCodeModifierType = {
    INTERVAL: "I",
    SPEED: "S"
}
dubugMessages = [];
var tcodeVersions = [];
var testDeviceUseIModifier = false;
var testDeviceDisableModifier = false;
var testDeviceModifierValue = "1000";
var restartClicked = false;
var serverPollingTimeOut = null;
var staticIPAddressTimeout = null;
var hostnameTimeout = null;
var serverPollRetryCount = 0;
var channelSliderList = [];
var restartingAndChangingAddress = false;
var resettingAllToDefault = false;
var startUpHostName;
var startUpWebPort;
var startUpStaticIP;
var startUpLocalIP;
const defaultDebounce = 3000;

//PWM availible on: 2,4,5,12-19,21-23,25-27,32-33
let validPWMpins = [2,4,5,12,13,14,15,16,17,18,19,21,22,23,25,26,27,32,33];
let inputOnlypins = [34,35,36,39];
let adc1Pins = [36,37,38,39,32,33,34,35];
let adc2Pins = [4,0,2,15,13,12,14,27,25,26];

document.addEventListener("DOMContentLoaded", function() {
    onDocumentLoad();
    document.getElementById("page-body").style.visibility = "visible";
    hideLoading();
});

function logdebug(message) {
    if(debugEnabled)
        console.log(message);
}
function get(name, uri, callback, callbackFail) {
	var xhr = new XMLHttpRequest();
	xhr.open('GET', uri, true);
	xhr.responseType = 'json';
	xhr.onload = function() {
        var status = xhr.status;
        if (status !== 200) {
			showError("Error loading "+name+"!");
            if(callbackFail) 
                callbackFail(xhr);
		} else {
            if(callback)
                callback(xhr);
		}
	};
    if(callbackFail) {
        xhr.onerror = function() {
            callbackFail(xhr);
        }
    }
	xhr.send();
}
function onDocumentLoad() {
    getSystemInfo(true);
    createImportSettingsInputElement();
    
    // debugTextElement = document.getElementById("debugText");
    // debugTextElement.scrollTop = debugTextElement.scrollHeight;
}

function getSystemInfo(chain) {
    let polling = false;
    if(serverPollingTimeOut) {
        polling = true;
        clearTimeout(serverPollingTimeOut);
        serverPollingTimeOut = null;
    }
    showLoading("Loading system info...");
    get("system info", EndPointType.System.uri, function(xhr) {
        systemInfo = xhr.response;
        if(!systemInfo) {
            if(!polling)
                showError("Error getting system info!");
            startServerPoll();
            return;
        } else if(systemInfo.status === "restarting") {
            startServerPoll();
            return;
        } else
            setSystemInfo();
        if(chain)
            getPinSettings(chain);
        else if(!polling)
            hideLoading();
        serverPollRetryCount = 0;
    }, function(xhr) {
        if(!polling)
            showError("Error getting system info!");
        startServerPoll();
    });
}
function getPinSettings(chain) {
    showLoading("Loading pinout...");
    get("Pinout settings", EndPointType.Pins.uri, function(xhr) {
        pinoutSettings = xhr.response;
        if(!pinoutSettings) {
            showError("Error getting pinout!");
        }
        else
            setPinoutSettings();
        if(chain)
            getWifiSettings(chain);
        else
            hideLoading();
    });
}

function getWifiSettings(chain) {
    showLoading("Loading network settings...");
    get("wifi settings", EndPointType.Wifi.uri, function(xhr) {
        wifiSettings = xhr.response;
        if(!wifiSettings || !wifiSettings["ssid"]) {
            showError("Error getting wifi settings!");
        }
        else {
            startUpStaticIP = wifiSettings["staticIP"];
            startUpLocalIP = wifiSettings["localIP"];
            startUpWebPort = wifiSettings["webServerPort"];
            startUpHostName = wifiSettings["hostname"];
            setWifiSettings();
        }
        if(chain)
            getMotionProviderSettings(chain);
        else
            hideLoading();
    });
}

function getMotionProviderSettings(chain) {
    showLoading("Loading motion generator settings...");
    get("motion settings", EndPointType.MotionProfile.uri, function(xhr) {
        motionProviderSettings = xhr.response;
        if(!motionProviderSettings || !motionProviderSettings["motionProfiles"]) {
            showError("Error getting motion provider settings!");
        }
        if(chain)
            getButtonSettings(chain);
        else
            hideLoading();
    });
}

function getButtonSettings(chain) {
    showLoading("Loading button settings...");
    get("button settings", EndPointType.Buttons.uri, function(xhr) {
        buttonSettings = xhr.response;
        if(!buttonSettings || !buttonSettings["bootButtonCommand"]) {
            showError("Error getting button settings!");
        }
        if(chain)
            getChannelsProfileSettings(chain);
        else
            hideLoading();
    });
}

function getChannelsProfileSettings(chain) {
    showLoading("Loading channel settings...");
    get("channel settings", EndPointType.ChannelProfiles.uri, function(xhr) {
        channelsProfileSettings = xhr.response;
        if(!channelsProfileSettings) {
            showError("Error getting channels profile settings!");
        }
        if(chain)
            getUserSettings();
        else
            hideLoading();
    });
}

function postCommonSettings(debounce, callback) {
    updateUserSettings(debounce, EndPointType.Common.uri, userSettings, callback);
}
function postAndValidatePinoutSettings(debounce, callback) {
    if(!debounce || debounce < 0)
        debounce = 0;
    if(upDatePinsTimeout) 
    {
        clearTimeout(upDatePinsTimeout);
    }
    upDatePinsTimeout = setTimeout(() => 
    {
        if(validatePins()) {
            updateUserSettings(0, EndPointType.Pins.uri, pinoutSettings, callback);
        } else if(callback)
            callback();
    }, debounce);
}
function postPinoutSettings(debounce, callback) {
    updateUserSettings(debounce, EndPointType.Pins.uri, pinoutSettings, callback);
}
function postWifiSettings(debounce, callback) {
    updateUserSettings(debounce, EndPointType.Wifi.uri, wifiSettings, callback);
}
function postButtonSettings(debounce, callback) {
    updateUserSettings(debounce, EndPointType.Buttons.uri, buttonSettings, callback);
}
function postMotionProfileSettings(debounce, callback) {
    updateUserSettings(debounce, EndPointType.MotionProfile.uri, motionProviderSettings, callback);
}
function postChannelsProfileSettings(debounce, callback) {
    updateUserSettings(debounce, EndPointType.ChannelProfiles.uri, channelsProfileSettings, callback);
}


function updateALLUserSettings() {
    postCommonSettings(0, updatePinoutChain);
}
var updatePinoutChain = function() {
    postAndValidatePinoutSettings(0, updateWifiSettingsChain);
}
var updateWifiSettingsChain = function() {
    postWifiSettings(0, updateButtonSettingsChain);
}
var updateButtonSettingsChain = function() {
    postButtonSettings(0, updateMotionProfileSettingsChain);
}
var updateMotionProfileSettingsChain = function() {
    postMotionProfileSettings(0, updateChannelsProfileSettingsChain);
}
var updateChannelsProfileSettingsChain = function() {
    postChannelsProfileSettings(0);
}

// ALWAYS CALL setUserSettings/getUserSettings LAST! 
// This is so it can set all the values from the various sources and other
// methods can call a single method instead of all of them.
function getUserSettings() {
    showLoading("Loading common settings...");
    get("common settings", EndPointType.Common.uri, function(xhr) {
        userSettings = xhr.response;
        if(!userSettings || userSettings["TCodeVersion"] == undefined) {
            showError("Error getting user settings!");
        } 
        else
            setUserSettings();
        initWebSocket();
    });
}

function initWebSocket() {
	try {
		var wsUri = (hasFeature(BuildFeature.HTTPS) ? "wss://" : "ws://") + window.location.host + "/ws";
		if (typeof MozWebSocket == 'function')
			WebSocket = MozWebSocket;
		if ( websocket )
			websocket.close();
		websocket = new WebSocket( wsUri );
		websocket.onopen = function (evt) {
			//xtpConnected = true;
			logdebug("CONNECTED");
            if(restartClicked) {
                getUserSettings();
                restartClicked = false;
            }
            hideLoading();
            if(serverPollingTimeOut) {
                clearTimeout(serverPollingTimeOut);
                serverPollingTimeOut = null;
            }
			//updateSettingsUI();
		};
		websocket.onclose = function (evt) {
			logdebug("DISCONNECTED");
            
            if(!serverPollingTimeOut && !restartingAndChangingAddress) {
                let message = "Server disconnected, waiting for restart...";
                if(systemInfo.apMode) {
                    message += "\n(Hint: Make sure you are connected to the AP in wifi networks."
                }
                showLoading(message);
                startServerPoll();
            }
            //alert('Web socket disconnected: To use some features you need to make sure the device is on and connected and refresh the page.');
			//xtpConnected = false;
		};
		websocket.onmessage = function (evt) {
			wsCallBackFunction(evt);
			logdebug("MESSAGE RECIEVED: "+ evt.data);
		};
		websocket.onerror = function (evt) {
            if(!serverPollingTimeOut && !restartingAndChangingAddress) {
                showLoading("Server error, waiting for restart...");
                startServerPoll();
            }
			//alert('ERROR: ' + evt.data + ", Address: "+wsUri);
			//xtpConnected = false;
		};
	} catch (exception) {
        if(!serverPollingTimeOut && !restartingAndChangingAddress) {
            showLoading("Server exception, waiting for restart...");
            startServerPoll();
        }
        //alert('ERROR: ' + exception + ", Address: "+wsUri);
		//xtpConnected = false;
	}
}

function wsCallBackFunction(evt) {
	try {
		var data = JSON.parse(evt.data);
		switch(data["command"]) {
			case "sleeveTempStatus":
				var status = data["message"];
				var tempStatus = status["status"];
				var temp = status["temp"];
                document.getElementById("currentTempStatus").value = tempStatus;
                document.getElementById("currentTemp").value = temp;
				break;
            case "internalTempStatus":
				var status = data["message"];
				var tempStatus = status["status"];
				var temp = status["temp"];
                document.getElementById("internalTempStatus").value = tempStatus;
                document.getElementById("internalTemp").value = temp;
                break;
            case "tempReached":
                playSuccess();
				break;
            // case "failSafeTriggered":
            //     playFail();
            //     break;
            case "batteryStatus":
                wsBatteryStatus(data);
                break;
            case "debug":
				var message = data["message"];
                debug(message);
                break;
            case "channelRangesEnabled":
				var enabled = data["message"];
                DeviceRangeSlider.updateChannelRangesTemp(enabled == "true");
                break;

		}
	}
	catch(e) {
		console.error(e.toString());
	}
}

function debug(message) {
    if(debugTextElement) {
        if(dubugMessages.length > 1000)
            dubugMessages.shift();
        dubugMessages.push(message);
        debugTextElement.value = dubugMessages.join("\n");
        debugTextElement.scrollTop = debugTextElement.scrollHeight;
    }
}

function debounceInput(name, callBack, debounceInMs)
{
    if(debounceTimeouts[name] !== null) 
    {
        clearTimeout(debounceTimeouts[name]);
    }
    debounceTimeouts[name] = setTimeout(() => {
        debounceTimeouts[name] = null;
        callBack();
    }, debounceInMs == null || debounceInMs == undefined ? defaultDebounce : debounceInMs);

}

function setLogLevelUI() {
    const clearTagsButton = document.getElementById('clearTagsButton');
    const clearFiltersButton = document.getElementById('clearFiltersButton');
    const selectedIncludes = userSettings["log-include-tags"];
    const selectedExcludes = userSettings["log-exclude-tags"];
    clearTagsButton.disabled = !selectedIncludes?.length;
    clearFiltersButton.disabled = !selectedExcludes?.length;

    clearTagsButton.innerText = "Clear included" + (selectedIncludes.length ? ": "+ selectedIncludes.length : "");
    clearFiltersButton.innerText = "Clear excluded" + (selectedExcludes.length ? ": "+ selectedExcludes.length : "");
}
function setLogLevel() {
    userSettings["logLevel"] = parseInt(document.getElementById('logLevel').value);
    const selectedIncludes = document.querySelectorAll('#log-include-tags option:checked');
    userSettings["log-include-tags"] =  Array.from(selectedIncludes).map(el => el.value);

    const selectedExcludes = document.querySelectorAll('#log-exclude-tags option:checked');
    userSettings["log-exclude-tags"] = Array.from(selectedExcludes).map(el => el.value);

    setLogLevelUI();
    //document.getElementById("log-exclude-tags").disabled = selectedIncludes.length > 0;
    // if(userSettings["logLevel"] == LogLevel.VERBOSE)
    //     alert("There are not enough resources to send VERBOSE messages to the site.\nUse serial to view them.")
	updateUserSettings();
}
function clearTags(name) {
    userSettings[name] = [];
    var element = document.getElementById(name);
    for (var i = 0; i < element.options.length; i++) {
        element.options[i].selected = false;
    }
    setLogLevelUI();
	updateUserSettings();
}
function clearLog() {
    if(debugTextElement) {
        dubugMessages = [];
        debugTextElement.value = "";
    }
}

//https://base64.guru/converter/encode/audio
//http://freesoundeffect.net/tags/alert?page=40
function playSuccess() {
    if(playSounds) {
        var snd = new Audio("data:audio/mp3;base64,SUQzAwAAAAAfdlRJVDIAAAA+AAAAZWFyY29uIGV2ZW50IG5vdGlmaWNhdGlvbiBhbGVydCBtdXNpY2FsICAodmVyIDMpIHNvdW5kIGVmZmVjdFRQRTEAAAARAAAAc291bmRlZmZlY3QuaW5mb1RBTEIAAAARAAAAc291bmRlZmZlY3QuaW5mb1RZRVIAAAAFAAAAMjAxNlRDT04AAAAFAAAAUm9ja0NPTU0AAAAPAAAAZW5nAGV4Y2VsbGVudCFUUkNLAAAABgAAADA0LzE2AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA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nbI6LOGdZdS5FqNy3twsesxGOco+c6rmlO+piSUEEAxBQ0IhEy4ayp4XrN7rVqcxzMZQ6Bbnlk390YD/+mBgGCvDAAMSIVRrWqJOAAAJcAAAAQuU01Gt5GtwAAAlwAAABFzE1JsAhFZSgaAb4dVZNHRGAeiKkSC4wUJJ732jX+o7FPGGVggQ2foDrbyx+I3/UpO+oojemxgtmMYzmpL5z8j/mQVoHM6dCMBBgopKUHMUHu/9SsQHJnWrta2AYYCX/pC6MmFlIUmpXl3F2GJBrFXYYVU4iQ8go/xMQhvW6SJrQYkcQO5l/n/++/zk1L65iYb8Y4TdPllTBuRRB5ycSm8uRwJIlF3a0sJmsXLtlzdaP8AElXGrrK0BKUh9KFkAAk6ZRo+2kgygdFdHgyBkL4Yb5kZFRf5+qpVCnHGs2s7/+mJg8knTAAMdNlRreTLcAAAJcAAAAQtQn0+s6KlwAAAlwAAABFj/6752QyPiAI9XG1EWRKIlXdFe9eVHVRGJHMiGREFyyxFiloEThWKLfTV/X//////////////////////////////////iCm87HbZGgB4EUyqiQYSBlcw6YB08zWaC0WqvMRNU+rbF734mSU3q1eGFhzAFArcazTY5Y/JL6bnEhqShAs0iBXDCFqEjD6ECgfMAEg8ylI0XvlxI1i/ZTsQFJXWrbI0ANARYDNF1x0ERFgbaQ0WHd8zBldM2MHR3JZlvuRj0Fq7UfdShmxwCMCd5//c9MGRMPOBUWFQKkas7//pgYP5+44AC4C7U61ka3AAACXAAAAELiL9PrWhrcAAAJcAAAASShgWUkXkAKwHFQWaXY6bOE3vMsV9f//////////////////////////////////////////////////////////////5AJvSt3a1oCKl+ImQKh4m8YyEeFY33GHMig0RjwKzeU0GqpvZzuW7kfVlCX4VZ7j37l75nn+/nH2Du6UwZ6wEtRXVV8TWXEK+TRR5TUGRjXJtZnWGsxT1r1aQC55ZLbIkA9DObhbFRcrHgmCLNYtdC4psDkjSVPMwgCD5BZpjUvKOxbljWwQOMRAd2mxrbyuybRk8UKhyw7lIS//pgYKgb9oADoTFS6zoq3AAACXAAAAELEElJreNG8AAAJcAAAARnZDuR32n1Veziw+lisYLkGPatwvStvuGdGLI////////////////////////////////////////////////////////////////////////////+kAuZ1qSNogKE0WYOLFUa2cGa0F3YjBa9rKdBj/NcLZxSn7iaj963GHbTkYufTcI7//93ilCltj3kI905TPKa8fTSi4GFtuPWiirEFN6Zu2yJAAoKHArKMCSqJBljo+HXo+g61mN4RnhLZKlXOsbs09SU8QaWtkfXgbHuPfr+UFzAooPwgGj4IXrF//pgYPSg/4AELRJSa1nRvAAACXAAAAELKMVLrGRrcAAAJcAAAARWCo8JnQdMk7FhUkE6n/f0b+iWegl///////////////////////////////////////////////////////////////////////////5Alz3W22RgBThf1QOJwCRCABKEo07pBDJX6FktXTlnqTCqeHc7W3SO2FFDq+ZzLsbVXsEz+3aEtN75dq9bqdbKq+rtFfv5/5t9mkrV7m1abo+oAyaaOyNogJmsygkwAhhBMYEcpTSG6cDIMSZcDaIYgCaxxuHDNaqww7aGCV58Vvxn//+N/+Y56smhezPHbgvp//piYBiM/4AEri1R61oa3AAACXAAAAEJQElDrWcm8AAAJcAAAARxB9usky0CJQ81iyWEFs6/1f//////////////////////////////////////////////////////////////////////////////////////////////9ICb0zckbIAhhcs0h6VQlqmGURgKxvuOSXlHxCeBXfh2/2qcUt6tdgBVdeI+892Ped+5aQRcWH2kD1ywi9PYIhXUjTh329CoAuaaSSNogLsYLbMMIhAQrBuI1LFrohM8D+h0WLsMn7P1jdcv0W7MNjAxzNOLTY5crRwKkG0mIocvWRPTyWklPv/6YGAxi/+ABG0SUetYybwAAAlwAAABCixLSa1jJvgAACXAAAAEHhHbFwsdcQW5BYornbAnZ0///////////////////////////////////////////////////////////////////////////////////////////UAbddbJG0QIEfWoDlXqHjxjFr8osGiCiqtQweHIa3YkeVKd1s7bpI2tAtOd3MAb7zvICsKYdqHuuk2sS1lipU07KtxzPJdp9laEFOa6SSNkAQJjklk8ZMcZSo/G2kMEJ24zoNe5sGUlPjWPGqXVLcoYGuUH5Qfj3n8rXrD0IOSPThciSY0DKNgFbP/6YGAvAf+ABMMVUOtZycwAAAlwAAABCNBJQazjJvAAACXAAAAEQf2mrOr/////////////////////////////////////////////////////////////////////////////////////////////////////hADlkn/SAlazKOgoQVBKAICRBKFZ0FHtHaEA2p5vrb5lVNQhs63j5AKOEYbWmx7z93sLb4sVnuzRr9t/Rb/t79keedxT+a2lRV27d+hKaAMltsbaRAE7coACDKgFAYI4ixIbpwUYup3hDFQSjvLc0b9hTcwp30AokzUmOWdc/lDZ3Ws2KJr6WUm/YYeaYP/6YGB0av+ABK0R0Gs4ybwAAAlwAAABCShJQ6znJvAAACXAAAAE52hbFd79X///////////////////////////////////////////////////////////////////////////////////////////////////////////EAnLbJJGyAHNmI+OkbCTMBoCcDT4bHCNykoVS3ueGtVTqds63UibugNaRcy33dCYEgxRSEgibhU2sNO5dxi8JxQ+kKxV9+xfiALtssbaRAD8SWOGAG8A0SIYx5WexQRQQ7p0GnHEO43qxqzk9z/wKog5wxpsus6xqTNvC6MMMZbzL0Vl0XteJP/6YmDE4P+ABJASUGs4ybwAAAlwAAABCbRLOY1jRvgAACXAAAAESVu0lYoGrCP/6f//////////////////////////////////////////////////////////////////////////////////////////////////////////ZAMksv/QBFJnIaBIuJgiKCu+GKQHPXS1wK+lE3h2tLT2jnf/tIFSRTuJVe/r921lXH/nFtY572c53v/DnDYJtsvVsv/7QQA1Z5eNbZEAJNnwoBKhAkGZCY+2tR9BlrIbAVO9tNr/khtz09rOpK2gnDJN/Z86DQczgrD9l1KjVTFy603iDQn/+mBghTL/gAS3Es7rWdG8AAAJcAAAAQj4Rz2sYybwAAAlwAAABKtavar///////////////////////////////////////////////////////////////////////////////////////////////////////////////VAEniKidtrWAJN80Cl2fERwM8Fup3QWW1V7gUlqePebqnYvLeYZVEpg3RtaazLaxrcdE9hYkYcspJjjKnniq3xKPYTf69AVAAoh5edrrWAIrQQWBCH3FjhD4xCN04OQ051hnVbDvMbh9RW//wQngXcPYVmiIqBlt9EIpWLkyQ1j3IvUsueafr/+mBglZL/gATQEs7rONG8AAAJcAAAAQigSTeNYyb4AAAlwAAABHROMThcLf///////////////////////////////////////////////////////////////////////////////////////////////////////////////+ZgBq6u0SSNkATnJoQDkoaFRkoD5i133GFMCekEraDn/92Anea7qu0A7UpfjdW0ZfTagmlQi2E69RR6pFY0oItGnoNIMwAWhXZ5JGyAKLlcwwiwCuAG7jWM9sQqepxTMFodz/5oHJX987gzInAg4ClhUwKNQtr2i2biSxSs/sGXVuL6vcj/+mBgjVP/gATED8x7ONG4AAAJcAAAAQjEPzXs4ybgAAAlwAAABP/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////0QQOZeAi22RACi7YDkV8jxopa5cMUgQtiLLBHyUXcemiICqn4nsGw5s2tyqXtFVo1vCNCy9azb+KEC4v11kgZvdfJI0ABRZ3CIuMDyJlKj661HYGXshsA0b5Zb/HSFX4b1uHDKC2IRcE2oSxzxeWcpU4eEi5AdFaPY5u1np//+mBgQRH/gATbD8z7OMG4AAAJcAAAAQh0Py3s4ybgAAAlwAAABP/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////0yFvdNJJIwBO5SkIEIACgUGXDV0huCI7wweW1c/HvEGC3lTah9AupqbLetavc0fM0OYaFBtxsUOuFqLEvdX9MWzCF9LN/0AXvjwXHJQnuEfTWCbL4X0IMSQQyTdgWT+JuBfgU7Wq6ioUQm7tQUxgaGpiV7cXio9etf///////////+mJg8UD/gAUUDst7OMm4AAAJcAAAAQesOzHs4gbgAAAlwAAABP/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////+GDLXRf/4C9yVjISwRQWZrA+4rG7ZARwqqw8t7zTrBSV+SgJ04EU4+JZh7LLHEaVT7it1hscTWaVmRRfwDgwVrI7n/Ac+4Cgb5MeBfRb2e2IyQRLWc2+d1m5xb/+IQRMweapvqikdhqm9San1Fq83cKfucr////////////////pgYHXm/4AE9A7K6zjBuAAACXAAAAEICD8prOJGwAAAJcAAAAT////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////DIVsjlWoBe/IWGVTKAxTN22CIXtlZBTsX1qcQPX2qG7iy3rx0fcfHqDMHAoWUlDENQffpURE7q45/+IXvlI0YQCxw0ox/Mkh+Ad0jo53WN7WEy6yARKOvQkWYEWJrte9DK1yC1S6Hsdchn//Z8p////////////pgYFvO/4AFDw5KYzBpuAAACXAAAAEHmDsljOGmwAAAJcAAAAT//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////+Kk7XHCpQDn3Ag9O0WIBmA1lFwO1rjGjv/FxCzX4KRn0k0kCFKaLKaSCc573S8VGWOOf/wHPiKCCUkSAzQ+grRhiUgO9Z2QZSzVCcPZxr2RpTWeUlsK5Sm1wpM1va7Uz1f///////////////////pgYKL5/4AFUg7JYzh5sAAACXAAAAEGjDsljL1m4AAAJcAAAAT/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////iuWwSRxttADnzCE1RweJMtofYBPNwUJWbt/rjHbGtjKZPPYIQ+erupOmLWD7nNkIsWeDbW/4tFsWyWiORySIAc+0nUSjMZCto1aSgSOIiK/kFNtUSTPku0WeUSJp6w+4oKrlljEvWJIRFfV///////////////////piYAlk/4AFkw5JYy85sAAACXAAAAEFsDkljL1G4AAAJcAAAAT////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////4sdkjcqlAOfglyn8JDilsBghAfPFT6t2ZEQtQdJ6lfqPJ6aXpdfMudbINB9YW7ayWySSMAc/BocqKHmKJH8kKA6rjf/vz7UMJDqhjaIopxNWAZQXOtH5Q2g+gl1L1awpp///////////////////6YGAsC/+ABTkOSWMpObAAAAlwAAABBvQ7JaysxsAAACXAAAAE//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////8X+2SSedADSoBCf4YsDOhcKdEyusXcVQiLqexFrvlTG9yhe08gjSKvFqcO2RuN1agDYTUTeoVugADoSqQXB4G1rTijHqUmrFC4EPCdJEjQmwz7S65f///////////////////////////////6YGDxQv+ABZUOyesnEbAAAAlwAAABBYg5JYyYpuAAACXAAAAE/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////E3kgbdUoA1QiUbSAMMCfD0IwbdXl2JW2pMQoSzU2tfGTSmrrY5ZtaAFeAVERFEkkYAawAxFC0RxjUuGYdXksACTupVY3chUgTM7maGi7Un5dmyun//////////////////////////////6YGCXof+ABagOSmsJGbAAAAlwAAABBTgZKYPkwmAAACXAAAAE//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////BcSIRMqoBw2ONqMZgkAJnWYkCfDouaMPKyhYaK2bJ9UAInntnvDSn7regdALAAAGozVUV0PaXsbvoj5OiWWtEe4gmaXY8jWMl1qJm6///////////////////////////////////////////6YGB/wf+ABbYGSWC5GJgAAAlwAAABBQAZJYPgYmAAACXAAAAE///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////ALBAAHOR2GwRUab1wq8Uctr2Ct6mJSlFGz/ZabbNKBcSTSXgUcNGU+Lua8QsxtSWy5WeNjVIMx86LjnLmXvxdiUISr//////////////////////////////////////6YmCI//+ABaQFyni5EJAAAAlwAAABBWwTHYLgIkAAACXAAAAE//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////AKBBAE4gENYFFCx1DIvKViKkHJSYFCX8dZ0gFylNajClaF8M2WGS94kIiRmeR1ck5mNfoVsHFtrRguhrhQOL4un///////////////////////////////////////////+mBgJ4H/gAYCBMYgTwiQAAAJcAAAAQPYExiAvCJAAAAlwAAABP//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////CoGaiUA6BuZqLHz29jVpFmv/dCL1Le1vVfvtJPsRBVlX8Ot7gYFnJQX1tWIWur+SHpnDJitkQmLExqklBTo1Vp//////////////////////////////////////////+mBgsS3/gAX8BEhYBXgYAAAJcAAAAQPoERiABMBAAAAlwAAABP////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////817KoBsV7erFNJowD79N7noEgEXzVpJsrl6EfW673h1f3ut/6ZFHK+a///////////////////////////////////////////////////////////////////+mBg4Hr/gAYIBEdAATAQAAAJcAAAAQO8Dx8gBEBAAAAlwAAABP///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////P//////////////////////////////////////////////////////////////////////////////////////+mJg+Ir/gAYwAEdAAAAAAAAJcAAAAQNAARzAAAAAAAAlwAAABP///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////wAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA//pgYBbJ/4AG9gDGAAAAAAAACXAAAAEAAAEuAAAAIAAAJcAAAAQAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA//pgQPcd/4AG+ABLgAAACAAACXAAAAEAAAEuAAAAIAAAJcAAAAQAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA");
        snd.play();
    }
}
// function playFail() {
//     if(playSounds) {
//         var snd = new Audio("data:audio/mp3;base64,SUQzAwAAAAAfdlRJVDIAAAAbAAAAYmFkIGFuc3dlciAwMSBzb3VuZCBlZmZlY3RUUEUxAAAAFAAAAGZyZWVzb3VuZGVmZmVjdC5uZXRUQUxCAAAAFAAAAGZyZWVzb3VuZGVmZmVjdC5uZXRUWUVSAAAABQAAADIwMTZUQ09OAAAABQAAAFJvY2tDT01NAAAADwAAAGVuZwBleGNlbGxlbnQhVFJDSwAAAAYAAAAwNC8xNgAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA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");  
//         snd.play();
//     }
// }
function onDefaultClick() 
{		
	if (confirm("WARNING! Are you sure you wish to reset ALL settings?\nThis will restart your device and you may need to reconfigure your WiFi settings!")) 
	{
        showInfo("Setting default...");
		var xhr = new XMLHttpRequest();
		xhr.open("POST", "/default", true);
		xhr.onreadystatechange = function() 
		{
			if (xhr.readyState === 4) 
			{
                if (xhr.status !== 200) {
                    showError("Error setting default!");
                } else {
                    showInfoSuccess("Settings reset!");
                    //resettingAllToDefault = true;
                    showLoading("Restarting...");
                    startServerPoll();
                    //onRestartClick("\nYou may need to reconfigure your wifi with the instructions provided in the zip.");
                }
			}
		}
		xhr.send();
	}
}
function postBoardType(newBoardType) {
    showInfo("Changing board...");
    var xhr = new XMLHttpRequest();
    xhr.open("POST", "/changeBoard/"+newBoardType, true);
    xhr.onreadystatechange = function() 
    {
        if (xhr.readyState === 4) 
        {
            if (xhr.status !== 200) {
                showError("Error setting pinout default!");
            } else {
                showInfoSuccess("Board changed!");
                getPinSettings();
                showRestartRequired();
            }
        }
    }
    xhr.send();
}
function postDeviceType(deviceType) {
    showInfo("Changing device...");
    var xhr = new XMLHttpRequest();
    xhr.open("POST", "/changeDevice/"+deviceType, true);
    xhr.onreadystatechange = function() 
    {
        if (xhr.readyState === 4) 
        {
            if (xhr.status !== 200) {
                showError("Error setting pinout default!");
            } else {
                showInfoSuccess("Device changed!");
                getPinSettings();
                showRestartRequired();
            }
        }
    }
    xhr.send();
}

function onRestartClick(optionalMessage) 
{		
    var warningmessage = "Device restarting...";
    if(optionalMessage) {
        warningmessage += optionalMessage;
    }
    showLoading(warningmessage);
    restartClicked = true;
    var xhr = new XMLHttpRequest();
    xhr.open("POST", "/restart", true);
	xhr.responseType = 'json';
    xhr.onreadystatechange = function() 
    {
        if (xhr.readyState === 4) 
        {
            if(xhr.status == 200) {
                var response = xhr.response;
                var message = "";
                logdebug("Restart succeed!");
                if(wifiSettings["bluetoothEnabled"] && systemInfo["moduleType"] == ModuleType.WROOM32) {
                    message += "The web server will be disabled because bluetooth has been enabled.<br>You will need to disable bluetooth via serial usb commands<br>to get back to this page after rebooting.<br><br>";
                    
                    showLoading(message);
                    return;
                }
                if(!resettingAllToDefault) {// There redirect should be to the default IP address.
                    checkRestartRedirect();
                } else {
                    restartingAndChangingAddress = true;
                }
                
                if(restartingAndChangingAddress) {
                    var isIPStatic = wifiSettings["staticIP"];
                    var localIP = wifiSettings["localIP"];
                    var webServerPort = wifiSettings["webServerPort"];
                    var hostname = wifiSettings["hostname"];
                    var url = "http://"+hostname+".local";
                    url += webServerPort === 80 ? "" : ":"+webServerPort;
                    if(resettingAllToDefault) {
                        url = "http://"+systemInfo.defaultIP+"/";
                        isIPStatic = false;
                    }
                    var staticIPUrl = "http://"+localIP;
                    staticIPUrl += webServerPort === 80 ? "" : ":"+webServerPort;
                    message += "Device restarting, the page will redirect to<br><a href='"+url+"'>"+url+"</a> in 10 seconds<br>";
                    if(!resettingAllToDefault) {
                        message += isIPStatic ? "If this doesnt work, you can try using this url<br><a href='"+staticIPUrl+"'>"+staticIPUrl+"</a> when the device reboots." 
                            : "If this doesnt work, you will need<br>to find the dymanic ip address of the esp32."
                    }
                    showLoading(message);
                    setTimeout(() => {
                        logdebug("Redirecting to: " + url)
                        window.location.href = url;
                    }, 10000);
                    //startServerPoll(url);
                }
                hideRestartRequired();
            } else {
                logdebug("Restart api call fail!");
            }
        }
    }
    xhr.send();
}
function isWebSocketConnected() {
    if(!websocket)
        return false;
    return websocket.readyState === WebSocket.OPEN;
}

function isSR6() {
    return userSettings["deviceType"] == DeviceType.SR6;
}
function isOSR() {
    return userSettings["deviceType"] == DeviceType.OSR;
}
function isSSR1() {
    return userSettings["deviceType"] == DeviceType.SSR1;
}
function isBoardType(boardType) {
    return userSettings["boardType"] === boardType;
}
function isModuleType(moduleType) {
    return systemInfo["moduleType"] == moduleType;
}
function isMotorType(motorType) {
    return systemInfo.motorType == motorType
}
function isBLDCSPI() {
    return userSettings["BLDC_Encoder"] == BLDCEncoderType.SPI || userSettings["BLDC_Encoder"] == BLDCEncoderType.MT6701;
}

function startServerPoll() {
    if(serverPollRetryCount > 10) {
        showLoading("Waiting for restart timed out<br>Please manually refresh the page when the device is back online.");
        return;
    }
    if(serverPollingTimeOut)
        clearTimeout(serverPollingTimeOut);
    serverPollRetryCount++;
    serverPollingTimeOut = setTimeout(function() {getSystemInfo(true);}, 2000);
}
function checkForServer() {
    // if(serverPollingTimeOut) {
    //     clearTimeout(serverPollingTimeOut);
    //     serverPollingTimeOut = null;
    // }
    // if(serverPollRetryCount > 10) {
    //     showLoading("Websocket timed out. Please refresh the page for full functionality.");
    //     return;
    // }
    // if(isWebSocketConnected() && websocket.readyState !== WebSocket.CONNECTING) {
    //     logdebug("Websocket closed retrying..");
    //     initWebSocket();
    //     serverPollRetryCount++;
    //     serverPollingTimeOut = setTimeout(checkForServer, 2000);
    // } else if(isWebSocketConnected()) {
    //     logdebug("Websocket open..");
    //     if(serverPollingTimeOut) {
    //         clearTimeout(serverPollingTimeOut);
    //         serverPollingTimeOut = null;
    //     }
    // }
}

function setSystemInfo() {
    if(!systemInfo)
        showError("Error getting system info!");
    document.getElementById('version').value = systemInfo.esp32Version;
    document.getElementById('macAddressSystemInfo').value = systemInfo.mac;
    document.getElementById('ipAddressSystemInfo').value = systemInfo.localIP;
    document.getElementById('gatewaySystemInfo').value = systemInfo.gateway;
    document.getElementById('subnetSystemInfo').value = systemInfo.subnet;
    document.getElementById('dnsSystemInfo').value = systemInfo.dns1;
    document.getElementById('chipModel').value = systemInfo.chipModel;
    document.getElementById('chipRevision').value = systemInfo.chipRevision;
    document.getElementById('chipCores').value = systemInfo.chipCores;
    document.getElementById('chipID').value = systemInfo.chipID;

    tcodeVersions = systemInfo.tcodeVersions;

    const logLevelElement = document.getElementById('logLevel');
    removeAllChildren(logLevelElement);
    for(let i=0;i<systemInfo.logLevels.length;i++) {
        const option = document.createElement("option");
        option.innerText = systemInfo.logLevels[i].name;
        option.value = systemInfo.logLevels[i].value;
        logLevelElement.appendChild(option);
    }

    var excludedTagsElement = document.getElementById('log-exclude-tags');
    removeAllChildren(excludedTagsElement);
    systemInfo.availableTags.forEach(element => {
        var option = document.createElement("option");
        option.value = element;
        option.innerText = element;
        excludedTagsElement.appendChild(option);
    });

    var includedTagsElement = document.getElementById('log-include-tags');
    removeAllChildren(includedTagsElement);
    systemInfo.availableTags.forEach(element => {
        var option = document.createElement("option");
        option.value = element;
        option.innerText = element;
        includedTagsElement.appendChild(option);
    });
    
    var i2cAddressesElement = document.getElementById("Display_I2C_Address");
    removeAllChildren(i2cAddressesElement);
    systemInfo.systemI2CAddresses.forEach(element => {
        var option = document.createElement("option");
        option.value = element;
        option.innerText = element;
        i2cAddressesElement.appendChild(option);
    });
    var bleDeviceTypeElement = document.getElementById("bleDeviceType");
    removeAllChildren(bleDeviceTypeElement);
    systemInfo.bleDeviceTypes.forEach(element => {
        var option = document.createElement("option");
        option.value = element.value;
        option.innerText = element.name;
        bleDeviceTypeElement.appendChild(option);
        BLEDeviceType[element.name] = element.value;
    });
    var bleLoveDeviceTypeElement = document.getElementById("bleLoveDeviceType");
    removeAllChildren(bleLoveDeviceTypeElement);
    systemInfo.bleLoveDeviceTypes.forEach(element => {
        var option = document.createElement("option");
        option.value = element.value;
        option.innerText = element.name;
        bleLoveDeviceTypeElement.appendChild(option);
        BLELoveDeviceType[element.name] = element.value;
    });
    if(systemInfo.motorType === MotorType.BLDC)
        setupEncoderTypes();
        
    setupBoardTypes();
    setupDeviceTypes();
    toggleBuildOptions();
    toggleMotorTypeOptions();
    setupTimerChannels();
    toggleBLESettings();
    toggleBluetoothSettings();
    
    if(systemInfo["moduleType"] == ModuleType.S3) {
        validPWMpins = [];
        for(let i=1;i<44;i++) {
            validPWMpins.push(i);
        }
        inputOnlypins = [46];
        
        adc1Pins = [1,2,3,4,5,6,7,8,9,10];
        adc2Pins = [11,12,13,14,15,16,17,18,19,20];
    }

    document.getElementById('lastRebootReason').value = systemInfo.lastRebootReason;
}
function setWifiSettings() {
    document.getElementById("ssid").value = wifiSettings["ssid"];
    document.getElementById("wifiPass").value = wifiSettings["wifiPass"];
    document.getElementById('bluetoothEnabled').checked = wifiSettings["bluetoothEnabled"];

    document.getElementById('bleEnabled').checked = wifiSettings["bleEnabled"];
    document.getElementById('bleDeviceType').value = wifiSettings["bleDeviceType"];
    toggleBLEDeviceTypes();
    document.getElementById('bleLoveDeviceType').value = wifiSettings["bleLoveDeviceType"];
    toggleBLELoveDeviceTypes();
    
    document.getElementById("staticIP").checked = wifiSettings["staticIP"];
    document.getElementById("localIPInput").value = wifiSettings["localIP"];
    document.getElementById("gatewayInput").value = wifiSettings["gateway"];
    document.getElementById("subnetInput").value = wifiSettings["subnet"];
    document.getElementById("dns1Input").value = wifiSettings["dns1"];
    document.getElementById("dns2Input").value = wifiSettings["dns2"];
    toggleStaticIPSettings(wifiSettings["staticIP"]);
    document.getElementById("udpServerPort").value = wifiSettings["udpServerPort"];
    document.getElementById("webServerPort").value = wifiSettings["webServerPort"];
    document.getElementById("hostname").value = wifiSettings["hostname"];
    document.getElementById("friendlyName").value = wifiSettings["friendlyName"];
}
function setPinoutSettings() {
    if(systemInfo.motorType === MotorType.BLDC) {
        BLDCMotor.setupPins();
    } else {
        document.getElementById("RightServo_PIN").value = pinoutSettings["RightServo_PIN"];
        document.getElementById("LeftServo_PIN").value = pinoutSettings["LeftServo_PIN"];
        document.getElementById("RightUpperServo_PIN").value = pinoutSettings["RightUpperServo_PIN"];
        document.getElementById("LeftUpperServo_PIN").value = pinoutSettings["LeftUpperServo_PIN"];
        document.getElementById("PitchLeftServo_PIN").value = pinoutSettings["PitchLeftServo_PIN"];
        document.getElementById("PitchRightServo_PIN").value = pinoutSettings["PitchRightServo_PIN"];
        // if(isOSR() || isSR6()) {
            setPinChannel("RightServo_CHANNEL", pinoutSettings["RightServo_CHANNEL"]);
            setPinChannel("LeftServo_CHANNEL", pinoutSettings["LeftServo_CHANNEL"]);
            setPinChannel("PitchLeftServo_CHANNEL", pinoutSettings["PitchLeftServo_CHANNEL"]);
        // }
        // if(isSR6()) {
            setPinChannel("RightUpperServo_CHANNEL", pinoutSettings["RightUpperServo_CHANNEL"]);
            setPinChannel("LeftUpperServo_CHANNEL", pinoutSettings["LeftUpperServo_CHANNEL"]);
            setPinChannel("PitchRightServo_CHANNEL", pinoutSettings["PitchRightServo_CHANNEL"]);
        // }
    }
    document.getElementById("TwistFeedBack_PIN").value = pinoutSettings["TwistFeedBack_PIN"];
    document.getElementById("ValveServo_PIN").value = pinoutSettings["ValveServo_PIN"];
	document.getElementById("TwistServo_PIN").value = pinoutSettings["TwistServo_PIN"];
    document.getElementById("Vibe0_PIN").value = pinoutSettings["Vibe0_PIN"];
    document.getElementById("Vibe1_PIN").value = pinoutSettings["Vibe1_PIN"];
    document.getElementById("Vibe2_PIN").value = pinoutSettings["Vibe2_PIN"];
    document.getElementById("Vibe3_PIN").value = pinoutSettings["Vibe3_PIN"];
	document.getElementById("LubeButton_PIN").value = pinoutSettings["LubeButton_PIN"];
	document.getElementById("Squeeze_PIN").value = pinoutSettings["Squeeze_PIN"];
	// document.getElementById("Display_Rst_PIN").value = pinoutSettings["Display_Rst_PIN"];
	// document.getElementById("Display_Rst_PIN").readOnly = true;
	document.getElementById("Temp_PIN").value = pinoutSettings["Temp_PIN"];
	document.getElementById("Heater_PIN").value = pinoutSettings["Heater_PIN"];
    document.getElementById('Case_Fan_PIN').value = pinoutSettings["Case_Fan_PIN"];
    document.getElementById('Internal_Temp_PIN').value = pinoutSettings["Internal_Temp_PIN"];
    document.getElementById('i2cSda_PIN').value = pinoutSettings["i2cSda_PIN"];
    document.getElementById('i2cScl_PIN').value = pinoutSettings["i2cScl_PIN"];


    setPinChannel("Vibe0_CHANNEL", pinoutSettings["Vibe0_CHANNEL"]);
    setPinChannel("Vibe1_CHANNEL", pinoutSettings["Vibe1_CHANNEL"]);
    setPinChannel("Vibe2_CHANNEL", pinoutSettings["Vibe2_CHANNEL"]);
    setPinChannel("Vibe3_CHANNEL", pinoutSettings["Vibe3_CHANNEL"]);
    setPinChannel("ValveServo_CHANNEL", pinoutSettings["ValveServo_CHANNEL"]);
    setPinChannel("TwistServo_CHANNEL", pinoutSettings["TwistServo_CHANNEL"]);
    setPinChannel("Squeeze_CHANNEL", pinoutSettings["Squeeze_CHANNEL"]);
    setPinChannel("Heater_CHANNEL", pinoutSettings["Heater_CHANNEL"]);
    setPinChannel("Case_Fan_CHANNEL", pinoutSettings["Case_Fan_CHANNEL"]);
}
function setUserSettings()
{
    document.getElementById('TCodeVersion').value = userSettings["TCodeVersion"];
    setLogLevelUI();
    toggleNonTCodev3Options();
    toggleDeviceOptions(userSettings["deviceType"]);
    toggleFeedbackTwistSettings(userSettings["feedbackTwist"]);
    toggleBatterySettings(userSettings["batteryLevelEnabled"]);
    MotionGenerator.setEnabledStatus();
    // var xMin = userSettings["xMin"];
    // var xMax = userSettings["xMax"];
    //document.getElementById("xMin").value = xMin;
    //calculateAndUpdateMinUI("x", xMin);
    //document.getElementById("xMax").value = xMax;
    //calculateAndUpdateMaxUI("x", xMax);
    //updateRangePercentageLabel("x", tcodeToPercentage(xMin), tcodeToPercentage(xMax));

    // var yRollMin = userSettings["yRollMin"];
    // var yRollMax = userSettings["yRollMax"];
    // document.getElementById("yRollMin").value = yRollMin;
    // calculateAndUpdateMinUI("yRoll", yRollMin);
    // document.getElementById("yRollMax").value = yRollMax;
    // calculateAndUpdateMaxUI("yRoll", yRollMax);
    // updateRangePercentageLabel("yRoll", tcodeToPercentage(yRollMin), tcodeToPercentage(yRollMax));

    // var xRollMin = userSettings["xRollMin"];
    // var xRollMax = userSettings["xRollMax"];
    // document.getElementById("xRollMin").value =xRollMin;
    // calculateAndUpdateMinUI("xRoll", xRollMin);
    // document.getElementById("xRollMax").value =xRollMax;
    // calculateAndUpdateMaxUI("xRoll", xRollMax);
    // updateRangePercentageLabel("xRoll", tcodeToPercentage(xRollMin), tcodeToPercentage(xRollMax));

    //updateSpeedUI(userSettings["speed"]);
    
    document.getElementById('boardType').value = userSettings["boardType"];
    const isSSR1PCB = isBoardType(BoardType.SSR1PCB);
    document.getElementById("deviceType").disabled = isBoardType(BoardType.CRIMZZON) || isBoardType(BoardType.ISAAC) || isSSR1PCB;
    document.getElementById("BLDC_Encoder").disabled = isSSR1PCB;

	document.getElementById("maxServoRange").value = userSettings["maxServoRange"];
	
	document.getElementById("feedbackTwist").checked = userSettings["feedbackTwist"];
	document.getElementById("continuousTwist").checked = userSettings["continuousTwist"];
	document.getElementById("analogTwist").checked = userSettings["analogTwist"];
    
    ESPTimer.setup();
    if(systemInfo.motorType === MotorType.BLDC) 
        BLDCMotor.setup();

	Buttons.setup();
    
    document.getElementById("RightServo_ZERO").value = userSettings["RightServo_ZERO"];
    document.getElementById("LeftServo_ZERO").value = userSettings["LeftServo_ZERO"];
    document.getElementById("RightUpperServo_ZERO").value = userSettings["RightUpperServo_ZERO"];
    document.getElementById("LeftUpperServo_ZERO").value = userSettings["LeftUpperServo_ZERO"];
    document.getElementById("PitchLeftServo_ZERO").value = userSettings["PitchLeftServo_ZERO"];
    document.getElementById("PitchRightServo_ZERO").value = userSettings["PitchRightServo_ZERO"];
    document.getElementById("ValveServo_ZERO").value = userSettings["ValveServo_ZERO"];
	document.getElementById("TwistServo_ZERO").value = userSettings["TwistServo_ZERO"];
	document.getElementById("Squeeze_ZERO").value = userSettings["Squeeze_ZERO"];
	document.getElementById("lubeEnabled").checked = userSettings["lubeEnabled"];
	document.getElementById("lubeAmount").value = userSettings["lubeAmount"];
	document.getElementById("deviceType").value = userSettings["deviceType"];
	document.getElementById("autoValve").checked = userSettings["autoValve"];
	document.getElementById("inverseValve").checked = userSettings["inverseValve"];
	document.getElementById("valveServo90Degrees").checked = userSettings["valveServo90Degrees"];
	document.getElementById("inverseStroke").checked = userSettings["inverseStroke"];
	document.getElementById("inversePitch").checked = userSettings["inversePitch"];
	document.getElementById("inverseTwist").checked = userSettings["inverseTwist"];
    

	document.getElementById("displayEnabled").checked = userSettings["displayEnabled"];
	document.getElementById("sleeveTempDisplayed").checked = userSettings["sleeveTempDisplayed"];
	document.getElementById("internalTempDisplayed").checked = userSettings["internalTempDisplayed"];
	document.getElementById("versionDisplayed").checked = userSettings["versionDisplayed"];
	document.getElementById("tempSleeveEnabled").checked = userSettings["tempSleeveEnabled"];
    document.getElementById('tempInternalEnabled').checked = userSettings["tempInternalEnabled"];
	document.getElementById("Display_Screen_Width").value = userSettings["Display_Screen_Width"];
	document.getElementById("Display_Screen_Height").value = userSettings["Display_Screen_Height"];
    
    if(userSettings["Display_Screen_Height"] == 32) {
        document.getElementById('displayIs32Px').checked = true;
    }
	document.getElementById("TargetTemp").value = userSettings["TargetTemp"];
	document.getElementById("HeatPWM").value = userSettings["HeatPWM"];
	document.getElementById("HoldPWM").value = userSettings["HoldPWM"];
	document.getElementById("Display_I2C_Address").value = userSettings["Display_I2C_Address"];
    document.getElementById("Display_I2C_Address_text").value = userSettings["Display_I2C_Address"];
	// document.getElementById("heaterFailsafeTime").value = userSettings["heaterFailsafeTime"];
	document.getElementById("heaterThreshold").value = userSettings["heaterThreshold"];
	document.getElementById("heaterResolution").value = userSettings["heaterResolution"];
    
	// document.getElementById("Display_Rst_PIN").readOnly = newtoungeHatExists;

	document.getElementById("Display_Screen_Width").readOnly = true;
	document.getElementById("Display_Screen_Height").readOnly = true;
    document.getElementById('fanControlEnabled').checked = userSettings["fanControlEnabled"];
    document.getElementById('internalTempForFan').value = userSettings["internalTempForFan"];
    document.getElementById('internalMaxTemp').value = userSettings["internalMaxTemp"];
    document.getElementById('caseFanResolution').value = userSettings["caseFanResolution"];

    document.getElementById('vibTimeout').value = userSettings["vibTimeout"];
    document.getElementById('vibTimeoutEnabled').checked = userSettings["vibTimeoutEnabled"];

    batterySetup();
    
    document.getElementById('logLevel').value = userSettings["logLevel"];

    var includedElement = document.getElementById('log-include-tags');
    for (var i = 0; i < includedElement.options.length; i++) {
        includedElement.options[i].selected = userSettings["log-include-tags"].indexOf(includedElement.options[i].value) >= 0;
    }
    var excludedElement = document.getElementById('log-exclude-tags');
    for (var i = 0; i < excludedElement.options.length; i++) {
        excludedElement.options[i].selected = userSettings["log-exclude-tags"].indexOf(excludedElement.options[i].value) >= 0;
    }
    
    document.getElementById('voiceEnabled').checked = userSettings['voiceEnabled'];
    document.getElementById('voiceMuted').checked = userSettings['voiceMuted'];
    document.getElementById('voiceVolume').value = userSettings['voiceVolume'];
    document.getElementById('voiceWakeTime').value = userSettings['voiceWakeTime'];

    setupChannelSliders();
    
    documentLoaded = true;
    //document.getElementById('debugLink').hidden = !userSettings["debug"];
}
function removeAllChildren(element) {
    if(!element) {
        return;
    }
    while (element.firstChild) {
        element.removeChild(element.lastChild);
    }
}
function removeByName(name) {
    if(!name) {
        return;
    }
    let nodes = document.getElementsByName(name);
    if(!nodes.length) {
        console.warn("WARNING: No elemnts found when removing children by name")
        return;
    }
    while(nodes[0]) {
        nodes[0].parentNode.removeChild(nodes[0]);
    }
}
function removeByClass(name) {
    if(!name) {
        return;
    }
    let nodes = document.getElementsByClassName(name);
    if(!nodes.length) {
        console.warn("WARNING: No elemnts found when removing children by class")
        return;
    }
    while(nodes[0]) {
        nodes[0].parentNode.removeChild(nodes[0]);
    }
}

function hasFeature(buildFeature) {
    return systemInfo.buildFeatures.includes(buildFeature);
}
function toggleBuildOptions() {
    var hasTemp = hasFeature(BuildFeature.TEMP);
    Utils.toggleControlVisibilityByClassName('build_temprature', hasTemp);
    Utils.toggleControlVisibilityByID('internalTempDisplayedRow', hasTemp && userSettings["tempInternalEnabled"]);
    Utils.toggleControlVisibilityByID('sleeveTempDisplayedRow',  hasTemp && userSettings["tempSleeveEnabled"]);

    Utils.toggleControlVisibilityByClassName('build_display', hasFeature(BuildFeature.DISPLAY_));
        
    var tcodeVersionElement = document.getElementById('TCodeVersion');

    tcodeVersions.forEach(x => {
        const optionElement = document.createElement("option");
        optionElement.value=x.value;
        optionElement.innerText=x.name;
        tcodeVersionElement.appendChild(optionElement);
    });
}

function toggleMotorTypeOptions() {
    if(systemInfo.motorType === MotorType.Servo) {
        Utils.toggleControlVisibilityByClassName('servoOnly', true);
    } else {
        Utils.toggleControlVisibilityByClassName('BLDCOnly', true);
    }
}

function toggleBLDCEncoderOptions() {
    Utils.toggleControlVisibilityByClassName("BLDCPWM", userSettings["BLDC_Encoder"] == BLDCEncoderType.PWM);
    Utils.toggleControlVisibilityByClassName("BLDCSPI", isBLDCSPI());
}

function updateUserSettings(debounceInMs, uri, objectToSave, callback) 
{
    if (documentLoaded) {
        if(debounceInMs == null || debounceInMs == undefined) {
            debounceInMs = defaultDebounce;
        }
        if(!uri) {
            uri = "/settings"
        }
        if(!objectToSave) {
            objectToSave = userSettings;
        }
        if(upDateTimeout !== null) 
        {
            clearTimeout(upDateTimeout);
        }
        upDateTimeout = setTimeout(() => 
        {
            checkRestartRedirect();
            
            showInfo("Saving...");
            var xhr = new XMLHttpRequest();
            var response = {};
            xhr.open("POST", uri, true);
            xhr.onreadystatechange = function() 
            {
                if (xhr.readyState === 4) 
				{
                    if(!xhr.responseText.length)
                    {
                        response["msg"] = xhr.status + ': ' + xhr.statusText;
                    }
                    else if(xhr.responseText.startsWith("{"))
                    {
                        response = JSON.parse(xhr.responseText);
                    } 
                    else
                    {
                        response["msg"] = xhr.responseText;
                    }
                    if (response["msg"] !== "done") 
                    {
                        hideInfo();
                        showError("Error saving: " + response["msg"]);
                        getUserSettings();
                    } 
                    else 
                    {
                        if (restartRequired) 
                        {
                            showRestartRequired();
                        }
                        if(callback) {
                            callback();
                        } else {
                            showInfoSuccess("Settings saved!");
                        }
                    }
                }
            }
            xhr.setRequestHeader('Content-Type', 'application/json');
            var body = JSON.stringify(objectToSave);
            xhr.send(body);
            upDateTimeout = null;
        }, debounceInMs);
    }
}

function checkRestartRedirect() {
    // if(startUpWebPort !== userSettings["webServerPort"] || 
    //     startUpHostName !== userSettings["hostname"] || 
    //     startUpStaticIP !== userSettings["staticIP"] ||
    //     startUpLocalIP !== userSettings["localIP"]) {
    //     restartingAndChangingAddress = true;
    // }
    // if(!restartingAndChangingAddress) {
        // Apmode configuration
        const passwordChanged = wifiSettings.ssid !== "YOUR SSID HERE" && wifiSettings.wifiPass != "YOUR PASSWORD HERE" && wifiSettings.wifiPass != systemInfo.decoyPass;
        const wifiConfiguredAndNOTConnected = systemInfo.apMode && passwordChanged;

        if(wifiConfiguredAndNOTConnected) {
            restartingAndChangingAddress = true;
        } else {// Connect to wifi configuration
            //Uri change
            const isPort80 = window.location.port.length == 0;
            const isUserPort80 = wifiSettings["webServerPort"] == 80;
            const port80Changed = isPort80 && !isUserPort80 || !isPort80 && isUserPort80;
            const portChanged = port80Changed || (!isPort80 && window.location.port != wifiSettings["webServerPort"]);
            const connectedAndPortChanged = !systemInfo.apMode && portChanged; 
            //Not using IP address and hostname change
            const isCurrentIPAddress = isValidIP(window.location.hostname);
            const connectedAndHostnameChanged = !isCurrentIPAddress && !systemInfo.apMode && window.location.hostname != wifiSettings["hostname"];

            if(connectedAndPortChanged || connectedAndHostnameChanged) {
                restartingAndChangingAddress = true;
            }
        }
    // }
}

function isValidIP(ip) {
    return ip && ip.match(/^((25[0-5]|(2[0-4]|1\d|[1-9]|)\d)\.?\b){4}$/);
}
function isValidHostName(hostname) {
    return hostname && hostname.match(/^(?![0-9]+$)(?!.*-$)(?!-)[a-zA-Z0-9-_]{1,63}$/g)
}

function showRestartRequired() {
    //document.getElementById('requiresRestart').hidden = false;
    document.getElementById('resetBtn').classList.add("restart-required");
    document.getElementById('menu-button').classList.add("restart-required");
}

function hideRestartRequired() {
    //document.getElementById('requiresRestart').hidden = true;
    document.getElementById('resetBtn').classList.remove("restart-required");
    document.getElementById('menu-button').classList.remove("restart-required");
}
function showLoading(message) {
    var loadingModal = document.getElementById("loadingModal");
    var loadingStatus = document.getElementById("loadingStatus");
    loadingStatus.innerHTML = message;
    loadingModal.style.visibility = "visible";
    loadingVisible = true;
}
function hideLoading() {
    var loadingModal = document.getElementById("loadingModal");
    loadingModal.style.visibility = "hidden";
    loadingVisible = false;
}
function toggleMenu() {
    var menu = document.getElementById("menu");
    var menuButton = document.getElementById("menu-button");
    var menuContent = document.getElementById("menu-content");
    menuContent.classList.toggle("menu-hidden")
    menuContent.classList.toggle("menu-shown")
    menuButton.classList.toggle("button-pressed");
    menu.classList.toggle("menu-hidden");
    menu.classList.toggle("menu-shown");
}
function clearErrors(name) 
{
    var errorText = document.getElementById("errorText");
    if(name) {
        var errors = document.getElementsByName(name);
        for(var i=0;i<errors.length;i++) {
            errorText.removeChild(errors[i].parentNode);
        }
        errors = document.getElementsByName("errorItem");
        if(!errors.length) {
            document.getElementById("errorMessage").hidden = true;
        }
    } 
    // else {
    //     closeError();
    // }
}

function closeError() 
{
    var errorText = document.getElementById("errorText");
    removeAllChildren(errorText);
    document.getElementById("errorMessage").hidden = true;
}

function showError(message) 
{
    var div = document.createElement("div");
    div.setAttribute("name", "errorItem");
    div.innerHTML = message;
    document.getElementById("errorText").appendChild(div);
    document.getElementById("errorMessage").hidden = false;
}

function showInfo(message) {
	const infoNode = document.getElementById('info');
    infoNode.innerText = message;
    infoNode.style.color = "white";
	const modals = document.getElementsByTagName('modal-component');
    for(let i=0;i<modals.length;i++) {
        if(modals[i].visible()) {
            const node = modals[i].shadowRoot.querySelector(".info");
            node.innerText = message;
            node.style.color = "white";
        }
    }
}

function showInfoSuccess(message) {
	const infoNode = document.getElementById('info');
    infoNode.innerText = message;
    infoNode.style.color = "green";
	const modals = document.getElementsByTagName('modal-component');
    for(let i=0;i<modals.length;i++) {
        if(modals[i].visible()) {
            const node = modals[i].shadowRoot.querySelector(".info");
            node.innerText = message;
            node.style.color = "green";
        }
    }
    setTimeout(() => {
        hideInfo();
    }, 5000);
}

function hideInfo() {
	const infoNode = document.getElementById('info');
    infoNode.innerText = "";
    infoNode.style.color = "";
	const modals = document.getElementsByTagName('modal-component');
    for(let i=0;i<modals.length;i++) {
        if(modals[i].visible()) {
            const node = modals[i].shadowRoot.querySelector(".info");
            node.innerText = "";
            node.style.color = "";
        }
    }
}

function sendWebsocketCommand(command, message) {
    if(!isWebSocketConnected()) {
        startServerPoll();
        return;
    }
    websocket.send("{\"command\":\""+command+"\", \"message\": \""+message+"\"}")
}

function sendTCode(tcode) {
    if(!isWebSocketConnected()) {
        startServerPoll();
        return;
    }
    websocket.send(tcode+String.fromCharCode(10))
}

function sendTCodeValue(channelName, value, tcodeModifierType, modifierValue) {
    var tcode = getTCodeValue(channelName, value, tcodeModifierType, modifierValue)
    sendTCode(tcode);
}

function getTCodeValue(channelName, value, tcodeModifierType, modifierValue) {
    var tcode = channelName + value.toString().padStart(4, "0");
    if(tcodeModifierType) {
        tcode += tcodeModifierType
        tcode += modifierValue;
    }
    return tcode;
}

function sendDeviceHome() {
    channelSliderList.forEach(x => x.value = x.channelModel.isSwitch ? 0 : 50);
    var availibleChannels = getChannelMap();
    var tcode = "";
    availibleChannels.forEach((element, index, array) => {
        tcode += getSliderTCode(element.name, element.isSwitch ? 0 : 50, false, 1000, false);
        if (index !== array.length - 1){ 
            tcode += " ";
        }
    });
    sendTCode(tcode);
}

function getSliderTCode(channel, sliderValue, useIModifier, modifierValue, disableModifier) {
    var value = percentageToTcode(sliderValue);
    if(disableModifier) {
        return getTCodeValue(channel, value);
    }
    return getTCodeValue(channel, value, useIModifier ? TCodeModifierType.INTERVAL : TCodeModifierType.SPEED, modifierValue);
}

function setupChannelSliders() 
{
    var channelTestsNode = document.getElementById("channelTestsTable");
    deleteAllChildren(channelTestsNode);
    var bodyNode = document.createElement("div");
    channelTestsNode.appendChild(bodyNode);
    
    var headerRowNode = document.createElement("div");
    var headerCellNode = document.createElement("div");
    headerCellNode.classList.add("tHeader");
    headerCellNode.colSpan = "2";
    headerCellNode.style.textAlign = "center";
    var headerH3Node = document.createElement("h3");
    headerH3Node.innerText = "Test";
    headerCellNode.appendChild(headerH3Node);
    headerRowNode.appendChild(headerCellNode);
    bodyNode.appendChild(headerRowNode);

    var feedbackRowNode = document.createElement("div");
    feedbackRowNode.classList.add("tRow");
    var feedbackCellNode = document.createElement("div");
    feedbackCellNode.classList.add("tCell");
    feedbackCellNode.innerText = "TCode input: awaiting user...";
    feedbackCellNode.colSpan = "2";
    feedbackCellNode.style.justifyContent = "flex-start";
    feedbackRowNode.appendChild(feedbackCellNode);
    bodyNode.appendChild(feedbackRowNode);

    channelSliderList = [];
    var availibleChannels = getChannelMap();
    for(var i=0; i<availibleChannels.length;i++)
    {
        if(!isSR6() && availibleChannels[i].sr6Only) {
            continue;
        }
        var channel = availibleChannels[i].name;
        var channelName = availibleChannels[i].friendlyName;

        var rowNode = document.createElement("div");
        rowNode.classList.add("tRow");
        var titleCellNode = document.createElement("div");
        titleCellNode.classList.add("tCell");
        titleCellNode.innerText = channelName + " ("+channel+")";
        var inputCellNode = document.createElement("div");
        inputCellNode.classList.add("tCell");
        rowNode.appendChild(titleCellNode);
        rowNode.appendChild(inputCellNode);
        var sliderNode = document.createElement("input");
        sliderNode.style.width = "100%";
        sliderNode.type = "range";
        sliderNode.id = channel + "TestSlider";
        sliderNode.min = 0;
        sliderNode.max = 99;
        sliderNode.channelModel = availibleChannels[i];
        sliderNode.value = availibleChannels[i].isSwitch ? 0 : 50;
        sliderNode.addEventListener("input", function (sliderNode, channel, channelName, feedbackCellNode) {
            var tcode = getSliderTCode(channel, sliderNode.value, testDeviceUseIModifier, testDeviceModifierValue, testDeviceDisableModifier);
            sendTCode(tcode);
            feedbackCellNode.innerText = "TCode input: " + tcode + " ("+channelName+")";
        }.bind(null, sliderNode, channel, channelName, feedbackCellNode));
        channelSliderList.push(sliderNode);
        inputCellNode.appendChild(sliderNode);
        bodyNode.appendChild(rowNode);
    }

    var testDeviceModifierValueNode = document.createElement("input");
    testDeviceModifierValueNode.value = testDeviceModifierValue;
    testDeviceModifierValueNode.type="number"
    testDeviceModifierValueNode.min="1"
    testDeviceModifierValueNode.step="1"
    testDeviceModifierValueNode.addEventListener("input", (event) => {
        testDeviceModifierValue = event.target.value;
    });
    var testDeviceModifierValueRowNode = document.createElement("div");
    testDeviceModifierValueRowNode.classList.add("tRow");
    var testDeviceModifierValueCellNode = document.createElement("div");
    testDeviceModifierValueCellNode.classList.add("tCell");
    testDeviceModifierValueCellNode.innerText = "Magnitude (I/S) value";
    var testDeviceModifierValueInputCellNode = document.createElement("div");
    testDeviceModifierValueInputCellNode.classList.add("tCell");
    testDeviceModifierValueInputCellNode.appendChild(testDeviceModifierValueNode);
    testDeviceModifierValueRowNode.appendChild(testDeviceModifierValueCellNode);
    testDeviceModifierValueRowNode.appendChild(testDeviceModifierValueInputCellNode);
    bodyNode.appendChild(testDeviceModifierValueRowNode);

    var testDeviceUseIModifierNode = document.createElement("input");
    testDeviceUseIModifierNode.type = "checkbox"
    testDeviceUseIModifierNode.checked = testDeviceUseIModifier;
    testDeviceUseIModifierNode.addEventListener("click", (event) => {
        testDeviceUseIModifier = event.target.checked;
    });
    var testDeviceUseIModifierRowNode = document.createElement("div");
    testDeviceUseIModifierRowNode.classList.add("tRow");
    var testDeviceUseIModifierCellNode = document.createElement("div");
    testDeviceUseIModifierCellNode.classList.add("tCell");
    testDeviceUseIModifierCellNode.innerText = "Use Magnitude + Time Interval (I)";
    var testDeviceUseIModifierInputCellNode = document.createElement("div");
    testDeviceUseIModifierInputCellNode.classList.add("tCell");
    testDeviceUseIModifierInputCellNode.appendChild(testDeviceUseIModifierNode);
    testDeviceUseIModifierRowNode.appendChild(testDeviceUseIModifierCellNode);
    testDeviceUseIModifierRowNode.appendChild(testDeviceUseIModifierInputCellNode);
    bodyNode.appendChild(testDeviceUseIModifierRowNode);
    
    var testDeviceDisableModifierNode = document.createElement("input");
    testDeviceDisableModifierNode.type = "checkbox"
    testDeviceDisableModifierNode.checked = testDeviceDisableModifier;
    testDeviceDisableModifierNode.addEventListener("click", (event) => {
        testDeviceDisableModifier = event.target.checked;
    });
    var testDeviceDisableModifierRowNode = document.createElement("div");
    testDeviceDisableModifierRowNode.classList.add("tRow");
    var testDeviceDisableModifierCellNode = document.createElement("div");
    testDeviceDisableModifierCellNode.classList.add("tCell");
    testDeviceDisableModifierCellNode.innerText = "Disable magnitude";
    var testDeviceDisableModifierInputCellNode = document.createElement("td");
    testDeviceDisableModifierInputCellNode.classList.add("tCell");
    testDeviceDisableModifierInputCellNode.appendChild(testDeviceDisableModifierNode);
    testDeviceDisableModifierRowNode.appendChild(testDeviceDisableModifierCellNode);
    testDeviceDisableModifierRowNode.appendChild(testDeviceDisableModifierInputCellNode);
    bodyNode.appendChild(testDeviceDisableModifierRowNode);

    
    var testDeviceHomeNode = document.createElement("button");
    testDeviceHomeNode.addEventListener("click", (event) => {
        sendDeviceHome();
    });
    testDeviceHomeNode.innerText = "All home"
    var testDeviceHomeRowNode = document.createElement("div");
    testDeviceHomeRowNode.classList.add("tRow");
    var testDeviceHomeCellNode = document.createElement("div");
    testDeviceHomeCellNode.classList.add("tCell");
    testDeviceHomeCellNode.style.justifyContent = "flex-end";
    testDeviceHomeCellNode.colSpan = 2;
    testDeviceHomeCellNode.appendChild(testDeviceHomeNode);
    testDeviceHomeRowNode.appendChild(testDeviceHomeCellNode);
    bodyNode.appendChild(testDeviceHomeRowNode);

    DeviceRangeSlider.setup();
    MotionGenerator.setup();
}

function deleteAllChildren(parentNode) {
    while (parentNode.firstChild) {
        parentNode.removeChild(parentNode.firstChild);
    }
}
function isTCodeV3() {
    return userSettings["TCodeVersion"] >= TCodeVersion.V3;
}
function hasTCodeV2()  {
    return hasFeature(BuildFeature.HAS_TCODE_V2);
}
function getTCodeMax() {
    return 9999;
}
function getTCodeMin() {
    return 0;
}
function getChannelMap() {
    return systemInfo["availableChannels"]
};
function tcodeToPercentage(tcodeValue) {
    return convertRange(0, getTCodeMax(), 0, 99, tcodeValue);
}
function percentageToTcode(value) {
    return convertRange(0, 99, 0, getTCodeMax(), value);
}
function convertRange(input_start, input_end, output_start, output_end, value) {
    var slope = (output_end - output_start) / (input_end - input_start);
    return Math.round((output_start + slope * (value - input_start)));
}
function onSpeedInput() {
    var speedInMillisecs = parseInt(document.getElementById("speedInput").value);
    userSettings["speed"] = speedInMillisecs > 999 ? speedInMillisecs : 0;
    updateSpeedUI(speedInMillisecs);
    updateUserSettings();
}

function updateUdpPort() {
    wifiSettings["udpServerPort"] = parseInt(document.getElementById('udpServerPort').value);
    setRestartRequired();
    postWifiSettings();
}

function updateWebPort() {
    wifiSettings["webServerPort"] = parseInt(document.getElementById('webServerPort').value);
    setRestartRequired();
    postWifiSettings();
}

function updateMaxServoRange() {
    debounceInput("maxServoRange", () => {
        var control = document.getElementById('maxServoRange');
        if(!validateIntControl(control, userSettings, "maxServoRange")) {
            return false;
        }
        setRestartRequired();
        updateUserSettings(0);
    });
    return true;
}

function updateContinuousTwist() {
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
function updateAnalogTwist() {
	var checked = document.getElementById('analogTwist').checked;
    userSettings["analogTwist"] = checked;
    
    if(checked ) {
        document.getElementById("TwistFeedBack_PIN").value = 32;
        pinoutSettings["TwistFeedBack_PIN"] = 32;
        //if(!newtoungeHatExists)
        alert("Note, twist feedback pin has been changed to analog input pin 32.\nPlease adjust your hardware accordingly.");
    } else {
        document.getElementById("TwistFeedBack_PIN").value = 26;
        pinoutSettings["TwistFeedBack_PIN"] = 26;
        alert("Note, twist feedback pin reset to 26.\nPlease adjust your hardware accordingly.");
    }
    setRestartRequired();
    postAndValidatePinoutSettings(defaultDebounce, postCommonSettings);
}
function updateFeedbackTwist() {
    var checked = document.getElementById('feedbackTwist').checked;
    userSettings["feedbackTwist"] = checked;
    toggleFeedbackTwistSettings(checked);
    setRestartRequired();
    updateUserSettings();
}

function toggleFeedbackTwistSettings(feedbackChecked) {
    var feedbackTwistOnly = document.getElementsByClassName('feedbackTwistOnly');
    for(var i=0;i < feedbackTwistOnly.length; i++)
        feedbackTwistOnly[i].style.display = feedbackChecked ? "flex" : "none";
        
    if(feedbackChecked && !isTCodeV3()) {
        document.getElementById("analogTwistRow").style.display = 'none';
    }
}
function updateHostName() 
{
    if(hostnameTimeout !== null) 
    {
        clearTimeout(hostnameTimeout);
    }
    hostnameTimeout = setTimeout(() => 
    {
        clearErrors("hostnameValidation");
        let value = document.getElementById('hostname').value;
        if(isValidHostName(value)) 
        {
            wifiSettings["hostname"] = value;
            setRestartRequired();
            postWifiSettings(0);
        } else {
            var errorString = "<div name='hostnameValidation'>Invalid hostname</div>";
            
            errorString += "</div>";
            showError(errorString);
        }
        hostnameTimeout = null;
    }, defaultDebounce);
}

function updateFriendlyName() 
{
    wifiSettings["friendlyName"] = document.getElementById('friendlyName').value;
    setRestartRequired();
    postWifiSettings();
}
function setupBoardTypes() {
    const boardTypeElement = document.getElementById('boardType');
    removeAllChildren(boardTypeElement);
    for(let i=0;i<systemInfo.boardTypes.length;i++) {
        const boardTypeOption = document.createElement("option");
        boardTypeOption.innerText = systemInfo.boardTypes[i].name;
        boardTypeOption.value = systemInfo.boardTypes[i].value;
        boardTypeElement.appendChild(boardTypeOption);
        BoardType[systemInfo.boardTypes[i].name] = systemInfo.boardTypes[i].value;
    }
}
function setEncoderType() {
    userSettings["BLDC_Encoder"] = document.getElementById('BLDC_Encoder').value;
    toggleBLDCEncoderOptions();
    updateUserSettings(0);
}
function setupEncoderTypes() {
    const element = document.getElementById('BLDC_Encoder');
    removeAllChildren(element);
    for(let i=0;i<systemInfo.encoderTypes.length;i++) {
        const option = document.createElement("option");
        option.innerText = systemInfo.encoderTypes[i].name;
        option.value = systemInfo.encoderTypes[i].value;
        element.appendChild(option);
        BLDCEncoderType[systemInfo.encoderTypes[i].name] = systemInfo.encoderTypes[i].value;
    }
}
function setBoardType() {
    var element = document.getElementById('boardType');
    if(confirm("This will reset the current pinout to default and you will need to restart the device. Continue?")) {
        userSettings["boardType"] = parseInt(element.value);
        const isSSR1PCB = isBoardType(BoardType.SSR1PCB);
        document.getElementById("deviceType").disabled = isBoardType(BoardType.CRIMZZON) || isBoardType(BoardType.ISAAC) || isSSR1PCB;
        document.getElementById("BLDC_Encoder").disabled = isSSR1PCB;
        postBoardType(userSettings["boardType"]);
    } else {
        element.value = userSettings["boardType"];
    }
}
function setupDeviceTypes() {
    const element = document.getElementById('deviceType');
    removeAllChildren(element);
    for(let i=0;i<systemInfo.deviceTypes.length;i++) {
        const option = document.createElement("option");
        option.innerText = systemInfo.deviceTypes[i].name;
        option.value = systemInfo.deviceTypes[i].value;
        element.appendChild(option);
        DeviceType[systemInfo.deviceTypes[i].name] = systemInfo.deviceTypes[i].value;
    }
}
function setDeviceType() {
    var element = document.getElementById('deviceType');
    let newValue = element.value;
    if(confirm("This will reset the current pinout to default. Continue?")) {
        //toggleDeviceOptions(userSettings["deviceType"]);
        //setupChannelSliders();
        //setRestartRequired();
        //updateUserSettings(1);
        postDeviceType(newValue);
    } else {
        element.value = userSettings["deviceType"];
    }
}

function setupTimerChannels() {
    const elements = document.getElementsByName('timerChannels');
    for (let index = 0; index < elements.length; index++) {
        const element = elements[index];
        removeAllChildren(element);
        for(let i=0;i<systemInfo.timerChannels.length;i++) {
            const option = document.createElement("option");
            option.innerText = systemInfo.timerChannels[i].name;
            option.value = systemInfo.timerChannels[i].value;
            element.appendChild(option);
        }
    }
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
function setInverseTwist() {
    userSettings["inverseTwist"] = document.getElementById('inverseTwist').checked;
	updateUserSettings();
}
function disablePinValidation() {
    if (!userSettings["disablePinValidation"] && confirm("This will disable ALL PIN validations.\nBe sure you know what you're doing!")) {
        userSettings["disablePinValidation"] = true;
    } else {
        userSettings["disablePinValidation"] = false;
        document.getElementById("disablePinValidation").checked = false;
    }
	updateUserSettings();
}

function setPinChannel(id, value) {
    let element = document.getElementById(id);
    element.value = value;
    toggleEnableTimerChannels(element);
}
function onSelectPinChannel(element) {
    pinoutSettings[element.id] = element.value;
    // let option = selectElement.options[selectElement.selectedIndex];
    // option.disabled = true
    toggleEnableTimerChannels(element);
    setRestartRequired();
	postPinoutSettings();
}
function toggleEnableTimerChannels(element) {
    const timerSelects = document.getElementsByName('timerChannels');
    for (let index = 0; index < timerSelects.length; index++) {
        const otherElement = timerSelects[index];
        if(element.id !== otherElement.id)
            otherElement.options[element.selectedIndex].disabled = element.value > -1;
    }
}
function updatePins() 
{
    if(systemInfo.motorType == MotorType.BLDC) {
        updateBLDCPins();
        return;
    }
    if(upDateTimeout !== null) 
    {
        clearTimeout(upDateTimeout);
    }
    upDateTimeout = setTimeout(() => 
    {
        var pinValues = validatePins();
        if(pinValues) {
            pinoutSettings["RightServo_PIN"] = pinValues.rightPin;
            pinoutSettings["LeftServo_PIN"] = pinValues.leftPin;
            pinoutSettings["RightUpperServo_PIN"] = pinValues.rightUpper;
            pinoutSettings["LeftUpperServo_PIN"] = pinValues.leftUpper;
            pinoutSettings["PitchLeftServo_PIN"] = pinValues.pitchLeft;
            pinoutSettings["PitchRightServo_PIN"] = pinValues.pitchRight;
            updateCommonPins(pinValues);
            setRestartRequired();
            postPinoutSettings(0);
        }
    }, defaultDebounce);
}

function updateCommonPins(pinValues) {
    pinoutSettings["TwistServo_PIN"] = pinValues.twistServo;
    pinoutSettings["ValveServo_PIN"] = pinValues.valveServo;
    pinoutSettings["Squeeze_PIN"] = pinValues.squeezeServo;
    pinoutSettings["Vibe0_PIN"] = pinValues.vibe0;
    pinoutSettings["Vibe1_PIN"] = pinValues.vibe1;
    pinoutSettings["Vibe2_PIN"] = pinValues.vibe2;
    pinoutSettings["Vibe3_PIN"] = pinValues.vibe3;
    pinoutSettings["LubeButton_PIN"] = pinValues.lubeButton;
    pinoutSettings["Heater_PIN"] = pinValues.heat;
    pinoutSettings["Case_Fan_PIN"] = pinValues.caseFanPin
    pinoutSettings["Temp_PIN"] = pinValues.temp;
    pinoutSettings["TwistFeedBack_PIN"] = pinValues.twistFeedBack;
    pinoutSettings["Internal_Temp_PIN"] = pinValues.internalTemp;
    pinoutSettings["i2cSda_PIN"] = pinValues.i2cSda;
    pinoutSettings["i2cScl_PIN"] = pinValues.i2cScl;
    //     pinoutSettings["Battery_Voltage_PIN"] = pinValues.Battery_Voltage_PIN;
    // }

}
// function updateNonPWMPins(assignedPins) {
//     var errors = [];
//     var invalidPins = [];
//     var lubeButton = parseInt(document.getElementById('LubeButton_PIN').value);
//     var pinDupeIndex = assignedPins.findIndex(x => x.pin === lubeButton);
//     if(validPWMpins.indexOf(lubeButton) == -1 || inputOnlypins.indexOf(lubeButton) == -1)
//         invalidPins.push("Invalid Lube button pin: "+lubeButton);
//     if(pinDupeIndex > -1)
//         errors.push("Button lube pin and "+assignedPins[pinDupeIndex].name);
//     assignedPins.push({name:"Button lube", pin:lubeButton});

//     var temp;
//     if(userSettings.tempSleeveEnabled) {
//         temp = parseInt(document.getElementById('Temp_PIN').value);
//         if(validPWMpins.indexOf(temp) == -1 || inputOnlypins.indexOf(temp) == -1)
//             invalidPins.push("Invalid Sleeve temp pin: "+temp);
//         pinDupeIndex = assignedPins.findIndex(x => x.pin === temp);
//         if(pinDupeIndex > -1)
//             errors.push("Temp pin and "+assignedPins[pinDupeIndex].name);
//         assignedPins.push({name:"Temp", pin:temp});
//     }
//     var internalTemp;
//     if(userSettings.tempInternalEnabled) {
//         internalTemp = parseInt(document.getElementById('Internal_Temp_PIN').value);
//         if(validPWMpins.indexOf(internalTemp) == -1 || inputOnlypins.indexOf(internalTemp) == -1)
//             invalidPins.push("Invalid Internal temp pin: "+internalTemp);
//         pinDupeIndex = assignedPins.findIndex(x => x.pin === internalTemp);
//         if(pinDupeIndex > -1)
//             errors.push("Internal temp pin and "+assignedPins[pinDupeIndex].name);
//         assignedPins.push({name:"Internal temp", pin:internalTemp});
//     }
    
//     var twistFeedBack
//     if(userSettings.feedbackTwist) {
//         twistFeedBack = parseInt(document.getElementById('TwistFeedBack_PIN').value);
//         if(validPWMpins.indexOf(twistFeedBack) == -1 || inputOnlypins.indexOf(twistFeedBack) == -1)
//             invalidPins.push("Invalid Twist feedback pin: "+twistFeedBack);
//         pinDupeIndex = assignedPins.findIndex(x => x.pin === twistFeedBack);
//         if(pinDupeIndex > -1)
//             errors.push("Twist feedback pin and "+assignedPins[pinDupeIndex].name);
//         assignedPins.push({name:"Twist feed back", pin:twistFeedBack});
//     }

//     var errors = [];
//     var invalidPins = [];
//     if(validateNonPWMPins(assignedPins, errors, invalidPins)) {
//         if(userSettings.tempSleeveEnabled) {
//             userSettings["LubeButton_PIN"] = lubeButton;
//             userSettings["Temp_PIN"] = temp;
//         }
//         if(userSettings.feedbackTwist) {
//             userSettings["TwistFeedBack_PIN"] = twistFeedBack;
//         }
//         if(userSettings.tempInternalEnabled) {
//             userSettings["Internal_Temp_PIN"] = internalTemp;
//         }
//     }
//     return { errors: errors, invalidPins: invalidPins } ;
// }

function setIntMax(id, max) {
    var max = document.getElementById(id);
    max.setAttribute('max', parseInt(max));
}
/** Sets two numeric int controls to not overlap */
function setIntMinAndMax(minID, maxID) {
    var min = document.getElementById(minID);
    var max = document.getElementById(maxID);
    setElementsIntMinAndMax(min, max);
}
/** Sets two numeric int controls to not overlap */
function setElementsIntMinAndMax(minElement, maxElement) {
    minElement.setAttribute('max', parseInt(maxElement.value) - 1);
    maxElement.setAttribute('min', parseInt(minElement.value) + 1);
}
/** additionalValidations takes a parameter with the value */
function validateIntControl(controlIDOrElement, settingsObject, settingVariableName, additionalValidations) {
    var control = controlIDOrElement;
    if(typeof controlIDOrElement === "string") {
        control = document.getElementById(controlIDOrElement);
    }
    clearErrors(control.id);
    if(additionalValidations && additionalValidations(control.value) || control.checkValidity()) {
        settingsObject[settingVariableName] = parseInt(control.value);
        return true;
    }
    var message = control.validationMessage;
    if(!message) {
        message = control.errorText;
    }
    showError(`<div name="${control.id}"> ${control.id} is invalid: ${message ??  ""}</div>`);
    return false;
}
/** additionalValidations takes a parameter with the value */
function validateFloatControl(controlIDOrElement, settingsObject, settingVariableName, additionalValidations) {
    var control = controlIDOrElement;
    if(typeof controlIDOrElement === "string") {
        control = document.getElementById(controlIDOrElement);
    }
    clearErrors(control.id);
    if(additionalValidations && additionalValidations(control.value) || control.checkValidity()) {
        settingsObject[settingVariableName] = parseFloat(control.value);
        return true;
    }
    var message = control.validationMessage;
    if(!message) {
        message = control.errorText;
    }
    showError(`<div name="${control.id}"> ${control.id} is invalid: ${message ??  ""}</div>`);
    return false;
}
/** additionalValidations takes a parameter with the value */
function validateStringControl(controlIDOrElement, settingsObject, settingVariableName, additionalValidations) {
    var control = controlIDOrElement;
    if(typeof controlIDOrElement === "string") {
        control = document.getElementById(controlIDOrElement);
    }
    clearErrors(control.id);
    if(additionalValidations && additionalValidations(control.value) || control.checkValidity()) {
        settingsObject[settingVariableName && settingVariableName.trim().length ? settingVariableName : controlID] = control.value;
        return true;
    }
    var message = control.validationMessage;
    if(!message) {
        message = control.errorText;
    }
    showError(`<div name="${control.id}"> ${control.id} is invalid: ${message ??  ""}</div>`);
    return false;
}

function validatePin(pin, pinName, assignedPins, duplicatePins, isInput, invalidPins) {
    if(pin > -1) {
        let pinDupeIndex = assignedPins.findIndex(x => x.pin === pin);
        if(pinDupeIndex > -1) {
            duplicatePins.push(pinName+" pin and "+assignedPins[pinDupeIndex].name);
        }
        if(isInput === true && inputOnlypins.indexOf(pin) == -1 && validPWMpins.indexOf(pin) == -1) {
            invalidPins.push(pinName+" pin: "+pin);
        }
        assignedPins.push({name:pinName, pin:pin});
    }
}
function validatePWMPin(pin, pinName, assignedPins, duplicatePins, pwmErrors) {
    if(pin > -1) {
        validatePin(pin, pinName, assignedPins, duplicatePins)
        if(validPWMpins.indexOf(pin) == -1)
            pwmErrors.push(pinName+" pin: "+pin);
    }
}
/** 
 * Validates the pin number values in the forms inputs. 
 * Shows an error and returns the pin values or undefined if error
*/
function validatePins() {
    if(systemInfo.motorType == MotorType.BLDC) {
        return validateBLDCPins();
    }
    clearErrors("pinValidation"); 
    var assignedPins = [];
    var duplicatePins = [];
    var pwmErrors = [];
    var pinValues = getServoPinValues();
    if(userSettings["disablePinValidation"])
        return pinValues;

    validatePWMPin(pinValues.rightPin, "Right servo", assignedPins, duplicatePins, pwmErrors);

    // OSR / SR6
    validatePWMPin(pinValues.leftPin, "Left servo", assignedPins, duplicatePins, pwmErrors);
    validatePWMPin(pinValues.pitchLeft, "Pitch left servo", assignedPins, duplicatePins, pwmErrors);

    // SR6
    validatePWMPin(pinValues.rightUpper, "Right upper servo", assignedPins, duplicatePins, pwmErrors);
    validatePWMPin(pinValues.leftUpper, "Left upper servo", assignedPins, duplicatePins, pwmErrors);
    validatePWMPin(pinValues.pitchRight, "Pitch right servo", assignedPins, duplicatePins, pwmErrors);

    validateCommonPWMPins(assignedPins, duplicatePins, pinValues, pwmErrors);

    var invalidPins = [];
    validateNonPWMPins(assignedPins, duplicatePins, invalidPins, pinValues);

    if (duplicatePins.length || pwmErrors.length || invalidPins.length) {
        var errorString = "<div name='pinValidation'>Pins NOT saved due to invalid input.<br>";
        if(duplicatePins.length )
            errorString += "<div style='margin-left: 25px;'>The following pins are duplicated:<br><div style='color: white; margin-left: 25px;'>"+duplicatePins.join("<br>")+"</div></div>";
        if(invalidPins.length) {
            if(duplicatePins.length)
                errorString += "<br>";
            errorString += "<div style='margin-left: 25px;'>The following pins are invalid:<br><div style='color: white; margin-left: 25px;'>"+invalidPins.join("<br>")+"</div></div>";
        }
        if (pwmErrors.length) {
            if(duplicatePins.length || invalidPins.length) {
                errorString += "<br>";
            } 
            errorString += "<div style='margin-left: 25px;'>The following pins are invalid PWM pins:<br><div style='color: white; margin-left: 25px;'>"+pwmErrors.join("<br>")+"</div></div>";
        }
        
        errorString += "</div>";
        showError(errorString);
        return undefined;
    }
    return pinValues;
}


function validateCommonPWMPins(assignedPins, duplicatePins, pinValues, pwmErrors) {
    validatePWMPin(pinValues.twistServo, "Twist servo", assignedPins, duplicatePins, pwmErrors);
    validatePWMPin(pinValues.squeezeServo, "Squeeze servo", assignedPins, duplicatePins, pwmErrors);
    validatePWMPin(pinValues.valveServo, "Valve servo", assignedPins, duplicatePins, pwmErrors);
    validatePWMPin(pinValues.vibe0, "Vibe 1", assignedPins, duplicatePins, pwmErrors);
    validatePWMPin(pinValues.vibe1, "Vibe 2", assignedPins, duplicatePins, pwmErrors);
    validatePWMPin(pinValues.vibe2, "Vibe 3", assignedPins, duplicatePins, pwmErrors);
    validatePWMPin(pinValues.vibe3, "Vibe 4", assignedPins, duplicatePins, pwmErrors);

    if(userSettings.tempSleeveEnabled) {
        validatePWMPin(pinValues.heat, "Heater", assignedPins, duplicatePins, pwmErrors);
    }
    
    if(userSettings.tempInternalEnabled) {
        validatePWMPin(pinValues.caseFanPin, "Case fan ", assignedPins, duplicatePins, pwmErrors);
    }
}
/** Does not show an error. Just returns true/false */
function validateNonPWMPins(assignedPins, duplicatePins, invalidPins, pinValues) {
    if(userSettings.displayEnabled || userSettings.voiceEnabled || userSettings.batteryLevelEnabled) {
        let enabledValues = []; 
        if(userSettings.displayEnabled) {
            enabledValues.push("Display");
        }
        if(userSettings.voiceEnabled) {
            enabledValues.push("Voice");
        }
        if(userSettings.batteryLevelEnabled) {
            enabledValues.push("Battery level");
        }
        if(pinValues.i2cScl < 0) {
            invalidPins.push("I2C SCL pin: "+pinValues.i2cScl + " and I2C modules enabled: " + enabledValues.join(","));
        }
        if(pinValues.i2cSda < 0) {
            invalidPins.push("I2C SDA pin: "+pinValues.i2cSda + " and I2C modules enabled: " + enabledValues.join(","));
        }
        validatePin(pinValues.i2cSda, "I2C SDA", assignedPins, duplicatePins);
        validatePin(pinValues.i2cScl, "I2C SCL", assignedPins, duplicatePins);
    } else if(isMotorType(MotorType.BLDC) && isBLDCSPI()) {
        if(pinValues.i2cScl < 0) {
            invalidPins.push("I2C SCL pin: "+pinValues.i2cScl);
        }
        if(pinValues.i2cSda < 0) {
            invalidPins.push("I2C SDA pin: "+pinValues.i2cSda);
        }
        validatePin(pinValues.i2cSda, "I2C SDA", assignedPins, duplicatePins);
        validatePin(pinValues.i2cScl, "I2C SCL", assignedPins, duplicatePins);
    }
    
    validatePin(pinValues.lubeButton, "Lube button", assignedPins, duplicatePins, true, invalidPins);

    if(userSettings.tempSleeveEnabled) {
        validatePin(pinValues.temp, "Temp", assignedPins, duplicatePins, true, invalidPins);
    }
    if(userSettings.tempInternalEnabled) {
        validatePin(pinValues.internalTemp, "Internal temp", assignedPins, duplicatePins, true, invalidPins);
    }

    // if(userSettings.batteryLevelEnabled) {
        // if(adc1Pins.indexOf(pinValues.Battery_Voltage_PIN) == -1) 
        //     invalidPins.push("Battery voltage pin: "+pinValues.Battery_Voltage_PIN + " is not a valid adc1 pin.");
        // pinDupeIndex = assignedPins.findIndex(x => x.pin === pinValues.Battery_Voltage_PIN);
        // if(pinDupeIndex > -1)
        //     duplicatePins.push("Battery voltage pin and "+assignedPins[pinDupeIndex].name);
        // assignedPins.push({name:"Battery voltage", pin:pinValues.Battery_Voltage_PIN});
    // }
    
    if(userSettings.feedbackTwist && pinValues.twistFeedBack) {
        validatePin(pinValues.twistFeedBack, "Twist feedback", assignedPins, duplicatePins, true, invalidPins);
    }
    
    if(Buttons.isButtonSetsEnabled()) {
        for(var i=0; i<pinValues.buttonSets.length; i++) {
            var buttonSetPin = pinValues.buttonSets[i];
            validatePin(buttonSetPin, "Button set "+i, assignedPins, duplicatePins, true, invalidPins);
        }
    }

    if(duplicatePins.length || invalidPins.length) 
        return false;
    return true;
}

function getServoPinValues() {
    var pinValues = {};
    pinValues.rightPin = parseInt(document.getElementById('RightServo_PIN').value);
    pinValues.leftPin = parseInt(document.getElementById('LeftServo_PIN').value);
    pinValues.rightUpper = parseInt(document.getElementById('RightUpperServo_PIN').value);
    pinValues.leftUpper = parseInt(document.getElementById('LeftUpperServo_PIN').value);
    pinValues.pitchRight = parseInt(document.getElementById('PitchRightServo_PIN').value);
    pinValues.pitchLeft = parseInt(document.getElementById('PitchLeftServo_PIN').value);
    getCommonPinValues(pinValues);
    return pinValues;
}

function getCommonPinValues(pinValues) {
    pinValues.twistServo = parseInt(document.getElementById('TwistServo_PIN').value);
    pinValues.squeezeServo = parseInt(document.getElementById('Squeeze_PIN').value);
    pinValues.valveServo = parseInt(document.getElementById('ValveServo_PIN').value);
    pinValues.vibe0 = parseInt(document.getElementById('Vibe0_PIN').value);
    pinValues.vibe1 = parseInt(document.getElementById('Vibe1_PIN').value);
    pinValues.vibe2 = parseInt(document.getElementById('Vibe2_PIN').value);
    pinValues.vibe3 = parseInt(document.getElementById('Vibe3_PIN').value);
    
    //pinValues.Battery_Voltage_PIN = parseInt(document.getElementById('Battery_Voltage_PIN').value);

    pinValues.heat = parseInt(document.getElementById('Heater_PIN').value);

    pinValues.caseFanPin = parseInt(document.getElementById('Case_Fan_PIN').value);

    pinValues.i2cSda = parseInt(document.getElementById('i2cSda_PIN').value);
    pinValues.i2cScl = parseInt(document.getElementById('i2cScl_PIN').value);

    pinValues.lubeButton = parseInt(document.getElementById('LubeButton_PIN').value);
    pinValues.temp = parseInt(document.getElementById('Temp_PIN').value);
    pinValues.internalTemp = parseInt(document.getElementById('Internal_Temp_PIN').value);
    pinValues.twistFeedBack = parseInt(document.getElementById('TwistFeedBack_PIN').value);

    var buttonSetPins = document.getElementsByName('buttonSetPins');
    pinValues.buttonSets = [];
    buttonSetPins.forEach((node, index) => {
        pinValues.buttonSets[index] = parseInt(document.getElementById('buttonSetPin'+index).value);;
    });
}

function updateZeros() 
{
    if(upDateTimeout !== null) 
    {
        clearTimeout(upDateTimeout);
    }
    upDateTimeout = setTimeout(() => 
    {
        const maxZero = 2500;
        const minZero = 500;
        clearErrors("zeroValidation"); 
        var validValue = true;
        var invalidValues = [];
        var RightServo_ZERO = parseInt(document.getElementById('RightServo_ZERO').value);
        if(!RightServo_ZERO || RightServo_ZERO > maxZero || RightServo_ZERO < minZero)
        {
            validValue = false;
            invalidValues.push("Right servo ZERO")
        }
        var LeftServo_ZERO = parseInt(document.getElementById('LeftServo_ZERO').value);
        if(!LeftServo_ZERO || LeftServo_ZERO > maxZero || LeftServo_ZERO < minZero)
        {
            validValue = false;
            invalidValues.push("Left servo ZERO")
        }
        var RightUpperServo_ZERO = parseInt(document.getElementById('RightUpperServo_ZERO').value);
        if(!RightUpperServo_ZERO || RightUpperServo_ZERO > maxZero || RightUpperServo_ZERO < minZero)
        {
            validValue = false;
            invalidValues.push("Right upper servo ZERO")
        }
        var LeftUpperServo_ZERO = parseInt(document.getElementById('LeftUpperServo_ZERO').value);
        if(!LeftUpperServo_ZERO || LeftUpperServo_ZERO > maxZero || LeftUpperServo_ZERO < minZero)
        {
            validValue = false;
            invalidValues.push("Left upper servo ZERO")
        }
        var PitchLeftServo_ZERO = parseInt(document.getElementById('PitchLeftServo_ZERO').value);
        if(!PitchLeftServo_ZERO || PitchLeftServo_ZERO > maxZero || PitchLeftServo_ZERO < minZero)
        {
            validValue = false;
            invalidValues.push("Pitch left servo ZERO")
        }
        var PitchRightServo_ZERO = parseInt(document.getElementById('PitchRightServo_ZERO').value);
        if(!PitchRightServo_ZERO || PitchRightServo_ZERO > maxZero || PitchRightServo_ZERO < minZero)
        {
            validValue = false;
            invalidValues.push("Pitch right servo ZERO")
        }
        var ValveServo_ZERO = parseInt(document.getElementById('ValveServo_ZERO').value);
        if(!ValveServo_ZERO || ValveServo_ZERO > maxZero || ValveServo_ZERO < minZero)
        {
            validValue = false;
            invalidValues.push("Valve servo ZERO")
        }
        var TwistServo_ZERO = parseInt(document.getElementById('TwistServo_ZERO').value);
        if(!TwistServo_ZERO || TwistServo_ZERO > maxZero || TwistServo_ZERO < minZero)
        {
            validValue = false;
            invalidValues.push("Twist servo ZERO")
        }
        var Squeeze_ZERO = parseInt(document.getElementById('Squeeze_ZERO').value);
        if(!Squeeze_ZERO || Squeeze_ZERO > maxZero || Squeeze_ZERO < minZero)
        {
            validValue = false;
            invalidValues.push("Squeeze servo ZERO")
        }

        if(validValue)
        {
            userSettings["RightServo_ZERO"] = RightServo_ZERO;
            userSettings["LeftServo_ZERO"] = LeftServo_ZERO;
            userSettings["RightUpperServo_ZERO"] = RightUpperServo_ZERO;
            userSettings["LeftUpperServo_ZERO"] = LeftUpperServo_ZERO;
            userSettings["PitchLeftServo_ZERO"] = PitchLeftServo_ZERO;
            userSettings["PitchRightServo_ZERO"] = PitchRightServo_ZERO;
            userSettings["ValveServo_ZERO"] = ValveServo_ZERO;
            userSettings["TwistServo_ZERO"] = TwistServo_ZERO;
            updateUserSettings();
        }
        else
        {
            showError("<div name='zeroValidation'>Zeros NOT saved due to invalid input.<br><div style='margin-left: 25px;'>The values should be between "+minZero+" and "+maxZero+" for the following:<br><div style='color: white; margin-left: 25px;'>"+invalidValues.join("<br>")+"</div></div></div>");
        }
    }, 2000);
}
function updateLubeAmount()
{
    userSettings["lubeAmount"] = parseInt(document.getElementById('lubeAmount').value);
    updateUserSettings();
}
function setDiplayIs32Px() {
    var heightControl = document.getElementById('Display_Screen_Height');
    if(document.getElementById('displayIs32Px').checked) {
        heightControl.value = 32;
    } else {
        heightControl.value = 64;
    }
    userSettings["Display_Screen_Height"] = parseInt(heightControl.value);
    setRestartRequired();
    updateUserSettings();
}
function setDisplaySettings()
{
    const enabled = document.getElementById('displayEnabled').checked;
    if(!userSettings["displayEnabled"] && enabled) 
        validatePins();
    userSettings["displayEnabled"] = enabled;
    // userSettings["Display_Screen_Width"] = parseInt(document.getElementById('Display_Screen_Width').value);
    // userSettings["Display_Screen_Height"] = parseInt(document.getElementById('Display_Screen_Height').value);

    // pinoutSettings["Display_Rst_PIN"] = parseInt(document.getElementById('Display_Rst_PIN').value);
    userSettings["Display_I2C_Address"] = document.getElementById('Display_I2C_Address_text').value;
    userSettings["sleeveTempDisplayed"] = document.getElementById('sleeveTempDisplayed').checked;
    userSettings["internalTempDisplayed"] = document.getElementById('internalTempDisplayed').checked;
    userSettings["versionDisplayed"] = document.getElementById('versionDisplayed').checked;
    setRestartRequired();
    if(userSettings["displayEnabled"]) 
        validatePins();
    updateUserSettings();
}
function setDisplayAddress() {
    var selectValue = document.getElementById('Display_I2C_Address').value;
    document.getElementById('Display_I2C_Address_text').value = selectValue;
    userSettings["Display_I2C_Address"] = selectValue;
    setRestartRequired();
    updateUserSettings();
}
function setTempSettings() {
    const enabled = document.getElementById('tempSleeveEnabled').checked;
    if(!userSettings["tempSleeveEnabled"] && enabled) 
        validatePins();
    userSettings["tempSleeveEnabled"] = enabled;
    userSettings["TargetTemp"] = parseFloat(document.getElementById('TargetTemp').value);
    userSettings["HeatPWM"] = parseInt(document.getElementById('HeatPWM').value);
    userSettings["HoldPWM"] = parseInt(document.getElementById('HoldPWM').value);
    userSettings["heaterThreshold"] = parseInt(document.getElementById('heaterThreshold').value);
    userSettings["heaterResolution"] = parseInt(document.getElementById('heaterResolution').value);

    Utils.toggleControlVisibilityByID('sleeveTempDisplayedRow', hasFeature(BuildFeature.TEMP) && userSettings["tempSleeveEnabled"]);
    setRestartRequired();
    updateUserSettings();
}
function setInternalTempSettings() {
    userSettings["tempInternalEnabled"] = document.getElementById('tempInternalEnabled').checked;
    userSettings["caseFanResolution"] = parseInt(document.getElementById('caseFanResolution').value);

    Utils.toggleControlVisibilityByID('internalTempDisplayedRow', hasFeature(BuildFeature.TEMP) && userSettings["tempInternalEnabled"]);
    setRestartRequired();
    updateUserSettings();
}

function toggleVoiceSettings() {
    userSettings['voiceEnabled'] = document.getElementById('voiceEnabled').checked;  
    setRestartRequired();
    if(userSettings["voiceEnabled"]) 
        validatePins();
    updateUserSettings();
}
function setVoiceSettings() {
    userSettings['voiceMuted'] = document.getElementById('voiceMuted').checked;  
    if(validateIntControl("voiceVolume", userSettings, "voiceVolume") && validateIntControl("voiceWakeTime", userSettings, "voiceWakeTime")) {
        setRestartRequired();
        updateUserSettings(0);
    }
}
function setFanControl() {
    userSettings["fanControlEnabled"] = document.getElementById('fanControlEnabled').checked;
    toggleFanControlSettings(userSettings["fanControlEnabled"]);
    
    setRestartRequired();
    if(userSettings["fanControlEnabled"]) 
        validatePins();
    updateUserSettings();
}

function setFanOnTemp() {
    userSettings["internalTempForFan"] = parseFloat(document.getElementById('internalTempForFan').value);
    updateUserSettings();
}
function setMaxTemp() {
    userSettings["internalMaxTemp"] = parseFloat(document.getElementById('internalMaxTemp').value);
    updateUserSettings();
}
function setBuildFanControl(enabled) {
    var elements = document.getElementsByClassName('fan-control-build');
    for(var i=0;i < elements.length; i++)
        Utils.toggleElementShown(elements[i],  enabled);
    toggleFanControlSettings(enabled);
}
function toggleFanControlSettings(enabled) {
    var elements = document.getElementsByClassName('fan-control');
    for(var i=0;i < elements.length; i++)
        Utils.toggleElementShown(elements[i], enabled);
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

function updateWifiLoginSettings() {
    wifiSettings["ssid"] = document.getElementById('ssid').value;
    wifiSettings["wifiPass"] = document.getElementById('wifiPass').value;
    setRestartRequired();
    postWifiSettings();
}

function updateWifiSettings() {
    if(staticIPAddressTimeout !== null) 
    {
        clearTimeout(staticIPAddressTimeout);
    }
    var staticIP = document.getElementById('staticIP').checked;
    toggleStaticIPSettings(staticIP);
    staticIPAddressTimeout = setTimeout(() => 
    {
        var ips = {};
        ips.localIP = document.getElementById('localIPInput').value;
        ips.gateway = document.getElementById('gatewayInput').value;
        ips.subnet = document.getElementById('subnetInput').value;
        ips.dns1 = document.getElementById('dns1Input').value;
        ips.dns2 = document.getElementById('dns2Input').value;
        if(validateStaticIPAddresses(ips)) {
            wifiSettings["staticIP"] = staticIP;
            wifiSettings["localIP"] = ips.localIP;
            wifiSettings["gateway"] = ips.gateway;
            wifiSettings["subnet"] = ips.subnet;
            wifiSettings["dns1"] = ips.dns1;
            wifiSettings["dns2"] = ips.dns2;
            setRestartRequired();
            postWifiSettings(0);
        }
    }, defaultDebounce);
}

function validateStaticIPAddresses(ips) {
    var invalidIPS = [];
    clearErrors("staticIPValidation");
    if(!isValidIP(ips.localIP)) {
        invalidIPS.push("Invalid static IP address");
    }
    if(!isValidIP(ips.gateway)) {
        invalidIPS.push("Invalid static gateway address");
    }
    if(!isValidIP(ips.subnet)) {
        invalidIPS.push("Invalid static subnet address");
    }
    if(ips.dns1 && ips.dns1.length > 0 && !isValidIP(ips.dns1)) {
        invalidIPS.push("Invalid static dns1 address");
    }
    if(ips.dns1 && ips.dns2.length > 0 && !isValidIP(ips.dns1)) {
        invalidIPS.push("Invalid static dns2 address");
    }
    if(invalidIPS.length) {
        var errorString = "<div name='staticIPValidation'>Static Ip settings not saved due to invalid IP addresses<br>";
            errorString += "<div style='margin-left: 25px;'>The following addreses are invalid:<br><div style='color: white; margin-left: 25px;'>"+invalidIPS.join("<br>")+"</div></div>"
        
        errorString += "</div>";
        showError(errorString);
        return false;
    }
    return true;
}

function toggleStaticIPSettings(isStatic)
{
    Utils.toggleControlVisibilityByID('localIP', isStatic);
    Utils.toggleControlVisibilityByID('gateway', isStatic);
    Utils.toggleControlVisibilityByID('subnet', isStatic);
    Utils.toggleControlVisibilityByID('dns1', isStatic);
    Utils.toggleControlVisibilityByID('dns2', isStatic);
}
function toggleDeviceOptions(deviceType)
{
    var osrOnly = document.getElementsByClassName('osrOnly');
    var sr6Only = document.getElementsByClassName('sr6Only');
    for(var i=0;i < sr6Only.length; i++)
        sr6Only[i].style.display = deviceType == DeviceType.SR6 && deviceType != DeviceType.SSR1 ? "flex" : "none";
    for(var i=0;i < osrOnly.length; i++)
        osrOnly[i].style.display = deviceType == DeviceType.OSR && deviceType != DeviceType.SSR1 ? "flex" : "none";
}

function toggleNonTCodev3Options()
{
    var v2Only = document.getElementsByClassName('v2Only');
    var v3Only = document.getElementsByClassName('v3Only');
    for(var i=0;i < v3Only.length; i++)
        v3Only[i].style.display = isTCodeV3() ? "flex" : "none";
    for(var i=0;i < v2Only.length; i++)
        v2Only[i].style.display = isTCodeV3() ? "none" : "flex";
    toggleFeedbackTwistSettings(userSettings.feedbackTwist);
    if(userSettings.feedbackTwist && !isTCodeV3()) {
        document.getElementById("analogTwistRow").style.display = 'none';
    }
}

function updateBlueToothSettings()
{
    const element = document.getElementById('bluetoothEnabled');
    let value = element.checked;
    if(value && systemInfo["moduleType"] == ModuleType.WROOM32) {
        if(wifiSettings["bleEnabled"]) {
            alert("BLE and Bluetooth classic cannot be enabled at the same time due to ram constraints.")
            element.checked = false;
            return;
        }
        let message = "This will disable this web server. The Wroom32 chip does not have enough memory for bluetooth and the web server.\n\nYou will need to disable bluetooth to see this web page again.\n\nMost settings can be configured via tcode command #setting.\nUse the tcode command #help for more information\n\nUDP TCode will probably still work.\n\nYuu will be able to continue configuration on this page until you reboot the board.";
        if(confirm(message+"\n\nContinue?")) {
            wifiSettings["bluetoothEnabled"] = value;
            setRestartRequired();
            postWifiSettings();
        } else {
            element.checked = false;
        }
    } else {
        wifiSettings["bluetoothEnabled"] = value;
        setRestartRequired();
        postWifiSettings();
    }
}
function toggleBluetoothSettings() {
    
    if(!hasFeature(BuildFeature.BLUETOOTH)) {
        const elements = document.getElementsByClassName("bluetoothOnly")
        for(var i=0;i < elements.length; i++){
            elements[i].classList.add("hidden");
        };
    }
}
function updateBleSettings()
{
    const element = document.getElementById('bleEnabled');
    let value = element.checked;
    if(value && systemInfo["moduleType"] == ModuleType.WROOM32) {
        if(wifiSettings["bluetoothEnabled"]) {
            alert("BLE and Bluetooth classic cannot be enabled at the same time due to ram constraints.")
            element.checked = false;
            return;
        }
        let message = "This will disable this web server. The Wroom32 chip does not have enough memory for bluetooth and the web server.\n\nYou will need to disable bluetooth to see this web page again.\n\nMost settings can be configured via tcode command #setting.\nUse the tcode command #help for more information\n\nUDP TCode will probably still work.\n\nYuu will be able to continue configuration on this page until you reboot the board.";
        if(confirm(message+"\n\nContinue?")) {
            wifiSettings["bleEnabled"] = value;
            setRestartRequired();
            postWifiSettings();
        } else {
            element.checked = false;
        }
    } else {
        wifiSettings["bleEnabled"] = value;
        setRestartRequired();
        postWifiSettings();
    }
    toggleBLEDeviceTypes(wifiSettings["bleEnabled"]);
}

function toggleBLESettings() {
    if(!hasFeature(BuildFeature.BLE)) {
        const elements = document.getElementsByClassName("BLEOnly")
        for(var i=0;i < elements.length; i++){
            elements[i].classList.add("hidden");
        };
        return;
    }
}

function toggleBLEDeviceTypes() {
    const elements = document.getElementsByClassName("BLEEnabled")
    
    for(var i=0;i < elements.length; i++){
        if(wifiSettings["bleEnabled"] && hasFeature(BuildFeature.BLE))
            elements[i].classList.remove("hidden");
        else
            elements[i].classList.add("hidden");
    };
    
}

function setBLEDeviceType() {
    wifiSettings["bleDeviceType"] = parseInt(document.getElementById('bleDeviceType').value);
    toggleBLELoveDeviceTypes();
    setRestartRequired();
    postWifiSettings();
}

function toggleBLELoveDeviceTypes() {
    const elements = document.getElementsByClassName("BLELoveOnly");
    for(var i=0;i < elements.length; i++){
        if(wifiSettings["bleDeviceType"] == BLEDeviceType.LOVE && hasFeature(BuildFeature.BLE))
            elements[i].classList.remove("hidden");
        else
            elements[i].classList.add("hidden");
    };
}
function setBLELoveDeviceType() {
    wifiSettings["bleLoveDeviceType"] = parseInt(document.getElementById('bleLoveDeviceType').value);
    setRestartRequired();
    postWifiSettings();
}

function setTCodeVersion() 
{
    userSettings["TCodeVersion"] = parseInt(document.getElementById('TCodeVersion').value);
    toggleNonTCodev3Options()
    // setupChannelSliders();
	setRestartRequired();
	updateUserSettings();
}

function setRestartRequired() {
    if (documentLoaded) {
        restartRequired = true;
    }
}

function updateVibTimeout() {
    userSettings["vibTimeout"] = parseInt(document.getElementById('vibTimeout').value);
    userSettings["vibTimeoutEnabled"] = document.getElementById('vibTimeoutEnabled').checked;
    
    updateUserSettings();
}
function updateLubeEnabled() {
    userSettings["lubeEnabled"] = document.getElementById('lubeEnabled').checked;
    
    setRestartRequired();
    updateUserSettings();
}

function toggleSounds() {
	playSounds = document.getElementById('soundsEnabled').checked;
    var testSoundButtons = document.getElementsByName("testSoundButton");
    testSoundButtons.forEach(element => {
        Utils.toggleElementShown(element, playSounds);
    });
}

function exportToJsonFile() {
    alert("Wifi password will NOT be exported!");
    const userSettingsCopy = JSON.parse(JSON.stringify(userSettings));
    userSettingsCopy["esp32VersionNum"] = systemInfo.esp32VersionNum;
    //userSettingsCopy["wifiPass"] = "YOUR PASSWORD HERE";
    userSettingsCopy["wifiSettings"] = wifiSettings;
    //userSettingsCopy["wifiSettings"].wifiPass = "YOUR PASSWORD HERE";
    userSettingsCopy["motionDefaultProfileIndex"] = motionProviderSettings.motionDefaultProfileIndex;
    userSettingsCopy["motionProfiles"] = motionProviderSettings.motionProfiles;
    userSettingsCopy["buttonSettings"] = buttonSettings;
    userSettingsCopy["pinoutSettings"] = pinoutSettings;
    userSettingsCopy["channelsProfileSettings"] = channelsProfileSettings;
    
    let dataStr = JSON.stringify(userSettingsCopy);
    let dataUri = 'data:application/json;charset=utf-8,'+ encodeURIComponent(dataStr);

    let exportFileDefaultName = 'userSettings_'+systemInfo.esp32Version+'.json';

    let linkElement = document.createElement('a');
    linkElement.setAttribute('href', dataUri);
    linkElement.setAttribute('download', exportFileDefaultName);
    linkElement.click();
}

function createImportSettingsInputElement() {
    importSettingsInputElement = document.createElement("input");
    importSettingsInputElement.type = "file";
    importSettingsInputElement.accept = "application/json,.json";
    importSettingsInputElement.addEventListener("change", importSettings)
}
function openImportSettingsFileDialog() {  
    importSettingsInputElement.dispatchEvent(new MouseEvent("click")); 
}
function importSettings() {
    alert("Wifi password will NOT be imported!");
    if(importSettingsInputElement.files.length > 0) {
        var json = importSettingsInputElement.files[0];
        var reader = new FileReader();
        reader.addEventListener("load", () => {
            var text = reader.result;
            var json = JSON.parse(text);
            Object.keys(json).forEach(function(key,index) {
                if(key === "wifiPass")
                    return;
                var importedValue = json[key];
                var existingValue = userSettings[key];
                var wifiValue = wifiSettings[key];

                // If the key doesnt exist anymore, dont import it.
                if(existingValue != undefined && existingValue != null)// 0 can be valid
                    userSettings[key] = checkMigrateData(key, importedValue, json["esp32VersionNum"]);
                else if(wifiValue != undefined && wifiValue != null)// Not used
                    wifiSettings[key] = checkMigrateData(key, importedValue, json["esp32VersionNum"]);
                else
                    handleImportRenames(key, importedValue, json["esp32VersionNum"])
            });
            // If the new build doesnt have the old TCode version in it, set it to the latest version.
            if(tcodeVersions.findIndex(x => x.value == userSettings.TCodeVersion) == -1) {
                userSettings.TCodeVersion = latestTCodeVersion;
                document.getElementById('TCodeVersion').value = userSettings["TCodeVersion"];
                toggleNonTCodev3Options();
            }
            setWifiSettings();
            setUserSettings();
            setPinoutSettings();
            setRestartRequired();
            updateALLUserSettings();
        }, false);

        reader.readAsText(json);
    }
}

function checkMigrateData(key, value, firmwareVersion) {
    if(!firmwareVersion && key == "TCodeVersion" && value == 1) {
        return TCodeVersion.V3;
    } else if(key == "boardType") { 
        if(!firmwareVersion || firmwareVersion < 0.39) {
            if(value == 1) {
                return BoardType.CRIMZZON; 
            } else if(value == 2) {
                return BoardType.ISAAC; 
            }
        }
    }
    return value; 
}

function handleImportRenames(key, value, firmwareVersion) {
    switch(key) {
        case "BLDC_MotorA_Voltage":
        userSettings.BLDC_MotorA_VoltageLimit = value;
        return;
        case "LubeManual_PIN": 
        pinoutSettings.LubeButton_PIN = value;
        return;
        case "motionProfiles": 
        motionProviderSettings.motionProfiles = value;
        motionProviderSettings.motionProfiles.forEach(x => {
            x.edited = true;
            x.channels.forEach(y => y.edited = true);
        });
        return;
        case "motionDefaultProfileIndex": 
        motionProviderSettings.motionDefaultProfileIndex = value;
        return;
        case "wifiSettings": 
        Object.keys(value).forEach(function(key,index) {
            if(key !== "wifiPass")
                wifiSettings[key] = value[key];
        });
        return;
        case "ssid": 
        wifiSettings.ssid = value;
        return;
        case "staticIP": 
        wifiSettings.staticIP = value;
        return;
        case "localIP": 
        wifiSettings.localIP = value;
        return;
        case "gateway": 
        wifiSettings.gateway = value;
        return;
        case "subnet": 
        wifiSettings.subnet = value;
        return;
        case "dns1": 
        wifiSettings.dns1 = value;
        return;
        case "dns2": 
        wifiSettings.dns2 = value;
        return;
        case "udpServerPort": 
        wifiSettings.udpServerPort = value;
        return;
        case "webServerPort": 
        wifiSettings.webServerPort = value;
        return;
        case "hostname": 
        wifiSettings.hostname = value;
        return;
        case "friendlyName": 
        wifiSettings.friendlyName = value;
        return;
        case "bootButtonCommand":
        buttonSettings.bootButtonCommand = value;
        return;
        case "buttonSettings": 
        buttonSettings = value;
        // buttonSettings = {...buttonSettings, ...value}; // Merge objects? test this when adding property or removing?
        return;
        case "pinoutSettings":
        Object.keys(value).forEach(function(key,index) {
            pinoutSettings[key] = value[key];
        });
        return;
        case "channelsProfileSettings":
            channelsProfileSettings = value;
        return;
        case "sr6Mode":
        if(isMotorType(MotorType.BLDC))
            userSettings.deviceType = DeviceType.SSR1
        else
            userSettings.deviceType = value ? DeviceType.SR6 : DeviceType.OSR;
        return;
        case "BLDC_UsePWM": 
            userSettings.BLDC_Encoder = value ? BLDCEncoderType.PWM : BLDCEncoderType.SPI;
            return;
        case "BLDC_UseMT6701": 
            userSettings.BLDC_Encoder = value ? BLDCEncoderType.MT6701 : BLDCEncoderType.SPI;
            return;
        case "msPerRad":
            if(value == 425)// 425 is msPerRad for 270 servo.
                userSettings.maxServoRange = 270
            return;
    }
    if(key.endsWith("_PIN")) {
        pinoutSettings[key] = value;
    }
}
