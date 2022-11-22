#include <LCD.h>

const uint8_t i2cAddr = 0x27;
hd44780_I2Cexp lcd;

const uint8_t scroll_bar[][8] = {
	{B10001, B10001, B10001, B10001, B10001, B10001, B10001, B10001}, // scrollbar top
	{B11111, B11111, B10001, B10001, B10001, B10001, B10001, B10001}, // scroll state 1
	{B10001, B10001, B11111, B11111, B10001, B10001, B10001, B10001}, // scroll state 2
	{B10001, B10001, B10001, B10001, B11111, B11111, B10001, B10001}, // scroll state 3
	{B10001, B10001, B10001, B10001, B10001, B10001, B11111, B11111}  // scrollbar bottom
};

/*****************************************************************/
/************************* LCD FUNCTIONS *************************/
/*****************************************************************/

int LCD_begin(uint8_t cols, uint8_t rows) {
	return lcd.begin(cols, rows);
}

void print_LCD_line(const uint8_t c, int row, int col) {
	lcd.setCursor(col,row);
	lcd.write(c);
}

void print_LCD_line(const char *string, int row, int col) {
	lcd.setCursor(col,row);
	lcd.print(string);
}

void print_LCD_line(const __FlashStringHelper *string, int row, int col) {
	lcd.setCursor(col,row);
	lcd.print(string);
}

void LCD_clear() {
	lcd.clear();
}

void LCD_setCursor(int row, int col) {
   lcd.setCursor(col,row);
}

void LCD_write(uint8_t c) {
   lcd.write(c);
}

void LCD_createChar(uint8_t ch, const uint8_t map[]) {
   lcd.createChar(ch, map);
}

void LCD_SetScrollbarChars()
{
	// set special chars for scrollbar
	lcd.createChar(0, (uint8_t*)scroll_bar[0]);
	lcd.createChar(1, (uint8_t*)scroll_bar[1]);
	lcd.createChar(2, (uint8_t*)scroll_bar[2]);
	lcd.createChar(3, (uint8_t*)scroll_bar[3]);
	lcd.createChar(4, (uint8_t*)scroll_bar[4]);
}
