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

class BLDCMotor {
    name = "";
    Names = {};
    ModalNode;
    ParentNode;
    initialized = false;
    initializedPins = false;
    constructor(name = "") {
        this.name = name;
        this.ModalNode = document.getElementById(this.name + "MotorSettings");
        if(!this.ModalNode)
        {
            this.ModalNode = document.createElement("modal-component");
            this.ModalNode.id = this.name + "MotorSettings";
            const header = document.createElement("span");
            header.innerText = this.name.length == 0 ? "Stroke Motor Settings" : this.name + " Motor Settings"  
            header.setAttribute("slot", "title");
            this.ModalNode.appendChild(header);
            document.body.appendChild(this.ModalNode);

            const ParentTable = document.createElement("div");
            ParentTable.id = this.name + "MotorSettingsTable";
            ParentTable.setAttribute("name", this.name + "MotorSettingsTable");
            ParentTable.classList.add("formTable");
            this.ParentNode = document.createElement("div");
            ParentTable.appendChild(this.ParentNode);
            this.ModalNode.appendChild(ParentTable);
        }

        // this.Names.BLDC_Encoder = "BLDC_" + name + "Encoder";
        // this.Names.BLDC_UseHallSensor = "BLDC_" + name + "UseHallSensor";
        this.Names.BLDC_Encoder = "BLDC_Encoder";
        this.Names.BLDC_UseHallSensor = "BLDC_UseHallSensor";
        this.Names.BLDC_Pulley_Circumference = "BLDC_" + name + "Pulley_Circumference";
        this.Names.BLDC_Motor_VoltageLimit = "BLDC_" + name + "Motor_VoltageLimit";
        this.Names.BLDC_Motor_SupplyVoltage = "BLDC_" + name + "Motor_SupplyVoltage";
        this.Names.BLDC_Motor_Current = "BLDC_" + name + "Motor_Current";
        this.Names.BLDC_Motor_ZeroElecAngle = "BLDC_" + name + "Motor_ZeroElecAngle";
        this.Names.BLDC_Motor_ParametersKnown = "BLDC_" + name + "Motor_ParametersKnown";
        this.Names.BLDC_RailLength = "BLDC_" + name + "RailLength";
        this.Names.BLDC_Range = "BLDC_" + (name.length == 0 ? "Stroke" : name) + "Length";
        this.Names.BLDC_ChipSelect_PIN = "BLDC_" + name + "ChipSelect_PIN";
        this.Names.BLDC_Encoder_PIN = "BLDC_" + name + "Encoder_PIN";
        this.Names.BLDC_Enable_PIN = "BLDC_" + name + "Enable_PIN";
        this.Names.BLDC_PWMchannel1_PIN = "BLDC_" + name + "PWMchannel1_PIN";
        this.Names.BLDC_PWMchannel2_PIN = "BLDC_" + name + "PWMchannel2_PIN";
        this.Names.BLDC_PWMchannel3_PIN = "BLDC_" + name + "PWMchannel3_PIN";
        // this.Names.BLDC_HallEffect_PIN = "BLDC_" + name + "HallEffect_PIN";
        // this.Names.HallEffect_Row = name + "HallEffect",
        this.Names.ZeroElecAngle_Row = name + "ZeroElecAngle";
    }

