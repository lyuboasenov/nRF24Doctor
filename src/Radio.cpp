#include <Radio.h>


//**** MySensors Messages ****
MyMessage MsgCounter(CHILD_ID_COUNTER, V_CUSTOM);   				//Send Message Counter value

bool transportHwError = false;

/********************************************************************************/
/************************* MYSENSORS INDICATION CALLBACK ************************/
/********************************************************************************/
void indication( const indication_t ind )
{
	switch(ind)
	{
#ifdef LED_PIN
		// If transport is not ready, flash the LED to indicate something is happening
		case INDICATION_TX:
			if (not isTransportReady())
			{
				// Blink LED
				digitalWrite(LED_PIN, HIGH);
				delay_with_update(20);
				digitalWrite(LED_PIN, LOW);
			}
			break;
#endif
		case INDICATION_ERR_INIT_TRANSPORT:			// MySensors transport hardware (radio) init failure.
			transportHwError = true;
			break;
		default:
			break;
	}
}
