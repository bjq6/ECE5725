#ifndef RPI_PWM_H
#define RPI_PWM_H

#include "rpi-base.h"

#define RPI_PWM_BASE		(PERIPHERAL_BASE + 0x20C000)


typedef struct {
    volatile uint32_t ctl;
    volatile uint32_t sta;
    volatile uint32_t dmac;

    volatile uint32_t reserved1;

    volatile uint32_t rng1;
    volatile uint32_t dat1;
    volatile uint32_t fif1;

    volatile uint32_t reserved2;

    volatile uint32_t rng2;
    volatile uint32_t dat2;
} rpi_pwm_t;

rpi_pwm_t* _get_pwm_ctrl();

#endif