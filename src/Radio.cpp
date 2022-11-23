#include <Radio.h>
#include <Arduino.h>

#include "Config.h"

/********************************************************************************/
/****************************** VARIABLES & DEFINES *****************************/
/********************************************************************************/

#define MY_RADIO_RF24

#define DEFAULT_RF24_CHANNEL         (90)
#define DEFAULT_RF24_PA_LEVEL_NODE   (RF24_PA_LOW)
#define DEFAULT_RF24_PA_LEVEL_GW     (RF24_PA_LOW)
#define DEFAULT_RF24_DATARATE        (RF24_250KBPS)

#include "radio/MySensorsConfig.h"
#include <MySensors.h>
#include "Generic.h"
#include "ArrayUtils.h"

uint8_t iCePin = MY_RF24_CE_PIN;

uint8_t iRf24Channel;
uint8_t iRf24DataRate;
uint8_t iRf24PaLevel;

#ifndef MY_GATEWAY_FEATURE
// Node-only parameters
uint8_t iRf24PaLevelGw;  // PA Level for the Gateway
uint8_t iPayloadSize;

// Actual data exchanged in a message
uint8_t iMinPayloadSize = 2;           // The counter is always transmitted and is 2 bytes in size
uint8_t iMaxPayloadSize = MAX_PAYLOAD_SIZE;

uint8_t iSetMsgRate;
uint8_t iDestinationNode;
#endif

//**** MySensors Messages ****
MyMessage MsgCounter(CHILD_ID_COUNTER, V_CUSTOM);   				//Send Message Counter value

bool transportHwError = false;

//Message Rate
uint8_t iGetMsgRate = 0;

uint8_t iArrayArcCnt[iNrArcCnt] = {0};
uint8_t iArcCntAvg = 0;
uint8_t iArcCntMax = 0;

/**** Monitoring Constants&Variables ****/
uint8_t bArrayFailedMessages[(iMaxNumberOfMessages + 7) >> 3];  // Array for moving average storage
uint8_t bArrayNAckMessages[(iMaxNumberOfMessages + 7) >> 3];    // Array for moving average storage
uint16_t iNrFailedMessages = 0;                                 // total of Failed Messages
uint16_t iNrNAckMessages = 0;                                   // total of Not Acknowledged Messages
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

#pragma pack(push, 1)								// exact fit - no padding (to save space)
union t_MessageData {
  uint8_t m_dynMessage[MAX_PAYLOAD_SIZE];
};
#pragma pack(pop)									//back to the previous packing mode

/********************************************************************************/
/******************************** RADIO FUNCTIONS *******************************/
/********************************************************************************/
#ifdef MY_GATEWAY_FEATURE
void deserializeGwSettings(const MyMessage& msg) {
   const uint16_t packed = msg.getUInt();

   // Extract the new Gateway settings
   iRf24Channel = packed / 100U;
   iRf24PaLevel = (packed / 10U) % 10;  // yes iRf24PaLevel and not iRf24PaLevelGw, as iRf24PaLevel is sent to nRF24
   iRf24DataRate = packed % 10;
}
#else
void serializeGwSettings(MyMessage& msg) {
   const uint16_t packed = iRf24Channel * 100 + iRf24PaLevelGw * 10 + iRf24DataRate;
   msg.set(packed);
}
void updateGwSettings() {
   MyMessage MsgUpdateGateway(CHILD_ID_UPDATE_GATEWAY, V_CUSTOM);
   MsgUpdateGateway.setDestination(0);
   serializeGwSettings(MsgUpdateGateway);

   // Transmit message with software ack request (returned in "receive function")
   if (send(MsgUpdateGateway, true)) {
      // Got a reply from gateway that message was received correctly.
      // Gateway will change to new settings, so the node can also activate the settings.
      saveEepromAndReset();
      // Never return here...
   }
}

bool isRadioReady() {
   return isTransportReady();
}

void radioDisable() {
   transportDisable();
}

void radioStandBy() {
   transportStandBy();
}

