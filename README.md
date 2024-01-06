# TCodeESP32
This is an ESP32 fork of Tempests TCode sketch (repository doesnt exists yet). It adds Network Tcode capabilities, Modcase temperature control, internal temperature control, OLED display and WiFi/BLE configuration.

## INSTALLATION:
  ### FROM BINARIES (RECOMMENDED for most users)
  See the pdf in the latest release zip for flashing instructions 
  https://github.com/jcfain/TCodeESP32/releases
  ### FROM SOURCE
  First You need to install the dev environment for the ESP32 and arduino.
  It actually easier than you may think.
  1. Install VSCode
  2. Install the platformIO extension in VSCode
  3. Point VSCode to the directory containing platformio.ini
  4. Select the build you want from the bottom toolbar
  5. Select Upload button on the bottom toolbat. Has an arror pointing right.
  6. On the left side of VSCode there should be an alien head. Select it and find Upload file system.
  7. Profit
  For more infor here are some good placees:
  - https://platformio.org/install/ide?install=vscode 
  - https://docs.platformio.org/en/latest//integration/ide/vscode.html#quick-start
  - https://github.com/espressif/arduino-esp32#installation-instructions
  #### LIBRARIES 
    Depending on the build you select, you will need a few additional libraries to build this sketch. 
    If platformIO is installed with the correct directory selected (platformio.ini),
    these should download automatically on first build.
    - ESP32Servo
    - ESP Async Webserver
    - EAsyncTCP
    - ArduinoJson
    - paulstoffregen/OneWire
    - milesburton/DallasTemperature@
    - adafruit/Adafruit SSD1306
    - Adafruit GFX Library
    - Adafruit BusIO
    - SPI
  
  #### TOOLS
    I used Visual studio code and PlatformIO for this project.
    I believe the sketch could be built in the Arduino IDE with the correct libraries but I have not tested.
    
  #### FILE SYSTEM
    Modify /SettingsHandler.h with your wifi ssid and password etc.
    Flash the code and upload the filesystem image. Then start the monitor on the COM port to see the ip address if dynamic.
    
    You can also set the wifi settings via the web interface in AP mode or via the commands via serial monitoe on first boot.
