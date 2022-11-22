#include <nRF24DoctorNode.h>

/*****************************************************************************/
/************************************ STARTUP ********************************/
/*****************************************************************************/
void before() {						//Initialization before the MySensors library starts up
	pinMode(CURRENT_PIN, INPUT);	//Analog Input for Current Usage Pin
#ifdef TRIGGER_PIN
	pinMode(TRIGGER_PIN, OUTPUT);
	digitalWrite(TRIGGER_PIN,LOW);
#endif
#ifdef LED_PIN
	pinMode(LED_PIN, OUTPUT);
	digitalWrite(LED_PIN,LOW);
#endif
	pinMode(MOSFET_2P2OHM_PIN,OUTPUT);
	pinMode(MOSFET_100OHM_PIN,OUTPUT);
	digitalWrite(MOSFET_2P2OHM_PIN,HIGH);

	ADCSetup();

	//****  LCD *****
	//  Wire.begin();  // I2C
	void LCD_clear();
	LCD_begin();
	LCD_SetScrollbarChars();

	// Load radio settings from eeprom
	loadEeprom();
	logRadioSettings();
}

void setup() {
  int status;
  status = LCD_begin();
  if(status) // non zero status means it was unsuccesful
  {
    // hd44780 has a fatalError() routine that blinks an led if possible
    // begin() failed so blink error code using the onboard LED if possible
    hd44780::fatalError(status); // does not return
  }


	ClearStorageAndCounters();
//	activateRadioSettings();
//	logRadioSettings();
	Sprintln(F("Connecting..."));

	// Splash screen
	LCD_clear();
	print_LCD_line("nRF24 DOCTOR " SKETCH_VERSION_STRING,  0, 0);
	print_LCD_line("Connecting...", 1, 0);
	// Show splash screen for a short while, trying to connect to GW.
	// If GW connection has not been established after this delay the
	// node continues trying to connect.
	transportWaitUntilReady(2000);

	//**** MENU *****
	menuSetup();
	menuScreenDisable();
}

void presentation() {
	sendSketchInfo(F(SKETCH_NAME_STRING), F(SKETCH_VERSION_STRING));
	present(CHILD_ID_COUNTER, S_CUSTOM) ;  // "CUSTOM" counter
}

/*****************************************************************************/
/*********************************** MAIN LOOP *******************************/
/*****************************************************************************/
void loop()
{
	menuLoop();
	stateMachine();
}

// void MY_RF24_stopListening()
// {
// 	RF24_DEBUG(PSTR("RF24:SPL\n"));	// stop listening
// 	RF24_ce(LOW);
// 	// timing
// 	delayMicroseconds(130);
// 	RF24_setRFConfiguration(RF24_CONFIGURATION | _BV(RF24_PWR_UP) );
// 	// timing
// 	delayMicroseconds(100);
// }