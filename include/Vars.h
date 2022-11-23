#ifndef VARS_H
#define VARS_H

#include <Generic.h>

//**** Remote Gateway Update ****
//uint8_t iRetryGateway 	= 0;
extern bool bUpdateGateway;
extern uint8_t updateGatewayAttemptsRemaining;
const uint8_t updateGatewayNumAttempts = 10;

const uint16_t restartDelayMs = 3000u;

//**** RPD Channel Scanner ****
#define LCD_NUM_SPECIAL_CHARS    (8)
#define LCD_WIDTH_SPECIAL_CHARS  (5)
#define LCD_HEIGHT_SPECIAL_CHARS (8)
extern uint8_t iRf24ChannelScanStart;
extern uint8_t iRf24ChannelScanStop;
extern uint8_t iRf24ChannelScanCurrent;
extern uint8_t iRf24ChannelScanColDisplayed;
#define CHANNEL_SCAN_BUCKET_MAX_VAL (255)
extern uint8_t channelScanBuckets[LCD_WIDTH_SPECIAL_CHARS * LCD_NUM_SPECIAL_CHARS];
extern bool bChannelScanner;
#define SCANNEL_SCAN_MEASURE_TIME_US (1000)

#endif // NODE_VARS_H
