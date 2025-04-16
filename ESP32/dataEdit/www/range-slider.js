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

// https://medium.com/@predragdavidovic10/native-dual-range-slider-html-css-javascript-91e778134816

DeviceRangeSlider = {
  channels: [],
  minRange: 250,
  sliderColor: "cornflowerblue",
  rangeColor: "grey",
  setup() {
    this.channels = getChannelMap();
    var deviceRangesTable = document.getElementById("deviceRangesTable");
    deleteAllChildren(deviceRangesTable);

    var rangeContainer = document.createElement("div");
    rangeContainer.classList.add("range_container");

    var formControlFirst = document.createElement("div");
    formControlFirst.classList.add("form_control");

    var allEnabledContainer = document.createElement("div");
    allEnabledContainer.classList.add("form_control_container_header");
    var allEnabledLabel = document.createElement("label");
    allEnabledLabel.setAttribute("for", "allEnabledCheckbox");
    allEnabledLabel.innerText = "Toggle all";
    allEnabledLabel.classList.add("range_container_header");
    var allEnabledCheckbox = document.createElement("input");
    allEnabledCheckbox.type = "checkbox";
    allEnabledCheckbox.id = "allEnabledCheckbox";
    allEnabledCheckbox.onclick = function (checkBox) {
      let allRangeCheckboxes = document.getElementsByName("ChannelRangeEnabled");
      for(var i = 0; i < allRangeCheckboxes.length; i++) {
        allRangeCheckboxes[i].checked = checkBox.checked;
        // this.channels[i].rangeLimitEnabled = checkBox.checked;
        // channelsProfileSettings[i].rangeLimitEnabled = checkBox.checked;
        this.toggleChannelEnabled(this.channels[i].name, checkBox.checked);
      }
      this.updateSettings(0);
    }.bind(this, allEnabledCheckbox);


    var tempEnabledContainer = document.createElement("div");
    tempEnabledContainer.setAttribute("title", "This will temporarily toggle ALL ranges. This is always true on boot.")
    tempEnabledContainer.classList.add("form_control_container");
    var tempEnabledLabel = document.createElement("label");
    tempEnabledLabel.setAttribute("for", "channelRangesEnabled");
    tempEnabledLabel.innerText = "Enabled"
    var tempEnabledCheck = document.createElement("input");
    tempEnabledCheck.id = "channelRangesEnabled";
    tempEnabledCheck.type = "checkbox"
    tempEnabledCheck.onclick = function (checkBox) {
      sendTCode(checkBox.checked ? "#channel-ranges-enable" : "#channel-ranges-disable");
    }.bind(this, tempEnabledCheck);
    tempEnabledCheck.checked = systemInfo["channelRangesEnabled"];

    allEnabledContainer.appendChild(allEnabledCheckbox);
    allEnabledContainer.appendChild(allEnabledLabel);
    formControlFirst.appendChild(allEnabledContainer);
    tempEnabledContainer.appendChild(tempEnabledLabel);
    tempEnabledContainer.appendChild(tempEnabledCheck);
    formControlFirst.appendChild(tempEnabledContainer);
    rangeContainer.appendChild(formControlFirst);
    deviceRangesTable.appendChild(rangeContainer);

    for(var i = 0; i < this.channels.length; i++) {
      var channel = this.channels[i];
      if(!isSR6() && channel.sr6Only) {
          continue;
      }
      var name = channel.name;
      var friendlyName = channel.friendlyName;
      if(!channel)
        return;

      var min = 0;
      var max = getTCodeMax();
      max = channel.userMax;
      min = channel.userMin;

      //
      var sliderHeader = document.createElement("div");
      sliderHeader.classList.add("range_container_header");
      var headerSpan = document.createElement("span");
      var channelRangeEnabledChk = document.createElement("input");
      channelRangeEnabledChk.id = name+"ChannelRangeEnabled";
      channelRangeEnabledChk.setAttribute("name", "ChannelRangeEnabled");
      channelRangeEnabledChk.type = "checkbox";
      channelRangeEnabledChk.checked = channel.rangeLimitEnabled;
      channelRangeEnabledChk.onclick = function (channel, checkBox) {
        channel.rangeLimitEnabled = checkBox.checked;
        this.toggleChannelEnabled(channel.name, checkBox.checked);
        this.updateAllToggleChk();
        this.updateSettings();
      }.bind(this, channel, channelRangeEnabledChk);
      var headerLabel = document.createElement("label");
      headerLabel.innerText = friendlyName;
      headerLabel.setAttribute("for", channelRangeEnabledChk.id);
      headerSpan.appendChild(channelRangeEnabledChk);
      headerSpan.appendChild(headerLabel);
      sliderHeader.appendChild(headerSpan);

      var formControlContainerHeader = document.createElement("div");
      formControlContainerHeader.classList.add("form_control_container_header");
      formControlContainerHeader.appendChild(sliderHeader);

      var sliderControl = document.createElement("div");
      sliderControl.classList.add("sliders_control");

      // Range slider inputs
      var minSlider = document.createElement("input");
      minSlider.id = "minSlider" + name;
      minSlider.classList.add("fromSlider");
      minSlider.type = "range";
      minSlider.min = 0;
      minSlider.max = getTCodeMax() - 1;
      minSlider.value = min;
      sliderControl.appendChild(minSlider);
      var maxSlider = document.createElement("input");
      maxSlider.id = "maxSlider" + name;
      maxSlider.type = "range";
      maxSlider.min = 1;
      maxSlider.max = getTCodeMax();
      maxSlider.value = max
      sliderControl.appendChild(maxSlider);

      var formControl = document.createElement("div");
      formControl.classList.add("form_control");

      var formControlContainer = document.createElement("div");
      formControlContainer.classList.add("form_control_container");

      var formControlContainerMin = document.createElement("label");
      formControlContainerMin.classList.add("form_control_container_label");
      formControlContainerMin.innerText = "Min";
      formControlContainerMin.setAttribute("for", "minInput" + name);
      formControlContainer.appendChild(formControlContainerMin);

      var minInput = document.createElement("input");
      minInput.id = "minInput" + name;
      minInput.type = "number";
      minInput.min = 0;
      minInput.max = getTCodeMax() - 1;
      minInput.value = min;
      formControlContainer.appendChild(minInput);
      
      // Numeric inputs
      var formControlContainer2 = document.createElement("div");
      formControlContainer2.classList.add("form_control_container");
      formControl.appendChild(formControlContainer2);


      var formControlContainerMax = document.createElement("label");
      formControlContainerMax.classList.add("form_control_container_label");
      formControlContainerMax.innerText = "Max";
      formControlContainerMax.setAttribute("for", "maxInput" + name);
      formControlContainer2.appendChild(formControlContainerMax);

      var maxInput = document.createElement("input");
      maxInput.id = "maxInput" + name;
      maxInput.type = "number";
      maxInput.min = 1;
      maxInput.max = getTCodeMax();
      maxInput.value = max;
      formControlContainer2.appendChild(maxInput);

      formControl.appendChild(formControlContainerHeader);
      formControl.appendChild(formControlContainer);
      formControl.appendChild(formControlContainer2);

      var rangeContainer = document.createElement("div");
      rangeContainer.classList.add("range_container");
      // rangeContainer.appendChild(sliderHeader);
      rangeContainer.appendChild(formControl);
      rangeContainer.appendChild(sliderControl);
      deviceRangesTable.appendChild(rangeContainer);

      this.fillSlider(minSlider, maxSlider, this.rangeColor, this.sliderColor, maxSlider);
      this.setToggleAccessible(maxSlider, maxSlider);
      
      // minSlider.oninput = () => controlMinSlider(minSlider, maxSlider, minInput, name);
      // maxSlider.oninput = () => controlMaxSlider(minSlider, maxSlider, maxInput, name);
      // minInput.oninput = () => controlMinInput(minSlider, minInput, maxInput, maxSlider, name);
      // maxInput.oninput = () => controlMaxInput(minSlider, minInput, maxInput, maxSlider, name);

      minSlider.oninput = function (minSlider, maxSlider, minInput, maxInput, name) {
        this.controlMinSlider(minSlider, maxSlider, minInput, maxInput, maxSlider, name);
      }.bind(this, minSlider, maxSlider, minInput, maxInput, name);

      maxSlider.oninput = function (minSlider, maxSlider, minInput, maxInput, name) {
        this.controlMaxSlider(minSlider, maxSlider, minInput, maxInput, maxSlider, name);
      }.bind(this, minSlider, maxSlider, minInput, maxInput, name);

      minInput.oninput = function (minSlider, maxSlider, minInput, maxInput, name) {
        this.controlMinInput(minSlider, maxSlider, minInput, maxInput, maxSlider, name);
      }.bind(this, minSlider, maxSlider, minInput, maxInput, name);

      maxInput.oninput = function (minSlider, maxSlider, minInput, maxInput, name) {
        this.controlMaxInput(minSlider, maxSlider, minInput, maxInput, maxSlider, name);
      }.bind(this, minSlider, maxSlider, minInput, maxInput, name);
    }
    this.updateAllToggleChk();
  },
  updateSettings(debounce) {
      updateUserSettings(debounce, EndPointType.ChannelProfiles.uri, channelsProfileSettings);
  },
  updateAllToggleChk() {
    let anyEnabled = false;
    let anyDisabled = false;

    for(var i = 0; i < this.channels.length; i++) {
      if(this.channels[i].rangeLimitEnabled) {
        anyEnabled = true;
      } else {
        anyDisabled = true;
      }
    }

    let allEnabledCheckbox = document.getElementById("allEnabledCheckbox");

    if(anyDisabled && anyEnabled ) {
      allEnabledCheckbox.indeterminate = true;
    } else if(anyDisabled && !anyEnabled) {
      allEnabledCheckbox.checked = false;
      allEnabledCheckbox.indeterminate = false;
    } else if(!anyDisabled && anyEnabled) {
      allEnabledCheckbox.checked = true;
      allEnabledCheckbox.indeterminate = false;
    }
  },
  updateChannelRangesTemp(value) {
    document.getElementById("channelRangesEnabled").checked = value;
  },
  show() {
    document.getElementById("deviceRangesModal").show();
  },
//   <div class="range_container">
//     <div class="sliders_control">
//         <input id="fromSlider" type="range" value="10" min="0" max="100"/>
//         <input id="toSlider" type="range" value="40" min="0" max="100"/>
//     </div>
//     <div class="form_control">
//         <div class="form_control_container">
//             <div class="form_control_container__time">Min</div>
//             <input class="form_control_container__time__input" type="number" id="fromInput" value="10" min="0" max="100"/>
//         </div>
//         <div class="form_control_container">
//             <div class="form_control_container__time">Max</div>
//             <input class="form_control_container__time__input" type="number" id="toInput" value="40" min="0" max="100"/>
//         </div>
//     </div>
// </div>

  controlMinInput(minSlider, maxSlider, minInput, maxInput, controlSlider, channelName) {
      const [min, max] = this.getParsed(minInput, maxInput);
      var newValue = min + this.minRange;
      if(newValue > getTCodeMax())
      {
        newValue = getTCodeMax() - this.minRange;
      }
      maxInput.min = newValue;
      // maxInput.max = getTCodeMax();
      // minSlider.value = min;
      // if (min >= max) {
      //   var newMax = min + this.minRange;
      //   if(newMax > getTCodeMax()) {
      //     newMax = getTCodeMax();
      //     minInput.value = newMax - this.minRange;
      //     minSlider.value = newMax - this.minRange;
      //   } 
      //   maxSlider.value = newMax;
      //   maxInput.value = newMax;
      // }
      if(minInput.checkValidity() && maxInput.checkValidity()) {
          minSlider.value = minInput.value;
          this.setChannelRange(channelName, parseInt(minInput.value), max);
          this.fillSlider(minInput, maxInput, this.rangeColor, this.sliderColor, controlSlider);
          sendTCodeValue(channelName, minSlider.value, TCodeModifierType.SPEED, 1000);
      }
  },
    
  controlMaxInput(minSlider, maxSlider, minInput, maxInput, controlSlider, channelName) {
      const [min, max] = this.getParsed(minInput, maxInput);
      this.setToggleAccessible(maxInput, maxSlider);
      var newValue = max - this.minRange;
      if(newValue < getTCodeMin())
      {
        newValue = getTCodeMin() + this.minRange;
      }
      minInput.max = newValue;
      // minInput.min = getTCodeMin();
      // if (max <= min) {
      //   var newMin = max - this.minRange;
      //   if(newMin < getTCodeMin()) {
      //     newMin = getTCodeMin();
      //     maxInput.value = newMin + this.minRange;
      //     maxSlider.value = newMin + this.minRange;
      //   } 
      //   minSlider.value = newMin;
      //   minInput.value = newMin;
      // }
      if(maxInput.checkValidity() && minInput.checkValidity()) {
          maxSlider.value = maxInput.value;
          this.setChannelRange(channelName, min, parseInt(maxInput.value));
          this.fillSlider(minInput, maxInput, this.rangeColor, this.sliderColor, controlSlider);
          sendTCodeValue(channelName, maxInput.value, TCodeModifierType.SPEED, 1000);
      }
  },

  controlMinSlider(minSlider, maxSlider, minInput, maxInput, controlSlider, channelName) {
    const [min, max] = this.getParsed(minSlider, maxSlider);
    this.fillSlider(minSlider, maxSlider, this.rangeColor, this.sliderColor, controlSlider);
    minInput.value = min;
    if (min >= max) {
      var newMax = min + this.minRange;
      if(newMax > getTCodeMax()) {
        newMax = getTCodeMax();
        minInput.value = newMax - this.minRange;
        minSlider.value = newMax - this.minRange;
      } 
      maxSlider.value = newMax;
      maxInput.value = newMax;
    } 
    this.setChannelRange(channelName, parseInt(minInput.value), max);
    sendTCodeValue(channelName, minInput.value, TCodeModifierType.SPEED, 1000);
  },

  controlMaxSlider(minSlider, maxSlider, minInput, maxInput, controlSlider, channelName) {
    const [min, max] = this.getParsed(minSlider, maxSlider);
    this.fillSlider(minSlider, maxSlider, this.rangeColor, this.sliderColor, controlSlider);
    this.setToggleAccessible(maxSlider, maxSlider);
    maxInput.value = max;
    if (max <= min) {
      var newMin = max - this.minRange;
      if(newMin < getTCodeMin()) {
        newMin = getTCodeMin();
        maxInput.value = newMin + this.minRange;
        maxSlider.value = newMin + this.minRange;
      } 
      minSlider.value = newMin;
      minInput.value = newMin;
    }
    this.setChannelRange(channelName, min, parseInt(maxInput.value));
    sendTCodeValue(channelName, maxInput.value, TCodeModifierType.SPEED, 1000);
  },
  toggleChannelEnabled(channelName, enabled) {
    for(var i=0; i<channelsProfileSettings["channelProfile"].length; i++)
    {
      var profile = channelsProfileSettings["channelProfile"][i];
      if(profile["name"] == channelName)
      {
        profile.rangeLimitEnabled = enabled;
        
        break;
      }
    }
  },
  setChannelRange(channelName, min, max) {
    for(var i=0; i<channelsProfileSettings["channelProfile"].length; i++)
    {
      var profile = channelsProfileSettings["channelProfile"][i];
      if(profile["name"] == channelName)
      {
        profile.userMin = min;
        profile.userMax = max;
        
        this.updateSettings();
        break;
      }
    }
  },

  getParsed(currentFrom, currentTo) {
    const min = parseInt(currentFrom.value, 10);
    const max = parseInt(currentTo.value, 10);
    return [min, max];
  },

  fillSlider(minSlider, maxSlider, sliderColor, rangeColor, controlSlider) {
      const rangeDistance = maxSlider.max - minSlider.min;
      const minPosition = minSlider.value;
      const maxPosition = maxSlider.value
      const background = `linear-gradient(
        to right,
        ${sliderColor} 0%,
        ${sliderColor} ${((minPosition)/(rangeDistance))*100}%,
        ${rangeColor} ${((minPosition)/(rangeDistance))*100}%,
        ${rangeColor} ${((maxPosition)/(rangeDistance))*100}%, 
        ${sliderColor} ${((maxPosition)/(rangeDistance))*100}%, 
        ${sliderColor} 100%)`.replace(/[\n\r\t]/gm, "");
      controlSlider.style.background = background;
      // controlSlider.style.boxShadow = "-1px 1px 5px 0 #6495ed,1px -1px 5px 0 #add8e6,1px 1px 5px 0 #add8e6,-1px -1px 5px 0 #6495ed"
  },

  setToggleAccessible(currentTarget, maxSlider) {
    if (Number(currentTarget.value) <= 0 ) {
      maxSlider.style.zIndex = 2;
    } else {
      maxSlider.style.zIndex = 0;
    }
  }
}