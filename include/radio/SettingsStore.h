#pragma once

//**** EEPROM STORAGE LOCATIONS *****
#define EEPROM_FLAG_MAGIC              0xA5u  // Indication contents are valid. Empty eeprom will contain 0xFF
#define EEPROM_FLAG                    0
#define EEPROM_CHANNEL                 1
#define EEPROM_PA_LEVEL                2
#define EEPROM_PA_LEVEL_GW             3
#define EEPROM_DATARATE                4
#define EEPROM_DESTINATION_NODE        5
#define EEPROM_PAYLOAD_SIZE            6
#define EEPROM_MESSAGE_RATE            7

void logRadioSettings();
void loadDefaults();
void loadEeprom();
void saveEeprom();
void invalidateEeprom();
void saveEepromAndReset();
void reset();

void loadChannelFromStore();
