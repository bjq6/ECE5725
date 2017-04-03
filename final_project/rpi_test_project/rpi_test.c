
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------

#include <stdint.h>

#include "rpi-systimer.h"
#include "rpi-gpio.h"
#include "rpi-base.h"
#include "rpi-armtimer.h"
#include "rpi-interrupts.h"

#define IRQ_BASIC 0x3F00B200
#define IRQ_PEND1 0x3F00B204
#define IRQ_PEND2 0x3F00B208
#define IRQ_FIQ_CONTROL 0x3F00B210
#define IRQ_ENABLE_BASIC 0x3F00B218
#define IRQ_DISABLE_BASIC 0x3F00B224

IRQ() {
    static int on;

    if (_get_arm_timer()->maskedIRQ == 1) {
        if (on) {
            on = 0;
            PWR_LED_OFF();
        } else {
            on = 1;
            PWR_LED_ON();
        }
    }

    _get_arm_timer()->IRQclear = 1;
}

int kernel_main(void) {
    _disable_irq();
    uint32_t *gpio = _get_gpio_reg();
    rpi_arm_timer_t *timer = _get_arm_timer();
    rpi_irq_controller_t *irq_ctrl = _get_irq_controller();

    gpio[ACT_LED_GPFSEL] |= (1 << ACT_LED_GPFBIT);
    gpio[PWR_LED_GPFSEL] |= (1 << PWR_LED_GPFBIT);

    timer->load = 1000000;
    timer->control = 
            RPI_ARMTIMER_CTRL_23BIT |
            RPI_ARMTIMER_CTRL_ENABLE |
            RPI_ARMTIMER_CTRL_INT_ENABLE |
            RPI_ARMTIMER_CTRL_PRESCALE_1;
    
    irq_ctrl->enable_IRQ_basic = RPI_BASIC_ARM_TIMER_IRQ;

    //PUT32(IRQ_ENABLE_BASIC,1);
    _enable_irq();

    while(1) {
        ACT_LED_ON();
        waitcnt32(CNT32() + CLKFREQ);
        ACT_LED_OFF();
        waitcnt32(CNT32() + CLKFREQ);
    }
}