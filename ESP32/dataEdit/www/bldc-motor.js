
BLDCMotor = {
    setup() {
        document.getElementById("BLDC_UsePWM").checked = userSettings["BLDC_UsePWM"];
        document.getElementById("BLDC_UseMT6701").checked = userSettings["BLDC_UseMT6701"];
        document.getElementById("BLDC_UseSPI").checked = !userSettings["BLDC_UseMT6701"] && !userSettings["BLDC_UsePWM"];
        document.getElementById("BLDC_UseHallSensor").checked = userSettings["BLDC_UseHallSensor"];
        document.getElementById("BLDC_Pulley_Circumference").value = userSettings["BLDC_Pulley_Circumference"];
        document.getElementById("BLDC_MotorA_Voltage").value = Utils.round2(userSettings["BLDC_MotorA_Voltage"]);
        document.getElementById("BLDC_MotorA_Current").value = Utils.round2(userSettings["BLDC_MotorA_Current"]);
        document.getElementById("BLDC_ChipSelect_PIN").value = userSettings["BLDC_ChipSelect_PIN"];
        document.getElementById("BLDC_Encoder_PIN").value = userSettings["BLDC_Encoder_PIN"];
        document.getElementById("BLDC_Enable_PIN").value = userSettings["BLDC_Enable_PIN"];
        document.getElementById("BLDC_PWMchannel1_PIN").value = userSettings["BLDC_PWMchannel1_PIN"];
        document.getElementById("BLDC_PWMchannel2_PIN").value = userSettings["BLDC_PWMchannel2_PIN"];
        document.getElementById("BLDC_PWMchannel3_PIN").value = userSettings["BLDC_PWMchannel3_PIN"];
        document.getElementById("BLDC_HallEffect_PIN").value = userSettings["BLDC_HallEffect_PIN"];
        document.getElementById("BLDC_MotorA_ZeroElecAngle").value = Utils.round2(userSettings["BLDC_MotorA_ZeroElecAngle"]);
        document.getElementById("BLDC_MotorA_ParametersKnown").checked = userSettings["BLDC_MotorA_ParametersKnown"];

        Utils.toggleControlVisibilityByClassName("BLDCPWM", userSettings["BLDC_UsePWM"]);
        Utils.toggleControlVisibilityByClassName("BLDCSPI", !userSettings["BLDC_UsePWM"]);
        Utils.toggleControlVisibilityByID("HallEffect", userSettings["BLDC_UseHallSensor"]);
        Utils.toggleControlVisibilityByID("ZeroElecAngle", userSettings["BLDC_MotorA_ParametersKnown"]);
    }
    // TODO: move bldc stuff in to here. Follow this pattern moving forward.
}

function updateBLDCSettings() {
    userSettings["BLDC_UseHallSensor"] = document.getElementById('BLDC_UseHallSensor').checked;
    Utils.toggleControlVisibilityByID("HallEffect", userSettings["BLDC_UseHallSensor"]);
    userSettings["BLDC_Pulley_Circumference"] = parseInt(document.getElementById('BLDC_Pulley_Circumference').value);
    userSettings["BLDC_MotorA_Voltage"] = Utils.round2(parseFloat(document.getElementById('BLDC_MotorA_Voltage').value));
    userSettings["BLDC_MotorA_Current"] = Utils.round2(parseFloat(document.getElementById('BLDC_MotorA_Current').value));
    userSettings["BLDC_MotorA_ZeroElecAngle"] = Utils.round2(parseFloat(document.getElementById('BLDC_MotorA_ZeroElecAngle').value));
    userSettings["BLDC_MotorA_ParametersKnown"] = document.getElementById("BLDC_MotorA_ParametersKnown").checked;
    Utils.toggleControlVisibilityByID("ZeroElecAngle", userSettings["BLDC_MotorA_ParametersKnown"]);
    setRestartRequired();
    updateUserSettings();
}

