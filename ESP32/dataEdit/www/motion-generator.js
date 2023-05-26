MotionGenerator = {
    setup() {
        const motionProfilesElement = document.getElementById('motionProfiles');
        removeAllChildren(motionProfilesElement);
        userSettings["motionProfiles"].forEach((x, index) => {
            addMotionProfileOption(index, x, index == userSettings["motionDefaultProfileIndex"]);
        });
    
        const currentProfileIndex = userSettings["motionDefaultProfileIndex"];
        document.getElementById('motionProfiles').value = currentProfileIndex;
    
        document.getElementById('motionProfileName').value = userSettings['motionProfiles'][currentProfileIndex]["name"];
        document.getElementById('motionUpdateGlobal').value = userSettings['motionProfiles'][currentProfileIndex]["update"];
        document.getElementById('motionPeriodGlobal').value = userSettings['motionProfiles'][currentProfileIndex]["period"];
        document.getElementById('motionAmplitudeGlobal').value = userSettings['motionProfiles'][currentProfileIndex]["amp"];
        document.getElementById('motionOffsetGlobal').value = userSettings['motionProfiles'][currentProfileIndex]["offset"];  
        // document.getElementById('motionPhaseGlobal').value = userSettings["motionPhaseGlobal"];
        document.getElementById('motionPeriodGlobalRandom').checked = userSettings['motionProfiles'][currentProfileIndex]["periodRan"];
        document.getElementById('motionAmplitudeGlobalRandom').checked = userSettings['motionProfiles'][currentProfileIndex]["ampRan"];
        document.getElementById('motionOffsetGlobalRandom').checked = userSettings['motionProfiles'][currentProfileIndex]["offsetRan"];
        toggleMotionRandomSettings();
        document.getElementById('motionPeriodGlobalRandomMin').value = userSettings['motionProfiles'][currentProfileIndex]["periodMin"];
        document.getElementById('motionPeriodGlobalRandomMax').value = userSettings['motionProfiles'][currentProfileIndex]["periodMax"];
        document.getElementById('motionAmplitudeGlobalRandomMin').value = userSettings['motionProfiles'][currentProfileIndex]["ampMin"];
        document.getElementById('motionAmplitudeGlobalRandomMax').value = userSettings['motionProfiles'][currentProfileIndex]["ampMax"];
        document.getElementById('motionOffsetGlobalRandomMin').value = userSettings['motionProfiles'][currentProfileIndex]["offsetMin"];
        document.getElementById('motionOffsetGlobalRandomMax').value = userSettings['motionProfiles'][currentProfileIndex]["offsetMax"];
        document.getElementById('motionRandomChangeMax').value = userSettings['motionProfiles'][currentProfileIndex]["ranMax"];
        document.getElementById('motionRandomChangeMin').value = userSettings['motionProfiles'][currentProfileIndex]["ranMin"];
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
    Utils.toggleControlVisibilityByID("motionOffsetGlobalRandomMinRow", userSettings['motionProfiles'][currentProfile]["offsetRan"]);
    Utils.toggleControlVisibilityByID("motionOffsetGlobalRandomMaxRow", userSettings['motionProfiles'][currentProfile]["offsetRan"]);
    Utils.toggleControlVisibilityByID("motionAmplitudeGlobalRandomMinRow", userSettings['motionProfiles'][currentProfile]["ampRan"]);
    Utils.toggleControlVisibilityByID("motionAmplitudeGlobalRandomMaxRow", userSettings['motionProfiles'][currentProfile]["ampRan"]);
    Utils.toggleControlVisibilityByID("motionPeriodGlobalRandomMinRow", userSettings['motionProfiles'][currentProfile]["periodRan"]);
    Utils.toggleControlVisibilityByID("motionPeriodGlobalRandomMaxRow", userSettings['motionProfiles'][currentProfile]["periodRan"]);
    toggleMotionRandomMinMaxSettingsSettings();
}
function toggleMotionRandomMinMaxSettingsSettings() {
    var currentProfile = getMotionProfileSelectedIndex();
    Utils.toggleControlVisibilityByID("motionRandomChangeMinRow", userSettings['motionProfiles'][currentProfile]["offsetRan"] || userSettings['motionProfiles'][currentProfile]["ampRan"] || userSettings['motionProfiles'][currentProfile]["periodRan"]);
    Utils.toggleControlVisibilityByID("motionRandomChangeMaxRow", userSettings['motionProfiles'][currentProfile]["offsetRan"] || userSettings['motionProfiles'][currentProfile]["ampRan"] || userSettings['motionProfiles'][currentProfile]["periodRan"]);
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
    Utils.toggleControlVisibilityByID("motionPeriodGlobalRandomMinRow", userSettings['motionProfiles'][currentProfile]["periodRan"]);
    Utils.toggleControlVisibilityByID("motionPeriodGlobalRandomMaxRow", userSettings['motionProfiles'][currentProfile]["periodRan"]);
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
    Utils.toggleControlVisibilityByID("motionAmplitudeGlobalRandomMinRow", userSettings['motionProfiles'][currentProfile]["ampRan"]);
    Utils.toggleControlVisibilityByID("motionAmplitudeGlobalRandomMaxRow", userSettings['motionProfiles'][currentProfile]["ampRan"]);
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
    Utils.toggleControlVisibilityByID("motionOffsetGlobalRandomMinRow", userSettings['motionProfiles'][currentProfile]["offsetRan"]);
    Utils.toggleControlVisibilityByID("motionOffsetGlobalRandomMaxRow", userSettings['motionProfiles'][currentProfile]["offsetRan"]);
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
        validateIntControl('motionUpdateGlobal', userSettings['motionProfiles'][currentProfileIndex], "update");
        validateIntControl('motionPeriodGlobal', userSettings['motionProfiles'][currentProfileIndex], "period");
        validateIntControl('motionAmplitudeGlobal', userSettings['motionProfiles'][currentProfileIndex], "amp");
        validateIntControl('motionOffsetGlobal', userSettings['motionProfiles'][currentProfileIndex], "offset");
        // validateFloatControl('motionPhaseGlobal', userSettings["motionPhaseGlobal"]);

        setIntMinAndMax('motionPeriodGlobalRandomMin', 'motionPeriodGlobalRandomMax');
        validateIntControl('motionPeriodGlobalRandomMin', userSettings['motionProfiles'][currentProfileIndex], "periodMin");
        validateIntControl('motionPeriodGlobalRandomMax', userSettings['motionProfiles'][currentProfileIndex], "periodMax");
        setIntMinAndMax('motionAmplitudeGlobalRandomMin', 'motionAmplitudeGlobalRandomMax');
        validateIntControl('motionAmplitudeGlobalRandomMin', userSettings['motionProfiles'][currentProfileIndex], "ampMin");
        validateIntControl('motionAmplitudeGlobalRandomMax', userSettings['motionProfiles'][currentProfileIndex], "ampMax");
        setIntMinAndMax('motionOffsetGlobalRandomMin', 'motionOffsetGlobalRandomMax');
        validateIntControl('motionOffsetGlobalRandomMin', userSettings['motionProfiles'][currentProfileIndex], "offsetMin");
        validateIntControl('motionOffsetGlobalRandomMax', userSettings['motionProfiles'][currentProfileIndex], "offsetMax");
        setIntMinAndMax('motionRandomChangeMin', 'motionRandomChangeMax');
        validateIntControl('motionRandomChangeMin', userSettings['motionProfiles'][currentProfileIndex], "ranMin");
        validateIntControl('motionRandomChangeMax', userSettings['motionProfiles'][currentProfileIndex], "ranMax");

        
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
    button.innerText = profile.name;
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
    motionProfilesElements[optionsIndex].innerText = userSettings['motionProfiles'][profileIndex]["name"];
}
function setMotionGeneratorSettingsDefault() {
    var profileIndex = getMotionProfileSelectedIndex();
    if (confirm(`Are you sure you want to set the current profile '${userSettings['motionProfiles'][profileIndex]["name"]}' to the default settings?`)) {
        setProfileMotionGeneratorSettingsDefault(profileIndex);
        updateUserSettings(1);
    }
}
function setProfileMotionGeneratorSettingsDefault(profileIndex) {
    userSettings['motionProfiles'][profileIndex]["name"] = "Profile "+ (profileIndex + 1);
    userSettings['motionProfiles'][profileIndex]["update"] = 100;
    userSettings['motionProfiles'][profileIndex]["period"] = 2000;
    userSettings['motionProfiles'][profileIndex]["amp"] = 60;
    userSettings['motionProfiles'][profileIndex]["offset"] = 5000  
    userSettings['motionProfiles'][profileIndex]["periodRan"] = false;
    userSettings['motionProfiles'][profileIndex]["periodMin"] = 500;
    userSettings['motionProfiles'][profileIndex]["periodMax"] = 2000;
    userSettings['motionProfiles'][profileIndex]["ampRan"] = false;
    userSettings['motionProfiles'][profileIndex]["ampMin"] = 20;
    userSettings['motionProfiles'][profileIndex]["ampMax"] = 60;
    userSettings['motionProfiles'][profileIndex]["offsetRan"] = false;
    userSettings['motionProfiles'][profileIndex]["offsetMin"] = 3000;
    userSettings['motionProfiles'][profileIndex]["offsetMax"] = 7000;
    userSettings['motionProfiles'][profileIndex]["ranMin"] = 3000;
    userSettings['motionProfiles'][profileIndex]["ranMax"] = 30000;
    userSettings['motionProfiles'][profileIndex]["phase"] = 0;
    userSettings['motionProfiles'][profileIndex]["reverse"] = false;
    
    setMotionGeneratorSettingsProfile(profileIndex);
}
function setMotionGeneratorSettingsProfile(profileIndex) {
    document.getElementById('motionProfileName').value = userSettings['motionProfiles'][profileIndex]["name"];
    document.getElementById('motionUpdateGlobal').value = userSettings['motionProfiles'][profileIndex]["update"];
    document.getElementById('motionPeriodGlobal').value = userSettings['motionProfiles'][profileIndex]["period"];
    document.getElementById('motionAmplitudeGlobal').value = userSettings['motionProfiles'][profileIndex]["amp"];
    document.getElementById('motionOffsetGlobal').value = userSettings['motionProfiles'][profileIndex]["offset"];  
    //document.getElementById('motionPhaseGlobal').value = userSettings['motionProfiles'][profileIndex]["phase"];
    //document.getElementById('motionReversedGlobal').checked = userSettings['motionProfiles'][profileIndex]["reverse"];
    document.getElementById('motionPeriodGlobalRandom').checked = userSettings['motionProfiles'][profileIndex]["periodRan"];
    document.getElementById('motionPeriodGlobalRandomMin').value = userSettings['motionProfiles'][profileIndex]["periodMin"];  
    document.getElementById('motionPeriodGlobalRandomMax').value = userSettings['motionProfiles'][profileIndex]["periodMax"];  
    document.getElementById('motionAmplitudeGlobalRandom').checked = userSettings['motionProfiles'][profileIndex]["ampRan"];
    document.getElementById('motionAmplitudeGlobalRandomMin').value = userSettings['motionProfiles'][profileIndex]["ampMin"];  
    document.getElementById('motionAmplitudeGlobalRandomMax').value = userSettings['motionProfiles'][profileIndex]["ampMax"];  
    document.getElementById('motionOffsetGlobalRandom').checked = userSettings['motionProfiles'][profileIndex]["offsetRan"];
    document.getElementById('motionOffsetGlobalRandomMin').value = userSettings['motionProfiles'][profileIndex]["offsetMin"];  
    document.getElementById('motionOffsetGlobalRandomMax').value = userSettings['motionProfiles'][profileIndex]["offsetMax"];  
    document.getElementById('motionRandomChangeMin').value = userSettings['motionProfiles'][profileIndex]["ranMin"];  
    document.getElementById('motionRandomChangeMax').value = userSettings['motionProfiles'][profileIndex]["ranMax"];  
    toggleMotionRandomSettings();
}