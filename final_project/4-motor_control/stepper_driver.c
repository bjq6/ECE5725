#include "stepper_driver.h"

#include <stdlib.h>
#include <math.h>

#include "bcm/bcm-includes.h"

extern volatile uint32_t *gpio;
extern volatile rpi_irq_controller_t *irq_ctrl;

volatile uint32_t step_en = 0;

axis_t x_axis = {0};
axis_t y_axis = {0};
axis_t z_axis = {0};

uint32_t sd_state = SD_NOTREADY;

volatile unsigned int sd_stack[1024];
volatile unsigned int enc_stack[1024];

void pin_setup() {


	// ==== X_AXIS =====
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
	x_axis.nlim_pin = GPIO2;
	x_axis.nlim_fsel = GPIO2_FSEL;
	x_axis.nlim_fbit = GPIO2_FBIT;
	x_axis.nlim_lev = GPIO_GPLEV0;
	
	gpio[x_axis.nlim_fsel] &= ~(7 << x_axis.nlim_fbit);

	_set_gpio_pull(GPIO_BANK0, x_axis.nlim_pin, GPIO_PUP);

	/*
	// ==== Y_AXIS =====
	// setup step pin
	y_axis.step_pin = GPIO20;
	y_axis.step_fsel = GPIO20_FSEL;
	y_axis.step_fbit = GPIO20_FBIT;
	y_axis.step_set = GPIO_GPSET0;
	y_axis.step_clr	= GPIO_GPCLR0;

	gpio[y_axis.step_fsel] &= ~(7 << y_axis.enc.enc_fbit);
	gpio[y_axis.step_fsel] |= 1 << y_axis.step_fbit;

	// setup direction pin
	y_axis.dir_pin = GPIO21;
	y_axis.dir_fsel = GPIO21_FSEL;
	y_axis.dir_fbit = GPIO21_FBIT;
	y_axis.dir_set = GPIO_GPSET0;
	y_axis.dir_clr = GPIO_GPCLR0;

	gpio[y_axis.dir_fsel] &= ~(7 << y_axis.enc.enc_fbit);
	gpio[y_axis.dir_fsel] |= 1 << y_axis.dir_fbit;

	// setup encoder pin
	y_axis.enc.enc_pin = GPIO5;
	y_axis.enc.enc_fsel = GPIO5_FSEL;
	y_axis.enc.enc_fbit = GPIO5_FBIT;
	y_axis.enc.enc_lev = GPIO_GPLEV0;
	
	gpio[y_axis.enc.enc_fsel] &= ~(7 << y_axis.enc.enc_fbit);
	_set_gpio_pull(GPIO_BANK0, y_axis.enc.enc_pin, GPIO_PUP);

	// setup positive limit pin
	y_axis.nlim_pin = GPIO2;
	y_axis.nlim_fsel = GPIO2_FSEL;
	y_axis.nlim_fbit = GPIO2_FBIT;
	y_axis.nlim_lev = GPIO_GPLEV0;
	
	gpio[y_axis.nlim_fsel] &= ~(7 << y_axis.nlim_fbit);
	_set_gpio_pull(GPIO_BANK0, y_axis.nlim_pin, GPIO_PUP);
	*/
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
	

	if (x_axis.step_div) {
		ACT_LED_ON();

		uint32_t step_div_mag = abs(x_axis.step_div);
		if (x_axis.step_cntr >= step_div_mag) {
			if (x_axis.step_div > 0) {
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

	// y_axis.step_cntr++;
	// if (y_axis.step_div) {
	// 	ACT_LED_ON();

	// 	uint32_t step_div_mag = abs(y_axis.step_div);
	// 	if (y_axis.step_cntr >= step_div_mag) {
	// 		if (y_axis.step_div > 0) {
	// 			set_dir(&y_axis, 1);
	// 			y_axis.step_pos++;
	// 		} else {
	// 			set_dir(&y_axis, 0);
	// 			y_axis.step_pos--;
	// 		}

	// 		step(&y_axis);
	// 		y_axis.step_cntr = 0;
	// 	}
	// } else {
	// 	ACT_LED_OFF();
	// }

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
	volatile uint32_t *lev = &(gpio[a->nlim_lev]);
	uint8_t pin = 1 << a->nlim_pin;

	x_axis.step_div = -20;

	a->enc.home_offset = 0;
	while ((*lev & pin)) {
		read_enc(a);
		printf("%f\n", a->enc.a_abs);
		waitcnt32(CNT32() + CLKFREQ/10);
	}
	x_axis.step_div = 0;
 
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
	x_axis.step_div = 0;

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

	step_en = 1;

	run_homing(&x_axis);

	sd_state = SD_READY;

	uint32_t loop_t;

	while(1) {
		loop_t = CNT32();
		//update_pid(&x_axis);
		read_enc(&x_axis);

		float e = (x_axis.enc.a_abs - x_axis.target);
		x_axis.pid.de = e - x_axis.pid.de;
		x_axis.pid.ie += e;
		x_axis.pid.e = e;

		x_axis.step_div = -(int)(x_axis.pid.Kp/x_axis.pid.e + x_axis.pid.Kd/x_axis.pid.de + x_axis.pid.Ki/x_axis.pid.ie)*x_axis.speed_inv;


		if (x_axis.step_div == 0) {
			if (x_axis.pid.e < 0) {
				x_axis.step_div = x_axis.speed_inv;
			} else {
				x_axis.step_div = -x_axis.speed_inv;
			}
		}

		if (x_axis.step_div > 1000 || x_axis.step_div < -1000) {
			x_axis.step_div = 0;
		}

		waitcnt32(loop_t + CLKFREQ/1000); //1 ms update loop
    }
}

void update_pid(axis_t *a) {
	read_enc(a);

	float e = (a->enc.a_abs - a->target);
	a->pid.de = e - a->pid.de;
	a->pid.ie += e;
	a->pid.e = e;

	a->step_div = -(int)(a->pid.Kp/a->pid.e + a->pid.Kd/a->pid.de + a->pid.Ki/a->pid.ie)*a->speed_inv;


	if (a->step_div == 0) {
		if (a->pid.e < 0) {
			a->step_div = a->speed_inv;
		} else {
			a->step_div = -a->speed_inv;
		}
	}

	if (a->step_div > 1000 || a->step_div < -1000) {
		a->step_div = 0;
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