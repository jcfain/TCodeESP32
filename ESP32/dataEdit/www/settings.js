/* MIT License

Copyright (c) 2023 Jason C. Fain

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
var systemInfo = {};
var motionProviderSettings = {};
var upDateTimeout;
var restartRequired = false;
var documentLoaded = false;
var infoNode;
var debugEnabled = true;
var playSounds = false;
var importSettingsInputElement;
var websocket;
const EndPointType = {
    Common: {uri: "/settings"},
    MotionProfile: {uri: "/motionProfiles"}
}
const TCodeVersion = {
    V2: 0,
    V3: 1
}
// Modified in toggleBuildOptions if TCode V2 is not in build
const availableVersions = [
    {version: TCodeVersion.V2, versionName: "v0.2"},
    {version: TCodeVersion.V3, versionName: "v0.3"},
] 
const latestTCodeVersion = TCodeVersion.V3;
const LogLevel = {
    ERROR: 0,
    WARNING: 1,
    INFO: 2,
    DEBUG: 3,
    VERBOSE: 4
};
const BoardType = {
    DEVKIT: 0,
    CRIMZZON: 1,
    ISAAC: 2
}
const BuildFeature = {
    NONE: 0,
    DEBUG: 1,
    WIFI: 2,
    BLUETOOTH: 3,
    DA: 4,
    DISPLAY_: 5,
    TEMP: 6,
    HAS_TCODE_V2: 7,
    HTTPS: 8
}
const MotorType = {
    Servo: 0,
    BLDC: 1
};
const servoDegreeValue180 = 637; 
const servoDegreeValue270 = 425; 
dubugMessages = [];

var testDeviceUseIModifier = false;
var testDeviceDisableModifier = false;
var testDeviceModifierValue = "1000";
var restartClicked = false;
var serverPollingTimeOut = null;
var websocketRetryCount = 0;
var channelSliderList = [];
var restartingAndChangingAddress = false;
var startUpHostName;
var startUpWebPort;
var startUpStaticIP;
var startUpLocalIP;

//PWM availible on: 2,4,5,12-19,21-23,25-27,32-33
const validPWMpins = [2,4,5,12,13,14,15,16,17,18,19,21,22,23,25,26,27,32,33];
const inputOnlypins = [34,35,36,39];
const adc1Pins = [36,37,38,39,32,33,34,35];
const adc2Pins = [4,0,2,15,13,12,14,27,25,26];

const AvailibleChannelsV2 = [
    {channel: "L0", channelName: "Stroke", switch: false, sr6Only: false},
    {channel: "L1", channelName: "Surge", switch: false, sr6Only: true},
    {channel: "L2", channelName: "Sway", switch: false, sr6Only: true},
    {channel: "L3", channelName: "Suck", switch: false, sr6Only: false},
    {channel: "R0", channelName: "Twist", switch: false, sr6Only: false},
    {channel: "R1", channelName: "Roll", switch: false, sr6Only: false},
    {channel: "R2", channelName: "Pitch", switch: false, sr6Only: false},
    {channel: "V0", channelName: "Vibe 0", switch: true, sr6Only: false},
    {channel: "V1", channelName: "Vibe 1/Lube", switch: true, sr6Only: false}
]
const AvailibleChannelsV3 = [
    {channel: "L0", channelName: "Stroke", switch: false, sr6Only: false},
    {channel: "L1", channelName: "Surge", switch: false, sr6Only: true},
    {channel: "L2", channelName: "Sway", switch: false, sr6Only: true},
    {channel: "R0", channelName: "Twist", switch: false, sr6Only: false},
    {channel: "R1", channelName: "Roll", switch: false, sr6Only: false},
    {channel: "R2", channelName: "Pitch", switch: false, sr6Only: false},
    {channel: "V0", channelName: "Vibe 1", switch: true, sr6Only: false},
    {channel: "V1", channelName: "Vibe 2", switch: true, sr6Only: false},
    {channel: "V2", channelName: "Vibe 3", switch: true, sr6Only: false},
    {channel: "V3", channelName: "Vibe 4", switch: true, sr6Only: false},
    {channel: "A0", channelName: "Suck manual", switch: false, sr6Only: false},
    {channel: "A1", channelName: "Suck level", switch: false, sr6Only: false},
    {channel: "A2", channelName: "Lube", switch: true, sr6Only: false},
    {channel: "A3", channelName: "Auxiliary", switch: false, sr6Only: false}
]
const AvailibleChannelsBLDC = [
    {channel: "L0", channelName: "Stroke", switch: false, sr6Only: false},
    {channel: "R0", channelName: "Twist", switch: false, sr6Only: false},
    {channel: "V0", channelName: "Vibe 1", switch: true, sr6Only: false},
    {channel: "V1", channelName: "Vibe 2", switch: true, sr6Only: false},
    {channel: "V2", channelName: "Vibe 3", switch: true, sr6Only: false},
    {channel: "V3", channelName: "Vibe 4", switch: true, sr6Only: false},
    {channel: "A0", channelName: "Suck manual", switch: false, sr6Only: false},
    {channel: "A1", channelName: "Suck level", switch: false, sr6Only: false},
    {channel: "A2", channelName: "Lube", switch: true, sr6Only: false},
    {channel: "A3", channelName: "Auxiliary", switch: false, sr6Only: false}
]

document.addEventListener("DOMContentLoaded", function() {
    onDocumentLoad();
});

function logdebug(message) {
    if(debugEnabled)
        console.log(message);
}
function onDocumentLoad() {
	infoNode = document.getElementById('info');
    getSystemInfo();
    createImportSettingsInputElement();
    
    // debugTextElement = document.getElementById("debugText");
    // debugTextElement.scrollTop = debugTextElement.scrollHeight;
}

function getSystemInfo() {
	var xhr = new XMLHttpRequest();
	xhr.open('GET', "/systemInfo", true);
	xhr.responseType = 'json';
	xhr.onload = function() {
        var status = xhr.status;
        if (status !== 200) {
			showError("Error loading system info!");
		} else {
            systemInfo = xhr.response;
            setSystemInfo();
            getUserSettings();
		}
	};
	xhr.send();
}
function getUserSettings() {
	var xhr = new XMLHttpRequest();
	xhr.open('GET', EndPointType.Common.uri, true);
	xhr.responseType = 'json';
	xhr.onload = function() {
        var status = xhr.status;
        if (status !== 200) {
			showError("Error loading user settings!");
		} else {
            userSettings = xhr.response;
            if(!userSettings || !userSettings["TCodeVersion"]) {
                showError("Error getting user settings!");
                return;
            }
            setUserSettings()
            getMotionProviderSettings();
		}
	};
	xhr.send();
}

function getMotionProviderSettings() {
	var xhr = new XMLHttpRequest();
	xhr.open('GET', EndPointType.MotionProfile.uri, true);
	xhr.responseType = 'json';
	xhr.onload = function() {
        var status = xhr.status;
        if (status !== 200) {
			showError("Error loading motion profile settings!");
		} else {
            motionProviderSettings = xhr.response;
            if(!motionProviderSettings || !motionProviderSettings["motionProfiles"]) {
                showError("Error getting motion provider settings!");
                return;
            }
            setupChannelSliders();
            initWebSocket();
            documentLoaded = true;
		}
	};
	xhr.send();
}

function initWebSocket() {
	try {
		var wsUri = (hasFeature(BuildFeature.HTTPS) ? "wss://" : "ws://") + window.location.host + "/ws";
		if (typeof MozWebSocket == 'function')
			WebSocket = MozWebSocket;
		if ( websocket && websocket.readyState == 1 )
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
            websocketRetryCount = 0;
			//updateSettingsUI();
		};
		websocket.onclose = function (evt) {
			logdebug("DISCONNECTED");
            
            if(!serverPollingTimeOut && !restartingAndChangingAddress) {
                showLoading("Server disconnected, waiting for restart...");
                checkForServer();
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
                checkForServer();
            }
			//alert('ERROR: ' + evt.data + ", Address: "+wsUri);
			//xtpConnected = false;
		};
	} catch (exception) {
        if(!serverPollingTimeOut && !restartingAndChangingAddress) {
            showLoading("Server exception, waiting for restart...");
            checkForServer();
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

function setDebug() {
    userSettings["logLevel"] = parseInt(document.getElementById('debug').value);

    const selectedIncludes = document.querySelectorAll('#log-include-tags option:checked');
    userSettings["log-include-tags"] =  Array.from(selectedIncludes).map(el => el.value);

    const selectedExcludes = document.querySelectorAll('#log-exclude-tags option:checked');
    userSettings["log-exclude-tags"] = Array.from(selectedExcludes).map(el => el.value);

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
//         var snd = new Audio("data:audio/mp3;base64,SUQzAwAAAAAfdlRJVDIAAAAbAAAAYmFkIGFuc3dlciAwMSBzb3VuZCBlZmZlY3RUUEUxAAAAFAAAAGZyZWVzb3VuZGVmZmVjdC5uZXRUQUxCAAAAFAAAAGZyZWVzb3VuZGVmZmVjdC5uZXRUWUVSAAAABQAAADIwMTZUQ09OAAAABQAAAFJvY2tDT01NAAAADwAAAGVuZwBleGNlbGxlbnQhVFJDSwAAAAYAAAAwNC8xNgAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAP/7UAQAAAEXD1aFMGACIuHq0KYMAEWIf6O5M4AQsQ/0dyZwAn0gIAAAQJjJLEsGgiHlTszMz/3AwMDAwMWaAYGBh/gAe/AQ+kBAAACBMZJYlg0EQ8qdmZmf+4GBgYGBizQDAwMP8AD34COBwOBwOBwOBwAAAAAAAFsAADXXIoM74n0cXgJGfB842/cXFDf9z1PG//nk6OBwOBwOBwOBwAAAAAAAFsAADXXIoM74n0cXgJGfB842/cXFDf9z1PG//nk6FQEB0xduBwAAACUwzeT/+1IEBoABYwpfPjxAACzBy7PHiAAFeHGbuBSAGK8K8rcEYALYwHI/pS+2TEDOtVroXLTIf0jyZJepBXzD333Z1Nn9ZQltAAAAAAXrGOYtQWCOJU1iuxjMYM01V7oDubY5xSaR5MkvUUK35h59oVu3AAAAOi0b7YfgAAAAAANUc/+OTM/o22kSv/oneQDRdBH/sN1D7mS//7wDDHoAAACgkF1tGwwAAAAABmsDrSf7ySP56dHG/9gtZAULIZ/0ExAlzMCOYEm/QgAAFlr3LAAAAP/7UgQEgAFTFd/WMGAEKyPsTcQMAMVAW5m4FAAQnwhtAzCQAAAAC+bkRcC3rI16xTea0na9RIFXXQgHVauiJgOJSVWxooAAASUmm3JbsAAAAAAAWeYwPWlug71j16hQKv6EA9LLJJgMq0plP+HZhiCgAAAxax6PwMBwOBwAACjRr/JcYqdH+Gg9Otk/0q9yxg77ijfp/+X/8PlwJhlplGGIFYoGL3gpdqQrlXL2T7S1icchwyaDyH/uZ18mMxC5D7ijaoBQNBsMAAAqhnS7Mv////tSBAcAAVkraB4IRIQthQxTxAgAhZhVh7hlgBCvivH3BLAC///zrCr/5JGRv/zjHMcwJHN8oIqhOSBpSvhQTA2wKBWLRaAADpYhWHHdDlf/hvX///9Vl/+EcIMCG//jA8DzLfiLE5IGqvqEx7ioAABAiTTQbEgAAAAAAA80W4B0xNHEkIx2/CAjus5Jef5ns41aO/9c4SeHuL0AAAICtxxiQXYYAAAAACyg4DI5ZmJ8ICO6zmXn+Z7ONWj/9dizKevGnhRLuioAAABwWDUbj8D/+1IEBIABTBzmbgSgBisCvG3CoACFiFd7WPEAGLEK7IMykAEAAAAAApR1y+hqXAoHYpeAAGAiFKniYqLoIiowrfHkmaAAAILEYjEotAAAAAAACSiDCcdPQ1tjkivhCH9Kv4oIwpC1H+5oWDTOBCSgAAFVq5KAAAAAAADPfl5jDmuqaqpkSud+nrFMoAxQEvaJKU2tsSoKKG4f82xWFgYG1ZsYeTSw4l/RYouwNKvTF6aHK7otNWXAwwQgihuHJMMxZarb8P2bBQAAAgKBgMAABv/7UgQEgAFIDuRuIOAUKmJsPcRAAIV8ZXdY8QAQq4iwdx5SQgOBwAACKLwy88Wav1P4ZEv40LnO///0lP+fEo//jDGcAAACVovm2+AAAAAAAAEAcD+JXfGMv5j7IIiijj2xQA0iCI7qqPuAE3c5IAAAnH8ggEAAAAABNZBNn5OKxmRCcw908GLjklLkGYTXpuybe0SUcX7Vs/qAAAAAIbFgwHAAAAAAADphBHk+rOzKAsMt/E7DF/IxS7EIBfjmhMjlxdn0VQAACRG7JZtgAAAA//tSBAWAAVMP3u4xgAYq4mvdxDQAhXBDh7i1kFCyiHB3HoIKAAAAntd0RaGDXdlIn7TTi6Zd30Fiy+TLh8qr/OX1WsAAAAIBg+NxwAAAAAAAAOxZoAVAaFVUJT3pQ7R7eEoOQk+zzAljU30zbOUAAAAKDo0FA4AAAAAAAASLzTBDUDy7x8ZbfVXSd5HNEyo9+BGHk2Z0GvgMUAAAABCgoF44AAAAAAAFuVbBydtRoxt9CHl4P8u2bxOYWU34C4NRbwCahLgNigAAAASXHdPwAAD/+1IEBQABXhRb7jxABCujTH/HiSDFVFFvuPOAEKuNL3caoEMAAAALqkW1kHqS1qPLnK+tmmo21i3MZ8fAlpyiCDZa8PgQAAAEAMhUydWgcAAAAAAABvVItry28aeXr7Zpq3KwtzGffAilTnEEYv+LOsAAAAExKBsPwAAAAAAAOCcua4y4z9YdRsa7+n7wdDXfBoaGP4vLlgv5o+sAAAABgSjgcAAAAAAAAekFfGIs88ZBGfl9DwKwifikbFl/AvE4TBb/5cwfARAwFAFwSAbM4P/7UgQFAAFeF+V+NQwGKqLrXceIAMVoSY+4haCQkoruExKwAwAAAAAAAADSSiVowg518v0u7yvh+DQyv/DwF59tr1PiiCQAAAAiIBtuAAAAAAAABJXgtx5C4TsjOwwrYv31tc51bFgBPxAGc72suIEAAAECBYTjccAAAAAAAAHDjwEUNN8w8eZ72QhP9lASGBv/T7HYw+cpDCH8uAAAQKFheOBZBFwC4yzKmG3sfz0SAj+ygHiBv+ce80YfZ/nzghUAABAsOBgcDgAAAAAAA2PG//tSBAgAATcWXm4s4BQfYtvExZwChNRRdbjSghB/ie0TGHADlhuNn+UmvHpNXg5GxvzTf+Kx0gf9JlYAJCBYcD45seNLDcbH8ZSaw70mq2DkbG/NNnfxOOkDwAAGFXNoOBwAAAAAAAGYNli8bPKBu9ak3RpAgpMmBAOR/2D4u/wwAAAiZHcxxyIOeEotPS4+9Ncv9apBwQQ5EkBeh35AfD0AAQBEBWBoCZAAAAAAAUceRmSveAgf2i8WN8BMThZ7noo+FYMAQAwNwSYe6jAAAAD/+1IEGAABIhPddhVABiQCnG7DrQCEuEmp+HRIEJSHLrcW8gIAABw79K94iDfrMSY3PxgNlxNv5f5sbJsAFAAMAMwZAhv4AAAAAAAAAioPhc1pLoYb3eG8Iw8S//qBQEqHEQAAIhYPh+OAAAAAAAABRlzjl1bv0DRrfUajx/yTksZLZQQBDpUqAAMAcBkDumsHA4AAAAAAA8Iqf2BA3SBoU/t8FiJuf9XrleTFvkxwAADDonG44/AAAAAAAA8hGCKn+GD8QIGkP7fGiJ+TALM+3f/7UgQkgAE1EeJ+FSAkJSG7bcKkAISQJUTdgIAgmoQpa7AQBHk3tAD/gANlKRigkMmxO83y4nCeuZk1FVu54XgYGtMCyVDRcmr9m0AAFrKIAAINKxqGMCgqNQpzolJat69jdzDAMDWmGpUCYubUlPT/1wAArckjAAB0YFKeDF8DPA2SKoBEc1wpqDxWnMEdD7j1YAPgICxRQAANuQNgAA6MClPBi+BngbhzTCVoUy1bgHPOtrX/09/dFgLckalCxIM8acbCKIsDlllIj+zlPMmB//tSBC+JATMGVFEsEDwhoLqKJYIHhCxDSUekYrCUAit0kIhGiGs4Hapd1fnkfQAQCk7btrQALFkUGnGU1Qsgy4TCzUjBa5w5riyDuzSiu3XR1lkqAAG9yRsAApDqHr0NQKqDrZRqQyDybxsqKEuWn1u13qpr3PYzrAAABLjcjYAAeAkPXoagVUGJkCJrlk3rSBS0WK++avnRV/5i2oAAElyWShMMyizDvo2ibZmdBCEbbfgZykU73W3/LhyqrXnQAAFJSW7alBw7v91oTbMzoIT/+1IEPoERJgpTUYMZLCSAqm0lgwOEXHNNpgRHuIsKavRgiP7tt222jDxtw9CjKRQVrxKh9dEAEEpyyWtgAOCQeDN3imK2UzbETHhKMlkfsCo4JxwnAUBLvd136wAQUnbLbAAA4WGFB8lGynbtQzwEg/63dWPGs5T/e117u4q/q0AxYAglOSy2wGgc0emZWjBVFgNlue//UcKfio1uqPybtf/afVW9dAIBTdtssBoHOPUytGGofGlA9miwPio425tfBsBRThiXKVIEpNyXW2wAAf/7UgRNgREwGFNpARp8JoBafSDAAcSIJ0+hhET4h4Rp9DCMnowJZZSJl7HzOyppjfFiymjd2T+e7DqjU4kDtiwSk3JNbbAABjAlllImU7BptEeoQgEl/J1VGH/ZxkTQXkRk+UAABKbn/AREJK2Y4dF3FllDeV07wfSWMfY239v/+6tzx/P+vaCKjkw12kAABAKw1zeVc6uZndjIQ5NUSn/duv5QF0lElSjr91cAstySySMAAQcVaoCC6FWgAoXKJZ9flDj0YQoP3N/D+KdM1fdr//tSBFqBAR4e1GhhFEwjw9qNDCJthHQRP4MYQHiTDWo0MA5G94ClXn/AAsLhTmmrJQqoKKFDPO/8QySsPixuyjfz+leprdGr/g2nJbdv9trQALG2kMSFzCRR1gUUNfU5KHGFLKStou9ZBrcgL9BQAAamVTA0MVAGaiNJC5hpEEQMx8droS8DLUmrZkziRkmEOpPYBaYk2+2tgABgmeDxZiHnalY8owBExaxj3iwWNE2LqE8VaswiU9kaE2wJBZI2wADQsOEhZm9HgC0CoFZIk5z/+1IEaQARMwhRaGESjiTAyekkwwPElBFXoIhgMJCCJWS2GAQqETKGqrqVvU8uxtLSo4JtgSSxyNAAURk0zMhu19RaFi7joXXsIOWVTcdJiRcZGR3qyoYYgElkkdghdnyPv6CFsxHppJrBrN2HnNUoAH2Jpagkta4vWboEABhyf/AGf/X9UWURuk4TFJ9wsCZxEiyuSDAzxOVDwwIfsGgAIASWSSMADq9SUtWVIFzRYQrdGhwPjFk2l0LLLcAAM4e/cugkDAFtlkbAApgAPBFtTv/7UgR1gBE3AM7oIggIJWA6LQRDAYR8OUOggEhwkYtotCAJTgDL8SD7HHQlbqVTP1b9c2Z37bsQ5sKOyOKfQ2DDzdNgRc7y9vrjdnUXaPW9u3vPRuE9YRx9FkEAAJ/+wD9OTVWHUKQPQtRwGzwscCyxYwx4RfIXEiA6fLMEgwYBqgAAXq6AJa3Xz7zikh25lKfOvqi8tyugJ87lh46H70kwndaCay7qwiplAJeokf8n9xEclk+qUNYbZ6LCpwimmyy/bDrFnymkV/6NDEoskjYA//tSBIGIkR0iz+AhG3wmgBmdACMBBCRPRaAEbviGhyX0AImVFwsj/NdhakmVeZ1Yx2La5uy3WeplbpGGvcUSTdShAAAou0tsgAEHqpGjYBp/eCMinPrKs+EEr4YXa3/o/+3PH7u2gdOAAj/36BU6971R0Y5HZj8aDZ3WuhUTrDo5Aka3hCpjjixWB5D6QJpqAPX97QAPaAkCRAcaeikjFjsDpjwG9RF17CLlQi0DU6gJbJJNStyKw8bYXh5nmHAMvB6g4g4miTCPsr923aUAAAT/+1IEkYiBMABKSCEaYCZEKSkAIxoEkMEnIARpaIwTqLQAiZ62ySNgAfBDAk6i8o/bnsi6byz1K08oIAlVHBpnIxjW2qAAAG3+21rAGY50AYuYT35JlS62akm7yTBxwst441kq2MsGs00/X/tBkpBFAjUTNHRd8Yw8UQdYkroEBuo2ZSDrCIALqKhUrAAbfqKnj5c8POCyC0hi6J8slQ0Ve9YeGSQSmDyxY+caHQAAG1H/0AfiriaHPIpQtCGqOpnSImWqQeAbGskCzScNjUQb/f/7UgSeCJExHVNoARreJSOpSQQCCARQASUghElAdxFotACN95QoAACrA/m3f9X22cpNMrzJe3TtdOiwcGdXcm+9pnT6MAfCWt/E/oADEklkbT6E2SKx9FugTczIh4lMSLDT6BYylTFgBSIdRPfFwAILaNZY2AAuwpGI0gecR30JTYu5MWZaoZhAeOuF5Y+16zDwL+xKAACbcsiSQACzZeoZo2dHDaELzhQ3Ygm6MRcyaZHGWoZHzPrMf2596MZhwxji3HOGuoAADcl1jbZADd63//tSBK6PgRwfUWgDEywmI+qtACJlg+ADRAAEYDCNgCUkEIgEHInXKDBemIyscum5Iamfc7cy1kcmK5Loql6Q1eHijJtInTJayajPU3bW5I0AJ5TttCSx8cnldu5IKd1ptmWNRphK3T3hINzT44uwkWmyyEevTvUkrdlbkcs5JJM3KNli4lpvs4WH8RuSRQE3baJEWw+xDBRNGfnOLEvxf7sP++oAEAOOONJEAfvVQ1iSlx8r7D+TibkW6oJjTZoL+ULzjaSdsp31HQdJfx0uDDX/+1IEvwEBLgBL4AEQCCVgCSgEIydEZFk/oARncJmNaTQAjO7prP+c0UACAHHHG0gAP1jOTU7D6mycUb2QUivWA6mXBPaUmASAnAi+IA6HeqdF7G1LKMOv/9tspO1xxoASm5GkNo4oCDM4w4Y2LpsGyopLjYRsXlHP2jtR2Z93NERjIPSYvlKdwirI5ikUpyWttzkrK4MlqCVCkg41KnwqDz1fz3LGp6F66UEtUnCRXd/HCRt/bXsOvn/S+61/duoBACS667OIAepmts9sS0d1Xv/7UgTLCJGMK0roARoIMiVZfQBjTQWYOzGgBErorYdl9ACNXWtdESTIwZVMphwIBew17lTM795qVWL09xFRQMYNF2rJRoToSAgBJNLZVCAPdCZpfXFLURm7tEc3t2IEcf7oFYpSY2ziEk+d4ZSngvEDkbQkGyTzmZvZLIAQYlu90kaAHrrljPdcQOdVM2Q0EwfFgGbCK6+zWITlL3Q1gTEQbDNz8j2CWv/HXb3UI0y4cAAAKTn18Bmn3zcdJtixi2fJRZOKo1J8lFxE9zXO/Q3Z//tSBMOIkZMYyuggGaoxYpldBAM1RmzBJ6AEakDElOV0AI1NyzA7wmPp7sXNhKqP4yvN0y85NvsAAq4240AC5YL92pIk8TbbesGY+T3PmJK2J7OzlIvFMzb6ev1L1kYIKqCjM26Rn0iNfJ3odRL4fkACgpNY5GyAL29kQ6qeSBhkkmtH4bcQ3l3bsscUQu20qsXzGZCx7sb4Mau6Ie7R9HCNc3iOgBIKNtpxoAA4/aqvzmruftH1Qc896hI7mVzonIKDEIivWM2qzdhmppYD/2z/+1IEtYABpSvMaCAZKDPleX0EAyUGqI8xoIxNaNCR5LARiakbQ6EHkW9J8kAEgpJI3GgACg8jMnSuyCCPV0NhYw0LSObiEI5riswUFu8TpEsZ9ApqhGlT+XCUzHmxbd/cjtoBCvmugAH9pLCiDsC92G6kX+VGFcxXZVQ/T/hH6JILOuB8yIUqfv8Jna45xB+gAFJZuQJrjetsvmSg/Pr5RTv7KEXGxB9lVGVn9iemHhquGZnJ/Evv/Cy//PveuP//2ABBdt5rZEAB+NuTk6ecj//7UgSjAAHDJklQIzMyNMRpbQCjLUaIsSugBGSg2RYldACMlYMjSm73V2MHjyzG35kHBdTiUUOR6xzzQ18JbHp7WhfD3KDmGBpgHozBJp4SJNukExuVacvZRRgLMD5QeKi8kcDAKseHHq17pKhNlfsqAAFtu1lraAEh7edPlMqqmeYO6rCPMxo/KusN9hZx4bD8jZcKUpaQZ4QcL9QA4APtjxTBpZWmdzjMRQgULBV5UyiIlXIfYCKiVqHj6IqTP84PeMKvrr0WVxcABKZqoB3///tSBI2AgWwMSkgBGSoxIrkZBAMyRahbNaAEZmC5j6PUEYqI/c8Py4YRJKj46Ll8Jrsvpve6/qX5pB8Tm7HO+uz2P7O+m9VLvd1bZpoBPsOB6dE5cEiYAGETotCw5JBZxSkOSrNvU9o0qqoRotgCswpYCpSc9NUCUqoADvdzo7zP6kflzSBn34nO0S7FYsbKZmA4frXO927rAPXq3//QAWIjSSIArF0jH5IawzgoxYUhMTHaRVQobJHiT0BZbky5RV6gOINrtQAqd9VQFcvcMZH/+1IEhoCBXx7SaAEaPC0iWPUEIzgFoAMlIAxgCLCAJKQQjSgoukqIEFjFRIwNF4c4QFzKIwcyuoMIFbE1StjJolKoAEnv6+A5THtigogsJ5UPxcRB8cqT4XFDSawmfWohD6RJbSPqtbMkZZUAAW3XWSNoAMa6caONGzyaTEmERtQxZ64+63GCrTpPfQkUQbuq7MgAALbdrLJEAPuepdIlKRdocD7K1qJrHHH/EhwGwRYcc4NhgSpDF9a4ZkDoz66ECJi0FyGXfGlKzt45hwVbRf/7UgSDgAE8B8iwIBmCKKI5OgAjLgUcAycgBEAAoQBlJAGMAJhZxCy68mSWdQljFo6wJpYAgyqlIlapKG5QNuHF3TzzITvayWkj0bTE16mkCyGVtFq/fb0KAAoottskYABLl+yNaiTI3YFJiw3t7/0zA6q63ybKpMVb35+X+xThDVGmE67I0jhsYAAmTUPGIdpHE1Pe3t0a69gsRoih0+xrtYD347jaSD1POMnBqkGrWGwqVcbCEWLyDpVaIEZWrUTW5V6nhqoEgBJKLrLI2gBd//tSBIkIATkSUegBGhwmwApdACM5hGADJMAEQACiAGPAEIwAKjfqNKvv8K5iG6xhyAISVMA/lFt45Uv3upCN9O5lawAAJLbHI0QB/IJCCi0q0+XbS2XODjLxjCJ4WUfTvZbtbhOj8wAeIHGr74bQF1lWlpwoyhk09yI9qGruRubRUxLUhTv83v/X0AAbf//ba2AC6LNTVXlVKVT02fXRNH6M9nWw6s8cdF0MzoO29n0/pVQKUpQlqnhoUlhVaj7wu8CpPF2rdJuNMDi86fHxSJ3/+1IEkoABMRJS6AEZ7iMACREEAAAExAMmoARAEJ8K6PQAjK6XgKR9mhSKAA3//+w1sAHic+KDqnrF0WUI/6lPpUTwSy06LouQYRZUqp4EIiq6gQWSaolFGPXvfrCEIIZajMizpNu1SmPTA95faHkIv/R1ADiCWbPGtzJGDz1JAzUoatpZLL9CioBUfWAwCYoihLJbvrXV3/SLdrJG0ALnxrRSu8IhLEJYKMKz9yEJtHPTNDjyUtWGcM7+6MppwC0BYDVjlNHlTZXXoe106+FE5f/7UgSdgIEjAFDoARpcI4AJBQQiAASwhVegBEbwnIAkmBCNKPdezW4va19tqGrv96xULgtMhEAxQBrYMBowBtRaHRrHw66dRoeg9a29D0Osc5NKS15n3Wi4AXHruLqAMXGJZsllrQk0lRUypDK7mLOzya91OpGcXQ+ru8NZsLThMKqTGurqOsZMXNDoaaP2JCKQhEJBxtlKlOFLdMiYNQAarWUAPtcQMzEmByOVFz7ZYsKFUYaaH1RiFpAlLG9UVY0CSoXVIgAD//fba2AD548T//tSBKmAgSAA1egAGAwkIAkABGJMBMABIKAAAACNiSj0AI2WNTdBhDHFhRxxxVbacGjgIrj0yCaGsvrY2KLqdoALjCY0rWk3ACVOVGgaLLCC2LE6D+973NaqfkWkzrmzQuBJg2pe4Lmo4tKIMNShY7HLsUfU0UIJM7nOPOYtRyJWCp5/73br3eulAszMgI9w+0LMiOA3mCY4JnwWpNpCpC0iLMpfUsXZO1EaFQEpVik70vGWBFsSQYcbYNCkQ0OQLE7ECraDtYwUbZTaWW4GVuf/+1IEtw+BBgBIACESUCfgCPAEIkoEJAEgAIRgAJSAZFQAAAAi62a2675HjGAGPPFmqLJGixJ2iu09XNgZhkAWpwGRdc97WbxXeiec1Por49oGeMOlayqBwqnUmpJFqXKMDRc0B7ENkD78w5h/GtzG54r9VSoADDf/7W2wAf9ayXJFCkKbxYi2QT68j/r8IngAY7NUn2UX705cxtmgkAAcYf/bW2ADxHfkybQ6BVOKxeEwohINqBzpohVxc/GJS6tfvWxbWyTwtQ4DLeC+0a5zhf/7UgTGAAE3AMpIAAAIJ6AarQAjAYTkASKghGSAiIAkABENME7F8CH6XRG6yeLqWLhoBBG9eRffdqdjqfqVhDZtB0go5UkilKxSpikA5mawysTjlqtpNidhWysbuJ6iZig/u2JqAAtl2skjaAEvS7nG3CjsCTj59oil9JRCS5vR+1C9m4qr0eKIAFGiSqAqrFCKkC8OC51BRSzs5uvcGBq71vexSpoVrQRDLCIe2v1VqDObvTeE8yp1FLLMqxrM5M0LscxxNaoJCgMipNo2SqVQ//tSBNCI8S4ASTAhEcAoAAjwBCI4BIgBHgCESUCUgCPAEI0obAO2SjmjdbMOt77Se8FnrqYABeaXR84W7HT1OKTUid8nOSoo3fn7PaFlSDQBVPtbIOZVEDxCJCbKgTZAItUCVcGg2i4awpGFBKBkrCp51R2nOHiCGPS+o2gmJ13yihQt0ANOg5/9Ii5hN45pc4BRcRAFAwJEn1nt8+dak+5DpcwmIFOy+vMw4vT9W646hvIRHSNHDnEAJDwbAzhETGvbhgShE7UxKBRF7mtjgfb/+1IE2wABRznU6CEUfCgACq0EIl2EjAEeAYRCQJwAZAQAiACKhYyKHhZNQfW1u779RTXNQHu/drz9WPwDlBfkp5Gn6jnDbf9fmt3fNthJLhEYxkuTxceS+IbRUde/dgAAkA4224gAYQh/1X1Zm4dVdVX/4ak2q8Y1KNV//jNxuMf7AQ86VBp89rAHqBHtEQNeDSgZBVYwGiwNSo0sPLHioSCoSBnWCtxV0ShI8MBU7O4if1hoSxAACWWWAwoIOIHEhgaJ+asDBAgcvyqn6aaKgP/7UgTjAAEgDFHoABgsJ8AZKQBiAAWwXRwAhGAAwJBk5ACMRNYwADi4WiSBQI8u39pqqqqrppppqutVTEFNRTMuOTguNFVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV//tSBOSIgUMAyKghGAAnIBjwBGMABZwBIKAMYAC2AGSkAIwBVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVX/+1IE5gzBVCpI6AEaMCtACMUEAAADcIS+YABnyFuN3EwAGT9VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVQ==");  
//         snd.play();
//     }
// }
function onDefaultClick() 
{		
	if (confirm("WARNING! Are you sure you wish to reset ALL settings?")) 
	{
		infoNode.innerText = "Resetting...";
		infoNode.style.color = 'white';
		infoNode.hidden = false;
		var xhr = new XMLHttpRequest();
		xhr.open("POST", "/default", true);
		xhr.onreadystatechange = function() 
		{
			if (xhr.readyState === 4) 
			{
                getSystemInfo();
				infoNode.innerText = "Settings reset!";
                infoNode.style.color = 'green';
                showRestartRequired();
				//document.getElementById('resetBtn').disabled = false ;
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
function setPinoutDefault(newBoardType) {
    infoNode.innerText = "Resetting pinout...";
    infoNode.style.color = 'white';
    infoNode.hidden = false;
    var xhr = new XMLHttpRequest();
    xhr.open("POST", "/pinoutDefault/"+newBoardType, true);
    xhr.onreadystatechange = function() 
    {
        if (xhr.readyState === 4) 
        {
            getUserSettings();
            infoNode.innerText = "Pinout reset!";
            infoNode.style.color = 'green';
            showRestartRequired();
            //document.getElementById('resetBtn').disabled = false ;
            setTimeout(() => 
            {
                infoNode.hidden = true;
                infoNode.innerText = "";
            }, 5000)
        }
    }
    xhr.send();
}
function onRestartClick() 
{		
    showLoading("Device restarting...")
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
                logdebug("Restart succeed!");
                if(response.apMode && userSettings.ssid !== "YOUR SSID HERE" && userSettings.wifiPass != "YOUR PASSWORD HERE") {
                    restartingAndChangingAddress = true;
                }
                
                if(restartingAndChangingAddress) {
                    var isIPStatic = userSettings["staticIP"];
                    var localIP = userSettings["localIP"];
                    var webServerPort = userSettings["webServerPort"];
                    var hostname = userSettings["hostname"];
                    var url = "http://"+hostname+".local";
                    url += webServerPort === 80 ? "" : ":"+webServerPort;
                    var staticIPUrl = "http://"+localIP;
                    staticIPUrl += webServerPort === 80 ? "" : ":"+webServerPort;
                    var message = "Device restarting, the page will redirect to<br><a href='"+url+"'>"+url+"</a> in 15 seconds<br>";
                    message += isIPStatic ? "If this doesnt work, you can try using this url<br><a href='"+staticIPUrl+"'>"+staticIPUrl+"</a> when the device reboots." 
                        : "If this doesnt work, you will need<br>to find the dymanic ip address of the esp32."
                    showLoading(message);
                    setTimeout(() => {
                        logdebug("Redirecting to: " + url)
                        window.location.href = url;
                    }, 15000);
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
    return websocket.readyState !== WebSocket.OPEN;
}
function checkForServer() {
    if(serverPollingTimeOut) {
        clearTimeout(serverPollingTimeOut);
        serverPollingTimeOut = null;
    }
    if(websocketRetryCount > 10) {
        showLoading("Websocket timed out. Please refresh the page for full functionality.");
        return;
    }
    if(isWebSocketConnected() && websocket.readyState !== WebSocket.CONNECTING) {
        logdebug("Websocket closed retrying..");
        initWebSocket();
        websocketRetryCount++;
        serverPollingTimeOut = setTimeout(checkForServer, 2000);
    } else if(isWebSocketConnected()) {
        logdebug("Websocket open..");
        if(serverPollingTimeOut) {
            clearTimeout(serverPollingTimeOut);
            serverPollingTimeOut = null;
        }
    }
}

function setSystemInfo() {
    if(!systemInfo)
        showError("Error getting system info!");
    document.getElementById('version').value = systemInfo.esp32Version;
    document.getElementById('ipAddressSystemInfo').value = systemInfo.localIP;
    document.getElementById('gatewaySystemInfo').value = systemInfo.gateway;
    document.getElementById('subnetSystemInfo').value = systemInfo.subnet;
    document.getElementById('dnsSystemInfo').value = systemInfo.dns1;
    document.getElementById('chipModel').value = systemInfo.chipModel;
    document.getElementById('chipRevision').value = systemInfo.chipRevision;
    document.getElementById('chipCores').value = systemInfo.chipCores;
    document.getElementById('chipID').value = systemInfo.chipID;

    var excludedTagsElement = document.getElementById('log-exclude-tags');
    systemInfo.availableTags.forEach(element => {
        var option = document.createElement("option");
        option.value = element;
        option.innerText = element;
        excludedTagsElement.appendChild(option);
    });

    var includedTagsElement = document.getElementById('log-include-tags');
    systemInfo.availableTags.forEach(element => {
        var option = document.createElement("option");
        option.value = element;
        option.innerText = element;
        includedTagsElement.appendChild(option);
    });
    
    var i2cAddressesElement = document.getElementById("Display_I2C_Address");
    systemInfo.systemI2CAddresses.forEach(element => {
        var option = document.createElement("option");
        option.value = element;
        option.innerText = element;
        i2cAddressesElement.appendChild(option);
    });
    
    setupBoardTypes();
    toggleBuildOptions();
    toggleMotorTypeOptions();
    
    //validPWMpins = [17,25,27];
    //validPWMpins = [2,4,5,12,13,14,15,17,21,22,25,27,32];

    document.getElementById('lastRebootReason').value = systemInfo.lastRebootReason;
}
function setUserSettings() 
{
    document.getElementById('TCodeVersion').value = userSettings["TCodeVersion"];
    toggleNonTCodev3Options();
    toggleDeviceOptions(userSettings["sr6Mode"]);
    toggleStaticIPSettings(userSettings["staticIP"]);
    togglePitchServoFrequency(userSettings["pitchFrequencyIsDifferent"]);
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

    document.getElementById("udpServerPort").value = userSettings["udpServerPort"];
    document.getElementById("webServerPort").value = userSettings["webServerPort"];
    startUpWebPort = userSettings["webServerPort"];
    document.getElementById("hostname").value = userSettings["hostname"];
    startUpHostName = userSettings["hostname"];
    document.getElementById("friendlyName").value = userSettings["friendlyName"];
	document.getElementById("servoFrequency").value = userSettings["servoFrequency"];
	document.getElementById("pitchFrequency").value = userSettings["pitchFrequency"];
	document.getElementById("valveFrequency").value = userSettings["valveFrequency"];
	document.getElementById("twistFrequency").value = userSettings["twistFrequency"];
    document.getElementById("squeezeFrequency").value = userSettings.squeezeFrequency,
	document.getElementById("msPerRad").value = userSettings["msPerRad"];
	
	document.getElementById("feedbackTwist").checked = userSettings["feedbackTwist"];
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
    document.getElementById("Vibe2_PIN").value = userSettings["Vibe2_PIN"];
    document.getElementById("Vibe3_PIN").value = userSettings["Vibe3_PIN"];
	document.getElementById("LubeButton_PIN").value = userSettings["LubeButton_PIN"];
	document.getElementById("Squeeze_PIN").value = userSettings["Squeeze_PIN"];

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
	document.getElementById("sr6Mode").checked = userSettings["sr6Mode"];
	document.getElementById("autoValve").checked = userSettings["autoValve"];
	document.getElementById("inverseValve").checked = userSettings["inverseValve"];
	document.getElementById("valveServo90Degrees").checked = userSettings["valveServo90Degrees"];
	document.getElementById("inverseStroke").checked = userSettings["inverseStroke"];
	document.getElementById("inversePitch").checked = userSettings["inversePitch"];

	document.getElementById("displayEnabled").checked = userSettings["displayEnabled"];
	document.getElementById("sleeveTempDisplayed").checked = userSettings["sleeveTempDisplayed"];
	document.getElementById("internalTempDisplayed").checked = userSettings["internalTempDisplayed"];
	document.getElementById("versionDisplayed").checked = userSettings["versionDisplayed"];
	document.getElementById("tempSleeveEnabled").checked = userSettings["tempSleeveEnabled"];
    document.getElementById('tempInternalEnabled').checked = userSettings["tempInternalEnabled"];
	document.getElementById("pitchFrequencyIsDifferent").checked = userSettings["pitchFrequencyIsDifferent"];
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
	// document.getElementById("Display_Rst_PIN").value = userSettings["Display_Rst_PIN"];
	document.getElementById("Temp_PIN").value = userSettings["Temp_PIN"];
	document.getElementById("Heater_PIN").value = userSettings["Heater_PIN"];
	// document.getElementById("heaterFailsafeTime").value = userSettings["heaterFailsafeTime"];
	document.getElementById("heaterThreshold").value = userSettings["heaterThreshold"];
	document.getElementById("heaterResolution").value = userSettings["heaterResolution"];
	document.getElementById("heaterFrequency").value = userSettings["heaterFrequency"];
	
    document.getElementById("ssid").value = userSettings["ssid"];
    document.getElementById("wifiPass").value = userSettings["wifiPass"];
    document.getElementById("staticIP").checked = userSettings["staticIP"];
    startUpStaticIP = userSettings["staticIP"];
    document.getElementById("localIPInput").value = userSettings["localIP"];
    startUpLocalIP = userSettings["localIP"];
    document.getElementById("gatewayInput").value = userSettings["gateway"];
    document.getElementById("subnetInput").value = userSettings["subnet"];
    document.getElementById("dns1Input").value = userSettings["dns1"];
    document.getElementById("dns2Input").value = userSettings["dns2"];
    //document.getElementById('bluetoothEnabled').checked = userSettings["bluetoothEnabled"];
    
	// document.getElementById("Display_Rst_PIN").readOnly = newtoungeHatExists;

	document.getElementById("Display_Screen_Width").readOnly = true;
	document.getElementById("Display_Screen_Height").readOnly = true;
	// document.getElementById("Display_Rst_PIN").readOnly = true;
    document.getElementById('Internal_Temp_PIN').value = userSettings["Internal_Temp_PIN"];
    document.getElementById('fanControlEnabled').checked = userSettings["fanControlEnabled"];
    document.getElementById('internalTempForFan').value = userSettings["internalTempForFan"];
    document.getElementById('internalMaxTemp').value = userSettings["internalMaxTemp"];
    document.getElementById('Case_Fan_PIN').value = userSettings["Case_Fan_PIN"];
    document.getElementById('caseFanResolution').value = userSettings["caseFanResolution"];
    document.getElementById('caseFanFrequency').value = userSettings["caseFanFrequency"];

    batterySetup();
    
    document.getElementById('debug').value = userSettings["logLevel"];

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

    if(!hasTCodeV2()) {
        availableVersions.splice(availableVersions.findIndex(x => x.version === TCodeVersion.V2), 1);
    }
    availableVersions.forEach(x => {
        const optionElement = document.createElement("option");
        optionElement.value=x.version;
        optionElement.innerText=x.versionName;
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

function updateUserSettings(debounceInMs, uri, objectToSave, callback) 
{
    if (documentLoaded) {
        if(debounceInMs == null || debounceInMs == undefined) {
            debounceInMs = 3000;
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
            if(startUpWebPort !== userSettings["webServerPort"] || 
                startUpHostName !== userSettings["hostname"] || 
                startUpStaticIP !== userSettings["staticIP"] ||
                startUpLocalIP !== userSettings["localIP"]) {
                restartingAndChangingAddress = true;
            }
            
            infoNode.hidden = false;
            infoNode.innerText = "Saving...";
            infoNode.style.color = 'white';
            var xhr = new XMLHttpRequest();
            var response = {};
            xhr.open("POST", uri, true);
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
                            infoNode.visibility = "visible";
                            infoNode.innerText = "Settings saved!";
                            infoNode.style.color = 'green';
                            setTimeout(() => 
                            {
                                infoNode.hidden = true;
                                infoNode.innerText = "";
                            }, 5000)
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

var updateMotionProfileSettings = function() {
    MotionGenerator.updateSettings(0);
}

function updateALLUserSettings() {
    updateUserSettings(0, EndPointType.Common.uri, userSettings, updateMotionProfileSettings)
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
    var errors = document.getElementsByName(name);
    for(var i=0;i<errors.length;i++) {
        errorText.removeChild(errors[i]);
    }
    if(errorText.innerText == "" && !errorText.firstChild) {
        closeError();
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

function sendWebsocketCommand(command, message) {
    websocket.send("{\"command\":\""+command+"\", \"message\": \""+message+"\"}")
}

function sendTCode(tcode) {
    websocket.send(tcode+String.fromCharCode(10))
}

function sendDeviceHome() {
    channelSliderList.forEach(x => x.value = x.channelModel.switch ? 0 : 50);
    var availibleChannels = getChannelMap();
    var tcode = "";
    availibleChannels.forEach((element, index, array) => {
        tcode += getSliderTCode(element.channel, element.switch ? 0 : 50, false, 1000, false);
        if (index !== array.length - 1){ 
            tcode += " ";
        }
    });
    sendTCode(tcode);
}

function getSliderTCode(channel, sliderValue, useIModifier, modifierValue, disableModifier) {
    var value = percentageToTcode(sliderValue);
    var tcode = channel + value.toString().padStart(isTCodeV3() ? 4 : 3, "0");
    if(!disableModifier) {
        tcode += useIModifier ? "I" : "S"
        tcode += modifierValue;
    }
    return tcode;
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
        if(!userSettings.sr6Mode && availibleChannels[i].sr6Only) {
            continue;
        }
        var channel = availibleChannels[i].channel;
        var channelName = availibleChannels[i].channelName;

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
        sliderNode.value = availibleChannels[i].switch ? 0 : 50;
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
    return isTCodeV3() ? 9999 : 999;
}
function getChannelMap() {
    return systemInfo["motorType"] == MotorType.Servo ? isTCodeV3() ? AvailibleChannelsV3 : AvailibleChannelsV2 : AvailibleChannelsBLDC;
};
function onChannelSliderInput(channel, value) {
    sendTCode(channel+value.toString().padStart(isTCodeV3() ? 4 : 3, "0") + "S1000");
}
function getTCodeMax() {
    return isTCodeV3() ? 9999 : 999
}
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
    userSettings["udpServerPort"] = parseInt(document.getElementById('udpServerPort').value);
    setRestartRequired();
    updateUserSettings();
}

function updateWebPort() {
    userSettings["webServerPort"] = parseInt(document.getElementById('webServerPort').value);
    setRestartRequired();
    updateUserSettings();
}

function setPitchFrequencyIsDifferent() {
    var isChecked = document.getElementById('pitchFrequencyIsDifferent').checked;
    userSettings["pitchFrequencyIsDifferent"] = isChecked;
    togglePitchServoFrequency(isChecked);
}

function updateServoFrequency() {
    var servoFrequencyControl = document.getElementById('servoFrequency');
    if(!servoFrequencyControl.checkValidity()) {
        showError(servoFrequencyControl.validationMessage);
    } else
        userSettings["servoFrequency"] = parseInt(servoFrequencyControl.value);
        
    var pitchFrequencyControl = document.getElementById('pitchFrequency');
    if(!pitchFrequencyControl.checkValidity()) {
        showError(pitchFrequencyControl.validationMessage);
    } else
        userSettings["pitchFrequency"] = parseInt(pitchFrequencyControl.value);

    var valveFrequencyControl = document.getElementById('valveFrequency');
    if(!valveFrequencyControl.checkValidity()) {
        showError(valveFrequencyControl.validationMessage);
    } else
        userSettings["valveFrequency"] = parseInt(valveFrequencyControl.value);

    var twistFrequencyControl = document.getElementById('twistFrequency');
    if(!twistFrequencyControl.checkValidity()) {
        showError(twistFrequencyControl.validationMessage);
    } else
        userSettings["twistFrequency"] = parseInt(twistFrequencyControl.value);
        var twistFrequencyControl = document.getElementById('twistFrequency');

    var squeezeFrequencyControl = document.getElementById('squeezeFrequency');
    if(!squeezeFrequencyControl.checkValidity()) {
        showError(squeezeFrequencyControl.validationMessage);
    } else
        userSettings["squeezeFrequency"] = parseInt(squeezeFrequencyControl.value);
    setRestartRequired();
    updateUserSettings();
}
function updateMSPerRad(userChecked) {
    var control = document.getElementById('msPerRad');
    if(!control.checkValidity()) {
        showError(control.validationMessage);
        return false;
    }
    userSettings["msPerRad"] = parseInt(control.value);
    if(!userChecked) {
        document.getElementById('msPerRadIs270').checked = userSettings["msPerRad"] == servoDegreeValue270;
    }
    setRestartRequired();
    updateUserSettings();
    return true;
}

function toggleMsPerRadIs270() {
    var control = document.getElementById('msPerRad');
    var msPerRadIs270Checkbox = document.getElementById('msPerRadIs270');
    const backupvalue = control.value;
    if(msPerRadIs270Checkbox.checked) {
        control.value = servoDegreeValue270;//270 degree servo.
    } else {
        control.value = servoDegreeValue180;
    }
    if(!updateMSPerRad(true)) {
        msPerRadIs270Checkbox.checked = !msPerRadIs270Checkbox.checked;
        control.value = backupvalue;
    }
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
        userSettings["TwistFeedBack_PIN"] = 32;
        //if(!newtoungeHatExists)
        alert("Note, twist feedback pin has been changed to analog input pin 32.\nPlease adjust your hardware accordingly.");
    } else {
        document.getElementById("TwistFeedBack_PIN").value = 26;
        userSettings["TwistFeedBack_PIN"] = 26;
        alert("Note, twist feedback pin reset to 26.\nPlease adjust your hardware accordingly.");
    }
    setRestartRequired();
    updateUserSettings();
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
    userSettings["hostname"] = document.getElementById('hostname').value;
    setRestartRequired();
    updateUserSettings();
}

function updateFriendlyName() 
{
    userSettings["friendlyName"] = document.getElementById('friendlyName').value;
    setRestartRequired();
    updateUserSettings();
}
function setupBoardTypes() {
    const boardTypeElement = document.getElementById('boardType');
    for(let i=0;i<systemInfo.boardTypes.length;i++) {
        const boardTypeOption = document.createElement("option");
        boardTypeOption.innerText = systemInfo.boardTypes[i].name;
        boardTypeOption.value = systemInfo.boardTypes[i].value;
        boardTypeElement.appendChild(boardTypeOption);
    }
}
function setBoardType() {
    var element = document.getElementById('boardType');
    var newBoardType = element.value;
    if(confirm("This will reset the current pinout to default. Continue?")) {
        setPinoutDefault(newBoardType);
    } else {
        element.value = userSettings["boardType"];
    }
}
function setSR6Mode() {
    userSettings["sr6Mode"] = document.getElementById('sr6Mode').checked;
    toggleDeviceOptions(userSettings["sr6Mode"]);
    setupChannelSliders();
    setRestartRequired();
	updateUserSettings(1);
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
function disablePinValidation() {
    if (!userSettings["disablePinValidation"] && confirm("This will disable ALL PIN validations.\nBe sure you know what you're doing!")) {
        userSettings["disablePinValidation"] = true;
    } else {
        userSettings["disablePinValidation"] = false;
        document.getElementById("disablePinValidation").checked = false;
    }
	updateUserSettings();
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
            userSettings["RightServo_PIN"] = pinValues.rightPin;
            userSettings["LeftServo_PIN"] = pinValues.leftPin;
            userSettings["RightUpperServo_PIN"] = pinValues.rightUpper;
            userSettings["LeftUpperServo_PIN"] = pinValues.leftUpper;
            userSettings["PitchLeftServo_PIN"] = pinValues.pitchLeft;
            userSettings["PitchRightServo_PIN"] = pinValues.pitchRight;
            updateCommonPins(pinValues);
            setRestartRequired();
            updateUserSettings();
        }
    }, 2000);
}

function updateCommonPins(pinValues) {
    userSettings["TwistServo_PIN"] = pinValues.twistServo;
    userSettings["ValveServo_PIN"] = pinValues.valveServo;
    userSettings["Squeeze_PIN"] = pinValues.squeezeServo;
    userSettings["Vibe0_PIN"] = pinValues.vibe0;
    userSettings["Vibe1_PIN"] = pinValues.vibe1;
    userSettings["Vibe2_PIN"] = pinValues.vibe2;
    userSettings["Vibe3_PIN"] = pinValues.vibe3;
    userSettings["LubeButton_PIN"] = pinValues.lubeButton;
    if(userSettings.tempSleeveEnabled) {
        userSettings["Heater_PIN"] = pinValues.heat;
    }
    if(userSettings.tempInternalEnabled) {
        userSettings["Case_Fan_PIN"] = pinValues.caseFanPin;
    }
    if(userSettings.tempSleeveEnabled) {
        userSettings["Temp_PIN"] = pinValues.temp;
    }
    if(userSettings.feedbackTwist) {
        userSettings["TwistFeedBack_PIN"] = pinValues.twistFeedBack;
    }
    if(userSettings.tempInternalEnabled) {
        userSettings["Internal_Temp_PIN"] = pinValues.internalTemp;
    }
    // if(userSettings.batteryLevelEnabled) {
    //     userSettings["Battery_Voltage_PIN"] = pinValues.Battery_Voltage_PIN;
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
function validateIntControl(controlID, settingsObject, settingVariableName, additionalValidations) {
    var control = document.getElementById(controlID);
    if(additionalValidations && additionalValidations(control.value) || control.checkValidity()) {
        settingsObject[settingVariableName] = parseInt(control.value);
        return true;
    }
    showError(`${controlID} is invalid: ${control.errorText}`)
    return false;
}
var validateFloatDebounce;
/** additionalValidations takes a parameter with the value */
function validateFloatControl(controlID, settingsObject, settingVariableName, additionalValidations) {
    var control = document.getElementById(controlID);
    if(additionalValidations && additionalValidations(control.value) || control.checkValidity()) {
        settingsObject[settingVariableName] = parseFloat(control.value);
        return true;
    }
    showError(`${controlID} is invalid: ${control.errorText}`)
    return false;
}
/** additionalValidations takes a parameter with the value */
function validateStringControl(controlID, settingsObject, settingVariableName, additionalValidations) {
    var control = document.getElementById(controlID);
    if(additionalValidations && additionalValidations(control.value) || control.checkValidity()) {
        settingsObject[settingVariableName && settingVariableName.trim().length ? settingVariableName : controlID] = control.value;
        return true;
    }
    showError(`${controlID} is invalid: ${control.errorText}`)
    return false;
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
    var pmwErrors = [];
    var pinValues = getServoPinValues();
    if(userSettings["disablePinValidation"])
        return pinValues;

    var pinDupeIndex = -1;
    if(pinValues.rightPin > -1) {
        pinDupeIndex = assignedPins.findIndex(x => x.pin === pinValues.rightPin);
        if(pinDupeIndex > -1)
            duplicatePins.push("Right servo pin and "+assignedPins[pinDupeIndex].name);
        if(validPWMpins.indexOf(pinValues.rightPin) == -1)
            pmwErrors.push("Right servo pin: "+pinValues.rightPin);
        assignedPins.push({name:"Right servo", pin:pinValues.rightPin});
    }

    if(pinValues.leftPin > -1) {
        pinDupeIndex = assignedPins.findIndex(x => x.pin === pinValues.leftPin);
        if(pinDupeIndex > -1)
            duplicatePins.push("Left servo pin and "+assignedPins[pinDupeIndex].name);
        if(validPWMpins.indexOf(pinValues.leftPin) == -1)
            pmwErrors.push("Left servo pin: "+pinValues.leftPin);
        assignedPins.push({name:"Left servo", pin:pinValues.leftPin});
    }

    if(userSettings["sr6Mode"]) {
        if(pinValues.rightUpper > -1) {
            pinDupeIndex = assignedPins.findIndex(x => x.pin === pinValues.rightUpper);
            if(pinDupeIndex > -1)
                duplicatePins.push("Right upper servo pin and "+assignedPins[pinDupeIndex].name);
            if(validPWMpins.indexOf(pinValues.rightUpper) == -1)
                pmwErrors.push("Right upper servo pin: "+pinValues.rightUpper);
            assignedPins.push({name:"Right upper servo", pin:pinValues.rightUpper});
        }
        if(pinValues.leftUpper > -1) {
            pinDupeIndex = assignedPins.findIndex(x => x.pin === pinValues.leftUpper);
            if(pinDupeIndex > -1)
                duplicatePins.push("Left upper servo pin and "+assignedPins[pinDupeIndex].name);
            if(validPWMpins.indexOf(pinValues.leftUpper) == -1)
                pmwErrors.push("Left upper servo pin: "+pinValues.leftUpper);
            assignedPins.push({name:"Left upper servo", pin:pinValues.leftUpper});
        }
        if(pinValues.pitchRight > -1) {
            pinDupeIndex = assignedPins.findIndex(x => x.pin === pinValues.pitchRight);
            if(pinDupeIndex> -1)
                duplicatePins.push("Pitch right servo pin and "+assignedPins[pinDupeIndex].name);
            if(validPWMpins.indexOf(pinValues.pitchRight) == -1)
                pmwErrors.push("Pitch right servo pin: "+pinValues.pitchRight);
            assignedPins.push({name:"Pitch right servo", pin:pinValues.pitchRight});
        }
    }

    if(pinValues.pitchLeft > -1) {
        pinDupeIndex = assignedPins.findIndex(x => x.pin === pinValues.pitchLeft);
        if(pinDupeIndex > -1)
            duplicatePins.push("Pitch left servo pin and "+assignedPins[pinDupeIndex].name);
        if(validPWMpins.indexOf(pinValues.pitchLeft) == -1)
            pmwErrors.push("Pitch left servo pin: "+pinValues.pitchLeft);
        assignedPins.push({name:"Pitch left servo", pin:pinValues.pitchLeft});
    }

    validateCommonPWMPins(assignedPins, duplicatePins, pinValues);

    var invalidPins = [];
    validateNonPWMPins(assignedPins, duplicatePins, invalidPins, pinValues);

    if (duplicatePins.length || pmwErrors.length || invalidPins.length) {
        var errorString = "<div name='pinValidation'>Pins NOT saved due to invalid input.<br>";
        if(duplicatePins.length )
            errorString += "<div style='margin-left: 25px;'>The following pins are duplicated:<br><div style='color: white; margin-left: 25px;'>"+duplicatePins.join("<br>")+"</div></div>";
        if(invalidPins.length) {
            if(duplicatePins.length)
                errorString += "<br>";
            errorString += "<div style='margin-left: 25px;'>The following pins are invalid:<br><div style='color: white; margin-left: 25px;'>"+invalidPins.join("<br>")+"</div></div>";
        }
        if (pmwErrors.length) {
            if(duplicatePins.length || invalidPins.length) {
                errorString += "<br>";
            } 
            errorString += "<div style='margin-left: 25px;'>The following pins are invalid PWM pins:<br><div style='color: white; margin-left: 25px;'>"+pmwErrors.join("<br>")+"</div></div>";
        }
        
        errorString += "</div>";
        showError(errorString);
        return undefined;
    }
    return pinValues;
}


