#if !defined(GATEWAY)

#include <Menu.h>

#include <PinConfig.h>
#include "Radio.h"

//**** ENCODER & BUTTON  *****
//#define ENCODER_OPTIMIZE_INTERRUPTS //Only when using pin2/3 (or 20/21 on mega) - using this will screw up the other interrupt routine and we actually don't need it.
#include <Encoder.h>             // for Encoder      Download: https://github.com/PaulStoffregen/Encoder
#include <Bounce2.h>	            // button debounce  Download: https://github.com/thomasfredericks/Bounce2

static Encoder encoder(ENCODER_A_PIN, ENCODER_B_PIN);
static Bounce button = Bounce();

#include <Vars.h>
#include <LCD.h>

#include <ArrayUtils.h>

#include <StateMachine.h>
#include <CurrentMeasurement.h>

LCDMenuLib2_menu LCDML_0 (255, 0, 0, NULL, NULL); // root menu element (do not change)
LCDMenuLib2 LCDML(LCDML_0, LCD_ROWS, LCD_COLS, lcdml_menu_display, lcdml_menu_clear, lcdml_menu_control);

/*****************************************************************************/
/******************************* Attach menu callbacks ****************************/
/*****************************************************************************/
// add            (id   prev_layer      new_num                      lang_char_array     callback_function)
// addAdvanced    (id   prev_layer      new_num  condition           lang_char_array     callback_function    parameter (0-255)  menu function type )
//                                                                   "01234567890123"
LCDML_addAdvanced (0  , LCDML_0         , 1    , NULL              , "Statistics   >"  , menuPage           , PAGE_STATISTICS  , _LCDML_TYPE_default);
LCDML_addAdvanced (1  , LCDML_0         , 2    , NULL              , "Timing       >"  , menuPage           , PAGE_TIMING      , _LCDML_TYPE_default);
LCDML_addAdvanced (2  , LCDML_0         , 3    , NULL              , "Msg Rate     >"  , menuPage           , PAGE_MSGRATE     , _LCDML_TYPE_default);
LCDML_addAdvanced (3  , LCDML_0         , 4    , NULL              , "Counters     >"  , menuPage           , PAGE_COUNTERS    , _LCDML_TYPE_default);
LCDML_addAdvanced (4  , LCDML_0         , 5    , NULL              , "TxRx Power   >"  , menuPage           , PAGE_TXRXPOWER   , _LCDML_TYPE_default);
LCDML_addAdvanced (5  , LCDML_0         , 6    , NULL              , "Sleep Power  >"  , menuPage           , PAGE_SLEEPPOWER  , _LCDML_TYPE_default);
LCDML_add         (6  , LCDML_0         , 7                        , "Channel Scan >"  , NULL);
LCDML_addAdvanced (7  , LCDML_0_7     	, 1    , NULL              , "Run Scan     >"  , menuPage           , PAGE_SCANNER     , _LCDML_TYPE_default);
LCDML_addAdvanced (8  , LCDML_0_7     	, 2    , NULL              , ""                , menuCfgScanChStart	, 0                , _LCDML_TYPE_dynParam);
LCDML_addAdvanced (9  , LCDML_0_7     	, 3    , NULL              , ""                , menuCfgScanChStop	, 0                , _LCDML_TYPE_dynParam);
LCDML_add 		  (10 , LCDML_0_7       , 4                        , "Back         <"  , menuBack);
LCDML_add         (11 , LCDML_0         , 8                        , "Settings     >"  , NULL);
LCDML_add         (12 , LCDML_0_8       , 1                        , "Radio        >"  , NULL);
LCDML_addAdvanced (13 , LCDML_0_8_1     , 1    , NULL              , ""                , menuCfgChannel     , 0                , _LCDML_TYPE_dynParam);
LCDML_addAdvanced (14 , LCDML_0_8_1     , 2    , NULL              , ""                , menuCfgGwPa        , 0                , _LCDML_TYPE_dynParam);
LCDML_addAdvanced (15 , LCDML_0_8_1     , 3    , NULL              , ""                , menuCfgNodePa      , 0                , _LCDML_TYPE_dynParam);
LCDML_addAdvanced (16 , LCDML_0_8_1     , 4    , NULL              , ""                , menuCfgRate        , 0                , _LCDML_TYPE_dynParam);
LCDML_add         (17 , LCDML_0_8_1     , 5                        , "Back         <"  , menuBack);
LCDML_add         (18 , LCDML_0_8       , 2                        , "Doctor       >"  , NULL);
LCDML_addAdvanced (19 , LCDML_0_8_2     , 1    , NULL              , ""  			   , menuCfgPayload     , 0                , _LCDML_TYPE_dynParam);
LCDML_addAdvanced (20 , LCDML_0_8_2     , 2    , NULL              , ""  			   , menuCfgMsgRate     , 0                , _LCDML_TYPE_dynParam);
LCDML_addAdvanced (21 , LCDML_0_8_2     , 3    , NULL              , ""                , menuCfgDstNode     , 0                , _LCDML_TYPE_dynParam);
LCDML_add         (22 , LCDML_0_8_2     , 4                        , "Reset buff   x"  , menuResetBuf);
LCDML_add         (23 , LCDML_0_8_2     , 5                        , "Back         <"  , menuBack);
LCDML_add         (24 , LCDML_0_8       , 3                        , "Eeprom       >"  , NULL);
LCDML_add         (25 , LCDML_0_8_3     , 1                        , "Save node    x"  , menuSaveNodeEeprom);
LCDML_add         (26 , LCDML_0_8_3     , 2                        , "Save node&gw x"  , menuSaveNodeAndGwEeprom);
LCDML_add         (27 , LCDML_0_8_3     , 3                        , "Defaults nodex"  , menuDefaultNodeEeprom);
LCDML_add         (28 , LCDML_0_8_3     , 4                        , "Back         <"  , menuBack);
LCDML_add         (29 , LCDML_0_8       , 4                        , "Reset node   x"  , menuResetNode);
LCDML_add         (30 , LCDML_0_8       , 5                        , "Back         <"  , menuBack);

