#pragma once

#if !defined(GATEWAY)

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

void stateMachine();

#endif // !defined(MY_GATEWAY_FEATURE)
