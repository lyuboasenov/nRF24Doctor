#ifndef RADIO_H
#define RADIO_H

//**** DEBUG *****
#define LOCAL_DEBUG
//#define MY_DEBUG							// Enable debug prints to serial monitor
//#define MY_DEBUG_VERBOSE_RF24

//**** MySensors *****
#include "RadioConfig.h"

#define MY_SPLASH_SCREEN_DISABLED			// Disable splash screen (saves some flash)
#define MY_TRANSPORT_WAIT_READY_MS	(10)	// [ms] Init timeout for gateway not reachable
#define MY_NODE_ID					(250)	// Set a high node ID, which typically will not yet be used in the network
#define MY_PARENT_NODE_IS_STATIC			// Fixed parent Node ID, else MySensors Transport will attempt automatic fix after successive failures...but we don't want that while diagnosing our connection
#define MY_PARENT_NODE_ID			(0)		// Typically 0 for Gateway

#define MY_BAUD_RATE 115200
#define MY_INDICATION_HANDLER

#include <MySensors.h>
#include "Generic.h"
#include "RadioStorage.h"

//**** MySensors Messages ****
extern MyMessage MsgCounter;

// Actual data exchanged in a message
#define PAYLOAD_LENGTH_MIN         (2)				//The counter is always transmitted and is 2 bytes in size
#define PAYLOAD_LENGTH_MAX (MAX_PAYLOAD)

extern bool transportHwError;

#endif // NODE_RADIO_H