MotionGenerator = {
    setup() {
        const motionProfilesElement = document.getElementById('motionProfiles');
        removeAllChildren(motionProfilesElement);
        userSettings["motionProfiles"].forEach((x, index) => {
            addMotionProfileOption(index, x, index == userSettings["motionDefaultProfileIndex"]);
        });
    
        const currentProfileIndex = userSettings["motionDefaultProfileIndex"];
        document.getElementById('motionProfiles').value = currentProfileIndex;
    
        document.getElementById('motionProfileName').value = userSettings['motionProfiles'][currentProfileIndex]["motionProfileName"];
        document.getElementById('motionUpdateGlobal').value = userSettings['motionProfiles'][currentProfileIndex]["motionUpdateGlobal"];
        document.getElementById('motionPeriodGlobal').value = userSettings['motionProfiles'][currentProfileIndex]["motionPeriodGlobal"];
        document.getElementById('motionAmplitudeGlobal').value = userSettings['motionProfiles'][currentProfileIndex]["motionAmplitudeGlobal"];
        document.getElementById('motionOffsetGlobal').value = userSettings['motionProfiles'][currentProfileIndex]["motionOffsetGlobal"];  
        // document.getElementById('motionPhaseGlobal').value = userSettings["motionPhaseGlobal"];
        document.getElementById('motionPeriodGlobalRandom').checked = userSettings['motionProfiles'][currentProfileIndex]["motionPeriodGlobalRandom"];
        document.getElementById('motionAmplitudeGlobalRandom').checked = userSettings['motionProfiles'][currentProfileIndex]["motionAmplitudeGlobalRandom"];
        document.getElementById('motionOffsetGlobalRandom').checked = userSettings['motionProfiles'][currentProfileIndex]["motionOffsetGlobalRandom"];
        toggleMotionRandomSettings();
        document.getElementById('motionPeriodGlobalRandomMin').value = userSettings['motionProfiles'][currentProfileIndex]["motionPeriodGlobalRandomMin"];
        document.getElementById('motionPeriodGlobalRandomMax').value = userSettings['motionProfiles'][currentProfileIndex]["motionPeriodGlobalRandomMax"];
        document.getElementById('motionAmplitudeGlobalRandomMin').value = userSettings['motionProfiles'][currentProfileIndex]["motionAmplitudeGlobalRandomMin"];
        document.getElementById('motionAmplitudeGlobalRandomMax').value = userSettings['motionProfiles'][currentProfileIndex]["motionAmplitudeGlobalRandomMax"];
        document.getElementById('motionOffsetGlobalRandomMin').value = userSettings['motionProfiles'][currentProfileIndex]["motionOffsetGlobalRandomMin"];
        document.getElementById('motionOffsetGlobalRandomMax').value = userSettings['motionProfiles'][currentProfileIndex]["motionOffsetGlobalRandomMax"];
        document.getElementById('motionRandomChangeMax').value = userSettings['motionProfiles'][currentProfileIndex]["motionRandomChangeMax"];
        document.getElementById('motionRandomChangeMin').value = userSettings['motionProfiles'][currentProfileIndex]["motionRandomChangeMin"];
        setIntMinAndMax('motionPeriodGlobalRandomMin', 'motionPeriodGlobalRandomMax');
        setIntMinAndMax('motionAmplitudeGlobalRandomMin', 'motionAmplitudeGlobalRandomMax');
        setIntMinAndMax('motionOffsetGlobalRandomMin', 'motionOffsetGlobalRandomMax');
        setIntMinAndMax('motionRandomChangeMin', 'motionRandomChangeMax');
        
        //document.getElementById('motionReversedGlobal').checked = userSettings["motionReversedGlobal"];
    },
    setEnabledStatus() {
        var button = document.getElementById('motionEnabledToggle');
        button.innerText = userSettings["motionEnabled"] ? "Stop" : "Start"
        if(userSettings["motionEnabled"]) {
            button.classList.add("button-toggle-stop");
            button.classList.remove("button-toggle-start");
        } else {
            button.classList.remove("button-toggle-stop");
            button.classList.add("button-toggle-start");
        }
    },
    setupChannels() {
        var motionChannelsNode = document.getElementById("motionChannels");
        deleteAllChildren(motionChannelsNode);

        var channels = getChannelMap();
        var header = document.createElement("div");
        header.innerText = "Generate motion on channels:"
        motionChannelsNode.appendChild(header);

        var rowHeader = document.createElement("div");
        rowHeader.classList.add("tRow");
        var rowcell1 = document.createElement("div");
        rowcell1.classList.add("tCell");
        rowcell1.innerText = "Enabled"
        rowcell1.style = "border-right: solid 1px; padding-right: 6px;"
        var rowcell2 = document.createElement("div");
        rowcell2.classList.add("tCell");
        rowcell2.style = "justify-content: space-between;"
        var phaseSpan = document.createElement("span");
        phaseSpan.innerText = "Phase"
        var phaseTitle = "Initial phase in degrees. (0-180) The phase should ideally be between (offset-amplitude/2) and (offset+amplitude/2)";
        phaseSpan.title = phaseTitle;
        var reversedSpan = document.createElement("span");
        reversedSpan.innerText = "Reverse"
        rowcell2.appendChild(phaseSpan);
        rowcell2.appendChild(reversedSpan);
        rowHeader.appendChild(rowcell1);
        rowHeader.appendChild(rowcell2);
        motionChannelsNode.appendChild(rowHeader)
        for(var i=0; i<channels.length;i++) {
            var channel = channels[i];
            if(!userSettings.sr6Mode && channel.sr6Only) {
                continue;
            }
            
            var row = document.createElement("div");
            row.classList.add("tRow");
            var cell1 = document.createElement("div");
            cell1.classList.add("tCell");
            var cell2 = document.createElement("div");
            cell2.classList.add("tCell");
            var name = channel.channel;
            var friendlyName = channel.channelName;
            var motionChannelIndex = userSettings["motionChannels"].findIndex(x => x.name == name);
            var label = document.createElement("span");
            label.for = "motionChannelCheckbox" + name;
            label.innerText = friendlyName +" ("+name+")";
            var checkbox = document.createElement("input");
            checkbox.id = "motionChannelCheckbox" + name;
            checkbox.type = "checkbox";
            checkbox.value = name;
            checkbox.checked = motionChannelIndex > -1;
            checkbox.onclick = function () {
                var name = this.value;
                var phaseInput = document.getElementById("motionChannelPhaseInput" + name);
                var reverseInput = document.getElementById("motionChannelReverseInput" + name);
                phaseInput.readOnly = !this.checked;
                if(!this.checked) {
                    phaseInput.value = 0;
                    reverseInput.checked = false;
                }
                const index = userSettings["motionChannels"].findIndex(x => x.name == name);
                var motionChannel = {name: name, phase: 0.0}; 
                if(index > -1) {
                    motionChannel = userSettings["motionChannels"][index];
                    userSettings["motionChannels"].splice(index, 1);
                }
                if(this.checked) {
                    userSettings["motionChannels"].push(motionChannel);
                }
                updateUserSettings();
            };
            
            var phaseInput = document.createElement("input");
            phaseInput.id = "motionChannelPhaseInput" + name;
            phaseInput.type = "number";
            phaseInput.min = 0.0;
            phaseInput.step = 0.01;
            phaseInput.max = 180;
            phaseInput.readOnly = motionChannelIndex == -1;
            phaseInput.value = motionChannelIndex == -1 ? 0 : Utils.round2(userSettings["motionChannels"][motionChannelIndex].phase);
            phaseInput.name = name;
            phaseInput.title = phaseTitle;
            phaseInput.oninput = function () {
                const index = userSettings["motionChannels"].findIndex(x => x.name == this.name);
                if(index == -1) {
                    this.checked = false;
                } else {
                    Utils.debounce("phaseInput"+this.name, function() {
                        var name = this.name;
                        const index = userSettings["motionChannels"].findIndex(x => x.name == name);
                        if(index > -1) {
                            userSettings["motionChannels"][index].phase = parseFloat(this.value);
                            updateUserSettings(500);
                        }
                    }.bind(this), 3000);
                }
            };
            var reverseInput = document.createElement("input");
            reverseInput.id = "motionChannelReverseInput" + name;
            reverseInput.type = "checkbox";
            reverseInput.readOnly = motionChannelIndex == -1;
            reverseInput.checked = motionChannelIndex == -1 ? false : userSettings["motionChannels"][motionChannelIndex].reverse;
            reverseInput.name = name;
            reverseInput.onclick = function () {
                const index = userSettings["motionChannels"].findIndex(x => x.name == this.name);
                if(index == -1) {
                    this.checked = false;
                } else {
                    Utils.debounce("reverseInput"+this.name, function() {
                        var name = this.name;
                        const index = userSettings["motionChannels"].findIndex(x => x.name == name);
                        if(index > -1) {
                            userSettings["motionChannels"][index].reverse = this.value;
                            updateUserSettings(500);
                        } else {
                            this.checked = false;
                        }
                    }.bind(this), 3000);
                }
            };
            cell1.appendChild(label);
            cell1.appendChild(checkbox);
            cell2.appendChild(phaseInput);
            cell2.appendChild(reverseInput);
            row.appendChild(cell1);
            row.appendChild(cell2);
            motionChannelsNode.appendChild(row);
        }
    }
}

