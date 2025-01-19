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

MotionGenerator = {
    channelTemplates: [],
    setup() {
        const motionProfilesElement = document.getElementById('motionProfiles');
        removeAllChildren(motionProfilesElement);

        channelTemplates = [];
        motionProfilesElement.value = motionProviderSettings["motionDefaultProfileIndex"];

        var channels = getChannelMap();
        motionProviderSettings["motionProfiles"].forEach((x, profileIndex) => {
            addMotionProfileOption(profileIndex, x);
            var rootdiv = document.createElement("div");
            rootdiv.classList.add("formTable");
            rootdiv.style = "box-shadow: none; width: 100%;"
            rootdiv.id = "channelContainer"+profileIndex;
            rootdiv.name = "channelContainer";
            var channelParent = document.createElement("div");
            var header = document.createElement("div");
            header.classList.add("tHeader")
            // var title = document.createElement("h3");
            // title.innerText = "Edit profile";
            //header.appendChild(title);
            channelParent.appendChild(header);
            let profileRow = Utils.createTextFormRow(0, "Profile name", 'motionProfileName'+profileIndex, x.name, 31, function(profileIndex) {setMotionGeneratorName(profileIndex)}.bind(this, profileIndex));;
            channelParent.appendChild(profileRow.row);
            for(var i = 0; i < channels.length; i++) {
              const channelIndex = i;
              var channel = channels[i];
                if(!isSR6() && channel.sr6Only) {
                    continue;
                }
                var name = channel.channel;
                var friendlyName = channel.channelName;
                var motionChannelIndex = motionProviderSettings['motionProfiles'][profileIndex]["channels"].findIndex(y => y.name == name);
                const motionChannel = motionProviderSettings['motionProfiles'][profileIndex]["channels"][motionChannelIndex];
                
                var row = document.createElement("div");
                row.classList.add("tRow")
                row.classList.add("motion-profile-edit-channel-header");
                var cell1 = document.createElement("div");
                cell1.classList.add("tCell");
                var cell2 = document.createElement("div");
                cell2.classList.add("tCell");
                var label = document.createElement("span");
                label.for = "motionChannelCheckbox" + profileIndex + channelIndex;
                label.innerText = friendlyName +" ("+name+")";
                var checkbox = document.createElement("input");
                checkbox.id = "motionChannelCheckbox" + profileIndex + channelIndex;
                checkbox.type = "checkbox";
                checkbox.classList.add("motion-profile-edit-channel-header-checkbox");
                checkbox.value = `{\"channelName\": \"${name}\", \"profileIndex\": ${profileIndex}, \"channelIndex\": ${channelIndex}}`;
                checkbox.checked = motionChannelIndex > -1;
                checkbox.onclick = function () {
                    var properties = JSON.parse(this.value);
                    let index = getMotionChannelIndex(properties.profileIndex, properties.channelName);
                    if(index > -1) {
                        if (!confirm(`In order to save memory, this action will set the channel settings for this profile to DEFAULT. Are you sure?`)) {
                            this.checked = true;
                            return;
                        }
                        motionProviderSettings['motionProfiles'][properties.profileIndex]["channels"].splice(index, 1);
                        index = -1;
                    }
                    if(this.checked) {
                        motionProviderSettings['motionProfiles'][properties.profileIndex]["channels"].push(getDefaultMotionChannel(properties.channelName));
                    } else {
                        Utils.toggleControlVisibilityByID("channelTable"+properties.profileIndex+properties.channelIndex, false, true);
                    }
                    const button = document.getElementById("motionChannelEditButton"+properties.profileIndex+properties.channelIndex);
                    Utils.toggleElementShown(button, this.checked);
                    button.disabled = !this.checked;
                    MotionGenerator.updateSettings(profileIndex, index > -1 ? properties.channelIndex : -1);
                };
                var button = document.createElement("button");
                button.id = "motionChannelEditButton" + profileIndex + channelIndex;
                button.innerText = "Edit";
                Utils.toggleElementShown(button, checkbox.checked);
                button.disabled = !checkbox.checked;
                button.onclick = function() {
                    Utils.toggleControlVisibilityByName("channelTable", false, true);
                    const isPressed = this.classList.contains("button-pressed");
                    const pressedButtons = document.getElementsByClassName("button-pressed");
                    for(var i=0;i < pressedButtons.length; i++)
                        pressedButtons[i].classList.remove("button-pressed");
                    if(!isPressed) {
                        Utils.toggleControlVisibilityByID("channelTable"+profileIndex+channelIndex, true, true);
                        this.classList.add("button-pressed");
                    }
                }
                cell1.appendChild(label);
                cell1.appendChild(checkbox);
                cell2.appendChild(button);
                row.appendChild(cell1)
                row.appendChild(cell2)
                channelParent.appendChild(row);

                const channelTable = document.createElement("div");
                channelTable.id = "channelTable" + profileIndex + channelIndex;
                channelTable.setAttribute("name", "channelTable");
                channelTable.classList.add("formTable");
                Utils.toggleElementShown(channelTable, false, true);
                channelTable.style = "box-shadow: none; width: auto; margin: 0; padding: 0;"
                const channelTableDiv = document.createElement("div");

                let channelRow = Utils.createNumericFormRow(0, "Update rate (ms)", 'motionUpdate'+profileIndex+channelIndex, motionChannel ? motionChannel.update : 100, 0, 2147483647, 
                    function(profileIndex, channelIndex, name) {setMotionGeneratorSettings(profileIndex, channelIndex, name)}.bind(this, profileIndex, channelIndex, name));
                channelRow.title = `This is the time in between updates that gives the system time to process other tasks. (DO NOT SET TOO LOW ON ESP32!)
                It may be best to just leave at default.`
                channelTableDiv.appendChild(channelRow.row);

                //Period///////
                channelRow = Utils.createNumericFormRow(0, "Period (ms)", 'motionPeriod'+profileIndex+channelIndex, motionChannel ? motionChannel.period : 2000, 0, 2147483647,
                    function(profileIndex, channelIndex, name) {setMotionGeneratorSettings(profileIndex, channelIndex, name)}.bind(this, profileIndex, channelIndex, name));
                channelRow.title = `This is the time it takes to make one stroke.
                Lower is faster movement.`
                channelTableDiv.appendChild(channelRow.row);

                const periodRan = motionChannel ? motionChannel.periodRan : false;
                channelRow = Utils.createCheckboxFormRow(0, "Period random", 'motionPeriodRandom'+profileIndex+channelIndex, periodRan,
                    function(profileIndex, channelIndex, name) {setMotionPeriodRandomClicked(profileIndex, channelIndex, name)}.bind(this, profileIndex, channelIndex, name));
                channelRow.title = `This is the time it takes to make one stroke.
                Lower is faster movement.`
                channelTableDiv.appendChild(channelRow.row);

                channelRow = Utils.createNumericFormRow("motionPeriodRandomMinRow"+profileIndex+channelIndex, "Min (ms)", 'motionPeriodRandomMin'+profileIndex+channelIndex, motionChannel ? motionChannel.periodMin : 500, 0, 2147483647,
                    function(profileIndex, channelIndex, name) {setMotionGeneratorSettings(profileIndex, channelIndex, name)}.bind(this, profileIndex, channelIndex, name));
                    channelTableDiv.appendChild(channelRow.row);

                let randomMin = channelRow.input;
                let randomRowMin = channelRow;
                
                channelRow = Utils.createNumericFormRow("motionPeriodRandomMaxRow"+profileIndex+channelIndex, "Max (ms)", 'motionPeriodRandomMax'+profileIndex+channelIndex, motionChannel ? motionChannel.periodMax : 2000, 0, 2147483647,
                    function(profileIndex, channelIndex, name) {setMotionGeneratorSettings(profileIndex, channelIndex, name)}.bind(this, profileIndex, channelIndex, name));
                channelTableDiv.appendChild(channelRow.row);

                setElementsIntMinAndMax(randomMin, channelRow.input);
                toggleMotionRandomSettingsElement(randomRowMin.row, channelRow.row, periodRan);
                
                //Amplitude///////
                channelRow = Utils.createNumericFormRow(0, "Amplitude (%)", 'motionAmplitude'+profileIndex+channelIndex, motionChannel ? motionChannel.amp : 60, 0, 100,
                    function(profileIndex, channelIndex, name) {setMotionGeneratorSettings(profileIndex, channelIndex, name)}.bind(this, profileIndex, channelIndex, name));
                channelTableDiv.appendChild(channelRow.row);

                const ampRan = motionChannel ? motionChannel.ampRan : false;
                channelRow = Utils.createCheckboxFormRow(0, "Amplitude random", 'motionAmplitudeRandom'+profileIndex+channelIndex, ampRan,
                    function(profileIndex, channelIndex, name) {setMotionAmplitudeRandomClicked(profileIndex, channelIndex, name)}.bind(this, profileIndex, channelIndex, name));
                channelTableDiv.appendChild(channelRow.row);
                
                channelRow = Utils.createNumericFormRow("motionAmplitudeRandomMinRow"+profileIndex+channelIndex, "Min (%)", 'motionAmplitudeRandomMin'+profileIndex+channelIndex, motionChannel ? motionChannel.ampMin : 20, 0, 100,
                    function(profileIndex, channelIndex, name) {setMotionGeneratorSettings(profileIndex, channelIndex, name)}.bind(this, profileIndex, channelIndex, name));
                channelTableDiv.appendChild(channelRow.row);

                randomMin = channelRow.input;
                randomRowMin = channelRow;
                
                channelRow = Utils.createNumericFormRow("motionAmplitudeRandomMaxRow"+profileIndex+channelIndex, "Max (%)", 'motionAmplitudeRandomMax'+profileIndex+channelIndex, motionChannel ? motionChannel.ampMax : 60, 0, 100,
                    function(profileIndex, channelIndex, name) {setMotionGeneratorSettings(profileIndex, channelIndex, name)}.bind(this, profileIndex, channelIndex, name));
                channelTableDiv.appendChild(channelRow.row);

                setElementsIntMinAndMax(randomMin, channelRow.input);
                toggleMotionRandomSettingsElement(randomRowMin.row, channelRow.row, ampRan);

                //Offset///////
                channelRow = Utils.createNumericFormRow(0, "Offset (tcode)", 'motionOffset'+profileIndex+channelIndex, motionChannel ? motionChannel.offset : 5000, 0, 9999,
                    function(profileIndex, channelIndex, name) {setMotionGeneratorSettings(profileIndex, channelIndex, name)}.bind(this, profileIndex, channelIndex, name));
                channelTableDiv.appendChild(channelRow.row);

                const offsetRan = motionChannel ? motionChannel.offsetRan : false;
                channelRow = Utils.createCheckboxFormRow(0, "Offset random", 'motionOffsetRandom'+profileIndex+channelIndex, offsetRan,
                    function(profileIndex, channelIndex, name) {setMotionOffsetRandomClicked(profileIndex, channelIndex, name)}.bind(this, profileIndex, channelIndex, name));
                channelTableDiv.appendChild(channelRow.row);
                
                channelRow = Utils.createNumericFormRow("motionOffsetRandomMinRow"+profileIndex+channelIndex, "Min (tcode)", 'motionOffsetRandomMin'+profileIndex+channelIndex, motionChannel ? motionChannel.offsetMin : 3000, 0, 9999,
                    function(profileIndex, channelIndex, name) {setMotionGeneratorSettings(profileIndex, channelIndex, name)}.bind(this, profileIndex, channelIndex, name));
                channelTableDiv.appendChild(channelRow.row);

                randomMin = channelRow.input;
                randomRowMin = channelRow;
                
                channelRow = Utils.createNumericFormRow("motionOffsetRandomMaxRow"+profileIndex+channelIndex, "Max (tcode)", 'motionOffsetRandomMax'+profileIndex+channelIndex, motionChannel ? motionChannel.offsetMax : 7000, 0, 9999,
                    function(profileIndex, channelIndex, name) {setMotionGeneratorSettings(profileIndex, channelIndex, name)}.bind(this, profileIndex, channelIndex, name));
                channelTableDiv.appendChild(channelRow.row);

                setElementsIntMinAndMax(randomMin, channelRow.input);
                toggleMotionRandomSettingsElement(randomRowMin.row, channelRow.row, offsetRan);

                //Phase///////
                channelRow = Utils.createNumericFormRow(0, "Phase (degree)", 'motionPhase'+profileIndex+channelIndex, motionChannel ? motionChannel.phase : 0, 0.0, 180.0,
                    function(profileIndex, channelIndex, name) {setMotionGeneratorSettings(profileIndex, channelIndex, name)}.bind(this, profileIndex, channelIndex, name));
                channelTableDiv.appendChild(channelRow.row);

                const phaseRan = motionChannel ? motionChannel.phaseRan : false;
                channelRow = Utils.createCheckboxFormRow(0, "Phase random", 'motionPhaseRandom'+profileIndex+channelIndex, phaseRan,
                    function(profileIndex, channelIndex, name) {setMotionPhaseRandomClicked(profileIndex, channelIndex, name)}.bind(this, profileIndex, channelIndex, name));
                channelTableDiv.appendChild(channelRow.row);
                
                channelRow = Utils.createNumericFormRow("motionPhaseRandomMinRow"+profileIndex+channelIndex, "Min (degree)", 'motionPhaseRandomMin'+profileIndex+channelIndex, motionChannel ? motionChannel.phaseMin : 0.0, 0.0, 180.0,
                    function(profileIndex, channelIndex, name) {setMotionGeneratorSettings(profileIndex, channelIndex, name)}.bind(this, profileIndex, channelIndex, name));
                channelTableDiv.appendChild(channelRow.row);
                
                randomMin = channelRow.input;
                randomRowMin = channelRow;

                channelRow = Utils.createNumericFormRow("motionPhaseRandomMaxRow"+profileIndex+channelIndex, "Max (degree)", 'motionPhaseRandomMax'+profileIndex+channelIndex, motionChannel ? motionChannel.phaseMax : 180.0, 0.0, 180.0,
                    function(profileIndex, channelIndex, name) {setMotionGeneratorSettings(profileIndex, channelIndex, name)}.bind(this, profileIndex, channelIndex, name));
                channelTableDiv.appendChild(channelRow.row);

                setElementsIntMinAndMax(randomMin, channelRow.input);
                toggleMotionRandomSettingsElement(randomRowMin.row, channelRow.row, phaseRan);

                //Global///////
                channelRow = Utils.createNumericFormRow("motionRandomChangeMinRow"+profileIndex+channelIndex, "Random change max (ms)", 'motionRandomChangeMin'+profileIndex+channelIndex, motionChannel ? motionChannel.ranMin : 3000, 0, 2147483647,
                    function(profileIndex, channelIndex, name) {setMotionGeneratorSettings(profileIndex, channelIndex, name)}.bind(this, profileIndex, channelIndex, name));
                channelTableDiv.appendChild(channelRow.row);

                randomMin = channelRow.input;
                randomRowMin = channelRow;
                
                channelRow = Utils.createNumericFormRow("motionRandomChangeMaxRow"+profileIndex+channelIndex, "Random change max (ms)", 'motionRandomChangeMax'+profileIndex+channelIndex, motionChannel ? motionChannel.ranMax : 30000, 0, 2147483647,
                    function(profileIndex, channelIndex, name) {setMotionGeneratorSettings(profileIndex, channelIndex, name)}.bind(this, profileIndex, channelIndex, name));
                channelTableDiv.appendChild(channelRow.row);

                setElementsIntMinAndMax(randomMin, channelRow.input);
                toggleMotionRandomSettingsElement(randomRowMin.row, channelRow.row, phaseRan || ampRan || offsetRan || periodRan);

                const buttonDefault = document.createElement("button");
                buttonDefault.onclick = function(profileIndex, channelIndex, channelName) {
                    setMotionGeneratorSettingsDefault(profileIndex, channelIndex, channelName);
                }.bind(this, profileIndex, channelIndex, name);
                

                channelParent.appendChild(channelTable);
                channelTable.appendChild(channelTableDiv);
            };
            rootdiv.appendChild(channelParent);
            channelTemplates.push(rootdiv);
        });
        const defaultMotionProfileButton = document.createElement("button");
        defaultMotionProfileButton.innerText = "Set selected profile as default";
        defaultMotionProfileButton.onclick = setMotionProfileDefault;
        motionProfilesElement.appendChild(defaultMotionProfileButton);
        // const rangeSliderButton = document.createElement("button");
        // rangeSliderButton.innerText = "Edit device range limits";
        // rangeSliderButton.onclick = DeviceRangeSlider.show;
        // motionProfilesElement.appendChild(rangeSliderButton);
        
        selectMotionProfile(systemInfo["motionSelectedProfileIndex"]);
        this.setEnabledStatus();
    },
    setEnabledStatus() {
        var button = document.getElementById('motionEnabledToggle');
        button.innerText = systemInfo["motionEnabled"] ? "Stop" : "Start"
        if(systemInfo["motionEnabled"]) {
            button.classList.add("button-toggle-stop");
            button.classList.remove("button-toggle-start");
        } else {
            button.classList.remove("button-toggle-stop");
            button.classList.add("button-toggle-start");
        }
    },
    /*
    * Warning motionChannelIndex is the channel index in the motion profile and NOT the index of the AvailableChannels array!
    */
    updateSettings(profileIndex, motionChannelIndex, debounce) {
        if(profileIndex && motionChannelIndex) {
            this.setEdited(profileIndex, motionChannelIndex);
        }
        updateUserSettings(debounce, EndPointType.MotionProfile.uri, motionProviderSettings);
    },
    setEdited(profileIndex, motionChannelIndex) {
        if(profileIndex > -1)
            motionProviderSettings.motionProfiles[profileIndex]["edited"] = true;
        if(motionChannelIndex > -1)
            motionProviderSettings.motionProfiles[profileIndex]["channels"][motionChannelIndex]["edited"] = true;
    }
}