void radioStartListening() {
   // toggle PRX
   RF24_setRFConfiguration(RF24_CONFIGURATION | _BV(RF24_PWR_UP) | _BV(RF24_PRIM_RX));
   // all RX pipe addresses must be unique, therefore skip if node ID is RF24_BROADCAST_ADDRESS
   if (RF24_NODE_ADDRESS != RF24_BROADCAST_ADDRESS) {
      RF24_setPipeLSB(RF24_REG_RX_ADDR_P0, RF24_NODE_ADDRESS);
   }
   // start listening
   RF24_ce(HIGH);
}

void calculateMeanAndMax() {
   // Calculate Mean & Max Values for display purposes
   getMeanAndMaxFromIntArray(&iArcCntAvg, &iArcCntMax, iArrayArcCnt, iNrArcCnt);
   getMeanAndMaxFromArray(&iMeanDelayFirstHop_ms, &iMaxDelayFirstHop_ms, lTimeDelayBuffer_FirstHop_us, iNrTimeDelays);
   getMeanAndMaxFromArray(&iMeanDelayDestination_ms, &iMaxDelayDestination_ms, lTimeDelayBuffer_Destination_us, iNrTimeDelays);
}

void ClearStorageAndCounters() {
	(void)memset(bArrayFailedMessages, 0, COUNT_OF(bArrayFailedMessages));
	(void)memset(bArrayNAckMessages, 0, COUNT_OF(bArrayNAckMessages));
	iNrNAckMessages = iMessageCounter = iNrFailedMessages = 0;
}

void radioWaitUntilReady() {
   // Show splash screen for a short while, trying to connect to GW.
   // If GW connection has not been established after this delay the
   // node continues trying to connect.
   transportWaitUntilReady(2000);
}

void radioPresent() {
   sendSketchInfo(F(SKETCH_NAME_STRING), F(SKETCH_VERSION_STRING));
   present(CHILD_ID_COUNTER, S_CUSTOM);  // "CUSTOM" counter
}

/********************************************************************************/
/************************* MYSENSORS INDICATION CALLBACK ************************/
/********************************************************************************/
void indication(const indication_t ind) {
   switch (ind) {
#ifdef LED_PIN
      // If transport is not ready, flash the LED to indicate something is happening
      case INDICATION_TX:
         if (not isTransportReady()) {
            // Blink LED
            digitalWrite(LED_PIN, HIGH);
            delay_with_update(20);
            digitalWrite(LED_PIN, LOW);
         }
         break;
#endif
      case INDICATION_ERR_INIT_TRANSPORT:  // MySensors transport hardware (radio) init failure.
         transportHwError = true;
         break;
      default:
         break;
   }
}

#endif // MY_GATEWAY_FEATURE

/********************************************************************************/
/********************************* STORE SETTINGS *******************************/
/********************************************************************************/

void logRadioSettings() {
   Sprint(F("Channel:"));
   Sprint(iRf24Channel);
   Sprint(F("\tPaLevel:"));
   Sprint(rf24PaLevelToString(iRf24PaLevel));
#ifndef MY_GATEWAY_FEATURE
   Sprint(F("\tPaLevelGw:"));
   Sprint(rf24PaLevelToString(iRf24PaLevelGw));
#endif
   Sprint(F("\tDataRate:"));
   Sprint(rf24DataRateToString(iRf24DataRate));
#ifndef MY_GATEWAY_FEATURE
   Sprint(F("\tDest:"));
   Sprint(iDestinationNode);
   Sprint(F("\tPayload:"));
   Sprint(iPayloadSize);
   Sprint(F("\tRate:"));
   Sprint(iSetMsgRate);
#endif
   Sprintln();
}

void loadDefaults() {
   Sprintln(F("Load defaults"));

   iRf24Channel = DEFAULT_RF24_CHANNEL;
#ifdef MY_GATEWAY_FEATURE
   iRf24PaLevel = DEFAULT_RF24_PA_LEVEL_GW;
#else
   iRf24PaLevel = DEFAULT_RF24_PA_LEVEL_NODE;
#endif
   iRf24DataRate = DEFAULT_RF24_DATARATE;
#ifndef MY_GATEWAY_FEATURE
   iRf24PaLevelGw = DEFAULT_RF24_PA_LEVEL_GW;
   iDestinationNode = DEFAULT_DESTINATION_NODE;
   iPayloadSize = DEFAULT_PAYLOAD_SIZE;
   iSetMsgRate = DEFAULT_MESSAGE_RATE;
#endif
}

