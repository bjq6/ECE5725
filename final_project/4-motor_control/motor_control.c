#include <stdint.h>
#include <stdio.h>

#include "bcm/bcm-includes.h"

#include "stepper_driver.h"

volatile uint32_t *gpio;
volatile rpi_arm_timer_t *timer;
volatile rpi_irq_controller_t *irq_ctrl;
volatile aux_t *aux;


volatile unsigned int c1_stack[1024];
register int sp asm ("sp");

int x = 0;

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

    _uart_tx('-');
    _uart_tx('e');
    _uart_tx(48+get_core_id());
    _uart_tx('-');
    _uart_tx('\n');
    
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
    gpio[ACT_LED_GPFSEL] &= ~(7 << ACT_LED_GPFBIT);
    gpio[ACT_LED_GPFSEL] |= (1 << ACT_LED_GPFBIT);
    gpio[PWR_LED_GPFSEL] &= ~(7 << PWR_LED_GPFBIT);
    gpio[PWR_LED_GPFSEL] |= (1 << PWR_LED_GPFBIT);

    _uart_init(115200, 8, AUX_MU_RX_IRQ_ENABLE);

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

    printf("\n-----------------\n");
}

void __attribute__ ((naked)) main2() {
    _init_sp((unsigned int *)CORE1_STACK);
    _init_core();

    //PWR_LED_ON();
    waitcnt32(CLKFREQ*5 + CNT32());
    printf("Starting motion controller\n");

    axis_t *x = get_x_axis();

    uint32_t loop_t = 0;
    const uint32_t loop_freq = 10;
    int i = 0;
    while(1) {
        loop_t = CNT32();
        set_target(x, i++, 30/loop_freq);

        waitcnt32(loop_t + CLKFREQ/loop_freq);
    }
}

void __attribute__ ((naked)) kernel_main() {
    _init();

    ACT_LED_ON();

    start_core(main2, CORE2_ADR);

    pin_setup();
    sd_main();

}