#include "Adafruit_SSD1306_RSB.h"

Adafruit_SSD1306_RSB::Adafruit_SSD1306_RSB(uint8_t w, uint8_t h, TwoWire *twi,
				int8_t rst_pin, uint32_t clkDuring,
				uint32_t clkAfter) 
				: Adafruit_SSD1306(w, h, twi, rst_pin, clkDuring, clkAfter) {}
				
// Vertical scroll (like TV credits, and in reverse too)
void Adafruit_SSD1306_RSB::startvertscroll(uint8_t start, uint8_t stop, bool scrollup=true) {
	// Must stop any current scrolling to avoid RAM corruption
	stopscroll();

	// We want to scroll the entire screen.
	ssd1306_command(SSD1306_SET_VERTICAL_SCROLL_AREA); //A3
	ssd1306_command(0X00);
	ssd1306_command(32);

	// Now tell it we are doing a scroll command
	ssd1306_command(SSD1306_VERTICAL_AND_LEFT_HORIZONTAL_SCROLL);
	ssd1306_command(0X00); 	//A dummy value, not used

	// NB If start is not 4 (or greater) weird things happen. You have been warned!
	ssd1306_command(start);		//B : start - controls horizontal movement

	//	Set time interval between each scroll step in
	//	terms of frame frequency
	//	000b  5 frames 100b  3 frames
	//	001b  64 frames 101b  4 frames
	//	010b  128 frames 110b  25 frame
	//	011b  256 frames 111b  2 frame
	ssd1306_command(0x5);	//C interval

	// If the stop value is not 1F weird things happend. You have been warned
	ssd1306_command(stop);	//D ditto: end

	ssd1306_command(scrollup ? 0x01 : 0x1F);	//E controls vertical movement, up or down.
	ssd1306_command(SSD1306_ACTIVATE_SCROLL);
}