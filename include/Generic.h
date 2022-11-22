#ifndef GENERIC_H
#define GENERIC_H


#include <Arduino.h>

#define COUNT_OF(x) 			((sizeof(x)/sizeof(0[x])) / ((size_t)(!(sizeof(x) % sizeof(0[x])))))
#define CONSTRAIN_HI(amt,high)	((amt)>(high)?(high):(amt))
#define CONSTRAIN_LO(amt,low)	((amt)<(low)?(low):(amt))

#define BIT_SET_ARRAY(arr,bit)  (arr[bit>>8] |= 1u<<(bit))
#define BIT_CLR_ARRAY(arr,bit)  (arr[bit>>8] &= ~(1u<<(bit)))

#ifdef LOCAL_DEBUG
#define Sprint(a)   (Serial.print(a))			// macro as substitute for print, enable if no print wanted
#define Sprintln(a) (Serial.println(a))
#define Sflush() 	(Serial.flush())
#else											// macro for "no" debug print
#define Sprint(a)
#define Sprintln(a)
#define Sflush()
#endif

#define NRF24_MAX_CHANNEL       (125)

#define CHILD_ID_COUNTER        (0)
#define CHILD_ID_UPDATE_GATEWAY (1)

extern const char *pcPaLevelNames[];
extern const char *pcDataRateNames[];

const char* rf24PaLevelToString( const uint8_t level );
uint8_t rf24PaLevelConstrain( const uint8_t level );
const char* rf24DataRateToString( const uint8_t rate );
uint8_t rf24DataRateConstrain( const uint8_t rate );

#endif