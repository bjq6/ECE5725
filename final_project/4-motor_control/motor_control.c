#include <stdint.h>

#include "bcm/rpi-systimer.h"
#include "bcm/rpi-gpio.h"
#include "bcm/rpi-base.h"
#include "bcm/rpi-armtimer.h"
#include "bcm/rpi-interrupts.h"
#include "bcm/rpi-aux.h"
#include "bcm/printf.h"

volatile uint32_t *gpio;
volatile rpi_arm_timer_t *timer;
volatile rpi_irq_controller_t *irq_ctrl;
volatile aux_t *aux;

volatile int on = 0;

IRQ() {
    if (timer->maskedIRQ == 1) { // timer ISR 
        /*if (on) {
            gpio[GPIO_GPSET0] |= 1 << GPIO12;
            on = 0;
            PWR_LED_ON();
        } else {
            gpio[GPIO_GPCLR0] |= 1 << GPIO12;
            PWR_LED_OFF();
            on = 1;
        }*/
        gpio[GPIO_GPSET0] |= 1 << GPIO12;
        waitcnt32(CNT32() + CLKFREQ/1000000);
        gpio[GPIO_GPCLR0] |= 1 << GPIO12;
    } else if ((aux->MU_IIR & 0x6) == 4) { // UART ISR
        _uart_rx(); // get it out of the buffer to clear the interrupt
    }

    timer->IRQclear = 1;

}

void _init() {
    _disable_irq();

    // get the peripheral objects
    gpio = _get_gpio_reg();
    timer = _get_arm_timer();
    irq_ctrl = _get_irq_controller();
    aux = _get_aux();

    // set power and activity leds as outputs
    gpio[ACT_LED_GPFSEL] |= (1 << ACT_LED_GPFBIT);
    gpio[PWR_LED_GPFSEL] |= (1 << PWR_LED_GPFBIT);
    ACT_LED_ON();

    _uart_init(115200, 8, AUX_MU_RX_IRQ_ENABLE);
    init_printf(0, _uart_tx);

    // load the number of counts between interrupts. 
    timer->load = CLKFREQ/40000;

    // set up the arm timer. 23 bits, enable it, enable interrupt generation, prescaler of 1.
    timer->control = 
            RPI_ARMTIMER_CTRL_23BIT |
            RPI_ARMTIMER_CTRL_ENABLE |
            RPI_ARMTIMER_CTRL_INT_ENABLE |
            RPI_ARMTIMER_CTRL_PRESCALE_1;
    
    // enable IRQ to trigger on timer
    irq_ctrl->enable_IRQ_basic |= RPI_BASIC_ARM_TIMER_IRQ;
    // enable IRQ to trigger on UART rx
    irq_ctrl->enable_IRQ_1 |= (1 << 29);

    _enable_irq();

    gpio[GPIO12_FSEL] |= 1 << GPIO12_FBIT;

    ACT_LED_OFF();

    printf("Initialization Complete\n");
}

int kernel_main(void) {
    _init(); 
    
    while(1){
        ACT_LED_ON();
        waitcnt32(CNT32() + CLKFREQ/2);
        ACT_LED_OFF();
        waitcnt32(CNT32() + CLKFREQ/2);
    }
}