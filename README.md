# TCodeESP32
This is a fork of Tempests TCode sketch. 
It adds the following features: 
- Network TCode capabilities
- Motion generator
- Modcase temperature control
- Internal temperature control
- OLED display support 
- WiFi/Serial configuration.
- Voice control
- Battery gas guage
- Custom device commands
- Boot button command
- Bug fixes and more

![Settings configuration](/images/main_page.jpg)
![Flashing binaries](/images/flashing.jpg)
![Serial tester](/images/serial_test_device.jpg)
![Built in motion generator](/images/motion_generator.jpg)
![Voice support (Requires additional hardware)](/images/voice.jpg)

## INSTALLATION:
  ### FROM BINARIES (RECOMMENDED for most users)
  See the pdf in the latest release zip for flashing instructions 
  https://github.com/jcfain/TCodeESP32/releases
  ### FROM SOURCE
  First You need to install the dev environment for the ESP32 and arduino.
  It's actually easier than you may think.
  1. Install VSCode
  2. Install the platformIO extension in VSCode
  3. Point VSCode to the directory containing platformio.ini
  4. Select the build you want from the bottom toolbar
  5. Select Upload button on the bottom toolbar. (Has an arrow pointing right)
  6. On the left side of VSCode, there should be an alien head. Select it and execute "Upload Filesystem Image".
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
    Modify /SettingsHandler.h with your wifi ssid and password etc. (NOT RECOMMENDED)
    Flash the code and upload the filesystem image. Then start the monitor on the COM port to see the ip address if dynamic.
    
    You can also set the wifi settings via the web interface in AP mode or via the commands via serial monitoe on first boot.