void loadEeprom() {
   if (loadState(EEPROM_FLAG) == EEPROM_FLAG_MAGIC) {
      // Eeprom contents are valid
      Sprintln(F("Read eeprom"));

      iRf24Channel = loadState(EEPROM_CHANNEL);
      iRf24PaLevel = loadState(EEPROM_PA_LEVEL);
      iRf24DataRate = loadState(EEPROM_DATARATE);
#ifndef MY_GATEWAY_FEATURE
      iRf24PaLevelGw = loadState(EEPROM_PA_LEVEL_GW);
      iDestinationNode = loadState(EEPROM_DESTINATION_NODE);
      iPayloadSize = loadState(EEPROM_PAYLOAD_SIZE);
      iSetMsgRate = loadState(EEPROM_MESSAGE_RATE);
#endif
   } else {
      // Eeprom contents are invalid: Load defaults & save to eeprom
      loadDefaults();
      saveEepromAndReset();
      // Never return here...
   }
}

void saveEeprom() {
   Sprintln(F("Save eeprom"));

   saveState(EEPROM_CHANNEL, iRf24Channel);
   saveState(EEPROM_PA_LEVEL, iRf24PaLevel);
   saveState(EEPROM_DATARATE, iRf24DataRate);
#ifndef MY_GATEWAY_FEATURE
   saveState(EEPROM_PA_LEVEL_GW, iRf24PaLevelGw);
   saveState(EEPROM_DESTINATION_NODE, iDestinationNode);
   saveState(EEPROM_PAYLOAD_SIZE, iPayloadSize);
   saveState(EEPROM_MESSAGE_RATE, iSetMsgRate);
#endif

   // Mark eeprom contents valid
   saveState(EEPROM_FLAG, EEPROM_FLAG_MAGIC);
}

void invalidateEeprom() {
   Sprintln(F("Invalidate eeprom"));
   // Mark eeprom contents invalid
   saveState(EEPROM_FLAG, uint8_t(~EEPROM_FLAG_MAGIC));
}

void saveEepromAndReset() {
   saveEeprom();
   // Do a Soft Reset - This allows for the radio to correctly reload with the new settings from EEPROM
   reset();
   // Never return here...
}

void reset() __attribute__((noreturn));
void reset() {
   Sprintln(F("Reset"));
   Sflush();
   asm volatile("  jmp 0");
   __builtin_unreachable();
}

void loadChannelFromStore() {
   RF24_setChannel(loadState(EEPROM_CHANNEL));
}

bool getReceivedPowerDetector() {
   return RF24_getReceivedPowerDetector();
}

void setMeasureChannel(uint8_t channel) {
   // http://forum.diyembedded.com/viewtopic.php?f=4&t=809#p1047
   RF24_ce(LOW);
   RF24_setChannel(channel);
   RF24_flushRX();
   RF24_ce(HIGH);
}

// nRF24 register: AcknowledgeRequestCount Counter. Counts the number of (hardware) re-transmissions for the current transaction
uint8_t get_rf24_register_arc_cnt() {
   return static_cast<uint8_t>(RF24_getObserveTX() & 0x0F);
}

void store_ArcCnt_in_array() {
   static size_t iIndexInArray = 0;
   iIndexInArray++;
   iIndexInArray = iIndexInArray % iNrArcCnt;
   iArrayArcCnt[iIndexInArray] = get_rf24_register_arc_cnt();
}

