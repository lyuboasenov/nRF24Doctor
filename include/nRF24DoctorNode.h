/*
PROJECT: 		MySensors nRF24 Doctor
PROGRAMMER: 	Technovation & Yveaux
DATE: 		2018April18
FILE: 		nRF24DoctorNode.ino

Hardware: 	ATMega328p board
MySensorsAPI: 2.2.0

Summary:  	a portable nRF24 Radio Doctor to diagnose module performance

Change log:
	2018/03/27	New Release, based on: https://forum.mysensors.org/topic/3984/nrf24l01-connection-quality-meter
	2018/04/13	Fixed some bugs in Current Measurement & display of currents
	2018/04/17	Added support for TFT_ILI9163C display
*/

#ifndef NRF24_DOCTOR_NODE_H
#define NRF24_DOCTOR_NODE_H

#define SKETCH_NAME_STRING    "nRF24_Doctor_N250"
#define SKETCH_VERSION_STRING "1.2"

#include <Menu.h>

/*****************************************************************************/
/************************************ STARTUP ********************************/
/*****************************************************************************/
void before() ;

void setup() ;

void presentation() ;

/*****************************************************************************/
/*********************************** MAIN LOOP *******************************/
/*****************************************************************************/
void loop() ;

#endif