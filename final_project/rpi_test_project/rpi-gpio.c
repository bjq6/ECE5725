#include <stdint.h>

#include "rpi-gpio.h"

volatile uint32_t* gpio = (uint32_t*)GPIO_BASE;

uint32_t *_get_gpio_reg() {
	return (uint32_t *)gpio;
}