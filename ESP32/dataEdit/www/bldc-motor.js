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

BLDCMotor = {
    setup() {
        document.getElementById("BLDC_Encoder").value = userSettings["BLDC_Encoder"];
        document.getElementById("BLDC_UseHallSensor").checked = userSettings["BLDC_UseHallSensor"];
        document.getElementById("BLDC_Pulley_Circumference").value = userSettings["BLDC_Pulley_Circumference"];
        document.getElementById("BLDC_MotorA_VoltageLimit").value = Utils.round2(userSettings["BLDC_MotorA_VoltageLimit"]);
        document.getElementById("BLDC_MotorA_SupplyVoltage").value = Utils.round2(userSettings["BLDC_MotorA_SupplyVoltage"]);
        document.getElementById("BLDC_MotorA_Current").value = Utils.round2(userSettings["BLDC_MotorA_Current"]);
        document.getElementById("BLDC_MotorA_ZeroElecAngle").value = Utils.round2(userSettings["BLDC_MotorA_ZeroElecAngle"]);
        document.getElementById("BLDC_MotorA_ParametersKnown").checked = userSettings["BLDC_MotorA_ParametersKnown"];
        document.getElementById("BLDC_RailLength").value = userSettings["BLDC_RailLength"];
        document.getElementById("BLDC_StrokeLength").value = userSettings["BLDC_StrokeLength"];

        toggleBLDCEncoderOptions();
        Utils.toggleControlVisibilityByID("HallEffect", userSettings["BLDC_UseHallSensor"]);
        Utils.toggleControlVisibilityByID("ZeroElecAngle", userSettings["BLDC_MotorA_ParametersKnown"]);
    },
    setupPins() {
        document.getElementById("BLDC_ChipSelect_PIN").value = pinoutSettings["BLDC_ChipSelect_PIN"];
        document.getElementById("BLDC_Encoder_PIN").value = pinoutSettings["BLDC_Encoder_PIN"];
        document.getElementById("BLDC_Enable_PIN").value = pinoutSettings["BLDC_Enable_PIN"];
        document.getElementById("BLDC_PWMchannel1_PIN").value = pinoutSettings["BLDC_PWMchannel1_PIN"];
        document.getElementById("BLDC_PWMchannel2_PIN").value = pinoutSettings["BLDC_PWMchannel2_PIN"];
        document.getElementById("BLDC_PWMchannel3_PIN").value = pinoutSettings["BLDC_PWMchannel3_PIN"];
        document.getElementById("BLDC_HallEffect_PIN").value = pinoutSettings["BLDC_HallEffect_PIN"];
    }
    // TODO: move bldc stuff in to here. Follow this pattern moving forward.
}
function updateBLDCSettings() {
    userSettings["BLDC_UseHallSensor"] = document.getElementById('BLDC_UseHallSensor').checked;
    Utils.toggleControlVisibilityByID("HallEffect", userSettings["BLDC_UseHallSensor"]);
    userSettings["BLDC_Pulley_Circumference"] = parseInt(document.getElementById('BLDC_Pulley_Circumference').value);
    userSettings["BLDC_MotorA_VoltageLimit"] = Utils.round2(parseFloat(document.getElementById('BLDC_MotorA_VoltageLimit').value));
    userSettings["BLDC_MotorA_SupplyVoltage"] = Utils.round2(parseFloat(document.getElementById('BLDC_MotorA_SupplyVoltage').value));
    userSettings["BLDC_MotorA_Current"] = Utils.round2(parseFloat(document.getElementById('BLDC_MotorA_Current').value));
    userSettings["BLDC_MotorA_ZeroElecAngle"] = Utils.round2(parseFloat(document.getElementById('BLDC_MotorA_ZeroElecAngle').value));
    userSettings["BLDC_MotorA_ParametersKnown"] = document.getElementById("BLDC_MotorA_ParametersKnown").checked;
    userSettings["BLDC_RailLength"] = parseInt(document.getElementById('BLDC_RailLength').value);
    userSettings["BLDC_StrokeLength"] = parseInt(document.getElementById('BLDC_StrokeLength').value);
    Utils.toggleControlVisibilityByID("ZeroElecAngle", userSettings["BLDC_MotorA_ParametersKnown"]);
    setRestartRequired();
    updateUserSettings();
}