function toggleMotionRandomSettings() {
    var currentProfile = getMotionProfileSelectedIndex();
    Utils.toggleControlVisibilityByID("motionOffsetGlobalRandomMinRow", userSettings['motionProfiles'][currentProfile]["motionOffsetGlobalRandom"]);
    Utils.toggleControlVisibilityByID("motionOffsetGlobalRandomMaxRow", userSettings['motionProfiles'][currentProfile]["motionOffsetGlobalRandom"]);
    Utils.toggleControlVisibilityByID("motionAmplitudeGlobalRandomMinRow", userSettings['motionProfiles'][currentProfile]["motionAmplitudeGlobalRandom"]);
    Utils.toggleControlVisibilityByID("motionAmplitudeGlobalRandomMaxRow", userSettings['motionProfiles'][currentProfile]["motionAmplitudeGlobalRandom"]);
    Utils.toggleControlVisibilityByID("motionPeriodGlobalRandomMinRow", userSettings['motionProfiles'][currentProfile]["motionPeriodGlobalRandom"]);
    Utils.toggleControlVisibilityByID("motionPeriodGlobalRandomMaxRow", userSettings['motionProfiles'][currentProfile]["motionPeriodGlobalRandom"]);
    toggleMotionRandomMinMaxSettingsSettings();
}
function toggleMotionRandomMinMaxSettingsSettings() {
    var currentProfile = getMotionProfileSelectedIndex();
    Utils.toggleControlVisibilityByID("motionRandomChangeMinRow", userSettings['motionProfiles'][currentProfile]["motionOffsetGlobalRandom"] || userSettings['motionProfiles'][currentProfile]["motionAmplitudeGlobalRandom"] || userSettings['motionProfiles'][currentProfile]["motionPeriodGlobalRandom"]);
    Utils.toggleControlVisibilityByID("motionRandomChangeMaxRow", userSettings['motionProfiles'][currentProfile]["motionOffsetGlobalRandom"] || userSettings['motionProfiles'][currentProfile]["motionAmplitudeGlobalRandom"] || userSettings['motionProfiles'][currentProfile]["motionPeriodGlobalRandom"]);
}

