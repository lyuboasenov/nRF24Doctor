#include <ArrayUtils.h>

uint8_t IndexOfValueInArray(uint16_t val, uint16_t *array, uint8_t size) {
	// Find the (first) array element which equals val and return the index.
	// If val not found in the array return 255
	for (int i=0; i < size; i++) {
		if (array[i] == val){
			return i;
		}
	}
	return 255;	//Not Found
}

uint8_t GetNumBitsSetInArray(uint8_t arr[], const uint8_t sizeBytes) {
	uint8_t count = 0;
	for (uint8_t i = 0; i < sizeBytes; ++i)
	{
		uint8_t v = arr[i];
		// Counting bits set, Brian Kernighan's way
		// http://graphics.stanford.edu/~seander/bithacks.html#CountBitsSetKernighan
		for (; v; ++count)
		{
			v &= v - 1; // clear the least significant bit set
		}
	}
	return count;
}

void getMeanAndMaxFromIntArray(uint8_t *mean_value, uint8_t *max_value, uint8_t *buffer, uint8_t size) {
	//Note: excluding 0 values from mean calculation
	boolean bNotZero = false;
	uint8_t iMaxValue=0;	//max Array value
	uint16_t sum=0;
	for (int i=0; i < size; i++)
	{
		if (buffer[i] != 0){
			sum 		= sum + static_cast<uint16_t>(buffer[i]);
			iMaxValue	= max(iMaxValue,buffer[i]);
			bNotZero=true;
		}
	}
	*max_value		= iMaxValue;
	if (bNotZero){
		*mean_value 	= static_cast<uint8_t>((sum / size)+0.5);
	}
	else {
		*mean_value = 0;
	}
}

void getMeanAndMaxFromArray(uint16_t *mean_value, uint16_t *max_value, unsigned long *buffer, uint8_t size) {
	//Note: excluding 0 values from mean calculation
	uint8_t iNrOfSamples=0;		//max equals size
	unsigned long lMaxValue=0;	//max Array value
	float sum=0;
	for (int i=0; i < size; i++)
	{
		if (buffer[i] != 0){
			sum 		= sum + (float)buffer[i];
			lMaxValue	= max(lMaxValue,buffer[i]);
			iNrOfSamples++;
		}
	}
	if (iNrOfSamples !=0){
		*mean_value 	= (uint16_t) (((sum / (float)iNrOfSamples)+500)/1000);
		*max_value		= (uint16_t) ((lMaxValue+500)/1000L);
	}
	else {
		*mean_value = 65535;	//INF identifier
		*max_value 	= 65535;	//INF identifier
	}
}