    setup() {
        if(this.initialized)
            return;

                // let channelRow = Utils.createNumericFormRow(0, "Update rate (ms)", 'motionUpdate'+profileIndex+channelIndex, motionChannel ? motionChannel.update : 100, 0, 2147483647, 
                //     function(profileIndex, channelIndex, name) {setMotionGeneratorSettings(profileIndex, channelIndex, name)}.bind(this, profileIndex, channelIndex, name));
                // channelRow.title = `This is the time in between updates that gives the system time to process other tasks. (DO NOT SET TOO LOW ON ESP32!)
                // It may be best to just leave at default.`
                // channelTableDiv.appendChild(channelRow.row);

        //ToDo create combo box maybe
        // const encoderNode = Utils.createNumericFormRow(null, "Encoder", this.Names.BLDC_Encoder, userSettings[this.Names.BLDC_Encoder], null, null, this.setEncoderType);
        // motorSettingsTable.appendChild(encoderNode.row);
        // // document.getElementById(this.Names.BLDC_Encoder).value = userSettings[this.Names.BLDC_Encoder];
        // this.createBLDCCheckboxFormNode(this.Names.BLDC_UseHallSensor, "Use hall sensor", userSettings[this.Names.BLDC_UseHallSensor], () => this.updateBLDCSettings(0));
        this.createBLDCNumericFormNode(this.Names.BLDC_Pulley_Circumference, "Pulley Circumference (mm)", userSettings[this.Names.BLDC_Pulley_Circumference], () => this.updateBLDCSettings(), 0, 2147483647);
        this.createBLDCNumericFormNode(this.Names.BLDC_Motor_VoltageLimit, "Voltage limit (v)", userSettings[this.Names.BLDC_Motor_VoltageLimit], () => this.updateBLDCSettings(), 0.0, 2147483647.0);
        this.createBLDCNumericFormNode(this.Names.BLDC_Motor_SupplyVoltage, "Supply voltage (v)", userSettings[this.Names.BLDC_Motor_SupplyVoltage], () => this.updateBLDCSettings(), 0.0, 2147483647.0);
        this.createBLDCNumericFormNode(this.Names.BLDC_Motor_Current, "Motor current (a)", userSettings[this.Names.BLDC_Motor_Current], () => this.updateBLDCSettings(), 0.0, 2147483647.0);
        this.createBLDCCheckboxFormNode(this.Names.BLDC_Motor_ParametersKnown, "Parameters known", userSettings[this.Names.BLDC_Motor_ParametersKnown], () => this.updateBLDCSettings(0));
        this.createBLDCNumericFormNode(this.Names.BLDC_Motor_ZeroElecAngle, "Zero elec angle (rad)", userSettings[this.Names.BLDC_Motor_ZeroElecAngle], () => this.updateBLDCSettings(), 0.0, 2147483647.0, this.Names.ZeroElecAngle_Row);
        this.createBLDCNumericFormNode(this.Names.BLDC_RailLength, "Rail length (mm)", userSettings[this.Names.BLDC_RailLength], () => this.updateBLDCSettings(), 0, 2147483647);
        this.createBLDCNumericFormNode(this.Names.BLDC_Range, (this.name.length == 0 ? "Stroke" : this.name) + " length (mm)", userSettings[this.Names.BLDC_Range], () => this.updateBLDCSettings(), 0, 2147483647);

        // this.toggleBLDCEncoderOptions();
        // Utils.toggleControlVisibilityByID(this.Names.HallEffect_Row, userSettings[this.Names.BLDC_UseHallSensor]);
        Utils.toggleControlVisibilityByID(this.Names.ZeroElecAngle_Row, userSettings[this.Names.BLDC_Motor_ParametersKnown]);
        this.initialized = true;
    }

    createBLDCNumericFormNode(key, label, value, callback, min = undefined, max = undefined, rowName = undefined) {
        const node = Utils.createNumericFormRow(rowName, label, key, value, min, max, callback);
        this.ParentNode.appendChild(node.row);
    }
    createBLDCCheckboxFormNode(key, label, value, callback, rowName = undefined) {
        const node = Utils.createCheckboxFormRow(rowName, label, key, value, callback);
        this.ParentNode.appendChild(node.row);
    }