#define _LCDML_DISP_cnt 30  // Should equal last id in menu

LCDML_createMenu(_LCDML_DISP_cnt);

#if (_LCDML_DISP_rows > _LCDML_DISP_cfg_max_rows)
#error change value of _LCDML_DISP_cfg_max_rows in LCDMenuLib2.h
#endif

void menuSetup() {
   LCDML_setup(_LCDML_DISP_cnt);
}

void menuLoop() {
   LCDML.loop();
}

void menuScreenDisable() {
   LCDML.SCREEN_disable();
}

/*****************************************************************************/
/******************************* ENCODER & BUTTON ****************************/
/*****************************************************************************/
void lcdml_menu_control(void) {
   if (LCDML.BT_setup()) {
      // run once; init pins & debouncer
      pinMode(ENCODER_A_PIN, INPUT_PULLUP);
      pinMode(ENCODER_B_PIN, INPUT_PULLUP);
      pinMode(BUTTON_PIN, INPUT_PULLUP);
      button.attach(BUTTON_PIN);
      button.interval(5);  // interval in ms
   }

   // We're interested in relative encoder moves only, so 8bits position suffices.
   const int8_t enc = int8_t(encoder.read());
   button.update();
   const bool pressed = button.read() == LOW;
   static bool prevPressed = false;

   // Mechanical encoder generates 4 increments in 1 mechanical 'step'.
   const int8_t encStep = 4;
   for (;;) {
      static int8_t encPrev = 0;
      int8_t delta = enc - encPrev;
      if (delta <= -encStep) {
         LCDML.BT_down();
         encPrev -= encStep;
      } else if (delta >= encStep) {
         LCDML.BT_up();
         encPrev += encStep;
      } else {
         break;
      }
   }

   if (pressed and (not prevPressed)) {
      // Pressed and previously not pressed
      LCDML.BT_enter();
   }
   prevPressed = pressed;
}

void delay_with_update(unsigned long delay_ms) {
   unsigned long dTstart = millis();
   while ((millis() - dTstart) < delay_ms) {
      menuLoop();
   }
}

