#if !defined(GATEWAY)

#include <PinChangeInterrupt.h>
#include <Radio.h>
#include <StateMachine.h>
#include <Vars.h>
#include <CurrentMeasurement.h>
#include <PinConfig.h>

state currState = STATE_IDLE;

/*****************************************************************************/
/********************* TRANSMIT & MEASUREMENT STATEMACHINE *******************/
/*****************************************************************************/

void stateMachine() {
   static unsigned long timestamp = 0;  // reused in-between some states

   unsigned long iSetMsgDelay = (1000000L / iSetMsgRate);

   switch (currState) {
      case STATE_IDLE:
         if (bUpdateGateway) {
            // Start of gateway update
            currState = STATE_START_GW_UPDATE;
         } else if (bChannelScanner) {
            // Start of channel scanner
            currState = STATE_CH_SCAN;
         } else if (isRadioReady()) {
            // Start of next measurement round
            if ((micros() - timestamp) >= iSetMsgDelay) {
               currState = STATE_TX;
            }  // Message Rate limiter
         }
         break;

      case STATE_TX: {
         // Transmit Current Measurement - Trigger measurement on interrupt
         EIFR |= 0x01;  // Clear interrupt flag to prevent an immediate trigger
         attachPCINT(digitalPinToPinChangeInterrupt(iCePin), ISR_TransmitTriggerADC, RISING);
         unsigned long lTcurTransmit = transmit(iPayloadSize, timestamp);

         timestamp = lTcurTransmit;

         if (bAdcDone) {  // Get TX Current Measurement Data...it should already have finished
            TransmitCurrent_uA = uAperBit1 * ((float)iAdcSum / (float)(iStopStorageAfterNrAdcSamples - iStartStorageAfterNrAdcSamples + 1));
            bAdcDone = false;
         }
         // else{Sprintln(F("BAD ADC TIMING:"));} //Will happen if the node can not find the gateway @ startup - but no problem.
         store_ArcCnt_in_array();  // Store the number of auto re-transmits by the radio in the array

         currState = STATE_RX;
      } break;

      case STATE_RX:
         radioStartListening();  // Make sure I'm in RX mode
         ReceiveCurrent_uA = uAperBit1 * GetAvgADCBits(iNrCurrentMeasurements);
         // MY_RF24_stopListening();	//I will automatically get out of RX mode when applicable
         //	Sprint(F("TransmitCurrent_uA:"));Sprintln(TransmitCurrent_uA);
         //	Sprint(F("ReceiveCurrent_uA:"));Sprintln(ReceiveCurrent_uA);
         currState = STATE_PROCESS_DATA;
         break;

      case STATE_PROCESS_DATA:
         calculateMeanAndMax();

         currState = STATE_IDLE;
         break;

      case STATE_SLEEP: {
         // Sleep Current Measurement
         radioDisable();
         delay_with_update(20);  // Gate charge time and settle time, don't use wait as it will prevent the radio from sleep
         float SleepCurrent_uA_intermediate = uAperBit1 * GetAvgADCBits(iNrCurrentMeasurements);
         if (SleepCurrent_uA_intermediate < 1500) {
            // Set Higher Sensitivity: uAperBit2
            digitalWrite(MOSFET_2P2OHM_PIN, LOW);
            digitalWrite(MOSFET_100OHM_PIN, HIGH);
            delay_with_update(400);  // worst case settle time to charge through higher impedance
            SleepCurrent_uA_intermediate = uAperBit2 * GetAvgADCBits(iNrCurrentMeasurements);
         } else {
            SleepCurrent_uA = SleepCurrent_uA_intermediate;
         }

         if (SleepCurrent_uA_intermediate < 15) {
            // Set Higher Sensitivity: uAperBit3
            digitalWrite(MOSFET_2P2OHM_PIN, LOW);
            digitalWrite(MOSFET_100OHM_PIN, LOW);
            const float Init_Meas_SleepCurrent_uA = 0.4;
            const unsigned long lTimeOut_InitCurrent_ms = 6000;
            const unsigned long lTimeOut_Settled_SleepCurrent_ms = 30000;
            unsigned long ldT = Time_to_reach_InitCurrent_uA(Init_Meas_SleepCurrent_uA, lTimeOut_InitCurrent_ms);
            if (ldT < lTimeOut_InitCurrent_ms) {
               float fTarget_uA_per_sec = (0.4 / (float(ldT) / 1000)) / 20;  // Slew rate to which it should be reduced before we consider it settled
               SettledSleepCurrent_uA_reached(fTarget_uA_per_sec, lTimeOut_Settled_SleepCurrent_ms);
               SleepCurrent_uA = uAperBit3 * GetAvgADCBits(iNrCurrentMeasurements);  // Even if SettledSleepCurrent_uA_reached has timed out it should have settled by now
            } else {                                                                 // Radio Cap > 1000uF - this will take too long....
               SleepCurrent_uA = CurrentValueErrCap;
            }
         } else {
            SleepCurrent_uA = SleepCurrent_uA_intermediate;
         }
         //			Sprint(F("SleepCurrent_uA:"));Sprintln(SleepCurrent_uA);

         // Restore standby power state
         digitalWrite(MOSFET_2P2OHM_PIN, HIGH);  // Enable 2.2Ohm
         digitalWrite(MOSFET_100OHM_PIN, LOW);
         radioStandBy();

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
         setMeasureChannel(iRf24ChannelScanCurrent);
         delayMicroseconds(130 + 40);
         timestamp = micros();
         currState = STATE_CH_SCAN_WAIT;
         break;

      case STATE_CH_SCAN_WAIT:
         if (not bChannelScanner) {
            // Requested to stop scanner. Restore channel.
            loadChannelFromStore();
            currState = STATE_IDLE;
            break;
         }
         if ((micros() - timestamp) < SCANNEL_SCAN_MEASURE_TIME_US) {
            break;
         }
         if (getReceivedPowerDetector()) {
            // Determine bucket and increase vote
            uint8_t bucket = (iRf24ChannelScanCurrent - iRf24ChannelScanStart) * COUNT_OF(channelScanBuckets) / (iRf24ChannelScanStop - iRf24ChannelScanStart + 1);
            bucket = CONSTRAIN_HI(bucket, COUNT_OF(channelScanBuckets) - 1);  // just to be sure...
            if (channelScanBuckets[bucket] < CHANNEL_SCAN_BUCKET_MAX_VAL) {
               ++channelScanBuckets[bucket];
            }
         }
         if (iRf24ChannelScanCurrent >= iRf24ChannelScanStop) {
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
         if (updateGatewayAttemptsRemaining) {
            --updateGatewayAttemptsRemaining;
            updateGwSettings();
         } else {
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

#endif // !defined(MY_GATEWAY_FEATURE)
