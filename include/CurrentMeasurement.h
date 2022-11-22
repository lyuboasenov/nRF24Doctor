#ifndef CURRENT_MEASUREMENT_H
#define CURRENT_MEASUREMENT_H

#include <Menu.h>

//**** Current Measurement ****
// #include <PinChangeInterrupt.h>					// for Pin Change Interrupt      Download: https://github.com/NicoHood/PinChangeInterrupt

const uint8_t iNrCurrentMeasurements 	= 60;	//Nr of measurements for averaging current. <64 to prevent risk of overflow of iAdcSum
const float r1_ohm    = 2.2;
const float r2_ohm    = 100.0;
const float r3_ohm    = 10000.0;
const float Vref_volt = 1.1;
const float uAperBit1 = ((Vref_volt/1024.0)/r1_ohm)*1.0e6;
const float uAperBit2 = ((Vref_volt/1024.0)/r2_ohm)*1.0e6;
const float uAperBit3 = ((Vref_volt/1024.0)/r3_ohm)*1.0e6;

const float CurrentValueErrCap = -2.0;					// Will show 'Err cap' on display
const float CurrentValueWait   = -1.0;					// Will show 'WAIT' on display
const float CurrentValueErr    = 300000.0;				// Will show 'Err' on display

extern float SleepCurrent_uA;
extern float TransmitCurrent_uA;
extern float ReceiveCurrent_uA;

//**** Configure ADC ****
extern volatile uint8_t iStartStorageAfterNrAdcSamples; 	//Note this depends on the set ADC prescaler (currently: 16x prescaler)
extern volatile uint8_t iStopStorageAfterNrAdcSamples; 	//Note this depends on the set ADC prescaler (currently: 16x prescaler)
extern volatile uint16_t iAdcSum;								//Limit the number of samples to < 2^6 = 64
extern volatile uint8_t iNrAdcSamplesElapsed;
extern volatile boolean bAdcDone;

void ADCSetup();

float GetAvgADCBits(int iNrSamples);
unsigned long Time_to_reach_InitCurrent_uA(float Threshold_current_uA, unsigned long lTimeOut);
bool SettledSleepCurrent_uA_reached(float Threshold_current_uA_per_sec, unsigned long lTimeOut);
void ISR_TransmitTriggerADC();

#endif // CURRENT_MEASUREMENT_H