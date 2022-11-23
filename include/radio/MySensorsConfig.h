#pragma once

#include "radio/MySensorsSharedConfig.h"

#ifndef MY_GATEWAY_FEATURE

#define MY_SPLASH_SCREEN_DISABLED      // Disable splash screen (saves some flash)
#define MY_TRANSPORT_WAIT_READY_MS     (10)   // [ms] Init timeout for gateway not reachable
#define MY_NODE_ID                     (250)   // Set a high node ID, which typically will not yet be used in the network
#define MY_PARENT_NODE_IS_STATIC       // Fixed parent Node ID, else MySensors Transport will attempt automatic fix after successive failures...but we don't want that while diagnosing our connection
#define MY_PARENT_NODE_ID              (0)      // Typically 0 for Gateway

#define MY_BAUD_RATE 115200
#define MY_INDICATION_HANDLER

#define DEFAULT_DESTINATION_NODE       (0)      // Default 0 = gateway, Settable in Menu
#define DEFAULT_PAYLOAD_SIZE           (2)      // 2 Bytes is the minimum for the Counter data
#define DEFAULT_MESSAGE_RATE           (10)

#endif // !MY_GATEWAY_FEATURE