/*****************************************************************************/
/************************ RECEIVE & TRANSMIT FUNCTIONS ***********************/
/*****************************************************************************/
#ifdef MY_GATEWAY_FEATURE
void receive(const MyMessage &message) {
   if (message.type == V_CUSTOM && message.sensor == CHILD_ID_UPDATE_GATEWAY) {
      Sprintln(F("Received new Radio settings"));
      deserializeGwSettings(message);
      saveEeprom();
      Sprintln(F("Resetting..."));
      reset();
   }
}
#else
void receive(const MyMessage& message) {
   if (message.isAck() == 1 && message.type == V_CUSTOM && message.sensor == CHILD_ID_COUNTER) {  // Acknowledge message & of correct type
      const t_MessageData& ReceivedData = *(static_cast<t_MessageData*>(message.getCustom()));
      uint16_t iNewMessage = ((uint16_t)ReceivedData.m_dynMessage[0] << 8) | ((uint16_t)ReceivedData.m_dynMessage[1]);  // 2 Byte Counter

      uint16_t iIndexInArray = iNewMessage % iMaxNumberOfMessages;
      BIT_CLR_ARRAY(bArrayNAckMessages, iIndexInArray);  // set corresponding flag to received.

      // Check Message (Round trip) Delay
      uint8_t iIndexInTimeArray = IndexOfValueInArray(iNewMessage, iMessageIndexBuffer, iNrTimeDelays);  // Look-up if message is present in MessageIndexBuffer for delay calculation
      if ((iIndexInTimeArray != 255) && iIndexInTimeArray <= iNrTimeDelays) {
         lTimeDelayBuffer_Destination_us[iIndexInTimeArray] = micros() - lTimeOfTransmit_us[iIndexInTimeArray];
      }
      iNrNAckMessages--;  // Received an Acknowledge Message (so one less No Ack)
   }
}

unsigned long transmit(size_t iPayloadLength, unsigned long timestamp) {
   static int iIndexInArrayFailedMessages = 0;
   static int iIndexInArrayTimeMessages = 0;
   static t_MessageData MessageData;

   iPayloadLength = constrain(iPayloadLength, iMinPayloadSize, iMaxPayloadSize);
   iMessageCounter++;
   MessageData.m_dynMessage[0] = (uint8_t)((iMessageCounter & 0xFF00) >> 8);
   MessageData.m_dynMessage[1] = (uint8_t)(iMessageCounter & 0x00FF);
   // All other MessageData is just left undefined

   // Cyclic Index counters of arrays
   iIndexInArrayFailedMessages = iMessageCounter % iMaxNumberOfMessages;
   iIndexInArrayTimeMessages = iMessageCounter % iNrTimeDelays;

   BIT_SET_ARRAY(bArrayNAckMessages, iIndexInArrayFailedMessages);  // set corresponding flag to "Not Received Yet"

   // Prepare time stamp logging for transmit
   lTimeDelayBuffer_Destination_us[iIndexInArrayTimeMessages] = 0;    // Clear Buffer value, new value will be written when message is received
   iMessageIndexBuffer[iIndexInArrayTimeMessages] = iMessageCounter;  // To link the Time Stamp to the correct message when we receive the acknowledge
   iNrNAckMessages++;                                                 // Add one to the Not Acknowledged Message counter and remove it again if/when it is received.
   lTimeOfTransmit_us[iIndexInArrayTimeMessages] = micros();

#ifdef LED_PIN
   // Light LED
   digitalWrite(LED_PIN, HIGH);
#endif

   // Transmit message with software ack request (returned in "receive function"),
   // the boolean returned here is a Hardware hop-to-hop Ack
   boolean success = send(MsgCounter.setDestination(iDestinationNode).set(&MessageData, iPayloadLength), true);
   if (!success) {
      // Keep LED on to indicate failure
      lTimeDelayBuffer_FirstHop_us[iIndexInArrayTimeMessages] = 0;       // It failed, so I can't use it to determine a First Hop Delay (i.e. it is "infinite" delay as it failed)
      BIT_SET_ARRAY(bArrayFailedMessages, iIndexInArrayFailedMessages);  // Log it as a failed message (for rolling average)
      iNrFailedMessages++;
   } else {
      lTimeDelayBuffer_FirstHop_us[iIndexInArrayTimeMessages] = micros() - lTimeOfTransmit_us[iIndexInArrayTimeMessages];  // Log First Hop Delay in buffer
      //		unsigned long temptime = lTimeDelayBuffer_FirstHop_us[iIndexInArrayTimeMessages];
      BIT_CLR_ARRAY(bArrayFailedMessages, iIndexInArrayFailedMessages);  // Log it as a not-failed = succesful message (for rolling average)
#ifdef LED_PIN
      // LED off to indicate success
      digitalWrite(LED_PIN, LOW);
#endif
   }

   unsigned long lTcurTransmit = lTimeOfTransmit_us[iIndexInArrayTimeMessages];
   // Time rate of transmissions
   iGetMsgRate = static_cast<uint8_t>((1e6 / (lTcurTransmit - timestamp)) + 0.5);

   return lTcurTransmit;
}
#endif // MY_GATEWAY_FEATURE
