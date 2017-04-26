#include "stepper_driver.h"

#include <stdlib.h>
#include <math.h>

#include "bcm/bcm-includes.h"

extern volatile uint32_t *gpio;
extern volatile rpi_irq_controller_t *irq_ctrl;

volatile uint32_t step_div;
volatile uint32_t step_en = 0;

axis_t x_axis = {0};
axis_t y_axis = {0};
axis_t z_axis = {0};

void pin_setup() {
	x_axis.step_pin = GPIO19;
	x_axis.step_fsel = GPIO19_FSEL;
	x_axis.step_fbit = GPIO19_FBIT;
	x_axis.step_set = GPIO_GPSET0;
	x_axis.step_clr	= GPIO_GPCLR0;

	gpio[x_axis.enc.enc_fsel] &= ~(7 << x_axis.enc.enc_fbit);
	gpio[x_axis.step_fsel] |= 1 << x_axis.step_fbit;

	x_axis.dir_pin = GPIO26;
	x_axis.dir_fsel = GPIO26_FSEL;
	x_axis.dir_fbit = GPIO26_FBIT;
	x_axis.dir_set = GPIO_GPSET0;
	x_axis.dir_clr = GPIO_GPCLR0;

	gpio[x_axis.enc.enc_fsel] &= ~(7 << x_axis.enc.enc_fbit);
	gpio[x_axis.dir_fsel] |= 1 << x_axis.dir_fbit;

	x_axis.enc.enc_pin = GPIO6;
	x_axis.enc.enc_fsel = GPIO6_FSEL;
	x_axis.enc.enc_fbit = GPIO6_FBIT;
	x_axis.enc.enc_lev = GPIO_GPLEV0;
	
	gpio[x_axis.enc.enc_fsel] &= ~(7 << x_axis.enc.enc_fbit);

	_set_gpio_pull(GPIO_BANK0, GPIO6, GPIO_PUP);

	
}

/* 
 * IRQ for time step: send pulses here. 
 */

void sd_IRQ() {
	if (!step_en) {
		return;
	}

	x_axis.step_cntr++;

	if (x_axis.step_pos != x_axis.step_target) {
		x_axis.motion_active = 1;
		ACT_LED_ON();

		if (x_axis.step_cntr >= step_div) {
			if (x_axis.step_pos < x_axis.step_target) {
				set_dir(&x_axis, 1);
				x_axis.step_pos++;
			} else if (x_axis.step_pos > x_axis.step_target) {
				set_dir(&x_axis, 0);
				x_axis.step_pos--;
			}

			step(&x_axis);
			x_axis.step_cntr = 0;
		}
	} else {
		x_axis.motion_active = 0;
		ACT_LED_OFF();
	}
}

/* 
 * stepper control functions
 */
void step(axis_t *a) {
	gpio[a->step_set] |= 1 << a->step_pin;
    waitcnt32(CNT32() + 1);
    gpio[a->step_clr] |= 1 << a->step_pin;
}

void set_dir(axis_t *a, uint8_t dir) {
	if (dir) {
		gpio[a->dir_set] |= 1 << a->dir_pin;
	} else {
		gpio[a->dir_clr] |= 1 << a->dir_pin;
	}
}

void run_homing(axis_t *a) {
	a->step_pos = 0;
	set_dir(a, 1);
}

/* 
 * Star the cores
*/

void start_stepper_driver(uint32_t adr_sd, uint32_t adr_enc) {
	start_core(sd_main, adr_sd);
	//start_core(enc_main, adr_enc);
}


/*
 * Main functions for cores
 */ 
void sd_main() {
	_init_core();

	_lock_pf_mutex();
	printf("Starting from %d\n", (int)get_core_id());
	_unlock_pf_mutex();

	pin_setup();
	run_homing(&x_axis);
	waitcnt32(CNT32() + CLKFREQ);

	step_div = 100;

	while(1) {
		x_axis.step_target = -1000;
		waitcnt32(CNT32() + CLKFREQ/100); // delay a bit to let motion active get set.
		while (x_axis.motion_active);
		waitcnt32(CNT32() + CLKFREQ);

		_lock_pf_mutex();
		printf("Switching directions + \n");
		_unlock_pf_mutex();

		x_axis.step_target = 1000;
		waitcnt32(CNT32() + CLKFREQ/100);
		while (x_axis.motion_active);
		waitcnt32(CNT32() + CLKFREQ);

		_lock_pf_mutex();
		printf("Switching directions - \n");
		_unlock_pf_mutex();
    }
}

void enc_main() {
	_init_core();

	uint32_t t0;
	uint32_t t1;

	while(1) {
		while (gpio[x_axis.enc.enc_lev] & (1 << x_axis.enc.enc_pin)); // wait for pin to go low
		while (!(gpio[x_axis.enc.enc_lev] & (1 << x_axis.enc.enc_pin))); // wait for pin to go high
		t0 = CNT32();
		while (gpio[x_axis.enc.enc_lev] & (1 << x_axis.enc.enc_pin)); // wait for pin to go low
		t1 = CNT32();
		uint32_t w = t1-t0; // get the high width
		while (!(gpio[x_axis.enc.enc_lev] & (1 << x_axis.enc.enc_pin))); // wait once more for pin to go high
		t0 = CNT32();
		uint32_t tw = (t0-t1) + w;
		float a = (2*M_PI*w)/(float)tw;  // calculate the angle in radians
		//float da = a - x_axis.enc.a; // difference between current and last read.
		x_axis.enc.a = a;

	}
}