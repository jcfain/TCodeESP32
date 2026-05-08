#pragma once

#include <Arduino.h>
#include "LogHandler.h"
#include "DisplayHandler.h"
#include "VoiceHandler.hpp"
#include "WebSocketBase.h"
#if SECURE_WEB
#include "HTTPSHandler.hpp"
#include "WebHandler.h"
#endif


class TaskHandler
{
public:
    static TaskHandler* getInstance() {
        static TaskHandler instance;
        return &instance;
    }
    void init() 
    {

    }
#if BUILD_DISPLAY
    void startDisplayTask(DisplayHandler * displayHandler) 
    {
		LogHandler::debug(TagHandler::Main, "Start Display task");
		auto displayStatus = xTaskCreatePinnedToCore(
			DisplayHandler::startLoop,	  /* Function to implement the task */
			"DisplayTask",				  /* Name of the task */
			configMINIMAL_STACK_SIZE * 4, /* Stack size in words used to be 5000 */
			displayHandler,				  /* Task input parameter */
			1,							  /* Priority of the task */
			&displayTask,				  /* Task handle. */
			TASK_CPU_NUM);				  /* Core where the task should run */
		if (displayStatus != pdPASS)
		{
			LogHandler::error(TagHandler::Main, "Could not start display task.");
		}
    }
    void startDisplayAnimationTask(DisplayHandler* displayHandler) 
    {
        // LogHandler::debug(TagHandler::Main, "Start Display Animation task");
        //     auto status = xTaskCreatePinnedToCore(
        //         DisplayHandler::startAnimationDontPanic,/* Function to implement the task */
        //         "DisplayTask", /* Name of the task */
        //         10000,  /* Stack size in words */
        //         displayHandler,  /* Task input parameter */
        //         25,  /* Priority of the task */
        //         &animationTask,  /* Task handle. */
        //         APP_CPU_NUM); /* Core where the task should run */
        // if (status != pdPASS)
        // {
        //     LogHandler::error(TagHandler::Main, "Could not start Display Animation task.");
        // }
    }
#endif
#if BUILD_TEMP
    void startTemperatureTask(TemperatureHandler* temperatureHandler) 
    {
        LogHandler::debug(TagHandler::Main, "Start temperature task");
        auto tempStartStatus = xTaskCreatePinnedToCore(
            TemperatureHandler::startLoop, /* Function to implement the task */
            "TempTask",					   /* Name of the task */
            static_cast<uint16_t>(configMINIMAL_STACK_SIZE * 3),  /* Stack size in words used to be 5000 */
            temperatureHandler,			   /* Task input parameter */
            1,							   /* Priority of the task */
            &temperatureTask,			   /* Task handle. */
            TASK_CPU_NUM);				   /* Core where the task should run */
        if (tempStartStatus != pdPASS)
        {
            LogHandler::error(TagHandler::Main, "Could not start temperature task.");
        }
    }
#endif

    void startBatteryTask(BatteryHandler* batteryHandler) 
    {
        LogHandler::debug(TagHandler::Main, "Start Battery task");
        auto batteryStatus = xTaskCreatePinnedToCore(
            BatteryHandler::startLoop, /* Function to implement the task */
            "BatteryTask",			   /* Name of the task */
            configMINIMAL_STACK_SIZE,  /* Stack size in words used to be 4028 */
            batteryHandler,			   /* Task input parameter */
            1,						   /* Priority of the task */
            &batteryTask,			   /* Task handle. */
            TASK_CPU_NUM);			   /* Core where the task should run */
        if (batteryStatus != pdPASS)
        {
            LogHandler::error(TagHandler::Main, "Could not start battery task.");
        }
    }
    // void startBatteryTask(BatteryHandler* batteryHandler) 
    // {
    //     LogHandler::debug(TagHandler::Main, "Start Battery task");
    //     auto batteryStatus = xTaskCreatePinnedToCore(
    //         BatteryHandler::startLoop, /* Function to implement the task */
    //         "BatteryTask",			   /* Name of the task */
    //         configMINIMAL_STACK_SIZE,  /* Stack size in words used to be 4028 */
    //         batteryHandler,			   /* Task input parameter */
    //         1,						   /* Priority of the task */
    //         &batteryTask,			   /* Task handle. */
    //         APP_CPU_NUM);			   /* Core where the task should run */
    //     if (batteryStatus != pdPASS)
    //     {
    //         LogHandler::error(TagHandler::Main, "Could not start battery task.");
    //     }
    // }

    void startVoiceTask(VoiceHandler* voiceHandler) 
    {
        LogHandler::debug(TagHandler::Main, "Start Voice task");
        auto voiceStatus = xTaskCreatePinnedToCore(
            VoiceHandler::startLoop,  /* Function to implement the task */
            "VoiceTask",			  /* Name of the task */
            configMINIMAL_STACK_SIZE, /* Stack size in words used to be 4028 */
            voiceHandler,			  /* Task input parameter */
            1,						  /* Priority of the task */
            &voiceTask,				  /* Task handle. */
            TASK_CPU_NUM);			  /* Core where the task should run */
        if (voiceStatus != pdPASS)
        {
            LogHandler::error(TagHandler::Main, "Could not start voice task.");
        }
    }
#if SECURE_WEB
    void startHTTPSTask(WebHandler* webHandler) 
    {
        LogHandler::debug(TagHandler::Main, "Start https task");
        auto httpsStatus = xTaskCreateUniversal(
            HTTPSHandler::startLoop, /* Function to implement the task */
            "HTTPSTask",			 /* Name of the task */
            8192 * 3,				 /* Stack size in words */
            webHandler,				 /* Task input parameter */
            3,						 /* Priority of the task */
            &httpsTask,				 /* Task handle. */
            -1);					 /* Core where the task should run */
        if (httpsStatus != pdPASS)
        {
            LogHandler::error(TagHandler::Main, "Could not start https task.");
        }
    }
#endif

void startWebsocketLogging(WebSocketBase* webSocketBase) 
{
    LogHandler::debug(TagHandler::Main, "Start web socket logging");
    auto webSocketLoggingStatus = xTaskCreatePinnedToCore(
        WebSocketBase::startLoggingTask,  /* Function to implement the task */
        "WebsocketLoggingTask",			  /* Name of the task */
        configMINIMAL_STACK_SIZE * 4, /* Stack size in words */
        webSocketBase,			  /* Task input parameter */
        1,						  /* Priority of the task */
        &webSocketLoggingTask,	  /* Task handle. */
        TASK_CPU_NUM);			  /* Core where the task should run */
    if (webSocketLoggingStatus != pdPASS)
    {
        LogHandler::error(TagHandler::Main, "Could not start websocket logging task.");
    }
}

private:
    TaskHandle_t batteryTask;
    TaskHandle_t httpsTask;

    TaskHandle_t voiceTask;
    TaskHandle_t webSocketLoggingTask;


#if BUILD_DISPLAY
    TaskHandle_t displayTask;
    // #if ISAAC_NEWTONGUE_BUILD
    // 	TaskHandle_t animationTask;
    // #endif
#endif
#if BUILD_TEMP
    TaskHandle_t temperatureTask;
#endif
};