/*****************************************************************/
/************************* MENU HANDLERS *************************/
/*****************************************************************/
char *printBufCurrent(char *buf, int size, float curr) {
   if (CurrentValueErrCap == curr) {
      snprintf_P(buf, size, PSTR("Err cap"));
   } else if (CurrentValueWait == curr) {
      snprintf_P(buf, size, PSTR("Wait"));
   } else if (curr >= CurrentValueErr) {
      snprintf_P(buf, size, PSTR("Err"));
   } else {
      char scalePrefix = 'u';
      if (curr > 1000) {
         scalePrefix = 'm';
         curr /= 1000.0;
      }
      const uint16_t currInt = curr;
      const uint8_t currFrac = (curr - float(currInt)) * 100.0 + 0.5;
      snprintf_P(buf, size, PSTR("%" PRIu16 ".%02" PRIu8 " %cA"), currInt, currFrac, scalePrefix);
   }
   return buf;
}

void drawScannerChart(const uint8_t pointerCol) {
   const uint8_t numChannels = (iRf24ChannelScanStop - iRf24ChannelScanStart + 1);
   // Channel and step for each column (in fixed point 8.8)
   const uint16_t channelStep = (numChannels << 8) / COUNT_OF(channelScanBuckets);
   uint16_t channel = iRf24ChannelScanStart << 8;

   uint8_t col = 0;
   for (uint8_t i = 0; i < LCD_NUM_SPECIAL_CHARS; ++i) {
      uint8_t character[LCD_HEIGHT_SPECIAL_CHARS];
      (void)memset(character, 0, COUNT_OF(character));
      for (uint8_t mask = 1 << (LCD_WIDTH_SPECIAL_CHARS - 1); mask > 0; mask >>= 1) {
         uint8_t bucket = ((channel >> 8) - iRf24ChannelScanStart) * COUNT_OF(channelScanBuckets) / numChannels;
         bucket = CONSTRAIN_HI(bucket, COUNT_OF(channelScanBuckets) - 1);  // just to be sure...
         uint8_t v = channelScanBuckets[bucket];
         uint8_t lvl = CHANNEL_SCAN_BUCKET_MAX_VAL / 2;
         for (uint8_t h = 0; h < LCD_HEIGHT_SPECIAL_CHARS - 1; ++h) {
            if (v >= lvl) {
               character[h] |= mask;
            }
            lvl >>= 1;

            // Draw XOR'ed pointer at the top of the chart to indicate column
            if ((0 == h) and (col == pointerCol)) {
               character[h] ^= mask;
            }
         }
         channel += channelStep;
         ++col;
      }
      character[LCD_HEIGHT_SPECIAL_CHARS - 1] = 0xFF;
      LCD_createChar(i, character);
   }
}

