MotionGenerator = {
    channelTemplates: [],
    setup() {
        const motionProfilesElement = document.getElementById('motionProfiles');
        removeAllChildren(motionProfilesElement);

        channelTemplates = [];
        const currentProfileIndex = userSettings["motionDefaultProfileIndex"];
        motionProfilesElement.value = currentProfileIndex;

        var channels = getChannelMap();
        userSettings["motionProfiles"].forEach((x, index) => {
            addMotionProfileOption(index, x);
            var rootdiv = document.createElement("div");
            rootdiv.classList.add("formTable");
            rootdiv.style = "box-shadow: none; width: 100%;"
            rootdiv.id = "channelContainer"+index;
            rootdiv.name = "channelContainer";
            var channelParent = document.createElement("div");
            var header = document.createElement("div");
            header.classList.add("tHeader")
            var title = document.createElement("h3");
            title.innerText = "Edit profile";
            header.appendChild(title);
            channelParent.appendChild(header);
            let row = Utils.createTextFormRow(0, "Profile name", 'motionProfileName'+index, x.name, 31, function(index) {setMotionGeneratorName(index)}.bind(this, index));;
            channelParent.appendChild(row.row);
            channels.forEach((channel, channelIndex) => {
                var name = channel.channel;
                var friendlyName = channel.channelName;
                var motionChannelIndex = userSettings['motionProfiles'][currentProfileIndex]["channels"].findIndex(y => y.name == name);
                const motionChannel = userSettings['motionProfiles'][currentProfileIndex]["channels"][motionChannelIndex];
                
                var row = document.createElement("div");
                row.classList.add("tRow")
                row.classList.add("motion-profile-edit-channel-header");
                var cell1 = document.createElement("div");
                cell1.classList.add("tCell");
                var cell2 = document.createElement("div");
                cell2.classList.add("tCell");
                var label = document.createElement("span");
                label.for = "motionChannelCheckbox" + channelIndex;
                label.innerText = friendlyName +" ("+name+")";
                var checkbox = document.createElement("input");
                checkbox.id = "motionChannelCheckbox" + channelIndex;
                checkbox.type = "checkbox";
                checkbox.classList.add("motion-profile-edit-channel-header-checkbox");
                checkbox.value = {name: name, profileIndex: index, channelIndex: channelIndex};
                checkbox.checked = motionChannelIndex > -1;
                checkbox.onclick = function () {
                    var properties = this.value;
                    const index = userSettings['motionProfiles'][currentProfileIndex]["channels"].findIndex(x => x.name == properties.name);
                    if(index > -1) {
                        motionChannel = userSettings['motionProfiles'][currentProfileIndex]["channels"][index];
                        userSettings['motionProfiles'][currentProfileIndex]["channels"].splice(index, 1);
                    }
                    if(this.checked) {
                        setProfileMotionGeneratorSettingsDefault(properties.profileIndex, properties.channelIndex);
                    }
                    updateUserSettings();
                };
                var button = document.createElement("button");
                button.innerText = "Edit";
                button.onclick = function() {
                    
                }
                cell1.appendChild(label);
                cell1.appendChild(checkbox);
                cell2.appendChild(button);
                row.appendChild(cell1)
                row.appendChild(cell2)
                channelParent.appendChild(row);

                let channelRow = Utils.createNumericFormRow(0, "Update rate (ms)", 'motionUpdate'+channelIndex, motionChannel ? motionChannel.update : 100, 0, 2147483647, 
                    function(channelIndex) {setMotionGeneratorSettings(channelIndex)}.bind(this, channelIndex));
                channelRow.title = `This is the time in between updates that gives the system time to process other tasks. (DO NOT SET TOO LOW ON ESP32!)
                It may be best to just leave at default.`
                channelParent.appendChild(channelRow.row);

                //Period///////
                channelRow = Utils.createNumericFormRow(0, "Period (ms)", 'motionPeriod'+channelIndex, motionChannel ? motionChannel.period : 2000, 0, 2147483647,
                    function(channelIndex) {setMotionGeneratorSettings(channelIndex)}.bind(this, channelIndex));
                channelRow.title = `This is the time it takes to make one stroke.
                Lower is faster movement.`
                channelParent.appendChild(channelRow.row);

                channelRow = Utils.createCheckboxFormRow(0, "Period random", 'motionPeriodRandom'+channelIndex, motionChannel ? motionChannel.periodRan : false,
                    function(channelIndex) {setMotionPeriodRandomClicked(channelIndex)}.bind(this, channelIndex));
                channelRow.title = `This is the time it takes to make one stroke.
                Lower is faster movement.`
                channelParent.appendChild(channelRow.row);

                channelRow = Utils.createNumericFormRow("motionPeriodRandomMinRow"+channelIndex, "Min (ms)", 'motionPeriodRandomMin'+channelIndex, motionChannel ? motionChannel.periodMin : 500, 0, 2147483647,
                    function(channelIndex) {setMotionGeneratorSettings(channelIndex)}.bind(this, channelIndex));
                channelParent.appendChild(channelRow.row);

                let randomMin = channelRow.input;
                
                channelRow = Utils.createNumericFormRow("motionPeriodRandomMaxRow"+channelIndex, "Max (ms)", 'motionPeriodRandomMax'+channelIndex, motionChannel ? motionChannel.periodMax : 2000, 0, 2147483647,
                    function(channelIndex) {setMotionGeneratorSettings(channelIndex)}.bind(this, channelIndex));
                channelParent.appendChild(channelRow.row);

                setElementsIntMinAndMax(randomMin, channelRow.input);
                
                //Amplitude///////
                channelRow = Utils.createNumericFormRow(0, "Amplitude (%)", 'motionAmplitude'+channelIndex, motionChannel ? motionChannel.amp : 60, 0, 100,
                    function(channelIndex) {setMotionGeneratorSettings(channelIndex)}.bind(this, channelIndex));
                channelParent.appendChild(channelRow.row);

                channelRow = Utils.createCheckboxFormRow(0, "Amplitude random", 'motionAmplitudeRandom'+channelIndex, motionChannel ? motionChannel.ampRan : false,
                    function(channelIndex) {setMotionAmplitudeRandomClicked(channelIndex)}.bind(this, channelIndex));
                channelParent.appendChild(channelRow.row);
                
                channelRow = Utils.createNumericFormRow("motionAmplitudeRandomMinRow"+channelIndex, "Min (%)", 'motionAmplitudeRandomMin'+channelIndex, motionChannel ? motionChannel.ampMin : 20, 0, 100,
                    function(channelIndex) {setMotionGeneratorSettings(channelIndex)}.bind(this, channelIndex));
                channelParent.appendChild(channelRow.row);

                randomMin = channelRow.input;
                
                channelRow = Utils.createNumericFormRow("motionAmplitudeRandomMaxRow"+channelIndex, "Max (%)", 'motionAmplitudeRandomMax'+channelIndex, motionChannel ? motionChannel.ampMax : 60, 0, 100,
                    function(channelIndex) {setMotionGeneratorSettings(channelIndex)}.bind(this, channelIndex));
                channelParent.appendChild(channelRow.row);

                setElementsIntMinAndMax(randomMin, channelRow.input);

                //Offset///////
                channelRow = Utils.createNumericFormRow(0, "Offset (tcode)", 'motionOffset'+channelIndex, motionChannel ? motionChannel.offset : 5000, 0, 9999,
                    function(channelIndex) {setMotionGeneratorSettings(channelIndex)}.bind(this, channelIndex));
                channelParent.appendChild(channelRow.row);

                channelRow = Utils.createCheckboxFormRow(0, "Offset random", 'motionOffsetRandom'+channelIndex, motionChannel ? motionChannel.offsetRan : false,
                    function(channelIndex) {setMotionOffsetRandomClicked(channelIndex)}.bind(this, channelIndex));
                channelParent.appendChild(channelRow.row);
                
                channelRow = Utils.createNumericFormRow("motionOffsetRandomMinRow"+channelIndex, "Min (tcode)", 'motionOffsetRandomMin'+channelIndex, motionChannel ? motionChannel.offsetMin : 3000, 0, 9999,
                    function(channelIndex) {setMotionGeneratorSettings(channelIndex)}.bind(this, channelIndex));
                channelParent.appendChild(channelRow.row);

                randomMin = channelRow.input;
                
                channelRow = Utils.createNumericFormRow("motionOffsetRandomMaxRow"+channelIndex, "Max (tcode)", 'motionOffsetRandomMax'+channelIndex, motionChannel ? motionChannel.offsetMax : 7000, 0, 9999,
                    function(channelIndex) {setMotionGeneratorSettings(channelIndex)}.bind(this, channelIndex));
                channelParent.appendChild(channelRow.row);

                setElementsIntMinAndMax(randomMin, channelRow.input);

                //Phase///////
                channelRow = Utils.createNumericFormRow(0, "Phase (degrees)", 'motionPhase'+channelIndex, motionChannel ? motionChannel.phase : 0, 0.0, 180.0,
                    function(channelIndex) {setMotionGeneratorSettings(channelIndex)}.bind(this, channelIndex));
                channelParent.appendChild(channelRow.row);

                channelRow = Utils.createCheckboxFormRow(0, "Phase random", 'motionPhaseRandom'+channelIndex, motionChannel ? motionChannel.phaseRan : false,
                    function(channelIndex) {setMotionPhaseRandomClicked(channelIndex)}.bind(this, channelIndex));
                channelParent.appendChild(channelRow.row);
                
                channelRow = Utils.createNumericFormRow("motionPhaseRandomMinRow"+channelIndex, "Min (degree)", 'motionPhaseRandomMin'+channelIndex, motionChannel ? motionChannel.phaseMin : 0.0, 0.0, 180.0,
                    function(channelIndex) {setMotionGeneratorSettings(channelIndex)}.bind(this, channelIndex));
                channelParent.appendChild(channelRow.row);
                
                randomMin = channelRow.input;

                channelRow = Utils.createNumericFormRow("motionPhaseRandomMaxRow"+channelIndex, "Max (degree)", 'motionPhaseRandomMax'+channelIndex, motionChannel ? motionChannel.phaseMax : 180.0, 0.0, 180.0,
                    function(channelIndex) {setMotionGeneratorSettings(channelIndex)}.bind(this, channelIndex));
                channelParent.appendChild(channelRow.row);

                setElementsIntMinAndMax(randomMin, channelRow.input);

                //Global///////
                channelRow = Utils.createNumericFormRow("motionRandomChangeMinRow"+channelIndex, "Random change max (ms)", 'motionRandomChangeMin'+channelIndex, motionChannel ? motionChannel.ranMin : 3000, 0, 2147483647,
                    function(channelIndex) {setMotionGeneratorSettings(channelIndex)}.bind(this, channelIndex));
                channelParent.appendChild(channelRow.row);

                randomMin = channelRow.input;
                
                channelRow = Utils.createNumericFormRow("motionRandomChangeMaxRow"+channelIndex, "Random change max (ms)", 'motionRandomChangeMax'+channelIndex, motionChannel ? motionChannel.ranMax : 30000, 0, 2147483647,
                    function(channelIndex) {setMotionGeneratorSettings(channelIndex)}.bind(this, channelIndex));
                channelParent.appendChild(channelRow.row);
                
                setElementsIntMinAndMax(randomMin, channelRow.input);
            });
            rootdiv.appendChild(channelParent);
            channelTemplates.push(rootdiv);
        });
    
        selectMotionProfile(userSettings["motionDefaultProfileIndex"]);
        // document.getElementById('motionProfileName').value = userSettings['motionProfiles'][currentProfileIndex]["name"];
        // document.getElementById('motionUpdate').value = userSettings['motionProfiles'][currentProfileIndex]["update"];
        // document.getElementById('motionPeriod').value = userSettings['motionProfiles'][currentProfileIndex]["period"];
        // document.getElementById('motionAmplitude').value = userSettings['motionProfiles'][currentProfileIndex]["amp"];
        // document.getElementById('motionOffset').value = userSettings['motionProfiles'][currentProfileIndex]["offset"];  
        // // document.getElementById('motionPhase').value = userSettings["motionPhase"];
        // document.getElementById('motionPeriodRandom').checked = userSettings['motionProfiles'][currentProfileIndex]["periodRan"];
        // document.getElementById('motionAmplitudeRandom').checked = userSettings['motionProfiles'][currentProfileIndex]["ampRan"];
        // document.getElementById('motionOffsetRandom').checked = userSettings['motionProfiles'][currentProfileIndex]["offsetRan"];
        // toggleMotionRandomSettings();
        // document.getElementById('motionPeriodRandomMin').value = userSettings['motionProfiles'][currentProfileIndex]["periodMin"];
        // document.getElementById('motionPeriodRandomMax').value = userSettings['motionProfiles'][currentProfileIndex]["periodMax"];
        // document.getElementById('motionAmplitudeRandomMin').value = userSettings['motionProfiles'][currentProfileIndex]["ampMin"];
        // document.getElementById('motionAmplitudeRandomMax').value = userSettings['motionProfiles'][currentProfileIndex]["ampMax"];
        // document.getElementById('motionOffsetRandomMin').value = userSettings['motionProfiles'][currentProfileIndex]["offsetMin"];
        // document.getElementById('motionOffsetRandomMax').value = userSettings['motionProfiles'][currentProfileIndex]["offsetMax"];
        // document.getElementById('motionRandomChangeMax').value = userSettings['motionProfiles'][currentProfileIndex]["ranMax"];
        // document.getElementById('motionRandomChangeMin').value = userSettings['motionProfiles'][currentProfileIndex]["ranMin"];
        
        //document.getElementById('motionReversed').checked = userSettings["motionReversed"];
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
//     setupChannels() {
//         var channelsNode = document.getElementById("channels");
//         deleteAllChildren(channelsNode);

//         var channels = getChannelMap();
//         var header = document.createElement("div");
//         header.innerText = "Generate motion on channels:"
//         channelsNode.appendChild(header);

//         var rowHeader = document.createElement("div");
//         rowHeader.classList.add("tRow");
//         var rowcell1 = document.createElement("div");
//         rowcell1.classList.add("tCell");
//         rowcell1.innerText = "Enabled"
//         rowcell1.style = "border-right: solid 1px; padding-right: 6px;"
//         var rowcell2 = document.createElement("div");
//         rowcell2.classList.add("tCell");
//         rowcell2.style = "justify-content: space-between;"
//         var phaseSpan = document.createElement("span");
//         phaseSpan.innerText = "Phase"
//         var phaseTitle = "Initial phase in degrees. (0-180) The phase should ideally be between (offset-amplitude/2) and (offset+amplitude/2)";
//         phaseSpan.title = phaseTitle;
//         var reversedSpan = document.createElement("span");
//         reversedSpan.innerText = "Reverse"
//         rowcell2.appendChild(phaseSpan);
//         rowcell2.appendChild(reversedSpan);
//         rowHeader.appendChild(rowcell1);
//         rowHeader.appendChild(rowcell2);
//         channelsNode.appendChild(rowHeader)
//         for(var i=0; i<channels.length;i++) {
//             var channel = channels[i];
//             if(!userSettings.sr6Mode && channel.sr6Only) {
//                 continue;
//             }
            
//             var row = document.createElement("div");
//             row.classList.add("tRow");
//             var cell1 = document.createElement("div");
//             cell1.classList.add("tCell");
//             var cell2 = document.createElement("div");
//             cell2.classList.add("tCell");
//             var name = channel.channel;
//             var friendlyName = channel.channelName;
//             var motionChannelIndex = userSettings["channels"].findIndex(x => x.name == name);
//             var label = document.createElement("span");
//             label.for = "motionChannelCheckbox" + name;
//             label.innerText = friendlyName +" ("+name+")";
//             var checkbox = document.createElement("input");
//             checkbox.id = "motionChannelCheckbox" + name;
//             checkbox.type = "checkbox";
//             checkbox.value = name;
//             checkbox.checked = motionChannelIndex > -1;
//             checkbox.onclick = function () {
//                 var name = this.value;
//                 var phaseInput = document.getElementById("motionChannelPhaseInput" + name);
//                 var reverseInput = document.getElementById("motionChannelReverseInput" + name);
//                 phaseInput.readOnly = !this.checked;
//                 if(!this.checked) {
//                     phaseInput.value = 0;
//                     reverseInput.checked = false;
//                 }
//                 const index = userSettings["channels"].findIndex(x => x.name == name);
//                 var motionChannel = {name: name, phase: 0.0}; 
//                 if(index > -1) {
//                     motionChannel = userSettings["channels"][index];
//                     userSettings["channels"].splice(index, 1);
//                 }
//                 if(this.checked) {
//                     userSettings["channels"].push(motionChannel);
//                 }
//                 updateUserSettings();
//             };
            
//             var phaseInput = document.createElement("input");
//             phaseInput.id = "motionChannelPhaseInput" + name;
//             phaseInput.type = "number";
//             phaseInput.min = 0.0;
//             phaseInput.step = 0.01;
//             phaseInput.max = 180;
//             phaseInput.readOnly = motionChannelIndex == -1;
//             phaseInput.value = motionChannelIndex == -1 ? 0 : Utils.round2(userSettings["channels"][motionChannelIndex].phase);
//             phaseInput.name = name;
//             phaseInput.title = phaseTitle;
//             phaseInput.oninput = function () {
//                 const index = userSettings["channels"].findIndex(x => x.name == this.name);
//                 if(index == -1) {
//                     this.checked = false;
//                 } else {
//                     Utils.debounce("phaseInput"+this.name, function() {
//                         var name = this.name;
//                         const index = userSettings["channels"].findIndex(x => x.name == name);
//                         if(index > -1) {
//                             userSettings["channels"][index].phase = parseFloat(this.value);
//                             updateUserSettings(500);
//                         }
//                     }.bind(this), 3000);
//                 }
//             };
//             var reverseInput = document.createElement("input");
//             reverseInput.id = "motionChannelReverseInput" + name;
//             reverseInput.type = "checkbox";
//             reverseInput.readOnly = motionChannelIndex == -1;
//             reverseInput.checked = motionChannelIndex == -1 ? false : userSettings["channels"][motionChannelIndex].reverse;
//             reverseInput.name = name;
//             reverseInput.onclick = function () {
//                 const index = userSettings["channels"].findIndex(x => x.name == this.name);
//                 if(index == -1) {
//                     this.checked = false;
//                 } else {
//                     Utils.debounce("reverseInput"+this.name, function() {
//                         var name = this.name;
//                         const index = userSettings["channels"].findIndex(x => x.name == name);
//                         if(index > -1) {
//                             userSettings["channels"][index].reverse = this.value;
//                             updateUserSettings(500);
//                         } else {
//                             this.checked = false;
//                         }
//                     }.bind(this), 3000);
//                 }
//             };
//             cell1.appendChild(label);
//             cell1.appendChild(checkbox);
//             cell2.appendChild(phaseInput);
//             cell2.appendChild(reverseInput);
//             row.appendChild(cell1);
//             row.appendChild(cell2);
//             channelsNode.appendChild(row);
//         }
//     }
}

function toggleMotionRandomSettings(channelIndex) {
    var currentProfile = getMotionProfileSelectedIndex();
    Utils.toggleControlVisibilityByID("motionOffsetRandomMinRow"+channelIndex, userSettings['motionProfiles'][currentProfile]["channels"][channelIndex]["offsetRan"]);
    Utils.toggleControlVisibilityByID("motionOffsetRandomMaxRow"+channelIndex, userSettings['motionProfiles'][currentProfile]["channels"][channelIndex]["offsetRan"]);
    Utils.toggleControlVisibilityByID("motionAmplitudeRandomMinRow"+channelIndex, userSettings['motionProfiles'][currentProfile]["channels"][channelIndex]["ampRan"]);
    Utils.toggleControlVisibilityByID("motionAmplitudeRandomMaxRow"+channelIndex, userSettings['motionProfiles'][currentProfile]["channels"][channelIndex]["ampRan"]);
    Utils.toggleControlVisibilityByID("motionPeriodRandomMinRow"+channelIndex, userSettings['motionProfiles'][currentProfile]["channels"][channelIndex]["periodRan"]);
    Utils.toggleControlVisibilityByID("motionPeriodRandomMaxRow"+channelIndex, userSettings['motionProfiles'][currentProfile]["channels"][channelIndex]["periodRan"]);
    Utils.toggleControlVisibilityByID("motionPhaseRandomMinRow"+channelIndex, userSettings['motionProfiles'][currentProfile]["channels"][channelIndex]["phaseRan"]);
    Utils.toggleControlVisibilityByID("motionPhaseRandomMaxRow"+channelIndex, userSettings['motionProfiles'][currentProfile]["channels"][channelIndex]["phaseRan"]);
    toggleMotionRandomMinMaxSettingsSettings(channelIndex);
}
function toggleMotionRandomMinMaxSettingsSettings(channelIndex) {
    var currentProfile = getMotionProfileSelectedIndex();
    Utils.toggleControlVisibilityByID("motionRandomChangeMinRow"+channelIndex, 
        userSettings['motionProfiles'][currentProfile]["channels"][channelIndex]["offsetRan"] || 
        userSettings['motionProfiles'][currentProfile]["channels"][channelIndex]["ampRan"] || 
        userSettings['motionProfiles'][currentProfile]["channels"][channelIndex]["periodRan"] || 
        userSettings['motionProfiles'][currentProfile]["channels"][channelIndex]["phaseRan"]);
    Utils.toggleControlVisibilityByID("motionRandomChangeMaxRow"+channelIndex, 
        userSettings['motionProfiles'][currentProfile]["channels"][channelIndex]["offsetRan"] || 
        userSettings['motionProfiles'][currentProfile]["channels"][channelIndex]["ampRan"] || 
        userSettings['motionProfiles'][currentProfile]["channels"][channelIndex]["periodRan"] || 
        userSettings['motionProfiles'][currentProfile]["channels"][channelIndex]["phaseRan"]);
}

function setMotionEnabledClicked() {
    userSettings["motionEnabled"] = !userSettings["motionEnabled"];//document.getElementById('motionEnabled').checked;
    sendTCode(userSettings["motionEnabled"] ? "$motion-enable" : "$motion-disable");
    //userSettings["motionEnabled"] ? button.classList.add("button-toggle-stop") : button.classList.remove("button-toggle-stop");
    MotionGenerator.setEnabledStatus();
}
function setMotionPeriodRandomClicked(channelIndex) {
    var currentProfile = getMotionProfileSelectedIndex();
    var motionEnabled = document.getElementById('motionPeriodRandom'+channelIndex).checked;
    // if(motionEnabled) {
    //     sendTCode("$motion-period-random-on");
    // } else {
    //     sendTCode("$motion-period-random-off");
    // }
    userSettings['motionProfiles'][currentProfile]["channels"][channelIndex]["motionPeriodRandom"] = motionEnabled;
    Utils.toggleControlVisibilityByID("motionPeriodRandomMinRow"+channelIndex, userSettings['motionProfiles'][currentProfile]["channels"][channelIndex]["periodRan"]);
    Utils.toggleControlVisibilityByID("motionPeriodRandomMaxRow"+channelIndex, userSettings['motionProfiles'][currentProfile]["channels"][channelIndex]["periodRan"]);
    toggleMotionRandomMinMaxSettingsSettings(channelIndex);
    updateUserSettings();
}
function setMotionAmplitudeRandomClicked(channelIndex) {
    var currentProfile = getMotionProfileSelectedIndex();
    var motionEnabled = document.getElementById('motionAmplitudeRandom'+channelIndex).checked;
    // if(motionEnabled) {
    //     sendTCode("$motion-amplitude-random-on");
    // } else {
    //     sendTCode("$motion-amplitude-random-off");
    // }
    userSettings['motionProfiles'][currentProfile]["channels"][channelIndex]["ampRan"] = motionEnabled;
    Utils.toggleControlVisibilityByID("motionAmplitudeRandomMinRow"+channelIndex, motionEnabled);
    Utils.toggleControlVisibilityByID("motionAmplitudeRandomMaxRow"+channelIndex, motionEnabled);
    toggleMotionRandomMinMaxSettingsSettings(channelIndex);
    updateUserSettings();
}
function setMotionOffsetRandomClicked(channelIndex) {
    var currentProfile = getMotionProfileSelectedIndex();
    var motionEnabled = document.getElementById('motionOffsetRandom'+channelIndex).checked;
    // if(motionEnabled) {
    //     sendTCode("$motion-offset-random-on");
    // } else {
    //     sendTCode("$motion-offset-random-off");
    // }
    userSettings['motionProfiles'][currentProfile]["channels"][channelIndex]["offsetRan"] = motionEnabled;
    Utils.toggleControlVisibilityByID("motionOffsetRandomMinRow"+channelIndex, motionEnabled);
    Utils.toggleControlVisibilityByID("motionOffsetRandomMaxRow"+channelIndex, motionEnabled);
    toggleMotionRandomMinMaxSettingsSettings(channelIndex);
    updateUserSettings();
}
function setMotionPhaseRandomClicked(channelIndex) {
    var currentProfile = getMotionProfileSelectedIndex();
    var motionEnabled = document.getElementById('motionPhaseRandom'+channelIndex).checked;
    // if(motionEnabled) {
    //     sendTCode("$motion-offset-random-on");
    // } else {
    //     sendTCode("$motion-offset-random-off");
    // }
    userSettings['motionProfiles'][currentProfile]["channels"][channelIndex]["phaseRan"] = motionEnabled;
    Utils.toggleControlVisibilityByID("motionPhaseRandomMinRow"+channelIndex, motionEnabled);
    Utils.toggleControlVisibilityByID("motionPhaseRandomMaxRow"+channelIndex, motionEnabled);
    toggleMotionRandomMinMaxSettingsSettings(channelIndex);
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
    if(sendTCodeCommand)
        sendTCode(`$motion-set-profile:${profileIndex + 1}`);
}
function editMotionProfile(profileIndex) {
    const modal = document.getElementById("motionChannelsModal");
    removeAllChildren(modal);
    modal.appendChild(channelTemplates[profileIndex]);
    setMotionGeneratorSettingsProfile(profileIndex);
    modal.setAttribute("visible", true);
}

var validateGeneratorSettingsDebounce;
function setMotionGeneratorSettings(channelIndex) {
    if(validateGeneratorSettingsDebounce) {
        clearTimeout(validateGeneratorSettingsDebounce);
    }
    validateGeneratorSettingsDebounce = setTimeout(function (channelIndex) {
        var currentProfilechannelIndex = getMotionProfileSelectedchannelIndex();
        var motionChannel = userSettings['motionProfiles'][currentProfilechannelIndex]["channels"][channelIndex];
        validateIntControl('motionUpdate'+channelIndex, motionChannel, "update");
        validateIntControl('motionPeriod'+channelIndex, motionChannel, "period");
        validateIntControl('motionAmplitude'+channelIndex, motionChannel, "amp");
        validateIntControl('motionOffset'+channelIndex, motionChannel, "offset");
        // validateFloatControl('motionPhase', userSettings["motionPhase"]);

        setIntMinAndMax('motionPeriodRandomMin'+channelIndex, 'motionPeriodRandomMax'+channelIndex);
        validateIntControl('motionPeriodRandomMin'+channelIndex, motionChannel, "periodMin");
        validateIntControl('motionPeriodRandomMax'+channelIndex, motionChannel, "periodMax");
        setIntMinAndMax('motionAmplitudeRandomMin'+channelIndex, 'motionAmplitudeRandomMax'+channelIndex);
        validateIntControl('motionAmplitudeRandomMin'+channelIndex, motionChannel, "ampMin");
        validateIntControl('motionAmplitudeRandomMax'+channelIndex, motionChannel, "ampMax");
        setIntMinAndMax('motionOffsetRandomMin'+channelIndex, 'motionOffsetRandomMax'+channelIndex);
        validateIntControl('motionOffsetRandomMin'+channelIndex, motionChannel, "offsetMin");
        validateIntControl('motionOffsetRandomMax'+channelIndex, motionChannel, "offsetMax");
        setIntMinAndMax('motionRandomChangeMin'+channelIndex, 'motionRandomChangeMax'+channelIndex);
        validateIntControl('motionRandomChangeMin'+channelIndex, motionChannel, "ranMin");
        validateIntControl('motionRandomChangeMax'+channelIndex, motionChannel, "ranMax");

        
        //userSettings["motionReversed"] = document.getElementById('motionReversed').checked;
        updateUserSettings(1);
    }.bind(this, channelIndex), 3000);
}
var setMotionGeneratorNameDebounce;
function setMotionGeneratorName(profileIndex) {
    if(setMotionGeneratorNameDebounce) {
        clearTimeout(setMotionGeneratorNameDebounce);
    }
    
    setMotionGeneratorNameDebounce = setTimeout(function(profileIndex) {
        var currentProfileIndex = getMotionProfileSelectedIndex();
        if(validateStringControl("motionProfileName"+profileIndex, userSettings['motionProfiles'][currentProfileIndex]["motionProfileName"])) {
            //userSettings['motionProfiles'][currentProfileIndex]["motionProfileName"] = document.getElementById("motionProfileName").value;
            updateMotionProfileName(currentProfileIndex);
            updateUserSettings(1);
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
    button.onclick = function(profileIndex) {
        selectMotionProfile(profileIndex, true);
    }.bind(this, profileIndex)
    var buttonEdit = document.createElement("button");
    buttonEdit.id = `motionProfileEdit${profileIndex}`;
    buttonEdit.name = "motionProfileEditButton";
    buttonEdit.value = profileIndex;
    buttonEdit.innerText = "Edit";
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
    motionProfilesElements[optionsIndex].innerText = userSettings['motionProfiles'][profileIndex]["name"];
}
function setMotionGeneratorSettingsDefault(channelIndex) {
    var profileIndex = getMotionProfileSelectedIndex();
    if (confirm(`Are you sure you want to set the current profile '${userSettings['motionProfiles'][profileIndex]["name"]}' to the default settings?`)) {
        setProfileMotionGeneratorSettingsDefault(profileIndex, channelIndex);
        updateUserSettings(1);
    }
}
function setProfileMotionGeneratorSettingsDefault(profileIndex, channelIndex) {
    userSettings['motionProfiles'][profileIndex]["name"] = "Profile "+ (profileIndex + 1);
    userSettings['motionProfiles'][profileIndex]["channels"][channelIndex]["update"] = 100;
    userSettings['motionProfiles'][profileIndex]["channels"][channelIndex]["period"] = 2000;
    userSettings['motionProfiles'][profileIndex]["channels"][channelIndex]["amp"] = 60;
    userSettings['motionProfiles'][profileIndex]["channels"][channelIndex]["offset"] = 5000  
    userSettings['motionProfiles'][profileIndex]["channels"][channelIndex]["periodRan"] = false;
    userSettings['motionProfiles'][profileIndex]["channels"][channelIndex]["periodMin"] = 500;
    userSettings['motionProfiles'][profileIndex]["channels"][channelIndex]["periodMax"] = 2000;
    userSettings['motionProfiles'][profileIndex]["channels"][channelIndex]["ampRan"] = false;
    userSettings['motionProfiles'][profileIndex]["channels"][channelIndex]["ampMin"] = 20;
    userSettings['motionProfiles'][profileIndex]["channels"][channelIndex]["ampMax"] = 60;
    userSettings['motionProfiles'][profileIndex]["channels"][channelIndex]["offsetRan"] = false;
    userSettings['motionProfiles'][profileIndex]["channels"][channelIndex]["offsetMin"] = 3000;
    userSettings['motionProfiles'][profileIndex]["channels"][channelIndex]["offsetMax"] = 7000;
    userSettings['motionProfiles'][profileIndex]["channels"][channelIndex]["ranMin"] = 3000;
    userSettings['motionProfiles'][profileIndex]["channels"][channelIndex]["ranMax"] = 30000;
    userSettings['motionProfiles'][profileIndex]["channels"][channelIndex]["phase"] = 0;
    userSettings['motionProfiles'][profileIndex]["channels"][channelIndex]["phaseRan"] = false;
    userSettings['motionProfiles'][profileIndex]["channels"][channelIndex]["phaseMin"] = 0.0;
    userSettings['motionProfiles'][profileIndex]["channels"][channelIndex]["phaseMax"] = 180.0;
    userSettings['motionProfiles'][profileIndex]["channels"][channelIndex]["reverse"] = false;
    
    setMotionGeneratorSettingsProfile(profileIndex);
}
function setMotionGeneratorSettingsProfile(profileIndex) {
    document.getElementById('motionProfileName'+profileIndex).value = userSettings['motionProfiles'][profileIndex]["name"];
    const channels = userSettings['motionProfiles'][profileIndex]["channels"];
    channels.forEach((channel, channelIndex) => {
        document.getElementById('motionUpdate'+channelIndex).value = channel["update"];
        document.getElementById('motionPeriod'+channelIndex).value = channel["period"];
        document.getElementById('motionAmplitude'+channelIndex).value = channel["amp"];
        document.getElementById('motionOffset'+channelIndex).value = channel["offset"];  
        document.getElementById('motionPhase'+channelIndex).value = channel["phase"];
        document.getElementById('motionPhaseRandom'+channelIndex).checked = channel["phaseRan"];
        document.getElementById('motionPhaseRandomMin'+channelIndex).value = channel["phaseMin"];  
        document.getElementById('motionPhaseRandomMax'+channelIndex).value = channel["phaseMax"];  
        //document.getElementById('motionReversed'+channelIndex).checked = channel["reverse"];
        document.getElementById('motionPeriodRandom'+channelIndex).checked = channel["periodRan"];
        document.getElementById('motionPeriodRandomMin'+channelIndex).value = channel["periodMin"];  
        document.getElementById('motionPeriodRandomMax'+channelIndex).value = channel["periodMax"];  
        document.getElementById('motionAmplitudeRandom'+channelIndex).checked = channel["ampRan"];
        document.getElementById('motionAmplitudeRandomMin'+channelIndex).value = channel["ampMin"];  
        document.getElementById('motionAmplitudeRandomMax'+channelIndex).value = channel["ampMax"];  
        document.getElementById('motionOffsetRandom'+channelIndex).checked = channel["offsetRan"];
        document.getElementById('motionOffsetRandomMin'+channelIndex).value = channel["offsetMin"];  
        document.getElementById('motionOffsetRandomMax'+channelIndex).value = channel["offsetMax"];  
        document.getElementById('motionRandomChangeMin'+channelIndex).value = channel["ranMin"];  
        document.getElementById('motionRandomChangeMax'+channelIndex).value = channel["ranMax"];  
        toggleMotionRandomSettings(channelIndex);
    });
}