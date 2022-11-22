#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

#include <Arduino.h>
#include <ArrayUtils.h>
#include <Radio.h>
#include <Menu.h>

#include <PinChangeInterrupt.h>					// for Pin Change Interrupt      Download: https://github.com/NicoHood/PinChangeInterrupt

enum state {	STATE_IDLE,
				// Regular measurement states
				STATE_TX, STATE_RX, STATE_PROCESS_DATA, STATE_SLEEP,
				// Channel scanning Mode state
				STATE_CH_SCAN, STATE_CH_SCAN_RESTART, STATE_CH_SCAN_MEASURE, STATE_CH_SCAN_WAIT,
				// Gateway update states
				STATE_START_GW_UPDATE, STATE_TX_GW_UPDATE, STATE_FAILED_GW_UPDATE,
};
extern state currState;

//Message Rate
extern uint8_t iGetMsgRate;
extern uint8_t iArcCntAvg;
extern uint8_t iArcCntMax;

void stateMachine();
void store_ArcCnt_in_array();
uint8_t get_rf24_register_arc_cnt();
unsigned long transmit(size_t iPayloadLength);
void receive(const MyMessage &message);
void MY_RF24_startListening() ;

#endif // NODE_STATE_MACHINE_H