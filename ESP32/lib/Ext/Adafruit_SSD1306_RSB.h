	
#include <Adafruit_SSD1306.h>
//https://github.com/RalphBacon/SSD1306-TFT-Screen-I2C-SPI/blob/master/Adafruit_SSD1306_RSB/Adafruit_SSD1306.h
//https://www.youtube.com/watch?v=QFubFhrEfmQ
class Adafruit_SSD1306_RSB : public Adafruit_SSD1306
{
	public:
	Adafruit_SSD1306_RSB(uint8_t w, uint8_t h, TwoWire *twi = &Wire,
                   int8_t rst_pin = -1, uint32_t clkDuring = 400000UL,
                   uint32_t clkAfter = 100000UL);
				   
  	void startvertscroll(uint8_t start, uint8_t stop, bool scrollup); //RSB
};