function setMotionEnabledClicked() {
    userSettings["motionEnabled"] = !userSettings["motionEnabled"];//document.getElementById('motionEnabled').checked;
    sendTCode(userSettings["motionEnabled"] ? "$motion-enable" : "$motion-disable");
    //userSettings["motionEnabled"] ? button.classList.add("button-toggle-stop") : button.classList.remove("button-toggle-stop");
    MotionGenerator.setEnabledStatus();
}
function setMotionPeriodGlobalRandomClicked() {
    var currentProfile = getMotionProfileSelectedIndex();
    var motionEnabled = document.getElementById('motionPeriodGlobalRandom').checked;
    if(motionEnabled) {
        sendTCode("$motion-period-random-on");
    } else {
        sendTCode("$motion-period-random-off");
    }
    userSettings['motionProfiles'][currentProfile]["motionPeriodGlobalRandom"] = motionEnabled;
    Utils.toggleControlVisibilityByID("motionPeriodGlobalRandomMinRow", userSettings['motionProfiles'][currentProfile]["motionPeriodGlobalRandom"]);
    Utils.toggleControlVisibilityByID("motionPeriodGlobalRandomMaxRow", userSettings['motionProfiles'][currentProfile]["motionPeriodGlobalRandom"]);
    toggleMotionRandomMinMaxSettingsSettings();
    updateUserSettings();
}
function setMotionAmplitudeGlobalRandomClicked() {
    var currentProfile = getMotionProfileSelectedIndex();
    var motionEnabled = document.getElementById('motionAmplitudeGlobalRandom').checked;
    if(motionEnabled) {
        sendTCode("$motion-amplitude-random-on");
    } else {
        sendTCode("$motion-amplitude-random-off");
    }
    userSettings['motionProfiles'][currentProfile]["motionAmplitudeGlobalRandom"] = motionEnabled;
    Utils.toggleControlVisibilityByID("motionAmplitudeGlobalRandomMinRow", userSettings['motionProfiles'][currentProfile]["motionAmplitudeGlobalRandom"]);
    Utils.toggleControlVisibilityByID("motionAmplitudeGlobalRandomMaxRow", userSettings['motionProfiles'][currentProfile]["motionAmplitudeGlobalRandom"]);
    toggleMotionRandomMinMaxSettingsSettings();
    updateUserSettings();
}
function setMotionOffsetGlobalRandomRandomClicked() {
    var currentProfile = getMotionProfileSelectedIndex();
    var motionEnabled = document.getElementById('motionOffsetGlobalRandom').checked;
    if(motionEnabled) {
        sendTCode("$motion-offset-random-on");
    } else {
        sendTCode("$motion-offset-random-off");
    }
    userSettings['motionProfiles'][currentProfile]["motionOffsetGlobalRandom"] = motionEnabled;
    Utils.toggleControlVisibilityByID("motionOffsetGlobalRandomMinRow", userSettings['motionProfiles'][currentProfile]["motionOffsetGlobalRandom"]);
    Utils.toggleControlVisibilityByID("motionOffsetGlobalRandomMaxRow", userSettings['motionProfiles'][currentProfile]["motionOffsetGlobalRandom"]);
    toggleMotionRandomMinMaxSettingsSettings();
    updateUserSettings();
}
function setMotionProfileDefault() {
    userSettings["motionDefaultProfileIndex"] = getMotionProfileSelectedIndex();
    updateUserSettings(1);
}
var selectedMotionProfileIndex = 0;
function getMotionProfileSelectedIndex() {
    return selectedMotionProfileIndex;
}
function setSelectedMotionProfileIndex(value) {
    selectedMotionProfileIndex = parseInt(value);
}
function clearMotionProfileSelection() {
    var motionProfileButtons = document.getElementsByName("motionProfileButton");
    if(motionProfileButtons)
        motionProfileButtons.forEach(x => x.classList.remove("button-pressed"))
}
function selectMotionProfile(profileIndex, sendTCodeCommand) {
    clearMotionProfileSelection();
    setSelectedMotionProfileIndex(profileIndex);
    const motionProfilesElement = document.getElementById(`motionProfile${profileIndex}`);
    motionProfilesElement.classList.add("button-pressed");
    setMotionGeneratorSettingsProfile(profileIndex);
    if(sendTCodeCommand)
        sendTCode(`$motion-set-profile:${profileIndex + 1}`);
}

