#include <StateMachine.h>

#pragma pack(push, 1)								// exact fit - no padding (to save space)
union t_MessageData {
  uint8_t m_dynMessage[MAX_PAYLOAD];
};
#pragma pack(pop)									//back to the previous packing mode

state currState = STATE_IDLE;

//Message Rate
uint8_t iGetMsgRate = 0;

const int iNrArcCnt = 15;
uint8_t iArrayArcCnt[iNrArcCnt] = {0};
uint8_t iArcCntAvg = 0;
uint8_t iArcCntMax = 0;

/*****************************************************************************/
/********************* TRANSMIT & MEASUREMENT STATEMACHINE *******************/
/*****************************************************************************/

void stateMachine()
{
	static unsigned long timestamp = 0;	// reused inbetween some states

	unsigned long iSetMsgDelay = (1000000L/iSetMsgRate);

	switch (currState)
	{
		case STATE_IDLE:
			if (bUpdateGateway)
			{
				// Start of gateway update
				currState = STATE_START_GW_UPDATE;
			}
			else if (bChannelScanner)
			{
				// Start of channel scanner
				currState = STATE_CH_SCAN;
			}
			else if (isTransportReady())
			{
				// Start of next measurement round
				if ((micros() - timestamp) >= iSetMsgDelay){currState = STATE_TX;}	//Message Rate limiter
			}
			break;

		case STATE_TX:
			{
				// Transmit Current Measurement - Trigger measurement on interrupt
				EIFR |= 0x01;					//Clear interrupt flag to prevent an immediate trigger
				attachPCINT(digitalPinToPinChangeInterrupt(MY_RF24_CE_PIN), ISR_TransmitTriggerADC, RISING);
				unsigned long lTcurTransmit = transmit(iPayloadSize);

				//Time rate of transmissions
				iGetMsgRate = static_cast<uint8_t>((1e6/(lTcurTransmit-timestamp))+0.5);
				timestamp = lTcurTransmit;

				if (bAdcDone) {				//Get TX Current Measurement Data...it should already have finished
					TransmitCurrent_uA 	= uAperBit1 * ((float)iAdcSum / (float)(iStopStorageAfterNrAdcSamples - iStartStorageAfterNrAdcSamples + 1));
					bAdcDone = false;
				}
				//else{Sprintln(F("BAD ADC TIMING:"));} //Will happen if the node can not find the gateway @ startup - but no problem.
				store_ArcCnt_in_array();	//Store the number of auto re-transmits by the radio in the array

				currState = STATE_RX;
			}
			break;

		case STATE_RX:
				MY_RF24_startListening();	//Make sure I'm in RX mode
				ReceiveCurrent_uA 	= uAperBit1 * GetAvgADCBits(iNrCurrentMeasurements);
				//MY_RF24_stopListening();	//I will automatically get out of RX mode when applicable
				//	Sprint(F("TransmitCurrent_uA:"));Sprintln(TransmitCurrent_uA);
				//	Sprint(F("ReceiveCurrent_uA:"));Sprintln(ReceiveCurrent_uA);
				currState = STATE_PROCESS_DATA;
			break;

		case STATE_PROCESS_DATA:
			//Calculate Mean & Max Values for display purposes
			getMeanAndMaxFromIntArray(&iArcCntAvg, &iArcCntMax, iArrayArcCnt, iNrArcCnt);
			getMeanAndMaxFromArray(&iMeanDelayFirstHop_ms,&iMaxDelayFirstHop_ms,lTimeDelayBuffer_FirstHop_us,iNrTimeDelays);
			getMeanAndMaxFromArray(&iMeanDelayDestination_ms,&iMaxDelayDestination_ms,lTimeDelayBuffer_Destination_us,iNrTimeDelays);

			currState = STATE_IDLE;
			break;

		case STATE_SLEEP:
		{
			//Sleep Current Measurement
			transportDisable();
			delay_with_update(20);									//Gate charge time and settle time, don't use wait as it will prevent the radio from sleep
			float SleepCurrent_uA_intermediate = uAperBit1*GetAvgADCBits(iNrCurrentMeasurements);
			if (SleepCurrent_uA_intermediate < 1500){
				//Set Higher Sensitivity: uAperBit2
				digitalWrite(MOSFET_2P2OHM_PIN, LOW);
				digitalWrite(MOSFET_100OHM_PIN, HIGH);
				delay_with_update(400);								//worst case settle time to charge through higher impedance
				SleepCurrent_uA_intermediate = uAperBit2 * GetAvgADCBits(iNrCurrentMeasurements);
			}
			else {SleepCurrent_uA = SleepCurrent_uA_intermediate;}

			if (SleepCurrent_uA_intermediate < 15){
				//Set Higher Sensitivity: uAperBit3
				digitalWrite(MOSFET_2P2OHM_PIN, LOW);
				digitalWrite(MOSFET_100OHM_PIN, LOW);
				const float Init_Meas_SleepCurrent_uA = 0.4;
				const unsigned long lTimeOut_InitCurrent_ms = 6000;
				const unsigned long lTimeOut_Settled_SleepCurrent_ms = 30000;
				unsigned long ldT  = Time_to_reach_InitCurrent_uA(Init_Meas_SleepCurrent_uA, lTimeOut_InitCurrent_ms);
				if (ldT < lTimeOut_InitCurrent_ms){
					float fTarget_uA_per_sec = (0.4/(float(ldT)/1000))/20;	//Slew rate to which it should be reduced before we consider it settled
					SettledSleepCurrent_uA_reached(fTarget_uA_per_sec, lTimeOut_Settled_SleepCurrent_ms);
					SleepCurrent_uA = uAperBit3 * GetAvgADCBits(iNrCurrentMeasurements);	//Even if SettledSleepCurrent_uA_reached has timed out it should have settled by now
				}
				else{		//Radio Cap > 1000uF - this will take too long....
					SleepCurrent_uA = CurrentValueErrCap;
				}
			}
			else {SleepCurrent_uA = SleepCurrent_uA_intermediate;}
//			Sprint(F("SleepCurrent_uA:"));Sprintln(SleepCurrent_uA);

			//Restore standby power state
			digitalWrite(MOSFET_2P2OHM_PIN, HIGH);	//Enable 2.2Ohm
			digitalWrite(MOSFET_100OHM_PIN, LOW);
			transportStandBy();

			currState = STATE_IDLE;
			break;
		}

		case STATE_CH_SCAN:
			// Clear all buckets to start all over
			(void)memset(channelScanBuckets, 0, COUNT_OF(channelScanBuckets));
			currState = STATE_CH_SCAN_RESTART;
			break;

		case STATE_CH_SCAN_RESTART:
			iRf24ChannelScanCurrent = iRf24ChannelScanStart;
			currState = STATE_CH_SCAN_MEASURE;
			break;

		case STATE_CH_SCAN_MEASURE:
			// http://forum.diyembedded.com/viewtopic.php?f=4&t=809#p1047
			RF24_ce(LOW);
			RF24_setChannel(iRf24ChannelScanCurrent);
			RF24_flushRX();
			RF24_ce(HIGH);
			delayMicroseconds(130+40);
			timestamp = micros();
			currState = STATE_CH_SCAN_WAIT;
			break;

		case STATE_CH_SCAN_WAIT:
			if (not bChannelScanner)
			{
				// Requested to stop scanner. Restore channel.
				RF24_setChannel(loadState(EEPROM_CHANNEL));
				currState = STATE_IDLE;
				break;
			}
			if ((micros() - timestamp) < SCANNEL_SCAN_MEASURE_TIME_US)
			{
				break;
			}
			if (RF24_getReceivedPowerDetector())
			{
				// Determine bucket and increase vote
				uint8_t bucket = (iRf24ChannelScanCurrent-iRf24ChannelScanStart) * COUNT_OF(channelScanBuckets) / (iRf24ChannelScanStop-iRf24ChannelScanStart+1);
				bucket = CONSTRAIN_HI(bucket, COUNT_OF(channelScanBuckets)-1);	// just to be sure...
				if (channelScanBuckets[bucket] < CHANNEL_SCAN_BUCKET_MAX_VAL)
				{
					++channelScanBuckets[bucket];
				}
			}
			if (iRf24ChannelScanCurrent >= iRf24ChannelScanStop)
			{
/*				for (size_t i = 0; i < COUNT_OF(channelScanBuckets); ++i)
				{
					Sprint(channelScanBuckets[i]);
					Sprint('\t');
				}
				Sprintln();
*/
				currState = STATE_CH_SCAN_RESTART;
				break;
			}
			++iRf24ChannelScanCurrent;
			currState = STATE_CH_SCAN_MEASURE;
			break;

    	case STATE_START_GW_UPDATE:
			updateGatewayAttemptsRemaining = updateGatewayNumAttempts;
			currState = STATE_TX_GW_UPDATE;
			break;

		case STATE_TX_GW_UPDATE:
			if (updateGatewayAttemptsRemaining)
			{
				--updateGatewayAttemptsRemaining;
				MyMessage MsgUpdateGateway(CHILD_ID_UPDATE_GATEWAY, V_CUSTOM);
				MsgUpdateGateway.setDestination(0);
				serializeGwSettings( MsgUpdateGateway );

				// Transmit message with software ack request (returned in "receive function")
				if ( send(MsgUpdateGateway, true) )
				{
					// Got a reply from gateway that message was received correctly.
					// Gateway will change to new settings, so the node can also activate the settings.
					saveEepromAndReset();
					// Never return here...
				}
			}
			else
			{
				// Retry attempts exhausted. Give up.
				currState = STATE_FAILED_GW_UPDATE;
			}
			break;

		case STATE_FAILED_GW_UPDATE:
			// Signals the UI that GW update failed. On next button/encoder change => return to prev menu
			bUpdateGateway = false;
			currState = STATE_IDLE;
			break;

		default:
			break;
	}
}