void menuPage(uint8_t param) {
   if (LCDML.FUNC_setup()) {
      LCDML.FUNC_setLoopInterval(200);  // starts a trigger event for the loop function every 200 millisecounds
   }

   if (LCDML.FUNC_loop()) {
      LCD_clear();
      char buf[LCD_COLS + 1];
      bool exit = LCDML.BT_checkAny();  // check if any button is pressed (enter, up, down, left, right)

      // Not all pages require GW connection to be active
      const bool gwRequired = not((page(param) == PAGE_SLEEPPOWER) or (page(param) == PAGE_SCANNER));

      if (transportHwError) {
         print_LCD_line("Radio init error", 0, 0);
         print_LCD_line("Replace radio", 1, 0);
      } else if (gwRequired and not isRadioReady()) {
         print_LCD_line("Search Gateway..", 0, 0);
      } else {
         switch (page(param)) {
            case PAGE_STATISTICS:
               snprintf_P(buf, sizeof(buf), PSTR("P%-3dFAIL%4d%3d%%"), MY_PARENT_NODE_ID, iNrFailedMessages, GetNumBitsSetInArray(bArrayFailedMessages, COUNT_OF(bArrayFailedMessages)));
               print_LCD_line(buf, 0, 0);
               snprintf_P(buf, sizeof(buf), PSTR("D%-3dNACK%4d%3d%%"), iDestinationNode, iNrNAckMessages, GetNumBitsSetInArray(bArrayNAckMessages, COUNT_OF(bArrayNAckMessages)));
               print_LCD_line(buf, 1, 0);
               break;

            case PAGE_TIMING:
               if (iMaxDelayFirstHop_ms > 9999) {
                  snprintf_P(buf, sizeof(buf), PSTR("HOP1 dTmax   INF"));
               } else {
                  snprintf_P(buf, sizeof(buf), PSTR("HOP1 dTmax%4dms"), iMaxDelayFirstHop_ms);
               }
               print_LCD_line(buf, 0, 0);
               if (iMaxDelayDestination_ms > 9999) {
                  snprintf_P(buf, sizeof(buf), PSTR("D%-3d dTmax   INF"), iDestinationNode, iMaxDelayDestination_ms);
               } else {
                  snprintf_P(buf, sizeof(buf), PSTR("D%-3d dTmax%4dms"), iDestinationNode, iMaxDelayDestination_ms);
               }
               print_LCD_line(buf, 1, 0);
               break;

            case PAGE_MSGRATE:
               snprintf_P(buf, sizeof(buf), PSTR("MSG/SEC     %3d"), iGetMsgRate);
               print_LCD_line(buf, 0, 0);
               snprintf_P(buf, sizeof(buf), PSTR("ARC Avg%2d Max%2d"), iArcCntAvg, iArcCntMax);
               print_LCD_line(buf, 1, 0);
               break;

            case PAGE_COUNTERS:
               snprintf_P(buf, sizeof(buf), PSTR("MESSAGE COUNT:  "));
               print_LCD_line(buf, 0, 0);
               snprintf_P(buf, sizeof(buf), PSTR("           %5d"), iMessageCounter);
               print_LCD_line(buf, 1, 0);
               break;

            case PAGE_TXRXPOWER: {
               char buf1[LCD_COLS + 1];
               snprintf_P(buf, sizeof(buf), PSTR("Tx %s"), printBufCurrent(buf1, sizeof(buf1), TransmitCurrent_uA));
               print_LCD_line(buf, 0, 0);
               snprintf_P(buf, sizeof(buf), PSTR("Rx %s"), printBufCurrent(buf1, sizeof(buf1), ReceiveCurrent_uA));
               print_LCD_line(buf, 1, 0);
            } break;

            case PAGE_SLEEPPOWER: {
               currState = STATE_SLEEP;
               char buf1[LCD_COLS + 1];
               snprintf_P(buf, sizeof(buf), PSTR("Sleep %s"), printBufCurrent(buf1, sizeof(buf1), SleepCurrent_uA));
               print_LCD_line(buf, 0, 0);
            } break;

            case PAGE_SCANNER: {
               // Scanner only exits on button press, as rotation is used to navigate channels.
               exit = LCDML.BT_checkEnter();

               bChannelScanner = not exit;

               // Update position of pointer when encoder is rotated
               if (LCDML.BT_checkUp()) {
                  ++iRf24ChannelScanColDisplayed;
               }
               if (LCDML.BT_checkDown() and (iRf24ChannelScanColDisplayed > 0)) {
                  --iRf24ChannelScanColDisplayed;
               }
               iRf24ChannelScanColDisplayed = CONSTRAIN_HI(iRf24ChannelScanColDisplayed, (LCD_WIDTH_SPECIAL_CHARS * LCD_NUM_SPECIAL_CHARS - 1));

               // -- 1st line of display
               snprintf_P(buf, sizeof(buf), PSTR("%3" PRIu8 "["), iRf24ChannelScanStart);
               print_LCD_line(buf, 0, 0);

               for (uint8_t i = 0; i < LCD_NUM_SPECIAL_CHARS; ++i) {
                  LCD_write(i);
               }
               snprintf_P(buf, sizeof(buf), PSTR("]%-3" PRIu8), iRf24ChannelScanStop);
               print_LCD_line(buf, 0, 4 + LCD_NUM_SPECIAL_CHARS);

               // -- 2nd line of display
               // Calculate nRF channel indicated by pointer
               const uint8_t chan = iRf24ChannelScanStart + ((iRf24ChannelScanStop - iRf24ChannelScanStart) * iRf24ChannelScanColDisplayed + (LCD_WIDTH_SPECIAL_CHARS * LCD_NUM_SPECIAL_CHARS / 2)) / (LCD_WIDTH_SPECIAL_CHARS * LCD_NUM_SPECIAL_CHARS - 1);
               snprintf_P(buf, sizeof(buf), PSTR("CH%-03" PRIu8 " 2.%03" PRIu16 "G"), chan, 400 + chan);
               print_LCD_line(buf, 1, 0);
               // Show wifi channel
               const uint8_t wifi = (chan - 5) / 5;
               if ((wifi > 0) and (wifi <= 13)) {
                  snprintf_P(buf, sizeof(buf), PSTR("W%-2" PRIu8), wifi);
                  print_LCD_line(buf, 1, LCD_COLS - 3);
               }

               // -- Program the special characters to display the chart
               drawScannerChart(iRf24ChannelScanColDisplayed);
            } break;

            default:
               break;
         }
      }

      if (exit) {
         LCDML.FUNC_goBackToMenu();  // leave this function
         // Restore special characters if they got overwritten
         LCD_SetScrollbarChars();
      }
   }
}

