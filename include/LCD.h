#ifndef LCD_H
#define LCD_H

//**** LCD *****
#include <Wire.h>
#include <hd44780.h>                       // main hd44780 header
#include <hd44780ioClass/hd44780_I2Cexp.h> // i2c expander i/o class header

#define LCD_COLS 16
#define LCD_ROWS 2

int LCD_begin(uint8_t cols = LCD_COLS, uint8_t rows = LCD_ROWS);
void LCD_SetScrollbarChars();
void LCD_setCursor(int row, int col);
void LCD_clear();
void LCD_write(uint8_t c);
void LCD_createChar(uint8_t ch, const uint8_t map[]);
void print_LCD_line(const char *string, int row, int col);
void print_LCD_line(const uint8_t c, int row, int col);
void print_LCD_line(const __FlashStringHelper *string, int row, int col);

#endif // LCD_H