var validateGeneratorSettingsDebounce;
function setMotionGeneratorSettings() {
    if(validateGeneratorSettingsDebounce) {
        clearTimeout(validateGeneratorSettingsDebounce);
    }
    validateGeneratorSettingsDebounce = setTimeout(() => {
        var currentProfileIndex = getMotionProfileSelectedIndex();
        validateIntControl('motionUpdateGlobal', userSettings['motionProfiles'][currentProfileIndex], "motionUpdateGlobal");
        validateIntControl('motionPeriodGlobal', userSettings['motionProfiles'][currentProfileIndex], "motionPeriodGlobal");
        validateIntControl('motionAmplitudeGlobal', userSettings['motionProfiles'][currentProfileIndex], "motionAmplitudeGlobal");
        validateIntControl('motionOffsetGlobal', userSettings['motionProfiles'][currentProfileIndex], "motionOffsetGlobal");
        // validateFloatControl('motionPhaseGlobal', userSettings["motionPhaseGlobal"]);

        setIntMinAndMax('motionPeriodGlobalRandomMin', 'motionPeriodGlobalRandomMax');
        validateIntControl('motionPeriodGlobalRandomMin', userSettings['motionProfiles'][currentProfileIndex], "motionPeriodGlobalRandomMin");
        validateIntControl('motionPeriodGlobalRandomMax', userSettings['motionProfiles'][currentProfileIndex], "motionPeriodGlobalRandomMax");
        setIntMinAndMax('motionAmplitudeGlobalRandomMin', 'motionAmplitudeGlobalRandomMax');
        validateIntControl('motionAmplitudeGlobalRandomMin', userSettings['motionProfiles'][currentProfileIndex], "motionAmplitudeGlobalRandomMin");
        validateIntControl('motionAmplitudeGlobalRandomMax', userSettings['motionProfiles'][currentProfileIndex], "motionAmplitudeGlobalRandomMax");
        setIntMinAndMax('motionOffsetGlobalRandomMin', 'motionOffsetGlobalRandomMax');
        validateIntControl('motionOffsetGlobalRandomMin', userSettings['motionProfiles'][currentProfileIndex], "motionOffsetGlobalRandomMin");
        validateIntControl('motionOffsetGlobalRandomMax', userSettings['motionProfiles'][currentProfileIndex], "motionOffsetGlobalRandomMax");
        setIntMinAndMax('motionRandomChangeMin', 'motionRandomChangeMax');
        validateIntControl('motionRandomChangeMin', userSettings['motionProfiles'][currentProfileIndex], "motionRandomChangeMin");
        validateIntControl('motionRandomChangeMax', userSettings['motionProfiles'][currentProfileIndex], "motionRandomChangeMax");

        
        //userSettings["motionReversedGlobal"] = document.getElementById('motionReversedGlobal').checked;
        updateUserSettings(1);
    },3000);
}
var setMotionGeneratorNameDebounce;
function setMotionGeneratorName() {
    if(setMotionGeneratorNameDebounce) {
        clearTimeout(setMotionGeneratorNameDebounce);
    }
    
    setMotionGeneratorNameDebounce = setTimeout(() => {
        var currentProfileIndex = getMotionProfileSelectedIndex();
        if(validateStringControl("motionProfileName", userSettings['motionProfiles'][currentProfileIndex])) {
            //userSettings['motionProfiles'][currentProfileIndex]["motionProfileName"] = document.getElementById("motionProfileName").value;
            updateMotionProfileName(currentProfileIndex);
            updateUserSettings(1);
        }
    },3000);
}
function addMotionProfileOption(profileIndex, profile, selectNewProfile) {
    const motionProfilesElement = document.getElementById('motionProfiles');
    var button = document.createElement("button");
    button.id = `motionProfile${profileIndex}`;
    button.name = "motionProfileButton";
    button.value = profileIndex;
    button.innerText = profile.motionProfileName;
    button.onclick = function(profileIndex, event) {
        selectMotionProfile(profileIndex, true);
    }.bind(this, profileIndex)
    motionProfilesElement.appendChild(button);
    if(selectNewProfile) {
        selectMotionProfile(profileIndex);
        //motionProfilesElement.value = profileName;
    }
}
function updateMotionProfileName(profileIndex) {
    const motionProfilesElements = document.getElementsByName('motionProfileButton');
    var optionsIndex = getMotionProfileSelectedIndex();
    for (var i=0; i < motionProfilesElements.length; i++) {
        if (motionProfilesElements[i].value == profileIndex) {
            optionsIndex = i;
            break;
        }
    }
    motionProfilesElements[optionsIndex].innerText = userSettings['motionProfiles'][profileIndex]["motionProfileName"];
}
function setMotionGeneratorSettingsDefault() {
    var profileIndex = getMotionProfileSelectedIndex();
    if (confirm(`Are you sure you want to set the current profile '${userSettings['motionProfiles'][profileIndex]["motionProfileName"]}' to the default settings?`)) {
        setProfileMotionGeneratorSettingsDefault(profileIndex);
        updateUserSettings(1);
    }
}
function setProfileMotionGeneratorSettingsDefault(profileIndex) {
    userSettings['motionProfiles'][profileIndex]["motionProfileName"] = "Profile "+ profileIndex + 1;
    userSettings['motionProfiles'][profileIndex]["motionUpdateGlobal"] = 100;
    userSettings['motionProfiles'][profileIndex]["motionPeriodGlobal"] = 2000;
    userSettings['motionProfiles'][profileIndex]["motionAmplitudeGlobal"] = 60;
    userSettings['motionProfiles'][profileIndex]["motionOffsetGlobal"] = 5000  
    userSettings['motionProfiles'][profileIndex]["motionPeriodGlobalRandom"] = false;
    userSettings['motionProfiles'][profileIndex]["motionPeriodGlobalRandomMin"] = 500;
    userSettings['motionProfiles'][profileIndex]["motionPeriodGlobalRandomMax"] = 2000;
    userSettings['motionProfiles'][profileIndex]["motionAmplitudeGlobalRandom"] = false;
    userSettings['motionProfiles'][profileIndex]["motionAmplitudeGlobalRandomMin"] = 20;
    userSettings['motionProfiles'][profileIndex]["motionAmplitudeGlobalRandomMax"] = 60;
    userSettings['motionProfiles'][profileIndex]["motionOffsetGlobalRandom"] = false;
    userSettings['motionProfiles'][profileIndex]["motionOffsetGlobalRandomMin"] = 3000;
    userSettings['motionProfiles'][profileIndex]["motionOffsetGlobalRandomMax"] = 7000;
    userSettings['motionProfiles'][profileIndex]["motionRandomChangeMinDefault"] = 3000;
    userSettings['motionProfiles'][profileIndex]["motionRandomChangeMaxDefault"] = 30000;
    userSettings['motionProfiles'][profileIndex]["motionPhaseGlobal"] = 0;
    userSettings['motionProfiles'][profileIndex]["motionReversedGlobal"] = false;
    
    setMotionGeneratorSettingsProfile(profileIndex);
}
function setMotionGeneratorSettingsProfile(profileIndex) {
    document.getElementById('motionProfileName').value = userSettings['motionProfiles'][profileIndex]["motionProfileName"];
    document.getElementById('motionUpdateGlobal').value = userSettings['motionProfiles'][profileIndex]["motionUpdateGlobal"];
    document.getElementById('motionPeriodGlobal').value = userSettings['motionProfiles'][profileIndex]["motionPeriodGlobal"];
    document.getElementById('motionAmplitudeGlobal').value = userSettings['motionProfiles'][profileIndex]["motionAmplitudeGlobal"];
    document.getElementById('motionOffsetGlobal').value = userSettings['motionProfiles'][profileIndex]["motionOffsetGlobal"];  
    //document.getElementById('motionPhaseGlobal').value = userSettings['motionProfiles'][profileIndex]["motionPhaseGlobal"];
    //document.getElementById('motionReversedGlobal').checked = userSettings['motionProfiles'][profileIndex]["motionReversedGlobal"];
    document.getElementById('motionPeriodGlobalRandom').checked = userSettings['motionProfiles'][profileIndex]["motionPeriodGlobalRandom"];
    document.getElementById('motionPeriodGlobalRandomMin').value = userSettings['motionProfiles'][profileIndex]["motionPeriodGlobalRandomMin"];  
    document.getElementById('motionPeriodGlobalRandomMax').value = userSettings['motionProfiles'][profileIndex]["motionPeriodGlobalRandomMax"];  
    document.getElementById('motionAmplitudeGlobalRandom').checked = userSettings['motionProfiles'][profileIndex]["motionAmplitudeGlobalRandom"];
    document.getElementById('motionAmplitudeGlobalRandomMin').value = userSettings['motionProfiles'][profileIndex]["motionAmplitudeGlobalRandomMin"];  
    document.getElementById('motionAmplitudeGlobalRandomMax').value = userSettings['motionProfiles'][profileIndex]["motionAmplitudeGlobalRandomMax"];  
    document.getElementById('motionOffsetGlobalRandom').checked = userSettings['motionProfiles'][profileIndex]["motionOffsetGlobalRandom"];
    document.getElementById('motionOffsetGlobalRandomMin').value = userSettings['motionProfiles'][profileIndex]["motionOffsetGlobalRandomMin"];  
    document.getElementById('motionOffsetGlobalRandomMax').value = userSettings['motionProfiles'][profileIndex]["motionOffsetGlobalRandomMax"];  
    toggleMotionRandomSettings();
}