void menuCfgEntry(uint8_t &value) {
   // make only an action when the cursor stands on this menuitem
   // check Button
   if (LCDML.BT_checkAny()) {
      if (LCDML.BT_checkEnter()) {
         // this function checks returns the scroll disable status (0 = menu scrolling enabled, 1 = menu scrolling disabled)
         if (LCDML.MENU_getScrollDisableStatus() == 0) {
            // disable the menu scroll function to catch the cursor on this point
            // now it is possible to work with BT_checkUp and BT_checkDown in this function
            // this function can only be called in a menu, not in a menu function
            LCDML.MENU_disScroll();
         } else {
            // enable the normal menu scroll function
            LCDML.MENU_enScroll();
         }
         // dosomething for example save the data or something else
         LCDML.BT_resetEnter();
      }
   }
   if ((value < 255) and LCDML.BT_checkUp()) value++;
   if ((value > 0) and LCDML.BT_checkDown()) value--;
}

void menuCfgScanChStart(uint8_t line) {
   if (line == LCDML.MENU_getCursorPos()) {
      menuCfgEntry(iRf24ChannelScanStart);
      iRf24ChannelScanStart = CONSTRAIN_HI(iRf24ChannelScanStart, iRf24ChannelScanStop);
   }

   char buf[LCD_COLS + 1];
   snprintf_P(buf, sizeof(buf), PSTR("Start Ch. %3d"), iRf24ChannelScanStart);

   // use the line from function parameters
   print_LCD_line(buf, line, 1);
}

void menuCfgScanChStop(uint8_t line) {
   if (line == LCDML.MENU_getCursorPos()) {
      menuCfgEntry(iRf24ChannelScanStop);
      iRf24ChannelScanStop = constrain(iRf24ChannelScanStop, iRf24ChannelScanStart, NRF24_MAX_CHANNEL);
   }

   char buf[LCD_COLS + 1];
   snprintf_P(buf, sizeof(buf), PSTR("Stop  Ch. %3d"), iRf24ChannelScanStop);

   // use the line from function parameters
   print_LCD_line(buf, line, 1);
}

void menuCfgPayload(uint8_t line) {
   if (line == LCDML.MENU_getCursorPos()) {
      menuCfgEntry(iPayloadSize);
      iPayloadSize = constrain(iPayloadSize, iMinPayloadSize, iMaxPayloadSize);
   }

   char buf[LCD_COLS + 1];
   snprintf_P(buf, sizeof(buf), PSTR("Payload    %2d"), iPayloadSize);

   // use the line from function parameters
   print_LCD_line(buf, line, 1);
}

void menuCfgMsgRate(uint8_t line) {
   if (line == LCDML.MENU_getCursorPos()) {
      menuCfgEntry(iSetMsgRate);
   }

   char buf[LCD_COLS + 1];
   snprintf_P(buf, sizeof(buf), PSTR("Msg Rate  %3d"), iSetMsgRate);

   // use the line from function parameters
   print_LCD_line(buf, line, 1);
}

