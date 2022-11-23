#pragma once

//Message Rate
extern uint8_t iGetMsgRate;

const int iNrArcCnt = 15;
extern uint8_t iArrayArcCnt[iNrArcCnt];
extern uint8_t iArcCntAvg;
extern uint8_t iArcCntMax;

//**** Monitoring Constants&Variables ****
const int iMaxNumberOfMessages = 100;                                  // Number of Messages Used for MA calculation
extern uint8_t bArrayFailedMessages[(iMaxNumberOfMessages + 7) >> 3];  // Array for moving average storage
extern uint8_t bArrayNAckMessages[(iMaxNumberOfMessages + 7) >> 3];    // Array for moving average storage
extern uint16_t iNrFailedMessages;                                     // total of Failed Messages
extern uint16_t iNrNAckMessages;                                       // total of Not Acknowledged Messages
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
