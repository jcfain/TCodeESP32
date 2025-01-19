// SSR1-P_TCode_ESP32_Alpha2
// by TempestMAx 23-2-2023
// Please copy, share, learn, innovate, give attribution.
// Decodes T-code commands and uses them to control servos a single brushless motor
// It can handle:
//   3x linear channels (L0, L1, L2)
//   3x rotation channels (R0, R1, R2) 
//   3x vibration channels (V0, V1, V2)
//   3x auxilliary channels (A0, A1, A2)
// This code is designed to drive the SSR1 stroker robot, but is also intended to be
// used as a template to be adapted to run other t-code controlled arduino projects
// Have fun, play safe!
// History:
// Alpha1 - First release. 2-2-2023
// Alpha2 - Encoder moved to PIN33, End switch pin removed and start sequence changed. 23-2-2023
// Modifications by Khrull (Fain)


#pragma once


#include "datatypes/CommandDataTypes.h"
#include "observer/TObserver.h"
#include "Global.h"
#include "constants.h"

using namespace TCode::Observer;
using namespace TCode::Datatypes;

class EventHandler: public TCodeIObserver<TCodeEvent> {
public:
    void registerOnNotify(TCODE_FUNCTION_PTR_T f)
	{
		message_callback = f;
	}
    void notify(TCodeEvent message) override {
        switch(message.commandType) {
            case CommandType::None:
            break;
            case CommandType::Axis:
            break;
            case CommandType::Device: {
                sendDeviceCommand(message.deviceCommand);
            }
            break;
            case CommandType::Firmware:
                if(message_callback)
                    message_callback(message.firmwareCommand.value);
            break;
            case CommandType::Setup:
                // if(message_callback)
                //     message_callback(message.setupCommand.saveEntryData);
            break;
            default:
            break;
        }
    };

private:
    TCODE_FUNCTION_PTR_T message_callback = 0;
    void sendDeviceCommand(DeviceCommandEvent event) {
        switch(event.type) {
            case DeviceCommandType::None:
            break;
            case DeviceCommandType::StopDevice: {
                if(message_callback)
                    message_callback("DSTOP\n");
            }
            break;
            case DeviceCommandType::GetSoftwareVersion: {
                if(message_callback)
                    message_callback("Firmware v" FIRMWARE_VERSION_NAME "\n");
            }
            break;
            case DeviceCommandType::GetAssignedAxisValues: {
                
            }
            break;
            case DeviceCommandType::GetTCodeVersion: {
                if(message_callback)
                    message_callback("TCode v0.4\n");
            }
            break;
            default:
            break;

        }
    }
    void sendFirmwareCommand(FirmwareCommandEvent event) {
        if(message_callback)
            message_callback(event.value);
    }
};