function updateBLDCPins() {
    if(upDateTimeout !== null)
    {
        clearTimeout(upDateTimeout);
    }
    upDateTimeout = setTimeout(() =>
    {
        var pinValues = validateBLDCPins();
        if(pinValues) {
            pinoutSettings["BLDC_ChipSelect_PIN"] = pinValues.BLDC_ChipSelect_PIN;
            pinoutSettings["BLDC_Encoder_PIN"] = pinValues.BLDC_Encoder_PIN;
            pinoutSettings["BLDC_Enable_PIN"] = pinValues.BLDC_Enable_PIN;
            pinoutSettings["BLDC_PWMchannel1_PIN"] = pinValues.BLDC_PWMchannel1_PIN;
            pinoutSettings["BLDC_PWMchannel2_PIN"] = pinValues.BLDC_PWMchannel2_PIN;
            pinoutSettings["BLDC_PWMchannel3_PIN"] = pinValues.BLDC_PWMchannel3_PIN;
            pinoutSettings["BLDC_HallEffect_PIN"] = pinValues.BLDC_HallEffect_PIN;
            updateCommonPins(pinValues);
            setRestartRequired();
            postPinoutSettings();
        }
    }, 2000);
}

function getBLDCPinValues() {
    var pinValues = {};
    pinValues.BLDC_ChipSelect_PIN = parseInt(document.getElementById('BLDC_ChipSelect_PIN').value);
    pinValues.BLDC_Encoder_PIN = parseInt(document.getElementById('BLDC_Encoder_PIN').value);
    pinValues.BLDC_Enable_PIN = parseInt(document.getElementById('BLDC_Enable_PIN').value);
    pinValues.BLDC_PWMchannel1_PIN = parseInt(document.getElementById('BLDC_PWMchannel1_PIN').value);
    pinValues.BLDC_PWMchannel2_PIN = parseInt(document.getElementById('BLDC_PWMchannel2_PIN').value);
    pinValues.BLDC_PWMchannel3_PIN = parseInt(document.getElementById('BLDC_PWMchannel3_PIN').value);
    pinValues.BLDC_HallEffect_PIN = parseInt(document.getElementById('BLDC_HallEffect_PIN').value);
    getCommonPinValues(pinValues);
    return pinValues;
}

