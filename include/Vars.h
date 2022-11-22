#ifndef VARS_H
#define VARS_H

#include <Generic.h>

//**** Monitoring Constants&Variables ****
const int iMaxNumberOfMessages = 100 ;           					// Number of Messages Used for MA calculation
extern uint8_t bArrayFailedMessages[(iMaxNumberOfMessages+7)>>3];     		// Array for moving average storage
extern uint8_t bArrayNAckMessages[(iMaxNumberOfMessages+7)>>3];			// Array for moving average storage
extern uint16_t iNrFailedMessages;            							// total of Failed Messages
extern uint16_t iNrNAckMessages;              							// total of Not Acknowledged Messages
extern uint16_t iMessageCounter;

//**** Timing ****
const uint8_t iNrTimeDelays = 10;
extern uint16_t iMessageIndexBuffer[iNrTimeDelays];
extern unsigned long lTimeOfTransmit_us[iNrTimeDelays];
extern unsigned long lTimeDelayBuffer_Destination_us[iNrTimeDelays];
extern unsigned long lTimeDelayBuffer_FirstHop_us[iNrTimeDelays];
extern uint16_t iMeanDelayFirstHop_ms;
extern uint16_t iMaxDelayFirstHop_ms;
extern uint16_t iMeanDelayDestination_ms;
extern uint16_t iMaxDelayDestination_ms;

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

void ClearStorageAndCounters();

#endif // NODE_VARS_H