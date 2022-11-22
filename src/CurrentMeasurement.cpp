#include <CurrentMeasurement.h>

/**** Current Measurement ****/
float SleepCurrent_uA 	 	= CurrentValueWait;
float TransmitCurrent_uA 	= 0;
float ReceiveCurrent_uA  	= 0;

/**** Configure ADC ****/
volatile uint8_t iStartStorageAfterNrAdcSamples  = 7; 	//Note this depends on the set ADC prescaler (currently: 16x prescaler)
volatile uint8_t iStopStorageAfterNrAdcSamples 	= 28; 	//Note this depends on the set ADC prescaler (currently: 16x prescaler)
volatile uint16_t iAdcSum;								//Limit the number of samples to < 2^6 = 64
volatile uint8_t iNrAdcSamplesElapsed;
volatile boolean bAdcDone;

void ADCSetup() {
   //**** ADC SETUP ****
	ADCSRA =  bit (ADEN);                      				// turn ADC on
	ADCSRA |= bit (ADPS2);                               	// Prescaler of 16: To get sufficient samples in Tx Current Measurement
	ADMUX  =  bit (REFS0) | bit (REFS1) | (ADC_PIN_NR & 0x07);  // ARef internal and select input port
}

float GetAvgADCBits(int iNrSamples) {
	//iNrSamples < 64, else risk of overflowing iAdcSum
	iStartStorageAfterNrAdcSamples  = 0;
	iStopStorageAfterNrAdcSamples 	= iNrSamples;

	iNrAdcSamplesElapsed	= 0;
	iAdcSum 				   = 0;
	bAdcDone 				= false;
	ADCSRA |= bit (ADSC) | bit (ADIE);	  	//start new ADC conversion
	while (!bAdcDone) { delay_with_update(1); };			//Wait until all ADC conversions have completed
	bAdcDone 				= false;
	return ((float)iAdcSum/(float)(iNrSamples));
}

unsigned long Time_to_reach_InitCurrent_uA(float Threshold_current_uA, unsigned long lTimeOut){
	float Current_uA = 0;
	unsigned long lTstart = millis();
	unsigned long ldT = 0;
	while ((Current_uA < Threshold_current_uA) & (ldT < lTimeOut)){
		delay_with_update(50);	//don't measure to often as it will load the sleep current too much.
		Current_uA = uAperBit3 * GetAvgADCBits(iNrCurrentMeasurements);
		ldT = (millis()-lTstart);
	}
	return ldT;
}

bool SettledSleepCurrent_uA_reached(float Threshold_current_uA_per_sec, unsigned long lTimeOut){
	bool bReached = false;
	float Current_uA_new = 0;
	float Current_uA_prev = uAperBit3 * GetAvgADCBits(iNrCurrentMeasurements);
	float Current_uA_per_sec = 0;
	unsigned long lTstart = millis();
	int n = 0;
	unsigned long lTimeScaler = constrain(100 / Threshold_current_uA_per_sec, 100, 15000);//don't measure to often as it will load the sleep current too much.
	while ((n < 2) & ((millis() - lTstart) < lTimeOut)){
		delay_with_update(lTimeScaler);
		Current_uA_new 		= uAperBit3 * GetAvgADCBits(iNrCurrentMeasurements);
		Current_uA_per_sec 	= (Current_uA_new - Current_uA_prev) / (float(lTimeScaler) / 1000);
		Current_uA_prev = Current_uA_new;
		if (Current_uA_per_sec < Threshold_current_uA_per_sec) { n++; }
		else{ n=0; }
	}
	if (Current_uA_per_sec < Threshold_current_uA_per_sec){bReached = true;}
	return bReached;
}


/*****************************************************************************/
/******************************** ADC INTERRUPT ******************************/
/*****************************************************************************/
// ADC complete ISR
ISR (ADC_vect) {
	//Continuous sampling of ADC
	iNrAdcSamplesElapsed++;
	if (iNrAdcSamplesElapsed >= iStartStorageAfterNrAdcSamples){	//Skip first 130us for TX settling according to datasheet
#ifdef TRIGGER_PIN
		digitalWrite(TRIGGER_PIN,HIGH);				//Debugging purposes with scope
#endif
		iAdcSum = iAdcSum + ADC;
		if (iNrAdcSamplesElapsed < iStopStorageAfterNrAdcSamples){
			ADCSRA |= bit (ADSC) | bit (ADIE);	  	// start new conversion and enable interrupt flag on completion
		}
		else{
			bAdcDone = true;
#ifdef TRIGGER_PIN
			digitalWrite(TRIGGER_PIN,LOW);			//Debugging purposes with scope
#endif
		}
	}
	else{
		ADCSRA |= bit (ADSC) | bit (ADIE);	  		// start new conversion and enable interrupt flag on completion
	}
}

void ISR_TransmitTriggerADC() {
	detachPCINT(digitalPinToPinChangeInterrupt(MY_RF24_CE_PIN));
	//Settings for TX - Transmit measurement
	iStartStorageAfterNrAdcSamples  = 7; 	//Note this depends on the set ADC prescaler (currently: 16x prescaler) + Matched to TX timing
	switch (iRf24DataRate){
		case 0:
			iStopStorageAfterNrAdcSamples 	= 12 + uint8_t(iPayloadSize*0.25); 	//Note this depends on the set ADC prescaler (currently: 16x prescaler)	+ Matched to TX timing
			break;
		case 1:
			iStopStorageAfterNrAdcSamples 	= 8 + uint8_t(iPayloadSize*0.125); 	//Note this depends on the set ADC prescaler (currently: 16x prescaler)	+ Matched to TX timing
			break;
		case 2:
			iStopStorageAfterNrAdcSamples 	= 25 + uint8_t(iPayloadSize*1.4); 	//Note this depends on the set ADC prescaler (currently: 16x prescaler)	+ Matched to TX timing
			break;
	}

	iNrAdcSamplesElapsed	= 0;
	iAdcSum 				   = 0;
	bAdcDone 				= false;
	ADCSRA |= bit (ADSC) | bit (ADIE);	  	//start new ADC conversion
}
