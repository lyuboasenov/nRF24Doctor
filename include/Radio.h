#pragma once

#include "radio/MySensorsConfig.h"
#include "radio/SettingsStore.h"
#include "radio/Variables.h"

#ifndef MY_GATEWAY_FEATURE
void updateGwSettings();
bool isRadioReady();
void radioDisable();
void radioStandBy();
void radioStartListening();
void radioWaitUntilReady();
void radioPresent();
bool getReceivedPowerDetector();
void setMeasureChannel(uint8_t channel);
void store_ArcCnt_in_array();
unsigned long transmit(size_t iPayloadLength, unsigned long timestamp);
void calculateMeanAndMax();
void ClearStorageAndCounters();
#endif