function updateEncoderType(type) {
    userSettings["BLDC_UsePWM"] = false;
    userSettings["BLDC_UseMT6701"] = false;
    //userSettings["BLDC_UseSPI"] = false; // Not used. This state is represented by above two falsey
    userSettings[type] = true;
    Utils.toggleControlVisibilityByClassName("BLDCPWM", userSettings["BLDC_UsePWM"]);
    Utils.toggleControlVisibilityByClassName("BLDCSPI", !userSettings["BLDC_UsePWM"]);
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
            userSettings["BLDC_ChipSelect_PIN"] = pinValues.BLDC_ChipSelect_PIN;
            userSettings["BLDC_Encoder_PIN"] = pinValues.BLDC_Encoder_PIN;
            userSettings["BLDC_Enable_PIN"] = pinValues.BLDC_Enable_PIN;
            userSettings["BLDC_PWMchannel1_PIN"] = pinValues.BLDC_PWMchannel1_PIN;
            userSettings["BLDC_PWMchannel2_PIN"] = pinValues.BLDC_PWMchannel2_PIN;
            userSettings["BLDC_PWMchannel3_PIN"] = pinValues.BLDC_PWMchannel3_PIN;
            userSettings["BLDC_HallEffect_PIN"] = pinValues.BLDC_HallEffect_PIN;
            updateCommonPins(pinValues);
            setRestartRequired();
            updateUserSettings();
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
    var pmwErrors = [];
    var pinValues = getBLDCPinValues();
    if(userSettings["disablePinValidation"])
        return pinValues;

    //assignedPins.push({name:"SPI1", pin:5});
    assignedPins.push({name:"SPI1", pin:18});
    assignedPins.push({name:"SPI2", pin:19});

    var pinDupeIndex = assignedPins.findIndex(x => x.pin === pinValues.BLDC_Encoder_PIN);
    if(pinDupeIndex > -1)
        duplicatePins.push("Encoder pin and "+assignedPins[pinDupeIndex].name);
    assignedPins.push({name:"Encoder", pin:pinValues.BLDC_Encoder_PIN});
    
    pinDupeIndex = assignedPins.findIndex(x => x.pin === pinValues.BLDC_ChipSelect_PIN);
    if(pinDupeIndex > -1)
        duplicatePins.push("Chip select and "+assignedPins[pinDupeIndex].name);
    assignedPins.push({name:"Chip select", pin:pinValues.BLDC_ChipSelect_PIN});

    pinDupeIndex = assignedPins.findIndex(x => x.pin === pinValues.BLDC_Enable_PIN);
    if(pinDupeIndex > -1)
        duplicatePins.push("Enable pin and "+assignedPins[pinDupeIndex].name);
    assignedPins.push({name:"Enable", pin:pinValues.BLDC_Enable_PIN});

    pinDupeIndex = assignedPins.findIndex(x => x.pin === pinValues.BLDC_PWMchannel1_PIN);
    if(pinDupeIndex > -1)
        duplicatePins.push("PWMchannel1 pin and "+assignedPins[pinDupeIndex].name);
    if(validPWMpins.indexOf(pinValues.BLDC_PWMchannel1_PIN) == -1)
        pmwErrors.push("PWMchannel1 pin: "+pinValues.BLDC_PWMchannel1_PIN);
    assignedPins.push({name:"PWMchannel1", pin:pinValues.BLDC_PWMchannel1_PIN});

    pinDupeIndex = assignedPins.findIndex(x => x.pin === pinValues.BLDC_PWMchannel2_PIN);
    if(pinDupeIndex > -1)
        duplicatePins.push("PWMchannel2 pin and "+assignedPins[pinDupeIndex].name);
    if(validPWMpins.indexOf(pinValues.BLDC_PWMchannel2_PIN) == -1)
        pmwErrors.push("PWMchannel2 pin: "+pinValues.BLDC_PWMchannel2_PIN);
    assignedPins.push({name:"PWMchannel2", pin:pinValues.BLDC_PWMchannel2_PIN});

    pinDupeIndex = assignedPins.findIndex(x => x.pin === pinValues.BLDC_PWMchannel3_PIN);
    if(pinDupeIndex > -1)
        duplicatePins.push("PWMchannel3 pin and "+assignedPins[pinDupeIndex].name);
    if(validPWMpins.indexOf(pinValues.BLDC_PWMchannel3_PIN) == -1)
        pmwErrors.push("PWMchannel3pin: "+pinValues.BLDC_PWMchannel3_PIN);
    assignedPins.push({name:"PWMchannel3", pin:pinValues.BLDC_PWMchannel3_PIN});

    if(userSettings["BLDC_UsePWM"]) {
        pinDupeIndex = assignedPins.findIndex(x => x.pin === pinValues.BLDC_HallEffect_PIN);
        if(pinDupeIndex > -1)
            duplicatePins.push("Hall effect pin and "+assignedPins[pinDupeIndex].name);
        assignedPins.push({name:"HallEffect", pin:pinValues.BLDC_HallEffect_PIN});
    }
    
    validateCommonPWMPins(assignedPins, duplicatePins, pinValues, pmwErrors);

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