void menuCfgChannel(uint8_t line) {
   if (line == LCDML.MENU_getCursorPos()) {
      menuCfgEntry(iRf24Channel);
      iRf24Channel = CONSTRAIN_HI(iRf24Channel, NRF24_MAX_CHANNEL);
   }

   char buf[LCD_COLS + 1];
   snprintf_P(buf, sizeof(buf), PSTR("Channel   %3d"), iRf24Channel);

   // use the line from function parameters
   print_LCD_line(buf, line, 1);
}

void menuCfgDstNode(uint8_t line) {
   if (line == LCDML.MENU_getCursorPos()) {
      menuCfgEntry(iDestinationNode);
   }

   char buf[LCD_COLS + 1];
   snprintf_P(buf, sizeof(buf), PSTR("Dest Node %3d"), iDestinationNode);

   // use the line from function parameters
   print_LCD_line(buf, line, 1);
}

void menuCfgNodePa(uint8_t line) {
   if (line == LCDML.MENU_getCursorPos()) {
      menuCfgEntry(iRf24PaLevel);
      iRf24PaLevel = rf24PaLevelConstrain(iRf24PaLevel);
   }

   char buf[LCD_COLS + 1];
   snprintf_P(buf, sizeof(buf), PSTR("Node PA  %-4s"), rf24PaLevelToString(iRf24PaLevel));

   // use the line from function parameters
   print_LCD_line(buf, line, 1);
}

void menuCfgGwPa(uint8_t line) {
   if (line == LCDML.MENU_getCursorPos()) {
      menuCfgEntry(iRf24PaLevelGw);
      iRf24PaLevelGw = rf24PaLevelConstrain(iRf24PaLevelGw);
   }

   char buf[LCD_COLS + 1];
   snprintf_P(buf, sizeof(buf), PSTR("GW PA    %-4s"), rf24PaLevelToString(iRf24PaLevelGw));

   // use the line from function parameters
   print_LCD_line(buf, line, 1);
}

void menuCfgRate(uint8_t line) {
   if (line == LCDML.MENU_getCursorPos()) {
      menuCfgEntry(iRf24DataRate);
      iRf24DataRate = rf24DataRateConstrain(iRf24DataRate);
   }

   char buf[LCD_COLS + 1];
   snprintf_P(buf, sizeof(buf), PSTR("Datarate %-4s"), rf24DataRateToString(iRf24DataRate));

   // use the line from function parameters
   print_LCD_line(buf, line, 1);
}

void menuSaveNodeEeprom(__attribute__((unused)) uint8_t param) {
   if (LCDML.FUNC_setup()) {
      LCD_clear();
      print_LCD_line(F("Eeprom saved"), 0, 0);
      print_LCD_line(F("Restarting..."), 1, 0);
      delay(restartDelayMs);
      saveEepromAndReset();
      // Never return here...
   }
}

void menuDefaultNodeEeprom(__attribute__((unused)) uint8_t param) {
   if (LCDML.FUNC_setup()) {
      LCD_clear();
      print_LCD_line(F("Defaults saved"), 0, 0);
      print_LCD_line(F("Restarting..."), 1, 0);
      delay(restartDelayMs);
      loadDefaults();
      saveEepromAndReset();
      // Never return here...
   }
}

void menuSaveNodeAndGwEeprom(__attribute__((unused)) uint8_t param) {
   if (LCDML.FUNC_setup()) {
      // Trigger the gateway update sequence
      bUpdateGateway = true;
      LCD_clear();
      LCDML.FUNC_setLoopInterval(100);
   }

   if (LCDML.FUNC_loop()) {
      static bool prevUpdateGateway = true;
      if (not bUpdateGateway) {
         // Gateway update finished with error
         if (prevUpdateGateway != bUpdateGateway) {
            // Print message only once
            print_LCD_line(F("Failed"), 0, 0);
         }
         if (LCDML.BT_checkAny())  // check if any button is pressed (enter, up, down, left, right)
         {
            LCDML.FUNC_goBackToMenu();  // leave this function
         }
      }
      prevUpdateGateway = bUpdateGateway;
   }
}

void menuResetBuf(__attribute__((unused)) uint8_t param) {
   if (LCDML.FUNC_setup()) {
      ClearStorageAndCounters();
      LCD_clear();
      print_LCD_line(F("Cleared"), 0, 0);
   }
   if (LCDML.FUNC_loop()) {
      if (LCDML.BT_checkAny())  // check if any button is pressed (enter, up, down, left, right)
      {
         LCDML.FUNC_goBackToMenu();  // leave this function
      }
   }
}

