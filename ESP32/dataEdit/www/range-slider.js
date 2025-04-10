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
  setup() {
    var channels = getChannelMap();
    var deviceRangesTable = document.getElementById("deviceRangesTable");
    deleteAllChildren(deviceRangesTable);
    for(var i = 0; i < channels.length; i++) {
      var channel = channels[i];
      if(!isSR6() && channel.sr6Only || channel.switch) {
          continue;
      }
      var name = channel.channel;
      var friendlyName = channel.channelName;
      if(!userSettings["channelRanges"])
        return;

      var min = 1;
      var max = getTCodeMax();
      if(userSettings["channelRanges"][name]) {
        max = userSettings["channelRanges"][name].max;
        min = userSettings["channelRanges"][name].min;
      }

      var sliderHeader = document.createElement("div");
      sliderHeader.innerText = friendlyName;
      sliderHeader.classList.add("range_container_header");
      var sliderControl = document.createElement("div");
      sliderControl.classList.add("sliders_control");

      // Range slider inputs
      var minSlider = document.createElement("input");
      minSlider.id = "minSlider" + name;
      minSlider.type = "range";
      minSlider.min = 1;
      minSlider.max = getTCodeMax();
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
      minInput.min = 1;
      minInput.max = getTCodeMax();
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
      maxInput.min = min;
      minInput.max = max;
      // if (min > max) {
      //     minSlider.value = max;
      //     minInput.value = max;
      // } else {
      //     minSlider.value = min;
      // }
      if(minInput.checkValidity()) {
          minSlider.value = minInput.value;
          this.setChannelRange(channelName, minSlider.value, max);
          sendTCode(channelName + minSlider.value + "S1000");
      }
  },

  controlMaxInput(minSlider, maxSlider, minInput, maxInput, controlSlider, channelName) {
      const [min, max] = this.getParsed(minInput, maxInput);
      this.fillSlider(minInput, maxInput, '#C6C6C6', '#25daa5', controlSlider);
      this.setToggleAccessible(maxInput, maxSlider);
      maxInput.min = min;
      minInput.max = max;
      // if (min <= max) {
      //     maxSlider.value = max;
      //     maxInput.value = max;
      // } else {
      //     maxInput.value = min;
      // }
      if(maxInput.checkValidity()) {
          maxSlider.value = maxInput.value;
          this.setChannelRange(channelName, min, maxInput.value);
          sendTCode(channelName + maxInput.value + "S1000");
      }
  },

  controlMinSlider(minSlider, maxSlider, minInput, maxInput, controlSlider, channelName) {
    const [min, max] = this.getParsed(minSlider, maxSlider);
    this.fillSlider(minSlider, maxSlider, '#C6C6C6', '#25daa5', controlSlider);
    if (min > max) {
      minSlider.value = max;
      minInput.value = max;
    } else {
      minInput.value = min;
    }
    this.setChannelRange(channelName, minInput.value, max);
    sendTCode(channelName + minInput.value + "S1000");
  },

  controlMaxSlider(minSlider, maxSlider, minInput, maxInput, controlSlider, channelName) {
    const [min, max] = this.getParsed(minSlider, maxSlider);
    this.fillSlider(minSlider, maxSlider, '#C6C6C6', '#25daa5', controlSlider);
    this.setToggleAccessible(maxSlider, maxSlider);
    if (min <= max) {
      maxSlider.value = max;
      maxInput.value = max;
    } else {
      maxInput.value = min;
      maxSlider.value = min;
    }
    this.setChannelRange(channelName, min, maxSlider.value);
    sendTCode(channelName + maxSlider.value  + "S1000");
  },

  setChannelRange(channelName, min, max) {
      userSettings["channelRanges"][channelName].min = min;
      userSettings["channelRanges"][channelName].max = max;

      updateUserSettings();
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