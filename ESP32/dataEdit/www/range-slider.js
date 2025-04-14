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
  minRange: 250,
  setup() {
    var channels = getChannelMap();
    var deviceRangesTable = document.getElementById("deviceRangesTable");
    deleteAllChildren(deviceRangesTable);
    for(var i = 0; i < channels.length; i++) {
      var channel = channels[i];
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

      var sliderHeader = document.createElement("div");
      sliderHeader.innerText = friendlyName;
      sliderHeader.classList.add("range_container_header");
      var sliderControl = document.createElement("div");
      sliderControl.classList.add("sliders_control");

      // Range slider inputs
      var minSlider = document.createElement("input");
      minSlider.id = "minSlider" + name;
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

      var formControlContainerMin = document.createElement("div");
      formControlContainerMin.classList.add("form_control_container__min");
      formControlContainerMin.innerText = "Min";
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

      var formControlContainerMax = document.createElement("div");
      formControlContainerMax.classList.add("form_control_container__max");
      formControlContainerMax.innerText = "Max";
      formControlContainer2.appendChild(formControlContainerMax);

      var maxInput = document.createElement("input");
      maxInput.id = "maxInput" + name;
      maxInput.type = "number";
      maxInput.min = 1;
      maxInput.max = getTCodeMax();
      maxInput.value = max;
      formControlContainer2.appendChild(maxInput);

      formControl.appendChild(formControlContainer);
      formControl.appendChild(formControlContainer2);

      var rangeContainer = document.createElement("div");
      rangeContainer.classList.add("range_container");
      rangeContainer.appendChild(sliderHeader);
      rangeContainer.appendChild(sliderControl);
      rangeContainer.appendChild(formControl);
      deviceRangesTable.appendChild(rangeContainer);

      this.fillSlider(minSlider, maxSlider, '#C6C6C6', '#25daa5', maxSlider);
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
  },
  updateSettings(debounce) {
      updateUserSettings(debounce, EndPointType.ChannelProfiles.uri, channelsProfileSettings);
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
      this.fillSlider(minInput, maxInput, '#C6C6C6', '#25daa5', controlSlider);
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
          sendTCode(channelName + minSlider.value + "S1000");
      }
  },
    
  controlMaxInput(minSlider, maxSlider, minInput, maxInput, controlSlider, channelName) {
      const [min, max] = this.getParsed(minInput, maxInput);
      this.fillSlider(minInput, maxInput, '#C6C6C6', '#25daa5', controlSlider);
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
          sendTCode(channelName + maxInput.value + "S1000");
      }
  },

  controlMinSlider(minSlider, maxSlider, minInput, maxInput, controlSlider, channelName) {
    const [min, max] = this.getParsed(minSlider, maxSlider);
    this.fillSlider(minSlider, maxSlider, '#C6C6C6', '#25daa5', controlSlider);
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
    sendTCode(channelName + minInput.value + "S1000");
  },

  controlMaxSlider(minSlider, maxSlider, minInput, maxInput, controlSlider, channelName) {
    const [min, max] = this.getParsed(minSlider, maxSlider);
    this.fillSlider(minSlider, maxSlider, '#C6C6C6', '#25daa5', controlSlider);
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
    sendTCode(channelName + maxInput.value + "S1000");
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
      const rangeDistance = maxSlider.max - maxSlider.min;
      const minPosition = minSlider.value - maxSlider.min;
      const maxPosition = maxSlider.value - maxSlider.min;
      controlSlider.style.background = `linear-gradient(
        max right,
        ${sliderColor} 0%,
        ${sliderColor} ${(minPosition)/(rangeDistance)*100}%,
        ${rangeColor} ${((minPosition)/(rangeDistance))*100}%,
        ${rangeColor} ${(maxPosition)/(rangeDistance)*100}%, 
        ${sliderColor} ${(maxPosition)/(rangeDistance)*100}%, 
        ${sliderColor} 100%)`;
  },

  setToggleAccessible(currentTarget, maxSlider) {
    if (Number(currentTarget.value) <= 0 ) {
      maxSlider.style.zIndex = 2;
    } else {
      maxSlider.style.zIndex = 0;
    }
  }
}