function toggleMotionRandomSettings(profileIndex, channelIndex) {
    const offsetRan = document.getElementById('motionOffsetRandom'+profileIndex+channelIndex).checked;
    const ampRan = document.getElementById('motionAmplitudeRandom'+profileIndex+channelIndex).checked;
    const periodRan = document.getElementById('motionPeriodRandom'+profileIndex+channelIndex).checked;
    const phaseRan = document.getElementById('motionPhaseRandom'+profileIndex+channelIndex).checked;
    Utils.toggleControlVisibilityByID("motionOffsetRandomMinRow"+profileIndex+channelIndex, offsetRan);
    Utils.toggleControlVisibilityByID("motionOffsetRandomMaxRow"+profileIndex+channelIndex, offsetRan);
    Utils.toggleControlVisibilityByID("motionAmplitudeRandomMinRow"+profileIndex+channelIndex, ampRan);
    Utils.toggleControlVisibilityByID("motionAmplitudeRandomMaxRow"+profileIndex+channelIndex, ampRan);
    Utils.toggleControlVisibilityByID("motionPeriodRandomMinRow"+profileIndex+channelIndex, periodRan);
    Utils.toggleControlVisibilityByID("motionPeriodRandomMaxRow"+profileIndex+channelIndex, periodRan);
    Utils.toggleControlVisibilityByID("motionPhaseRandomMinRow"+profileIndex+channelIndex, phaseRan);
    Utils.toggleControlVisibilityByID("motionPhaseRandomMaxRow"+profileIndex+channelIndex, phaseRan);
    toggleMotionRandomMinMaxSettingsSettings(profileIndex, channelIndex);
}
function toggleMotionRandomSettingsElement(minElement, maxElement, ranChecked) {
    Utils.toggleElementShown(minElement, ranChecked);
    Utils.toggleElementShown(maxElement, ranChecked);
}
function toggleMotionRandomMinMaxSettingsSettings(profileIndex, channelIndex) {
    const offsetRan = document.getElementById('motionOffsetRandom'+profileIndex+channelIndex).checked;
    const ampRan = document.getElementById('motionAmplitudeRandom'+profileIndex+channelIndex).checked;
    const periodRan = document.getElementById('motionPeriodRandom'+profileIndex+channelIndex).checked;
    const phaseRan = document.getElementById('motionPhaseRandom'+profileIndex+channelIndex).checked;
    Utils.toggleControlVisibilityByID("motionRandomChangeMinRow"+profileIndex+channelIndex, offsetRan || ampRan || periodRan || phaseRan);
    Utils.toggleControlVisibilityByID("motionRandomChangeMaxRow"+profileIndex+channelIndex, offsetRan || ampRan || periodRan || phaseRan);
}