function validateBLDCPins() {
    clearErrors("pinValidation");
    var assignedPins = [];
    var duplicatePins = [];
    var pwmErrors = [];
    var pinValues = getBLDCPinValues();
    if(userSettings["disablePinValidation"])
        return pinValues;

    if(isModuleType(ModuleType.S3))
    {
        if(isBoardType(BoardType.ZERO)) {
            if(isBLDCSPI()) {
                assignedPins.push({name:"SPI MOSI", pin:11});
            }
        } else {
            // TODO validate this for N8R8
            //assignedPins.push({name:"SPI1", pin:5});
            assignedPins.push({name:"SPI CLK", pin:18});
            assignedPins.push({name:"SPI MISO", pin:19});
            if(isBLDCSPI()) {
                assignedPins.push({name:"SPI MOSI", pin:23});
            }
        }
    }
    else
    {
        //assignedPins.push({name:"SPI1", pin:5});
        assignedPins.push({name:"SPI CLK", pin:18});
        assignedPins.push({name:"SPI MISO", pin:19});
        if(isBLDCSPI()) {
            assignedPins.push({name:"SPI MOSI", pin:23});
        }
    }
    // var pinDupeIndex = -1;
    // if(pinValues.BLDC_Encoder_PIN > -1) {
    //     pinDupeIndex = assignedPins.findIndex(x => x.pin === pinValues.BLDC_Encoder_PIN);
    //     if(pinDupeIndex > -1)
    //         duplicatePins.push("Encoder pin and "+assignedPins[pinDupeIndex].name);
    //     assignedPins.push({name:"Encoder", pin:pinValues.BLDC_Encoder_PIN});
    // }
    validatePin(pinValues.BLDC_Encoder_PIN, "Encoder", assignedPins, duplicatePins);


    // if(pinValues.BLDC_ChipSelect_PIN > -1) {
    //     pinDupeIndex = assignedPins.findIndex(x => x.pin === pinValues.BLDC_ChipSelect_PIN);
    //     if(pinDupeIndex > -1)
    //         duplicatePins.push("Chip select and "+assignedPins[pinDupeIndex].name);
    //     assignedPins.push({name:"Chip select", pin:pinValues.BLDC_ChipSelect_PIN});
    // }
    validatePin(pinValues.BLDC_ChipSelect_PIN, "Chip select", assignedPins, duplicatePins);

    // if(pinValues.BLDC_Enable_PIN > -1) {
    //     pinDupeIndex = assignedPins.findIndex(x => x.pin === pinValues.BLDC_Enable_PIN);
    //     if(pinDupeIndex > -1)
    //         duplicatePins.push("Enable pin and "+assignedPins[pinDupeIndex].name);
    //     assignedPins.push({name:"Enable", pin:pinValues.BLDC_Enable_PIN});
    // }
    validatePin(pinValues.BLDC_Enable_PIN, "Enable", assignedPins, duplicatePins);

    // if(pinValues.BLDC_PWMchannel1_PIN > -1) {
    //     pinDupeIndex = assignedPins.findIndex(x => x.pin === pinValues.BLDC_PWMchannel1_PIN);
    //     if(pinDupeIndex > -1)
    //         duplicatePins.push("PWMchannel1 pin and "+assignedPins[pinDupeIndex].name);
    //     if(validPWMpins.indexOf(pinValues.BLDC_PWMchannel1_PIN) == -1)
    //         pwmErrors.push("PWMchannel1 pin: "+pinValues.BLDC_PWMchannel1_PIN);
    //     assignedPins.push({name:"PWMchannel1", pin:pinValues.BLDC_PWMchannel1_PIN});
    // }
    validatePWMPin(pinValues.rightPin, "PWMchannel1", assignedPins, duplicatePins, pwmErrors);

    // if(pinValues.BLDC_PWMchannel2_PIN > -1) {
    //     pinDupeIndex = assignedPins.findIndex(x => x.pin === pinValues.BLDC_PWMchannel2_PIN);
    //     if(pinDupeIndex > -1)
    //         duplicatePins.push("PWMchannel2 pin and "+assignedPins[pinDupeIndex].name);
    //     if(validPWMpins.indexOf(pinValues.BLDC_PWMchannel2_PIN) == -1)
    //         pwmErrors.push("PWMchannel2 pin: "+pinValues.BLDC_PWMchannel2_PIN);
    //     assignedPins.push({name:"PWMchannel2", pin:pinValues.BLDC_PWMchannel2_PIN});
    // }
    validatePWMPin(pinValues.rightPin, "PWMchannel2", assignedPins, duplicatePins, pwmErrors);

    // if(pinValues.BLDC_PWMchannel3_PIN > -1) {
    //     pinDupeIndex = assignedPins.findIndex(x => x.pin === pinValues.BLDC_PWMchannel3_PIN);
    //     if(pinDupeIndex > -1)
    //         duplicatePins.push("PWMchannel3 pin and "+assignedPins[pinDupeIndex].name);
    //     if(validPWMpins.indexOf(pinValues.BLDC_PWMchannel3_PIN) == -1)
    //         pwmErrors.push("PWMchannel3pin: "+pinValues.BLDC_PWMchannel3_PIN);
    //     assignedPins.push({name:"PWMchannel3", pin:pinValues.BLDC_PWMchannel3_PIN});
    // }
    validatePWMPin(pinValues.BLDC_PWMchannel3_PIN, "PWMchannel3", assignedPins, duplicatePins, pwmErrors);

    if(userSettings["BLDC_UseHallSensor"]) {
        // if(pinValues.BLDC_HallEffect_PIN > -1) {
        //     pinDupeIndex = assignedPins.findIndex(x => x.pin === pinValues.BLDC_HallEffect_PIN);
        //     if(pinDupeIndex > -1)
        //         duplicatePins.push("Hall effect pin and "+assignedPins[pinDupeIndex].name);
        //     assignedPins.push({name:"HallEffect", pin:pinValues.BLDC_HallEffect_PIN});
        // }
        validatePin(pinValues.BLDC_HallEffect_PIN, "HallEffect", assignedPins, duplicatePins);
    }

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
