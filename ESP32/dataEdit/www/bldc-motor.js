
BLDCMotor = {
    setup() {
        document.getElementById("BLDC_MotorA_Voltage").value = Utils.round2(userSettings["BLDC_MotorA_Voltage"]);
        document.getElementById("BLDC_MotorA_Current").value = Utils.round2(userSettings["BLDC_MotorA_Current"]);
        document.getElementById("BLDC_Encoder_PIN").value = userSettings["BLDC_Encoder_PIN"];
        document.getElementById("BLDC_Enable_PIN").value = userSettings["BLDC_Enable_PIN"];
        document.getElementById("BLDC_PWMchannel1_PIN").value = userSettings["BLDC_PWMchannel1_PIN"];
        document.getElementById("BLDC_PWMchannel2_PIN").value = userSettings["BLDC_PWMchannel2_PIN"];
        document.getElementById("BLDC_PWMchannel3_PIN").value = userSettings["BLDC_PWMchannel3_PIN"];
    }
    // TODO: move bldc stuff in to here. Follow this pattern moving forward.
}

