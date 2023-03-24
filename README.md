# TCodeESP32
This is an ESP32 fork of Tempests TCode sketch (repository doesnt exists yet). It adds Network Tcode capabilities, Modcase temperature control, internal temperature control, OLED display and WiFi/BLE configuration.

## INSTALLATION:
  ### FROM BINARIES (RECOMMENDED for most users)
  See the pdf in the latest release zip for flashing instructions 
  https://github.com/jcfain/TCodeESP32/releases
  ### FROM SOURCE
  First You need to install the dev environment for the ESP32 and arduino.
  Some good placees to start 
  - https://platformio.org/install/ide?install=vscode 
  - https://docs.platformio.org/en/latest//integration/ide/vscode.html#quick-start
  - https://github.com/espressif/arduino-esp32#installation-instructions
  #### LIBRARIES 
    Depending on the build you select, you will need a few additional libraries to build this sketch. 
    If platformIO is installed with the correct directory selected (platformio.ini), it should download these automatically on first build.
    - ESP32Servo
    - ESP Async Webserver
    - EAsyncTCP
    - ArduinoJson
    - paulstoffregen/OneWire@^2.3.5
    - milesburton/DallasTemperature@^3.9.1
    - adafruit/Adafruit SSD1306@^2.4.3
    - Adafruit GFX Library
    - Adafruit BusIO
    - SPI
  
  #### TOOLS
    I used Visual studio code and PlatformIO for this project.
    I believe the sketch could be built in the Arduino IDE with the correct libraries but I have not tested.
    
  #### FILE SYSTEM
    Modify /data/userSettingsDefault.json with your wifi ssid and password etc.
    Flash the code and upload the filesystem image. Then start the monitor on the COM port to see the ip address if dynamic.
    
    You can also set the wifi settings via the web interface in AP mode on first boot.
    Or you can use the Android configurator app I wrote found here:
    https://www.patreon.com/posts/tcodeconfig-0-5a-72658342