    setupPins() {
        if(this.initializedPins)
            return;
        this.createBLDCNumericFormNode(this.Names.BLDC_ChipSelect_PIN, "Chip select PIN", pinoutSettings[this.Names.BLDC_ChipSelect_PIN], () => this.updateBLDCPins(), -1, 2147483647);
        this.createBLDCNumericFormNode(this.Names.BLDC_Encoder_PIN, "Encoder PIN", pinoutSettings[this.Names.BLDC_Encoder_PIN], () => this.updateBLDCPins(), -1, 2147483647);
        this.createBLDCNumericFormNode(this.Names.BLDC_Enable_PIN, "Enable PIN", pinoutSettings[this.Names.BLDC_Enable_PIN], () => this.updateBLDCPins(), -1, 2147483647);
        this.createBLDCNumericFormNode(this.Names.BLDC_PWMchannel1_PIN, "PWM channel 1 PIN", pinoutSettings[this.Names.BLDC_PWMchannel1_PIN], () => this.updateBLDCPins(), -1, 2147483647);
        this.createBLDCNumericFormNode(this.Names.BLDC_PWMchannel2_PIN, "PWM channel 2 PIN", pinoutSettings[this.Names.BLDC_PWMchannel2_PIN], () => this.updateBLDCPins(), -1, 2147483647);
        this.createBLDCNumericFormNode(this.Names.BLDC_PWMchannel3_PIN, "PWM channel 3 PIN", pinoutSettings[this.Names.BLDC_PWMchannel3_PIN], () => this.updateBLDCPins(), -1, 2147483647);
        // this.createBLDCNumericFormNode(this.Names.BLDC_HallEffect_PIN, "Hall effect PIN", pinoutSettings[this.Names.BLDC_HallEffect_PIN], () => this.updateBLDCPins(), -1, 2147483647, this.Names.HallEffect_Row);
        this.initializedPins = true;
    }

    // TODO: move bldc stuff in to here. Follow this pattern moving forward.
    updateBLDCSettings(delay = defaultDebounce) {
        Utils.debounce("updateBLDCSettings", () => {
            userSettings[this.Names.BLDC_UseHallSensor] = document.getElementById(this.Names.BLDC_UseHallSensor).checked;
            // Utils.toggleControlVisibilityByID(this.Names.HallEffect_Row, userSettings[this.Names.BLDC_UseHallSensor]);
            userSettings[this.Names.BLDC_Pulley_Circumference] = parseInt(document.getElementById(this.Names.BLDC_Pulley_Circumference).value);
            userSettings[this.Names.BLDC_MotorA_VoltageLimit] = Utils.round2(parseFloat(document.getElementById(this.Names.BLDC_Motor_VoltageLimit).value));
            userSettings[this.Names.BLDC_MotorA_SupplyVoltage] = Utils.round2(parseFloat(document.getElementById(this.Names.BLDC_Motor_SupplyVoltage).value));
            userSettings[this.Names.BLDC_MotorA_Current] = Utils.round2(parseFloat(document.getElementById(this.Names.BLDC_Motor_Current).value));
            userSettings[this.Names.BLDC_MotorA_ZeroElecAngle] = Utils.round2(parseFloat(document.getElementById(this.Names.BLDC_Motor_ZeroElecAngle).value));
            userSettings[this.Names.BLDC_MotorA_ParametersKnown] = document.getElementById(this.Names.BLDC_Motor_ParametersKnown).checked;
            userSettings[this.Names.BLDC_RailLength] = parseInt(document.getElementById(this.Names.BLDC_RailLength).value);
            userSettings[this.Names.BLDC_Range] = parseInt(document.getElementById(this.Names.BLDC_Range).value);
            Utils.toggleControlVisibilityByID(this.Names.ZeroElecAngle_Row, userSettings[this.Names.BLDC_Motor_ParametersKnown]);
            setRestartRequired();
            updateUserSettings();
        }, delay);
    }

    updateBLDCPins(delay = defaultDebounce) {
        Utils.debounce("updateBLDCPins", () => {
            var pinValues = this.validateBLDCPins();
            if(pinValues) {
                pinoutSettings[this.Names.BLDC_ChipSelect_PIN] = pinValues.BLDC_ChipSelect_PIN;
                pinoutSettings[this.Names.BLDC_Encoder_PIN] = pinValues.BLDC_Encoder_PIN;
                pinoutSettings[this.Names.BLDC_Enable_PIN] = pinValues.BLDC_Enable_PIN;
                pinoutSettings[this.Names.BLDC_PWMchannel1_PIN] = pinValues.BLDC_PWMchannel1_PIN;
                pinoutSettings[this.Names.BLDC_PWMchannel2_PIN] = pinValues.BLDC_PWMchannel2_PIN;
                pinoutSettings[this.Names.BLDC_PWMchannel3_PIN] = pinValues.BLDC_PWMchannel3_PIN;
                // pinoutSettings[this.Names.BLDC_HallEffect_PIN] = pinValues.BLDC_HallEffect_PIN;
                pinoutSettings["BLDC_HallEffect_PIN"] = pinValues.BLDC_HallEffect_PIN;
                updateCommonPins(pinValues);
                setRestartRequired();
                postPinoutSettings();
            }
        }, delay);
    }

