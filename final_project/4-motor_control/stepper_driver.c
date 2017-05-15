#include "stepper_driver.h"

#include <stdlib.h>
#include <math.h>

#include "bcm/bcm-includes.h"

extern volatile uint32_t *gpio;
extern volatile rpi_irq_controller_t *irq_ctrl;

volatile int32_t step_div;
volatile uint32_t step_en = 0;

axis_t x_axis = {0};
axis_t y_axis = {0};
axis_t z_axis = {0};

uint32_t sd_state = SD_NOTREADY;

volatile unsigned int sd_stack[1024];
volatile unsigned int enc_stack[1024];

void pin_setup() {

	// setup step pin
	x_axis.step_pin = GPIO19;
	x_axis.step_fsel = GPIO19_FSEL;
	x_axis.step_fbit = GPIO19_FBIT;
	x_axis.step_set = GPIO_GPSET0;
	x_axis.step_clr	= GPIO_GPCLR0;

	gpio[x_axis.step_fsel] &= ~(7 << x_axis.enc.enc_fbit);
	gpio[x_axis.step_fsel] |= 1 << x_axis.step_fbit;

	// setup direction pin
	x_axis.dir_pin = GPIO26;
	x_axis.dir_fsel = GPIO26_FSEL;
	x_axis.dir_fbit = GPIO26_FBIT;
	x_axis.dir_set = GPIO_GPSET0;
	x_axis.dir_clr = GPIO_GPCLR0;

	gpio[x_axis.dir_fsel] &= ~(7 << x_axis.enc.enc_fbit);
	gpio[x_axis.dir_fsel] |= 1 << x_axis.dir_fbit;

	// setup encoder pin
	x_axis.enc.enc_pin = GPIO6;
	x_axis.enc.enc_fsel = GPIO6_FSEL;
	x_axis.enc.enc_fbit = GPIO6_FBIT;
	x_axis.enc.enc_lev = GPIO_GPLEV0;
	
	gpio[x_axis.enc.enc_fsel] &= ~(7 << x_axis.enc.enc_fbit);

	_set_gpio_pull(GPIO_BANK0, x_axis.enc.enc_pin, GPIO_PUP);

	// setup positive limit pin
	x_axis.plim_pin = GPIO2;
	x_axis.plim_fsel = GPIO2_FSEL;
	x_axis.plim_fbit = GPIO2_FBIT;
	x_axis.plim_lev = GPIO_GPLEV0;
	
	gpio[x_axis.plim_fsel] &= ~(7 << x_axis.plim_fbit);

	_set_gpio_pull(GPIO_BANK0, x_axis.plim_pin, GPIO_PUP);

	// setup negative limit pin
	x_axis.nlim_pin = GPIO27;
	x_axis.nlim_fsel = GPIO27_FSEL;
	x_axis.nlim_fbit = GPIO27_FBIT;
	x_axis.nlim_lev = GPIO_GPLEV0;
	
	gpio[x_axis.nlim_fsel] &= ~(7 << x_axis.nlim_fbit);

	_set_gpio_pull(GPIO_BANK0, x_axis.nlim_pin, GPIO_PUP);
	
}

/* 
 * IRQ for time step: send pulses here. 
 */