void store_ArcCnt_in_array(){
	static size_t iIndexInArray=0;
	iIndexInArray++;
	iIndexInArray = iIndexInArray % iNrArcCnt;
	iArrayArcCnt[iIndexInArray] = get_rf24_register_arc_cnt();
}

// nRF24 register: AcknowledgeRequestCount Counter. Counts the number of (hardware) re-transmissions for the current transaction
uint8_t get_rf24_register_arc_cnt() {
	return static_cast<uint8_t>(RF24_getObserveTX() & 0x0F);
}

/*****************************************************************************/
/************************ RECEIVE & TRANSMIT FUNCTIONS ***********************/
/*****************************************************************************/
void receive(const MyMessage &message) {
	if (message.isAck() == 1 && message.type == V_CUSTOM && message.sensor==CHILD_ID_COUNTER){	//Acknowledge message & of correct type
		const t_MessageData& ReceivedData = *(static_cast<t_MessageData*>(message.getCustom()));
		uint16_t iNewMessage = ((uint16_t)ReceivedData.m_dynMessage[0] << 8)|((uint16_t)ReceivedData.m_dynMessage[1]); //2 Byte Counter

		uint16_t iIndexInArray = iNewMessage % iMaxNumberOfMessages;
		BIT_CLR_ARRAY(bArrayNAckMessages, iIndexInArray); 			// set corresponding flag to received.

		// Check Message (Round trip) Delay
		uint8_t iIndexInTimeArray = IndexOfValueInArray(iNewMessage, iMessageIndexBuffer, iNrTimeDelays); //Look-up if message is present in MessageIndexBuffer for delay calculation
		if ((iIndexInTimeArray != 255) && iIndexInTimeArray <=iNrTimeDelays){
			lTimeDelayBuffer_Destination_us[iIndexInTimeArray] = micros()-lTimeOfTransmit_us[iIndexInTimeArray];
		}
		iNrNAckMessages--;	//Received an Acknowledge Message (so one less No Ack)
	}
}

