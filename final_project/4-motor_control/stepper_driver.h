#ifndef STEPPER_DRIVER_H
#define STEPPER_DRIVER_H

#include <stdint.h>
#include <stdio.h>

#define SD_T_STEP 			16

#define X_STEP_PER_REV 		1024
#define Y_STEP_PER_REV		1024
#define Z_STEP_PER_REV		1024

#define X_IN_PER_REV		1
#define Y_IN_PER_REV		1
#define Z_IN_PER_REV		1

int32_t x_pos; 		// current motor position
int32_t x_target;	// target position

typedef struct {
	float e; 		// error
	float de;		// de/dt
	float ie;		// int (e) dt

	float Kp;	// p gain
	float Ki;	// i gain
	float Kd;	// d gain
} pid_control_t;

typedef struct {
	float a; 		// angle (absolute)
	float v; 		// velocity
	uint32_t raw_n; 	// raw value (pwm width in clocks)
	uint32_t raw_d;		// raw value (denomenator)

	uint8_t enc_pin;
	uint8_t enc_fsel;
	uint8_t enc_fbit;
	uint8_t enc_lev;
} encoder_t;

typedef struct {
	encoder_t enc;
	
	float pos;
	float target;

	int32_t step_pos;
	int32_t step_target;
	int32_t step_cntr;

	volatile uint8_t motion_active;

	pid_control_t pid;

	uint8_t step_pin;
	uint8_t step_fsel;
	uint8_t step_fbit;
	uint8_t step_set;
	uint8_t step_clr;

	uint8_t dir_pin;
	uint8_t dir_fsel;
	uint8_t dir_fbit;
	uint8_t dir_set;
	uint8_t dir_clr;

} axis_t;

void step(axis_t *a);
void set_dir(axis_t *a, uint8_t dir);
void run_homing(axis_t *a);

void start_stepper_driver(uint32_t adr_sd, uint32_t adr_enc);
void start_encoder_driver(uint32_t adr);
void sd_IRQ();

void pin_setup();

void sd_main();
void enc_main();

#endif