function setMotionEnabledClicked() {
    systemInfo["motionEnabled"] = !systemInfo["motionEnabled"];//document.getElementById('motionEnabled').checked;
    sendTCode(systemInfo["motionEnabled"] ? "#motion-enable" : "#motion-disable");
    //systemInfo["motionEnabled"] ? button.classList.add("button-toggle-stop") : button.classList.remove("button-toggle-stop");
    MotionGenerator.setEnabledStatus();
}
function setMotionPeriodRandomClicked(profileIndex, channelIndex, channelName) {
    var motionEnabled = document.getElementById('motionPeriodRandom'+profileIndex+channelIndex).checked;
    // if(motionEnabled) {
    //     sendTCode("#motion-period-random-on");
    // } else {
    //     sendTCode("#motion-period-random-off");
    // }
    const motionChannelIndex = getMotionChannelIndex(profileIndex, channelName);
    motionProviderSettings['motionProfiles'][profileIndex]["channels"][motionChannelIndex]["periodRan"] = motionEnabled;
    Utils.toggleControlVisibilityByID("motionPeriodRandomMinRow"+profileIndex+channelIndex, motionProviderSettings['motionProfiles'][profileIndex]["channels"][motionChannelIndex]["periodRan"]);
    Utils.toggleControlVisibilityByID("motionPeriodRandomMaxRow"+profileIndex+channelIndex, motionProviderSettings['motionProfiles'][profileIndex]["channels"][motionChannelIndex]["periodRan"]);
    toggleMotionRandomMinMaxSettingsSettings(profileIndex, channelIndex);
    MotionGenerator.updateSettings(profileIndex, motionChannelIndex);
}
function setMotionAmplitudeRandomClicked(profileIndex, channelIndex, channelName) {
    var motionEnabled = document.getElementById('motionAmplitudeRandom'+profileIndex+channelIndex).checked;
    // if(motionEnabled) {
    //     sendTCode("#motion-amplitude-random-on");
    // } else {
    //     sendTCode("#motion-amplitude-random-off");
    // }
    const motionChannelIndex = getMotionChannelIndex(profileIndex, channelName);
    motionProviderSettings['motionProfiles'][profileIndex]["channels"][motionChannelIndex]["ampRan"] = motionEnabled;
    Utils.toggleControlVisibilityByID("motionAmplitudeRandomMinRow"+profileIndex+channelIndex, motionEnabled);
    Utils.toggleControlVisibilityByID("motionAmplitudeRandomMaxRow"+profileIndex+channelIndex, motionEnabled);
    toggleMotionRandomMinMaxSettingsSettings(profileIndex, channelIndex);
    MotionGenerator.updateSettings(profileIndex, motionChannelIndex);
}
function setMotionOffsetRandomClicked(profileIndex, channelIndex, channelName) {
    var motionEnabled = document.getElementById('motionOffsetRandom'+profileIndex+channelIndex).checked;
    // if(motionEnabled) {
    //     sendTCode("#motion-offset-random-on");
    // } else {
    //     sendTCode("#motion-offset-random-off");
    // }
    const motionChannelIndex = getMotionChannelIndex(profileIndex, channelName);
    motionProviderSettings['motionProfiles'][profileIndex]["channels"][motionChannelIndex]["offsetRan"] = motionEnabled;
    Utils.toggleControlVisibilityByID("motionOffsetRandomMinRow"+profileIndex+channelIndex, motionEnabled);
    Utils.toggleControlVisibilityByID("motionOffsetRandomMaxRow"+profileIndex+channelIndex, motionEnabled);
    toggleMotionRandomMinMaxSettingsSettings(profileIndex, channelIndex);
    MotionGenerator.updateSettings(profileIndex, motionChannelIndex);
}
function setMotionPhaseRandomClicked(profileIndex, channelIndex, channelName) {
    var motionEnabled = document.getElementById('motionPhaseRandom'+profileIndex+channelIndex).checked;
    // if(motionEnabled) {
    //     sendTCode("#motion-offset-random-on");
    // } else {
    //     sendTCode("#motion-offset-random-off");
    // }
    const motionChannelIndex = getMotionChannelIndex(profileIndex, channelName);
    motionProviderSettings['motionProfiles'][profileIndex]["channels"][motionChannelIndex]["phaseRan"] = motionEnabled;
    Utils.toggleControlVisibilityByID("motionPhaseRandomMinRow"+profileIndex+channelIndex, motionEnabled);
    Utils.toggleControlVisibilityByID("motionPhaseRandomMaxRow"+profileIndex+channelIndex, motionEnabled);
    toggleMotionRandomMinMaxSettingsSettings(profileIndex, channelIndex);
    MotionGenerator.updateSettings(profileIndex, motionChannelIndex);
}