unsigned long transmit(size_t iPayloadLength) {
	static int iIndexInArrayFailedMessages  = 0 ;
	static int iIndexInArrayTimeMessages  = 0 ;
	static t_MessageData MessageData;

	iPayloadLength = constrain(iPayloadLength,PAYLOAD_LENGTH_MIN,PAYLOAD_LENGTH_MAX);
	iMessageCounter++;
	MessageData.m_dynMessage[0] = (uint8_t)((iMessageCounter & 0xFF00) >> 8);
	MessageData.m_dynMessage[1] = (uint8_t)(iMessageCounter & 0x00FF);
	//All other MessageData is just left undefined

	// Cyclic Index counters of arrays
	iIndexInArrayFailedMessages = iMessageCounter % iMaxNumberOfMessages;
	iIndexInArrayTimeMessages 	= iMessageCounter % iNrTimeDelays;

	BIT_SET_ARRAY(bArrayNAckMessages, iIndexInArrayFailedMessages);		// set corresponding flag to "Not Received Yet"

	// Prepare time stamp logging for transmit
	lTimeDelayBuffer_Destination_us[iIndexInArrayTimeMessages] = 0; 		// Clear Buffer value, new value will be written when message is received
	iMessageIndexBuffer[iIndexInArrayTimeMessages]=iMessageCounter;		// To link the Time Stamp to the correct message when we receive the acknowledge
	iNrNAckMessages++;													// Add one to the Not Acknowledged Message counter and remove it again if/when it is received.
	lTimeOfTransmit_us[iIndexInArrayTimeMessages] = micros();

#ifdef LED_PIN
	// Light LED
	digitalWrite(LED_PIN, HIGH);
#endif

	// Transmit message with software ack request (returned in "receive function"),
	// the boolean returned here is a Hardware hop-to-hop Ack
	boolean success = send(MsgCounter.setDestination(iDestinationNode).set(&MessageData,iPayloadLength), true);
	if (!success) {
		// Keep LED on to indicate failure
		lTimeDelayBuffer_FirstHop_us[iIndexInArrayTimeMessages] = 0;	//It failed, so I can't use it to determine a First Hop Delay (i.e. it is "infinite" delay as it failed)
		BIT_SET_ARRAY(bArrayFailedMessages, iIndexInArrayFailedMessages);	//Log it as a failed message (for rolling average)
		iNrFailedMessages++ ;
	}
	else{
		lTimeDelayBuffer_FirstHop_us[iIndexInArrayTimeMessages] = micros() - lTimeOfTransmit_us[iIndexInArrayTimeMessages];	//Log First Hop Delay in buffer
//		unsigned long temptime = lTimeDelayBuffer_FirstHop_us[iIndexInArrayTimeMessages];
		BIT_CLR_ARRAY(bArrayFailedMessages, iIndexInArrayFailedMessages);	//Log it as a not-failed = succesful message (for rolling average)
#ifdef LED_PIN
		// LED off to indicate success
		digitalWrite(LED_PIN, LOW);
#endif
	}
	return lTimeOfTransmit_us[iIndexInArrayTimeMessages];
}

void MY_RF24_startListening()
{
	// toggle PRX
	RF24_setRFConfiguration(RF24_CONFIGURATION | _BV(RF24_PWR_UP) | _BV(RF24_PRIM_RX) );
	// all RX pipe addresses must be unique, therefore skip if node ID is RF24_BROADCAST_ADDRESS
	if(RF24_NODE_ADDRESS!= RF24_BROADCAST_ADDRESS) {
		RF24_setPipeLSB(RF24_REG_RX_ADDR_P0, RF24_NODE_ADDRESS);
	}
	// start listening
	RF24_ce(HIGH);
}