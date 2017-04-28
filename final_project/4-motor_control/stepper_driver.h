#ifndef STEPPER_DRIVER_H
#define STEPPER_DRIVER_H

#include <stdint.h>
#include <stdio.h>

#define SD_T_STEP 			64

#define X_STEP_PER_REV 		1024
#define Y_STEP_PER_REV		1024
#define Z_STEP_PER_REV		1024

#define X_IN_PER_REV		1
#define Y_IN_PER_REV		1
#define Z_IN_PER_REV		1

typedef struct {
	float e; 		// error
	float de;		// de/dt
	float ie;		// int (e) dt

	float Kp;	// p gain
	float Ki;	// i gain
	float Kd;	// d gain
} pid_control_t;

typedef struct {
	float a; 		// angle (relative)
	float a_abs;	// angle (absolute)
	int32_t r;		// rev count;

	uint8_t enc_pin;
	uint8_t enc_fsel;
	uint8_t enc_fbit;
	uint8_t enc_lev;


} encoder_t;

typedef struct {
	encoder_t enc;
	
	float pos;
	float target;
	uint32_t speed_inv;

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

axis_t* get_x_axis();
axis_t* get_y_axis();
axis_t* get_z_axis();

void set_target(axis_t *a, float t, float v_inv);

void step(axis_t *a);
void set_dir(axis_t *a, uint8_t dir);
void run_homing(axis_t *a);

void sd_IRQ();

void pin_setup();

void read_enc(axis_t *a);


void __attribute__ ((naked)) sd_main();
#endif