#pragma once

#include <Arduino.h>

extern uint8_t iCePin;

extern uint8_t iRf24Channel;
extern uint8_t iRf24DataRate;
extern uint8_t iRf24PaLevel;

#ifndef MY_GATEWAY_FEATURE
// Node-only parameters
extern uint8_t iRf24PaLevelGw;  // PA Level for the Gateway
extern uint8_t iPayloadSize;
extern uint8_t iMinPayloadSize;
extern uint8_t iMaxPayloadSize;
extern uint8_t iSetMsgRate;
extern uint8_t iDestinationNode;
#endif

extern bool transportHwError;