void sd_IRQ() {
	if (!step_en) {
		PWR_LED_OFF();
		return;
	}

	x_axis.step_cntr++;

	if (step_div) {
		ACT_LED_ON();

		uint32_t step_div_mag = abs(step_div);
		if (x_axis.step_cntr >= step_div_mag) {
			if (step_div > 0) {
				set_dir(&x_axis, 1);
				x_axis.step_pos++;
			} else {
				set_dir(&x_axis, 0);
				x_axis.step_pos--;
			}

			step(&x_axis);
			x_axis.step_cntr = 0;
		}
	} else {
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
	sd_state = SD_HOMING;
	printf("Start homing\n");
	volatile uint32_t *lev = &(gpio[a->plim_lev]);
	uint8_t pin = 1 << a->plim_pin;

	step_div = -10;

	a->enc.home_offset = 0;
	while ((*lev & pin));
	step_div = 0;
 
	read_enc(a);
	a->enc.home_offset = a->enc.a;
	a->enc.r = 0;
	a->enc.a_abs = 0;
	read_enc(a);
	printf("Home offset is %f\n", a->enc.home_offset);
}


void set_target(axis_t *a, float t, float v_inv) {
	a->target = 2*M_PI*t/a->mm_per_rev;
	a->speed_inv = v_inv;
}

axis_t* get_x_axis() {
	return &x_axis;
}

axis_t* get_y_axis() {
	return &y_axis;
}

axis_t* get_z_axis() {
	return &z_axis;
}

uint32_t get_sd_state() {
	return sd_state;
}

void setup_vars() {
	x_axis.pid.Kp = 1;
	x_axis.pid.Kd = 1;
	x_axis.pid.Ki = .1;
	x_axis.step_target = 0;
	x_axis.step_pos = 0;
	x_axis.enc.r = 0;
	x_axis.enc.a = 0;
	x_axis.enc.a_abs = 0;
	x_axis.speed_inv = 10000;

	x_axis.mm_per_rev = 8.0;
	set_dir(&x_axis, 1);

}

/*
 * Main functions for cores
 */ 
void sd_main() {

	PWR_LED_ON();
	printf("Starting from core %d\n", (int)get_core_id());

	setup_vars();

	step_div = 0;
	step_en = 1;

	run_homing(&x_axis);

	sd_state = SD_READY;

	uint32_t loop_t;

	while(1) {
		loop_t = CNT32();
		read_enc(&x_axis);

		float e = (x_axis.enc.a_abs - x_axis.target);
		x_axis.pid.de = e - x_axis.pid.de;
		x_axis.pid.ie += e;
		x_axis.pid.e = e;

		step_div = -(int)(x_axis.pid.Kp/x_axis.pid.e + x_axis.pid.Kd/x_axis.pid.de + x_axis.pid.Ki/x_axis.pid.ie)*x_axis.speed_inv;


		if (step_div == 0) {
			if (x_axis.pid.e < 0) {
				step_div = x_axis.speed_inv;
			} else {
				step_div = -x_axis.speed_inv;
			}
		}

		if (step_div > 1000 || step_div < -1000) {
			step_div = 0;
		}

		waitcnt32(loop_t + CLKFREQ/1000); //1 ms update loop
    }
}

void read_enc(axis_t *a) {

	uint32_t t0 = 0;
	uint32_t t1 = 0;
	const int32_t wmin = 32;
	const int32_t wmax = 377;

	float prev_x = a->enc.a;
	float prev_x_abs = a->enc.a_abs;

	volatile uint32_t *xlev = &(gpio[a->enc.enc_lev]);
	uint8_t xpin = 1 << a->enc.enc_pin;

	while (*xlev & xpin); // wait for pin to go low
	while (!(*xlev & xpin));
	t0 = CNT32();
	while (*xlev & xpin);
	t1 = CNT32();

	int32_t w = t1-t0-wmin; // get the high width

	double an = 2*M_PI*w/(double)wmax;  // calculate the angle in radians
	//float da = a - x_axis.enc.a; // difference between current and last read.
	if (an > 2*M_PI) an = 2*M_PI;
	if (an < 0) an = 0;

	a->enc.a = an;

	if (an > 4.5 && prev_x < 1.78) { // we overflowed, add a revolution.
		a->enc.r -= 1;
	} else if (an < 1.78 && prev_x > 4.5) { // we underflowed, sub a revolution. 
		a->enc.r += 1;
	}

	a->enc.a_abs = 2*M_PI*a->enc.r + an - a->enc.home_offset;
	a->enc.a_abs = (a->enc.a_abs + prev_x_abs)/2;

	a->pos = a->mm_per_rev*a->enc.a_abs/(2*M_PI);

}