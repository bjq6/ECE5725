// ================================================
//
// blink_v1.c
// Nikita Ermoshkin (ne75) and Brendan Quinn (bjq6)
// Mar 20, 2017
//
// ================================================

#include <stdio.h>
#include <wiringPi.h>

#define LED 27 // BCM.GPIO pin 17 = wiringPi pin 0

int main (void) {
	wiringPiSetup();
	pinMode(LED, OUTPUT);
	printf("Starting blink...\n");
	
	for (int i = 0; i < 15000000; i++) {
 		digitalWrite(LED, 1); // On
 		delayMicroseconds(100); // uS
 		digitalWrite(LED, 0); // Off
 		delayMicroseconds(100);
	}
	
	return 0;
}