    getBLDCPinValues() {
        var pinValues = {};
        pinValues.BLDC_ChipSelect_PIN = parseInt(document.getElementById(this.Names.BLDC_ChipSelect_PIN).value);
        pinValues.BLDC_Encoder_PIN = parseInt(document.getElementById(this.Names.BLDC_Encoder_PIN).value);
        pinValues.BLDC_Enable_PIN = parseInt(document.getElementById(this.Names.BLDC_Enable_PIN).value);
        pinValues.BLDC_PWMchannel1_PIN = parseInt(document.getElementById(this.Names.BLDC_PWMchannel1_PIN).value);
        pinValues.BLDC_PWMchannel2_PIN = parseInt(document.getElementById(this.Names.BLDC_PWMchannel2_PIN).value);
        pinValues.BLDC_PWMchannel3_PIN = parseInt(document.getElementById(this.Names.BLDC_PWMchannel3_PIN).value);
        // pinValues.BLDC_HallEffect_PIN = parseInt(document.getElementById(this.Names.BLDC_HallEffect_PIN).value);
        pinValues.BLDC_HallEffect_PIN = parseInt(document.getElementById("BLDC_HallEffect_PIN").value);
        getCommonPinValues(pinValues);
        return pinValues;
    }

    validateBLDCPins() {
        clearErrors("pinValidation"); 
        var assignedPins = [];
        var duplicatePins = [];
        var pwmErrors = [];
        var pinValues = this.getBLDCPinValues();
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
        validatePin(pinValues.BLDC_Encoder_PIN, "Encoder", assignedPins, duplicatePins);
        validatePin(pinValues.BLDC_ChipSelect_PIN, "Chip select", assignedPins, duplicatePins);
        validatePin(pinValues.BLDC_Enable_PIN, "Enable", assignedPins, duplicatePins);
        validatePWMPin(pinValues.BLDC_PWMchannel1_PIN, "PWMchannel1", assignedPins, duplicatePins, pwmErrors);
        validatePWMPin(pinValues.BLDC_PWMchannel2_PIN, "PWMchannel2", assignedPins, duplicatePins, pwmErrors);
        validatePWMPin(pinValues.BLDC_PWMchannel3_PIN, "PWMchannel3", assignedPins, duplicatePins, pwmErrors);

        if(userSettings["BLDC_UseHallSensor"]) {
            validatePin(pinValues.BLDC_HallEffect_PIN, "Hall effect", assignedPins, duplicatePins);
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
    // setEncoderType() {
    //     userSettings[this.Names.BLDC_Encoder] = parseInt(document.getElementById(this.Names.BLDC_Encoder).value);
    //     this.toggleBLDCEncoderOptions();
    //     setRestartRequired();
    //     updateUserSettings(0);
    // }
    // // Not used currently
    // setupEncoderTypes() {
    //     const element = document.getElementById(this.Names.BLDC_Encoder);
    //     removeAllChildren(element);
    //     for(let i=0;i<systemInfo.encoderTypes.length;i++) {
    //         const option = document.createElement("option");
    //         option.innerText = systemInfo.encoderTypes[i].name;
    //         option.value = systemInfo.encoderTypes[i].value;
    //         element.appendChild(option);
    //         BLDCEncoderType[systemInfo.encoderTypes[i].name] = systemInfo.encoderTypes[i].value;
    //     }
    // }
    // toggleBLDCEncoderOptions() {
    //     Utils.toggleControlVisibilityByClassName("BLDCPWM", userSettings[this.Names.BLDC_Encoder] == BLDCEncoderType.PWM);
    //     Utils.toggleControlVisibilityByClassName("BLDCSPI", isBLDCSPI());
    // }
}