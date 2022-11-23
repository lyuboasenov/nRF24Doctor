#include <Vars.h>

/**** Remote Gateway Update ****/
//uint8_t iRetryGateway 	= 0;
bool bUpdateGateway = false;
uint8_t updateGatewayAttemptsRemaining;

/**** RPD Channel Scanner ****/
uint8_t iRf24ChannelScanStart = 0;
uint8_t iRf24ChannelScanStop  = NRF24_MAX_CHANNEL;
uint8_t iRf24ChannelScanCurrent = 0;
uint8_t iRf24ChannelScanColDisplayed = LCD_WIDTH_SPECIAL_CHARS * LCD_NUM_SPECIAL_CHARS / 2;
uint8_t channelScanBuckets[LCD_WIDTH_SPECIAL_CHARS * LCD_NUM_SPECIAL_CHARS];
bool bChannelScanner = false;
