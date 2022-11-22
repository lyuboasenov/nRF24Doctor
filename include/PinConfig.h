#ifndef PIN_CONFIG_H
#define PIN_CONFIG_H

//**** CONNECTIONS *****
#define ENCODER_A_PIN       2		//Interrupt pin required for Encoder for optimal response
#define ENCODER_B_PIN       3		//Interrupt pin required for Encoder for optimal response
// Define either TRIGGER_PIN or LED_PIN, as they share a pin
//#define TRIGGER_PIN         4    	//Debugging purposes with scope
//PIN 9~13: NRF24 RADIO
#define MOSFET_2P2OHM_PIN   A2
#define MOSFET_100OHM_PIN   A3
#define BUTTON_PIN          4    	// physical pin , use internal pullup
#define CURRENT_PIN         A1
#define ADC_PIN_NR           5     	// A5, Match to CURRENT_PIN for configuring registers ADC

#endif // PIN_CONFIG_H