# TCodeESP32
This is an ESP32 fork of Tempests TCode sketch (repository doesnt exists yet). It adds Network capabilities and bluetooth.

## INSTALLATION:
  ### BINARIES
  See the pdf in the latest release zip https://github.com/jcfain/TCodeESP32/releases
  ### SOURCE
  First You need to install the dev environment for the ESP32 and arduino.
  A good place to start https://github.com/espressif/arduino-esp32#installation-instructions
  #### LIBRARIES 
    You will need a few additional libraries to build this sketch.
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
    I believe the sketch could be built in the Arduino IDE but I have not tested.
    
  #### FILE SYSTEM
    Modify /data/userSettings.json with your wifi ssid and password.
    Upload the filesystem image and monitor the COM port to se the ip address if dynamic.
    
    You can also set the wifi settings via the web interface in AP mode on first boot.
    Or you can use the Android configurator app I wrote found here:
    https://www.patreon.com/posts/tcode-android-51962566
