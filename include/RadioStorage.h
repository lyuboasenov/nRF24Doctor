#ifndef RADIO_STORAGE_H
#define RADIO_STORAGE_H

#include <stdint.h>
#include <Radio.h>

#define DEFAULT_DESTINATION_NODE			(0)				// Default 0 = gateway, Settable in Menu
#define DEFAULT_PAYLOAD_SIZE				(2)				// 2 Bytes is the minimum for the Counter data
#define DEFAULT_MESSAGE_RATE 				(10)

//**** EEPROM STORAGE LOCATIONS *****
#define EEPROM_FLAG_MAGIC				   0xA5u	// Indication contents are valid. Empty eeprom will contain 0xFF
#define EEPROM_FLAG						   0
#define EEPROM_CHANNEL					   1
#define EEPROM_PA_LEVEL					   2
#define EEPROM_PA_LEVEL_GW				   3
#define EEPROM_DATARATE					   4
#define EEPROM_DESTINATION_NODE			5
#define EEPROM_PAYLOAD_SIZE				6
#define EEPROM_MESSAGE_RATE				7

#ifndef MY_GATEWAY_FEATURE
// Node-only parameters
extern uint8_t iRf24PaLevelGw;		//PA Level for the Gateway
extern uint8_t iPayloadSize;
extern uint8_t iSetMsgRate;
extern uint8_t iDestinationNode;
#endif

void logRadioSettings();
void loadDefaults();
void loadEeprom();
void saveEeprom();
void invalidateEeprom();
void saveEepromAndReset();
void reset();

#ifndef MY_GATEWAY_FEATURE
void serializeGwSettings( MyMessage& msg );
#endif

#ifdef MY_GATEWAY_FEATURE
void deserializeGwSettings(const MyMessage& msg );
#endif

#endif