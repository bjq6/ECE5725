#include <stdint.h>
#include <stdio.h>

#include "bcm/bcm-includes.h"

#include "stepper_driver.h"

volatile uint32_t *gpio;
volatile rpi_arm_timer_t *timer;
volatile rpi_irq_controller_t *irq_ctrl;
volatile aux_t *aux;

volatile mutex_t m = 0;

IRQ() {
    if (timer->maskedIRQ == 1) { // timer ISR 
        sd_IRQ();
    } else if ((aux->MU_IIR & 0x6) == 4) { // UART ISR
        uint8_t c = _uart_rx(); // get it out of the buffer to clear the interrupt
        _uart_tx(c);
    }

    timer->IRQclear = 1;
}

HANG() {

    printf("-----HANG INTERRUPT FROM CORE %d-----\n", (int)get_core_id());

    while(1) {
        ACT_LED_ON();
        PWR_LED_OFF();
        waitcnt32(CNT32() + CLKFREQ/5);
        ACT_LED_OFF();
        PWR_LED_ON();
        waitcnt32(CNT32() + CLKFREQ/5);
    }
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
    gpio[PWR_LED_GPFSEL] &= ~(7 << PWR_LED_GPFBIT);
    gpio[PWR_LED_GPFSEL] |= (1 << PWR_LED_GPFBIT);
    ACT_LED_ON();

    _uart_init(115200, 8, AUX_MU_RX_IRQ_ENABLE);
    _unlock_pf_mutex();

    // load the number of counts between interrupts. 
    timer->load = SD_T_STEP;

    // set up the arm timer. 23 bits, enable it, enable interrupt generation, prescaler of 1.
    timer->control = 
            RPI_ARMTIMER_CTRL_23BIT |
            RPI_ARMTIMER_CTRL_ENABLE |
            RPI_ARMTIMER_CTRL_INT_ENABLE |
            RPI_ARMTIMER_CTRL_PRESCALE_1;
    
    // enable IRQ to trigger on UART rx
    irq_ctrl->enable_IRQ_1 |= (1 << 29);

    irq_ctrl->enable_IRQ_basic |= RPI_BASIC_ARM_TIMER_IRQ;

    _enable_irq();

    printf("Initialized\n");

    ACT_LED_OFF();
}

void main2() {
    _init_core();

    ACT_LED_ON();

    while(1) {
        _lock_pf_mutex();
        printf("printing from %d\n", (int)get_core_id());
        _unlock_pf_mutex();
        waitcnt32(CNT32() + CLKFREQ/4);
        ACT_LED_OFF();
        waitcnt32(CNT32() + CLKFREQ/4);
        ACT_LED_ON();
    }
}

void kernel_main() {
    _init(); 

    start_stepper_driver(CORE1_ADR, CORE2_ADR);

    start_core(main2, CORE1_ADR);

    while(1) {
        _lock_pf_mutex();
        printf("printing from %d\n", (int)get_core_id());
        _unlock_pf_mutex();
        waitcnt32(CNT32() + CLKFREQ);
    }
}