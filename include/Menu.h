#ifndef MENU_H
#define MENU_H

//**** ENCODER & BUTTON  *****
//#define ENCODER_OPTIMIZE_INTERRUPTS //Only when using pin2/3 (or 20/21 on mega) - using this will screw up the other interrupt routine and we actually don't need it.
#include <Encoder.h>    // for Encoder      Download: https://github.com/PaulStoffregen/Encoder
#include <Bounce2.h>	// button debounce  Download: https://github.com/thomasfredericks/Bounce2

#include <PinConfig.h>
#include <Vars.h>
#include <ArrayUtils.h>

#include <LCDMenuLib2.h>	// Download 1.2.7: https://github.com/Jomelo/LCDMenuLib2

#include <LCD.h>
#include <StateMachine.h>
#include <CurrentMeasurement.h>

//**** LCD Menu *****
#define _LCDML_DISP_cols             LCD_COLS
#define _LCDML_DISP_rows             LCD_ROWS
#define _LCDML_DISP_cfg_scrollbar    1      // enable a scrollbar
#define _LCDML_DISP_cfg_cursor       0x7E   // cursor Symbol

extern LCDMenuLib2_menu LCDML_0;
extern LCDMenuLib2 LCDML;

void menuSetup();
void menuLoop();
void menuScreenDisable();

void lcdml_menu_display();
void lcdml_menu_clear();
void lcdml_menu_control();
void delay_with_update(unsigned long delay_ms);
void menuPage(uint8_t param);
void menuCfgScanChStart(uint8_t line);
void menuCfgScanChStop(uint8_t line);
void menuBack(__attribute__((unused)) uint8_t param);
void menuCfgChannel(uint8_t line);
void menuCfgGwPa(uint8_t line);
void menuCfgPayload(uint8_t line);
void menuResetBuf(__attribute__((unused)) uint8_t param);
void menuCfgNodePa(uint8_t line);
void menuCfgMsgRate(uint8_t line);
void menuSaveNodeEeprom(__attribute__((unused)) uint8_t param);
void menuSaveNodeAndGwEeprom(__attribute__((unused)) uint8_t param);
void menuCfgRate(uint8_t line);
void menuCfgDstNode(uint8_t line);
void menuDefaultNodeEeprom(__attribute__((unused)) uint8_t param);
void menuResetNode(__attribute__((unused)) uint8_t param);

enum page { PAGE_STATISTICS, PAGE_TIMING, PAGE_MSGRATE, PAGE_COUNTERS, PAGE_TXRXPOWER, PAGE_SLEEPPOWER, PAGE_SCANNER };

#endif // MENU_H