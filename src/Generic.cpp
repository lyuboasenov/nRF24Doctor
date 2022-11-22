#include <Generic.h>

const char *pcPaLevelNames[]  = { "MIN", "LOW", "HIGH", "MAX" };
const char *pcDataRateNames[] = { "1MBPS", "2MBPS" , "250KBPS"};

const char* rf24PaLevelToString( const uint8_t level ) {
	const uint8_t idx = CONSTRAIN_HI( level, COUNT_OF(pcPaLevelNames)-1 );
	return pcPaLevelNames[idx];
}

uint8_t rf24PaLevelConstrain( const uint8_t level ) {
	return CONSTRAIN_HI( level, COUNT_OF(pcPaLevelNames)-1 );
}

const char* rf24DataRateToString( const uint8_t rate ) {
	const uint8_t idx = CONSTRAIN_HI( rate, COUNT_OF(pcDataRateNames)-1 );
	return pcDataRateNames[idx];
}

uint8_t rf24DataRateConstrain( const uint8_t rate ) {
	return CONSTRAIN_HI( rate, COUNT_OF(pcDataRateNames)-1 );
}