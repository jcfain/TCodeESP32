# TCodeESP32
This is an ESP32 fork of Tempests TCode sketch (repository doesnt exists yet). It adds Network capabilities and bluetooth.

## INSTALLATION:

  ### LIBRARIES 
    You will need a few additional libraries to build this sketch.
    - ESP32Servo
    - ESP Async Webserver
    - EAsyncTCP
    - EArduinoJson
    
  ### TOOLS
    I used Visual studio code and PlatformIO for this project.
    I believe the sketch could be built in the Arduino IDE but I have not tested.
    
  ### FILE SYSTEM
    Make sure modify /data/userSettings.json with your wifi ssid and password.
    Upload the filesystem image and monitor the COM port to se the ip address.
