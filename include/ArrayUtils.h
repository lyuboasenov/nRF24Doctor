#ifndef NODE_ARRAY_H
#define NODE_ARRAY_H

#include <Arduino.h>

/*****************************************************************/
/**************** ARRAY PROCESSING FUNCTIONS *********************/
/*****************************************************************/

uint8_t IndexOfValueInArray(uint16_t val, uint16_t *array, uint8_t size);
uint8_t GetNumBitsSetInArray(uint8_t arr[], const uint8_t sizeBytes);
void getMeanAndMaxFromIntArray(uint8_t *mean_value, uint8_t *max_value, uint8_t *buffer, uint8_t size);
void getMeanAndMaxFromArray(uint16_t *mean_value, uint16_t *max_value, unsigned long *buffer, uint8_t size);

#endif // NODE_ARRAY_H