void menuResetNode(__attribute__((unused)) uint8_t param) {
   if (LCDML.FUNC_setup()) {
      LCD_clear();
      print_LCD_line(F("Restarting..."), 0, 0);
      delay(restartDelayMs);
      reset();
      // Never return here...
   }
}

void menuBack(__attribute__((unused)) uint8_t param) {
   if (LCDML.FUNC_setup()) {
      // Go one level up
      LCDML.FUNC_goBackToMenu(1);
   }
}

void lcdml_menu_clear() {
   LCD_clear();
   LCD_setCursor(0, 0);
}

void lcdml_menu_display() {
   // update content
   if (LCDML.DISP_checkMenuUpdate()) {
      // clear menu
      LCDML.DISP_clear();

      // declaration of some variables
      // content variable
      char content_text[_LCDML_DISP_cols];  // save the content text of every menu element
      // menu element object
      LCDMenuLib2_menu *tmp;
      // some limit values
      uint8_t i = LCDML.MENU_getScroll();
      uint8_t maxi = _LCDML_DISP_rows + i;
      uint8_t n = 0;

      // check if this element has children
      tmp = LCDML.MENU_getObj()->getChild(LCDML.MENU_getScroll());
      if (tmp) {
         // loop to display lines
         do {
            // check if a menu element has a condetion and if the condetion be true
            if (tmp->checkCondition()) {
               // check the type of a menu element
               if (tmp->checkType_menu() == true) {
                  // display normal content
                  LCDML_getContent(content_text, tmp->getID());
                  print_LCD_line(content_text, n, 1);
               } else {
                  if (tmp->checkType_dynParam()) {
                     tmp->callback(n);
                  }
               }
               // increment some values
               i++;
               n++;
            }
            // try to go to the next sibling and check the number of displayed rows
         } while (((tmp = tmp->getSibling(1)) != NULL) && (i < maxi));
      }
   }

   if (LCDML.DISP_checkMenuCursorUpdate()) {
      uint8_t n_max = (LCDML.MENU_getChilds() >= _LCDML_DISP_rows) ? _LCDML_DISP_rows : (LCDML.MENU_getChilds());
      uint8_t scrollbar_min = 0;
      uint8_t scrollbar_max = LCDML.MENU_getChilds();
      uint8_t scrollbar_cur_pos = LCDML.MENU_getCursorPosAbs();
      uint8_t scroll_pos = ((1. * n_max * _LCDML_DISP_rows) / (scrollbar_max - 1) * scrollbar_cur_pos);

      // display rows
      for (uint8_t n = 0; n < n_max; n++) {
         LCD_setCursor(n, 0);

         // set cursor char
         if (n == LCDML.MENU_getCursorPos()) {
            LCD_write(_LCDML_DISP_cfg_cursor);
         } else {
            LCD_write(' ');
         }

         // delete or reset scrollbar
         if (_LCDML_DISP_cfg_scrollbar == 1) {
            if (scrollbar_max > n_max) {
               print_LCD_line((uint8_t)0, n, (_LCDML_DISP_cols - 1));
            } else {
               print_LCD_line((uint8_t)' ', n, (_LCDML_DISP_cols - 1));
            }
         }
      }

      // display scrollbar
      if (_LCDML_DISP_cfg_scrollbar == 1) {
         if (scrollbar_max > n_max) {
            // set scroll position
            if (scrollbar_cur_pos == scrollbar_min) {
               // min pos
               print_LCD_line((uint8_t)1, 0, (_LCDML_DISP_cols - 1));
            } else if (scrollbar_cur_pos == (scrollbar_max - 1)) {
               // max pos
               print_LCD_line((uint8_t)4, (n_max - 1), (_LCDML_DISP_cols - 1));
            } else {
               // between
               print_LCD_line((uint8_t)(scroll_pos % n_max) + 1, scroll_pos / n_max, (_LCDML_DISP_cols - 1));
            }
         }
      }
   }
}

#endif // !defined(MY_GATEWAY_FEATURE)
