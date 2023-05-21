// https://medium.com/@predragdavidovic10/native-dual-range-slider-html-css-javascript-91e778134816


function loadChannelRanges() {
    var channels = getChannelMap();
    var deviceRangesTable = document.getElementById("deviceRangesTable");
    var rangeHeader = document.createElement("div");
    rangeHeader.classList.add("tHeader")
    var rangeTitle = document.createElement("h3");
    rangeTitle.innerText = "Device ranges";
    rangeHeader.appendChild(rangeTitle);
    var rangeHeaderSubtext = document.createElement("div");
    rangeHeaderSubtext.innerText = "(only affects motion generator)";
    rangeHeaderSubtext.classList.add("range_container_header_subtext");
    deviceRangesTable.appendChild(rangeHeader);
    deviceRangesTable.appendChild(rangeHeaderSubtext);
    for(var i = 0; i < channels.length; i++) {
        var channel = channels[i];
        var name = channel.channel;
        var friendlyName = channel.channelName;
        var max = userSettings["channelRanges"][name].max;
        var min = userSettings["channelRanges"][name].min;

        var sliderHeader = document.createElement("div");
        sliderHeader.innerText = friendlyName;
        sliderHeader.classList.add("range_container_header");
        var sliderControl = document.createElement("div");
        sliderControl.classList.add("sliders_control");

        // Range slider inputs
        var minSlider = document.createElement("input");
        minSlider.id = "minSlider" + name;
        minSlider.type = "range";
        minSlider.min = 2;
        minSlider.max = getTCodeMax();
        minSlider.value = min;
        sliderControl.appendChild(minSlider);
        var maxSlider = document.createElement("input");
        maxSlider.id = "maxSlider" + name;
        maxSlider.type = "range";
        maxSlider.min = 1;
        maxSlider.max = getTCodeMax() - 1;
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
        maxInput.min = 2;
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

        fillSlider(minSlider, maxSlider, '#C6C6C6', '#25daa5', maxSlider);
        setToggleAccessible(maxSlider, maxSlider);
        
        // minSlider.oninput = () => controlMinSlider(minSlider, maxSlider, minInput, name);
        // maxSlider.oninput = () => controlMaxSlider(minSlider, maxSlider, maxInput, name);
        // minInput.oninput = () => controlMinInput(minSlider, minInput, maxInput, maxSlider, name);
        // maxInput.oninput = () => controlMaxInput(minSlider, minInput, maxInput, maxSlider, name);

		minSlider.oninput = function (minSlider, maxSlider, minInput, name) {
			controlMinSlider(minSlider, maxSlider, minInput, name);
		}.bind(minSlider, minSlider, maxSlider, minInput, name);

		maxSlider.oninput = function (minSlider, maxSlider, maxInput, name) {
			controlMaxSlider(minSlider, maxSlider, maxInput, name);
		}.bind(maxSlider, minSlider, maxSlider, maxInput, name);

		minInput.oninput = function (minSlider, minInput, maxInput, maxSlider, name) {
			controlMinInput(minSlider, minInput, maxInput, maxSlider, name);
		}.bind(minInput, minSlider, minInput, maxInput, maxSlider, name);

		maxInput.oninput = function (minSlider, minInput, maxInput, maxSlider, name) {
			controlMaxInput(minSlider, minInput, maxInput, maxSlider, name);
		}.bind(maxInput, minSlider, minInput, maxInput, maxSlider, name);
    }

}
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

function controlMinInput(minSlider, minInput, maxInput, controlSlider, channelName) {
    const [min, max] = getParsed(minInput, maxInput);
    fillSlider(minInput, maxInput, '#C6C6C6', '#25daa5', controlSlider);
    if (min > max) {
        minSlider.value = max;
        minInput.value = max;
    } else {
        minSlider.value = min;
    }
    setChannelRange(channelName, minSlider.value, max);
    sendTCode(channelName + minSlider.value + "S1000");
}
    
function controlMaxInput(maxSlider, minInput, maxInput, controlSlider, channelName) {
    const [min, max] = getParsed(minInput, maxInput);
    fillSlider(minInput, maxInput, '#C6C6C6', '#25daa5', controlSlider);
    setToggleAccessible(maxInput, maxSlider);
    if (min <= max) {
        maxSlider.value = max;
        maxInput.value = max;
    } else {
        maxInput.value = min;
    }
    setChannelRange(channelName, min, maxInput.value);
    sendTCode(channelName + maxInput.value + "S1000");
}

function controlMinSlider(minSlider, maxSlider, minInput, channelName) {
  const [min, max] = getParsed(minSlider, maxSlider);
  fillSlider(minSlider, maxSlider, '#C6C6C6', '#25daa5', maxSlider);
  if (min > max) {
    minSlider.value = max;
    minInput.value = max;
  } else {
    minInput.value = min;
  }
  setChannelRange(channelName, minInput.value, max);
  sendTCode(channelName + minInput.value + "S1000");
}

function controlMaxSlider(minSlider, maxSlider, maxInput, channelName) {
  const [min, max] = getParsed(minSlider, maxSlider);
  fillSlider(minSlider, maxSlider, '#C6C6C6', '#25daa5', maxSlider);
  setToggleAccessible(maxSlider, maxSlider);
  if (min <= max) {
    maxSlider.value = max;
    maxInput.value = max;
  } else {
    maxInput.value = min;
    maxSlider.value = min;
  }
  setChannelRange(channelName, min, maxSlider.value);
  sendTCode(channelName + maxSlider.value  + "S1000");
}

function setChannelRange(channelName, min, max) {
    userSettings["channelRanges"][channelName].min = min;
    userSettings["channelRanges"][channelName].max = max;
    
    updateUserSettings();
}

function getParsed(currentFrom, currentTo) {
  const min = parseInt(currentFrom.value, 10);
  const max = parseInt(currentTo.value, 10);
  return [min, max];
}

function fillSlider(minSlider, maxSlider, sliderColor, rangeColor, controlSlider) {
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
}

function setToggleAccessible(currentTarget, maxSlider) {
  if (Number(currentTarget.value) <= 0 ) {
    maxSlider.style.zIndex = 2;
  } else {
    maxSlider.style.zIndex = 0;
  }
}