function setMotionProfileDefault() {
    motionProviderSettings["motionDefaultProfileIndex"] = getMotionProfileSelectedIndex();
    MotionGenerator.updateSettings(undefined, undefined, 0);
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
    if(sendTCodeCommand)
        sendTCode(`#motion-profile-set:${profileIndex + 1}`);
}
function editMotionProfile(profileIndex) {
    const modal = document.getElementById("motionChannelsModal");
    removeAllChildren(modal);
    modal.appendChild(channelTemplates[profileIndex]);
    setMotionGeneratorSettingsProfile(profileIndex);
    const header = document.createElement("span");
    header.innerText = "Edit profile"  
    header.setAttribute("slot", "title");
    modal.appendChild(header);
    //modal.setAttribute("visible", true);
    modal.show();
}

var validateGeneratorSettingsDebounce;
function setMotionGeneratorSettings(profileIndex, channelIndex, channelName) {
    if(validateGeneratorSettingsDebounce) {
        clearTimeout(validateGeneratorSettingsDebounce);
    }
    validateGeneratorSettingsDebounce = setTimeout(function (profileIndex, channelIndex, channelName) {
        var motionChannelIndex = motionProviderSettings['motionProfiles'][profileIndex]["channels"].findIndex(x => x.name == channelName);
        if(motionChannel == -1) {
            showError("There was an error finding a channel with name: "+channelName);
            return;
        }
        var motionChannel = motionProviderSettings['motionProfiles'][profileIndex]["channels"][motionChannelIndex];
        validateIntControl('motionUpdate'+profileIndex+channelIndex, motionChannel, "update");
        validateIntControl('motionPeriod'+profileIndex+channelIndex, motionChannel, "period");
        validateIntControl('motionAmplitude'+profileIndex+channelIndex, motionChannel, "amp");
        validateIntControl('motionOffset'+profileIndex+channelIndex, motionChannel, "offset");
        validateIntControl('motionPhase'+profileIndex+channelIndex, motionChannel, "phase");
        // validateFloatControl('motionPhase', userSettings["motionPhase"]);

        setIntMinAndMax('motionPeriodRandomMin'+profileIndex+channelIndex, 'motionPeriodRandomMax'+profileIndex+channelIndex);
        validateIntControl('motionPeriodRandomMin'+profileIndex+channelIndex, motionChannel, "periodMin");
        validateIntControl('motionPeriodRandomMax'+profileIndex+channelIndex, motionChannel, "periodMax");
        setIntMinAndMax('motionAmplitudeRandomMin'+profileIndex+channelIndex, 'motionAmplitudeRandomMax'+profileIndex+channelIndex);
        validateIntControl('motionAmplitudeRandomMin'+profileIndex+channelIndex, motionChannel, "ampMin");
        validateIntControl('motionAmplitudeRandomMax'+profileIndex+channelIndex, motionChannel, "ampMax");
        setIntMinAndMax('motionOffsetRandomMin'+profileIndex+channelIndex, 'motionOffsetRandomMax'+profileIndex+channelIndex);
        validateIntControl('motionOffsetRandomMin'+profileIndex+channelIndex, motionChannel, "offsetMin");
        validateIntControl('motionOffsetRandomMax'+profileIndex+channelIndex, motionChannel, "offsetMax");
        setIntMinAndMax('motionPhaseRandomMin'+profileIndex+channelIndex, 'motionPhaseRandomMax'+profileIndex+channelIndex);
        validateFloatControl('motionPhaseRandomMin'+profileIndex+channelIndex, motionChannel, "phaseMin");
        validateFloatControl('motionPhaseRandomMax'+profileIndex+channelIndex, motionChannel, "phaseMax");
        setIntMinAndMax('motionRandomChangeMin'+profileIndex+channelIndex, 'motionRandomChangeMax'+profileIndex+channelIndex);
        validateIntControl('motionRandomChangeMin'+profileIndex+channelIndex, motionChannel, "ranMin");
        validateIntControl('motionRandomChangeMax'+profileIndex+channelIndex, motionChannel, "ranMax");

        
        //userSettings["motionReversed"] = document.getElementById('motionReversed').checked;
        MotionGenerator.updateSettings(profileIndex, motionChannelIndex, 0);
    }.bind(this, profileIndex, channelIndex, channelName), 3000);
}
var setMotionGeneratorNameDebounce;
function setMotionGeneratorName(profileIndex) {
    if(setMotionGeneratorNameDebounce) {
        clearTimeout(setMotionGeneratorNameDebounce);
    }
    
    setMotionGeneratorNameDebounce = setTimeout(function(profileIndex) {
        if(validateStringControl("motionProfileName"+profileIndex, motionProviderSettings['motionProfiles'][profileIndex], "name")) {
            updateMotionProfileName(profileIndex);
            MotionGenerator.updateSettings(profileIndex, -1, 1);
        }
    }.bind(this, profileIndex), 3000);
}
function addMotionProfileOption(profileIndex, profile) {
    const channelContainers = document.getElementsByName("channelContainer");
    channelContainers.forEach(x => {
        Utils.toggleElementShown(x, false);
    });
    const motionProfilesElement = document.getElementById('motionProfiles');
    var button = document.createElement("button");
    button.id = `motionProfile${profileIndex}`;
    button.name = "motionProfileButton";
    button.value = profileIndex;
    button.innerText = profile.name;
    button.style.width = "80%";
    button.onclick = function(profileIndex) {
        selectMotionProfile(profileIndex, true);
    }.bind(this, profileIndex)
    var buttonEdit = document.createElement("button");
    buttonEdit.id = `motionProfileEdit${profileIndex}`;
    buttonEdit.name = "motionProfileEditButton";
    buttonEdit.value = profileIndex;
    buttonEdit.innerText = "Edit";
    buttonEdit.style.width = "20%";
    buttonEdit.onclick = function(profileIndex) {
        editMotionProfile(profileIndex);
    }.bind(this, profileIndex)

    motionProfilesElement.appendChild(button);
    motionProfilesElement.appendChild(buttonEdit);
        //motionProfilesElement.value = profileName;
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
    motionProfilesElements[optionsIndex].innerText = motionProviderSettings['motionProfiles'][profileIndex]["name"];
}
function setMotionGeneratorSettingsDefault(profileIndex, channelIndex, channelName) {
    if (confirm(`Are you sure you want to set the current channel '${channelName}' in profile '${motionProviderSettings['motionProfiles'][profileIndex]["name"]}' to the default settings?`)) {
        setProfileMotionGeneratorSettingsDefault(profileIndex, channelIndex, channelName);
        const motionChannelIndex = getMotionChannelIndex(profileIndex, channelName);
        MotionGenerator.updateSettings(profileIndex, motionChannelIndex, 0);
    }
}
function getMotionChannelIndex(profileIndex, channelName) {
    return motionProviderSettings['motionProfiles'][profileIndex]["channels"].findIndex(x => x.name == channelName);
}
function getDefaultMotionChannel(name) {
    return {
        name: name,
        update: 100,
        period: 2000,
        amp: 60,
        offset: 5000,
        periodRan: false,
        periodMin: 500,
        periodMax: 2000,
        ampRan: false,
        ampMin: 20,
        ampMax: 60,
        offsetRan: false,
        offsetMin: 3000,
        offsetMax: 7000,
        ranMin: 3000,
        ranMax: 30000,
        phase: 0,
        phaseRan: false,
        phaseMin: 0.0,
        phaseMax: 180.0,
        reverse: false
    }
}
function setProfileMotionGeneratorSettingsDefault(profileIndex, channelIndex, channelName) {
    motionProviderSettings['motionProfiles'][profileIndex]["name"] = "Profile "+ (profileIndex + 1);
    motionProviderSettings['motionProfiles'][profileIndex]["channels"][channelIndex] = getDefaultMotionChannel(channelName);
    
    setMotionGeneratorSettingsProfile(profileIndex);
}
function setMotionGeneratorSettingsProfile(profileIndex) {
    document.getElementById('motionProfileName'+profileIndex).value = motionProviderSettings['motionProfiles'][profileIndex]["name"];
    //const channels = motionProviderSettings['motionProfiles'][profileIndex]["channels"];
    const channels = getChannelMap();
    for(var i = 0; i < channels.length; i++) {
        const channelIndex = i;
        const channel = channels[i];
        if (!isSR6() && channel.sr6Only) 
            continue;
        const motionChannelIndex = getMotionChannelIndex(profileIndex, channel.channel);
        const profileHasChannel = motionChannelIndex > -1;
        const defaultChannel = getDefaultMotionChannel();
        document.getElementById('motionUpdate'+profileIndex+channelIndex).value = profileHasChannel ? motionProviderSettings['motionProfiles'][profileIndex]["channels"][motionChannelIndex]["update"] : defaultChannel["update"];
        document.getElementById('motionPeriod'+profileIndex+channelIndex).value = profileHasChannel ? motionProviderSettings['motionProfiles'][profileIndex]["channels"][motionChannelIndex]["period"] : defaultChannel["period"];
        document.getElementById('motionAmplitude'+profileIndex+channelIndex).value = profileHasChannel ? motionProviderSettings['motionProfiles'][profileIndex]["channels"][motionChannelIndex]["amp"] : defaultChannel["amp"];
        document.getElementById('motionOffset'+profileIndex+channelIndex).value = profileHasChannel ? motionProviderSettings['motionProfiles'][profileIndex]["channels"][motionChannelIndex]["offset"] : defaultChannel["offset"];  
        document.getElementById('motionPhase'+profileIndex+channelIndex).value = profileHasChannel ? motionProviderSettings['motionProfiles'][profileIndex]["channels"][motionChannelIndex]["phase"] : defaultChannel["phase"];
        document.getElementById('motionPhaseRandom'+profileIndex+channelIndex).checked = profileHasChannel ? motionProviderSettings['motionProfiles'][profileIndex]["channels"][motionChannelIndex]["phaseRan"] : defaultChannel["phaseRan"];
        document.getElementById('motionPhaseRandomMin'+profileIndex+channelIndex).value = profileHasChannel ? motionProviderSettings['motionProfiles'][profileIndex]["channels"][motionChannelIndex]["phaseMin"] : defaultChannel["phaseMin"];  
        document.getElementById('motionPhaseRandomMax'+profileIndex+channelIndex).value = profileHasChannel ? motionProviderSettings['motionProfiles'][profileIndex]["channels"][motionChannelIndex]["phaseMax"] : defaultChannel["phaseMax"];  
        //document.getElementById('motionReversed'+profileIndex+channelIndex).checked = profileHasChannel ? motionProviderSettings['motionProfiles'][profileIndex]["channels"][motionChannelIndex]["reverse"] : defaultChannel["reverse"];
        document.getElementById('motionPeriodRandom'+profileIndex+channelIndex).checked = profileHasChannel ? motionProviderSettings['motionProfiles'][profileIndex]["channels"][motionChannelIndex]["periodRan"] : defaultChannel["periodRan"];
        document.getElementById('motionPeriodRandomMin'+profileIndex+channelIndex).value = profileHasChannel ? motionProviderSettings['motionProfiles'][profileIndex]["channels"][motionChannelIndex]["periodMin"] : defaultChannel["periodMin"];  
        document.getElementById('motionPeriodRandomMax'+profileIndex+channelIndex).value = profileHasChannel ? motionProviderSettings['motionProfiles'][profileIndex]["channels"][motionChannelIndex]["periodMax"] : defaultChannel["periodMax"];  
        document.getElementById('motionAmplitudeRandom'+profileIndex+channelIndex).checked = profileHasChannel ? motionProviderSettings['motionProfiles'][profileIndex]["channels"][motionChannelIndex]["ampRan"] : defaultChannel["ampRan"];
        document.getElementById('motionAmplitudeRandomMin'+profileIndex+channelIndex).value = profileHasChannel ? motionProviderSettings['motionProfiles'][profileIndex]["channels"][motionChannelIndex]["ampMin"] : defaultChannel["ampMin"];  
        document.getElementById('motionAmplitudeRandomMax'+profileIndex+channelIndex).value = profileHasChannel ? motionProviderSettings['motionProfiles'][profileIndex]["channels"][motionChannelIndex]["ampMax"] : defaultChannel["ampMax"];  
        document.getElementById('motionOffsetRandom'+profileIndex+channelIndex).checked = profileHasChannel ? motionProviderSettings['motionProfiles'][profileIndex]["channels"][motionChannelIndex]["offsetRan"] : defaultChannel["offsetRan"];
        document.getElementById('motionOffsetRandomMin'+profileIndex+channelIndex).value = profileHasChannel ? motionProviderSettings['motionProfiles'][profileIndex]["channels"][motionChannelIndex]["offsetMin"] : defaultChannel["offsetMin"];  
        document.getElementById('motionOffsetRandomMax'+profileIndex+channelIndex).value = profileHasChannel ? motionProviderSettings['motionProfiles'][profileIndex]["channels"][motionChannelIndex]["offsetMax"] : defaultChannel["offsetMax"];  
        document.getElementById('motionRandomChangeMin'+profileIndex+channelIndex).value = profileHasChannel ? motionProviderSettings['motionProfiles'][profileIndex]["channels"][motionChannelIndex]["ranMin"] : defaultChannel["ranMin"];  
        document.getElementById('motionRandomChangeMax'+profileIndex+channelIndex).value = profileHasChannel ? motionProviderSettings['motionProfiles'][profileIndex]["channels"][motionChannelIndex]["ranMax"] : defaultChannel["ranMax"];  
        toggleMotionRandomSettings(profileIndex, channelIndex);
    };
}