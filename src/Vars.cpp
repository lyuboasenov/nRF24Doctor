#include <Vars.h>

/**** Monitoring Constants&Variables ****/
uint8_t bArrayFailedMessages[(iMaxNumberOfMessages+7)>>3];     		// Array for moving average storage
uint8_t bArrayNAckMessages[(iMaxNumberOfMessages+7)>>3];			// Array for moving average storage
uint16_t iNrFailedMessages = 0;            							// total of Failed Messages
uint16_t iNrNAckMessages = 0;              							// total of Not Acknowledged Messages
uint16_t iMessageCounter = 0;

/**** Timing ****/
uint16_t iMessageIndexBuffer[iNrTimeDelays];
unsigned long lTimeOfTransmit_us[iNrTimeDelays];
unsigned long lTimeDelayBuffer_Destination_us[iNrTimeDelays];
unsigned long lTimeDelayBuffer_FirstHop_us[iNrTimeDelays];
uint16_t iMeanDelayFirstHop_ms = 0;
uint16_t iMaxDelayFirstHop_ms = 0;
uint16_t iMeanDelayDestination_ms = 0;
uint16_t iMaxDelayDestination_ms = 0;

/**** Remote Gateway Update ****/
//uint8_t iRetryGateway 	= 0;
bool bUpdateGateway = false;
uint8_t updateGatewayAttemptsRemaining;

/**** RPD Channel Scanner ****/
uint8_t iRf24ChannelScanStart = 0;
uint8_t iRf24ChannelScanStop  = NRF24_MAX_CHANNEL;
uint8_t iRf24ChannelScanCurrent = 0;
uint8_t iRf24ChannelScanColDisplayed = LCD_WIDTH_SPECIAL_CHARS*LCD_NUM_SPECIAL_CHARS/2;
uint8_t channelScanBuckets[LCD_WIDTH_SPECIAL_CHARS*LCD_NUM_SPECIAL_CHARS];
bool bChannelScanner = false;

void ClearStorageAndCounters() {
	(void)memset(bArrayFailedMessages, 0, COUNT_OF(bArrayFailedMessages));
	(void)memset(bArrayNAckMessages, 0, COUNT_OF(bArrayNAckMessages));
	iNrNAckMessages = iMessageCounter = iNrFailedMessages = 0;
}