function validateCommonPWMPins(assignedPins, duplicatePins, pinValues, pmwErrors) {

    var pinDupeIndex =  -1;

    if(pinValues.twistServo > -1) {
        pinDupeIndex = assignedPins.findIndex(x => x.pin === pinValues.twistServo);
        if(pinDupeIndex > -1)
            duplicatePins.push("Twist servo pin and "+assignedPins[pinDupeIndex].name);
        if(validPWMpins.indexOf(pinValues.twistServo) == -1)
            pmwErrors.push("Twist servo pin: "+pinValues.twistServo);
        assignedPins.push({name:"Twist servo", pin:pinValues.twistServo});
    }

    if(pinValues.squeezeServo > -1) {
        pinDupeIndex = assignedPins.findIndex(x => x.pin === pinValues.squeezeServo);
        if(pinDupeIndex > -1)
            duplicatePins.push("Squeeze servo pin and "+assignedPins[pinDupeIndex].name);
        if(validPWMpins.indexOf(pinValues.squeezeServo) == -1)
            pmwErrors.push("Squeeze servo pin: "+pinValues.squeezeServo);
        assignedPins.push({name:"Squeeze servo", pin:pinValues.squeezeServo});
    }

    if(pinValues.valveServo > -1) {
        pinDupeIndex = assignedPins.findIndex(x => x.pin === pinValues.valveServo);
        if(pinDupeIndex > -1)
            duplicatePins.push("Valve servo pin and "+assignedPins[pinDupeIndex].name);
        if(validPWMpins.indexOf(pinValues.valveServo) == -1)
            pmwErrors.push("Valve servo pin: "+pinValues.valveServo);
        assignedPins.push({name:"Valve servo", pin:pinValues.valveServo});
    }

    if(pinValues.vibe0 > -1) {
        pinDupeIndex = assignedPins.findIndex(x => x.pin === pinValues.vibe0);
        if(pinDupeIndex > -1)
            duplicatePins.push("Vibe 1 pin and "+assignedPins[pinDupeIndex].name);
        if(validPWMpins.indexOf(pinValues.vibe0) == -1)
            pmwErrors.push("Vibe 1 pin: "+pinValues.vibe0);
        assignedPins.push({name:"Vibe 1", pin:pinValues.vibe0});
    }

    if(pinValues.vibe1 > -1) {
        pinDupeIndex = assignedPins.findIndex(x => x.pin === pinValues.vibe1);
        if(pinDupeIndex > -1)
            duplicatePins.push("Lube/Vibe 2 pin and "+assignedPins[pinDupeIndex].name);
        if(validPWMpins.indexOf(pinValues.vibe1) == -1)
            pmwErrors.push("Lube/Vibe 1 pin: "+pinValues.vibe1);
        assignedPins.push({name:"Lube/Vibe 1", pin:pinValues.vibe1});
    }

    if(pinValues.vibe2 > -1) {
        pinDupeIndex = assignedPins.findIndex(x => x.pin === pinValues.vibe2);
        if(pinDupeIndex > -1)
            duplicatePins.push("Vibe 3 pin and "+assignedPins[pinDupeIndex].name);
        if(validPWMpins.indexOf(pinValues.vibe2) == -1)
            pmwErrors.push("Vibe 3 pin: "+pinValues.vibe2);
        assignedPins.push({name:"Vibe 3", pin:pinValues.vibe2});
    }

    if(pinValues.vibe3 > -1) {
        pinDupeIndex = assignedPins.findIndex(x => x.pin === pinValues.vibe3);
        if(pinDupeIndex > -1)
            duplicatePins.push("Vibe 4 pin and "+assignedPins[pinDupeIndex].name);
        if(validPWMpins.indexOf(pinValues.vibe3) == -1)
            pmwErrors.push("Vibe 4 pin: "+pinValues.vibe3);
        assignedPins.push({name:"Vibe 4", pin:pinValues.vibe3});
    }

    if(userSettings.tempSleeveEnabled) {
        if(pinValues.heat > -1) {
            pinDupeIndex = assignedPins.findIndex(x => x.pin === pinValues.heat);
            if(pinDupeIndex > -1)
                duplicatePins.push("Heater pin and "+assignedPins[pinDupeIndex].name);
            if(validPWMpins.indexOf(pinValues.heat) == -1)
                pmwErrors.push("Heater pin: "+pinValues.heat);
            assignedPins.push({name:"Heater", pin:pinValues.heat});
        }
    }
    
    if(userSettings.tempInternalEnabled) {
        if(pinValues.caseFanPin > -1) {
            pinDupeIndex = assignedPins.findIndex(x => x.pin === pinValues.caseFanPin);
            if(pinDupeIndex > -1)
                duplicatePins.push("Case fan pin and "+assignedPins[pinDupeIndex].name);
            if(validPWMpins.indexOf(pinValues.caseFanPin) == -1)
                pmwErrors.push("Case fan pin: "+pinValues.caseFanPin);
            assignedPins.push({name:"Case fan pin", pin:pinValues.caseFanPin});
        }
    }
}
/** Does not show an error. Just returns true/false */
function validateNonPWMPins(assignedPins, duplicatePins, invalidPins, pinValues) {

    var pinDupeIndex = -1;

    if(userSettings.displayEnabled || userSettings.voiceEnabled || userSettings.batteryLevelEnabled) {
        pinDupeIndex = assignedPins.findIndex(x => x.pin === 21);
        if(pinDupeIndex > -1)
            duplicatePins.push("I2C pin 21 and "+assignedPins[pinDupeIndex].name);

        pinDupeIndex = assignedPins.findIndex(x => x.pin === 22);
        if(pinDupeIndex > -1)
            duplicatePins.push("I2C pin 22 and "+assignedPins[pinDupeIndex].name);
    }
    
    if(pinValues.lubeButton > -1) {
        pinDupeIndex = assignedPins.findIndex(x => x.pin === pinValues.lubeButton);
        if(validPWMpins.indexOf(pinValues.lubeButton) == -1 && inputOnlypins.indexOf(pinValues.lubeButton) == -1)
            invalidPins.push("Invalid Lube button pin: "+pinValues.lubeButton);
        if(pinDupeIndex > -1)
            duplicatePins.push("Lube button pin and "+assignedPins[pinDupeIndex].name);
        assignedPins.push({name:"Lube button", pin:pinValues.lubeButton});
    }

    if(userSettings.tempSleeveEnabled) {
        if(pinValues.temp > -1) {
            if(validPWMpins.indexOf(pinValues.temp) == -1 && inputOnlypins.indexOf(pinValues.temp) == -1)
                invalidPins.push("Invalid Sleeve temp pin: "+pinValues.temp);
            pinDupeIndex = assignedPins.findIndex(x => x.pin === pinValues.temp);
            if(pinDupeIndex > -1)
                duplicatePins.push("Temp pin and "+assignedPins[pinDupeIndex].name);
            assignedPins.push({name:"Temp", pin:pinValues.temp});
        }
    }
    if(userSettings.tempInternalEnabled) {
        if(pinValues.internalTemp > -1) {
            if(validPWMpins.indexOf(pinValues.internalTemp) == -1 && inputOnlypins.indexOf(pinValues.internalTemp) == -1)
                invalidPins.push("Invalid Internal temp pin: "+pinValues.internalTemp);
            pinDupeIndex = assignedPins.findIndex(x => x.pin === pinValues.internalTemp);
            if(pinDupeIndex > -1)
                duplicatePins.push("Internal temp pin and "+assignedPins[pinDupeIndex].name);
            assignedPins.push({name:"Internal temp", pin:pinValues.internalTemp});
        }
    }

    if(userSettings.batteryLevelEnabled) {
        // if(adc1Pins.indexOf(pinValues.Battery_Voltage_PIN) == -1) 
        //     invalidPins.push("Battery voltage pin: "+pinValues.Battery_Voltage_PIN + " is not a valid adc1 pin.");
        // pinDupeIndex = assignedPins.findIndex(x => x.pin === pinValues.Battery_Voltage_PIN);
        // if(pinDupeIndex > -1)
        //     duplicatePins.push("Battery voltage pin and "+assignedPins[pinDupeIndex].name);
        // assignedPins.push({name:"Battery voltage", pin:pinValues.Battery_Voltage_PIN});
    }
    
    if(userSettings.feedbackTwist && pinValues.twistFeedBack) {
        if(pinValues.twistFeedBack > -1) {
            if(validPWMpins.indexOf(pinValues.twistFeedBack) == -1 && inputOnlypins.indexOf(pinValues.twistFeedBack) == -1)
                invalidPins.push("Invalid Twist feedback pin: "+pinValues.twistFeedBack);
            pinDupeIndex = assignedPins.findIndex(x => x.pin === pinValues.twistFeedBack);
            if(pinDupeIndex > -1)
                duplicatePins.push("Twist feedback pin and "+assignedPins[pinDupeIndex].name);
            assignedPins.push({name:"Twist feed back", pin:pinValues.twistFeedBack});
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

    pinValues.lubeButton = parseInt(document.getElementById('LubeButton_PIN').value);
    pinValues.temp = parseInt(document.getElementById('Temp_PIN').value);
    pinValues.internalTemp = parseInt(document.getElementById('Internal_Temp_PIN').value);
    pinValues.twistFeedBack = parseInt(document.getElementById('TwistFeedBack_PIN').value);
}

function updateZeros() 
{
    if(upDateTimeout !== null) 
    {
        clearTimeout(upDateTimeout);
    }
    upDateTimeout = setTimeout(() => 
    {
        clearErrors("zeroValidation"); 
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
        var Squeeze_ZERO = parseInt(document.getElementById('Squeeze_ZERO').value);
        if(!Squeeze_ZERO || Squeeze_ZERO > 1750 || Squeeze_ZERO < 1250)
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
            showError("<div name='zeroValidation'>Zeros NOT saved due to invalid input.<br><div style='margin-left: 25px;'>The values should be between 1250 and 1750 for the following:<br><div style='color: white; margin-left: 25px;'>"+invalidValues.join("<br>")+"</div></div></div>");
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
    userSettings["displayEnabled"] = document.getElementById('displayEnabled').checked;
    // userSettings["Display_Screen_Width"] = parseInt(document.getElementById('Display_Screen_Width').value);
    // userSettings["Display_Screen_Height"] = parseInt(document.getElementById('Display_Screen_Height').value);

    // userSettings["Display_Rst_PIN"] = parseInt(document.getElementById('Display_Rst_PIN').value);
    userSettings["Display_I2C_Address"] = document.getElementById('Display_I2C_Address_text').value;
    userSettings["sleeveTempDisplayed"] = document.getElementById('sleeveTempDisplayed').checked;
    userSettings["internalTempDisplayed"] = document.getElementById('internalTempDisplayed').checked;
    userSettings["versionDisplayed"] = document.getElementById('versionDisplayed').checked;
    if(validatePins()) {
        setRestartRequired();
        updateUserSettings();
    }
}
function setDisplayAddress() {
    var selectValue = document.getElementById('Display_I2C_Address').value;
    document.getElementById('Display_I2C_Address_text').value = selectValue;
    userSettings["Display_I2C_Address"] = selectValue;
    setRestartRequired();
    updateUserSettings();
}
function setTempSettings() {
    userSettings["tempSleeveEnabled"] = document.getElementById('tempSleeveEnabled').checked;
    userSettings["TargetTemp"] = parseFloat(document.getElementById('TargetTemp').value);
    userSettings["HeatPWM"] = parseInt(document.getElementById('HeatPWM').value);
    userSettings["HoldPWM"] = parseInt(document.getElementById('HoldPWM').value);
    userSettings["heaterThreshold"] = parseInt(document.getElementById('heaterThreshold').value);
    userSettings["heaterResolution"] = parseInt(document.getElementById('heaterResolution').value);
    userSettings["heaterFrequency"] = parseInt(document.getElementById('heaterFrequency').value);

    Utils.toggleControlVisibilityByID('sleeveTempDisplayedRow', hasFeature(BuildFeature.TEMP) && userSettings["tempSleeveEnabled"]);
    if(validatePins()) {
        setRestartRequired();
        updateUserSettings();
    }
}
function setInternalTempSettings() {
    userSettings["tempInternalEnabled"] = document.getElementById('tempInternalEnabled').checked;
    userSettings["caseFanResolution"] = parseInt(document.getElementById('caseFanResolution').value);
    userSettings["caseFanFrequency"] = parseInt(document.getElementById('caseFanFrequency').value);

    Utils.toggleControlVisibilityByID('internalTempDisplayedRow', hasFeature(BuildFeature.TEMP) && userSettings["tempInternalEnabled"]);
    setRestartRequired();
    updateUserSettings();
}

function toggleVoiceSettings() {
    userSettings['voiceEnabled'] = document.getElementById('voiceEnabled').checked;  
    if(validatePins()) {
        setRestartRequired();
        updateUserSettings();
    }
}
function setVoiceSettings() {
    userSettings['voiceMuted'] = document.getElementById('voiceMuted').checked;  
    validateIntControl("voiceVolume", userSettings, "voiceVolume");
    validateIntControl("voiceWakeTime", userSettings, "voiceWakeTime");
    updateUserSettings();
}
function setFanControl() {
    userSettings["fanControlEnabled"] = document.getElementById('fanControlEnabled').checked;
    toggleFanControlSettings(userSettings["fanControlEnabled"]);
    
    if(validatePins()) {
        setRestartRequired();
        updateUserSettings();
    }
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

function updateWifiSettings() {
    userSettings["ssid"] = document.getElementById('ssid').value;
    userSettings["wifiPass"] = document.getElementById('wifiPass').value;
	var staticIP = document.getElementById('staticIP').checked;
	var localIP = document.getElementById('localIPInput').value;
	var gateway = document.getElementById('gatewayInput').value;
	var subnet = document.getElementById('subnetInput').value;
	var dns1 = document.getElementById('dns1Input').value;
	var dns2 = document.getElementById('dns2Input').value;
    userSettings["staticIP"] = staticIP;
    toggleStaticIPSettings(staticIP);
    userSettings["localIP"] = localIP;
    userSettings["gateway"] = gateway;
    userSettings["subnet"] = subnet;
    userSettings["dns1"] = dns1;
    userSettings["dns2"] = dns2;
	setRestartRequired();
	updateUserSettings();
	
}


function togglePitchServoFrequency(isChecked) 
{
    Utils.toggleControlVisibilityByID('pitchFrequencyRow', isChecked);
}
function toggleStaticIPSettings(isStatic)
{
    Utils.toggleControlVisibilityByID('localIP', isStatic);
    Utils.toggleControlVisibilityByID('gateway', isStatic);
    Utils.toggleControlVisibilityByID('subnet', isStatic);
    Utils.toggleControlVisibilityByID('dns1', isStatic);
    Utils.toggleControlVisibilityByID('dns2', isStatic);
}
function toggleDeviceOptions(sr6Mode)
{
    var osrOnly = document.getElementsByClassName('osrOnly');
    var sr6Only = document.getElementsByClassName('sr6Only');
    for(var i=0;i < sr6Only.length; i++)
        sr6Only[i].style.display = sr6Mode ? "flex" : "none";
    for(var i=0;i < osrOnly.length; i++)
        osrOnly[i].style.display = sr6Mode ? "none" : "flex";
        
    if(sr6Mode && userSettings["msPerRad"] == servoDegreeValue270) {
        document.getElementById('msPerRadIs270').checked = true;
    }
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
    userSettings["bluetoothEnabled"] = document.getElementById('bluetoothEnabled').checked;
    if(userSettings["bluetoothEnabled"])
        alert("EXPEREMENTAL! this is a bit slow and will not work will with fast input!\nThis will DISABLE Wifi connection and this configuration web page upon device reboot!\nThe BLE app will be REQUIRED for future configuration changes.")
	setRestartRequired();
	updateUserSettings();
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

function updateLubeEnabled() {
    userSettings["lubeEnabled"] = document.getElementById('lubeEnabled').checked;
    
    if(validatePins()) {
        setRestartRequired();
        updateUserSettings();
    }
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
    userSettingsCopy["wifiPass"] = "YOUR PASSWORD HERE";
    userSettingsCopy["motionDefaultProfileIndex"] = motionProviderSettings.motionDefaultProfileIndex;
    userSettingsCopy["motionProfiles"] = motionProviderSettings.motionProfiles;

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
                // If the key doesnt exist anymore, dont import it.
                if(existingValue != undefined && existingValue != null)// 0 can be valid
                    userSettings[key]=importedValue;
                else
                    handleImportRenames(key, importedValue)
            });
            // If the new build doesnt have the old TCode version in it, set it to the latest version.
            if(availableVersions.findIndex(x => x.version == userSettings.TCodeVersion) == -1) {
                userSettings.TCodeVersion = latestTCodeVersion;
                document.getElementById('TCodeVersion').value = userSettings["TCodeVersion"];
                toggleNonTCodev3Options();
            }
            setUserSettings();
            setupChannelSliders();
            if(validatePins()) {// Do not save if pin values are invalid.
                setRestartRequired();
                updateALLUserSettings();
            }
        }, false);

        reader.readAsText(json);
    }
}

function handleImportRenames(key, value) {
    switch(key) {
        case "LubeManual_PIN": 
        userSettings.LubeButton_PIN = value;
        break;
        case "motionProfiles": 
        motionProviderSettings.motionProfiles = value;
        break;
        case "motionDefaultProfileIndex": 
        motionProviderSettings.motionDefaultProfileIndex = value;
        break;
    }
}