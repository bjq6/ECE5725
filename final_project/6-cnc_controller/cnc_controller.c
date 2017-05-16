#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "interpreter/parser.h"
#include "interpreter/nuts_bolts.h"
#include "interpreter/queue.h"

#include "bcm/bcm-includes.h"

#include "stepper_driver.h"

#define SPEED_DIV 2.5

volatile uint32_t *gpio;
volatile rpi_arm_timer_t *timer;
volatile rpi_irq_controller_t *irq_ctrl;
volatile aux_t *aux;

volatile uint8_t uart_buffer[80] = {0};
volatile uint8_t buf_ptr = 0;

IRQ() {
    if (timer->maskedIRQ == 1) { // timer ISR 
        sd_IRQ();
    } else if ((aux->MU_IIR & 0x6) == 4) { // UART ISR
        uint8_t c = _uart_rx(); // get it out of the buffer to clear the interrupt
        uart_buffer[buf_ptr++] = c;
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
    buf_ptr = 0;

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

    _enable_irq();

    printf("\n-----------------\n");
}

void iface_main() {
    while(1) {
        if (buf_ptr) {
            printf("%s\n", (char*)uart_buffer);
            waitcnt32(CNT32() + CLKFREQ/5);
        }
    }
}

void mc_main() {
    while(get_sd_state() != SD_READY);

    printf("Starting path planner\n");
    axis_t *x = get_x_axis();
    axis_t *y = get_y_axis();

    set_target(10, 10, 0.5);
    while(motion_active());

    printf("At starting position (%f, %f)\n", x->pos, y->pos);

    char *buf = "G00 X20 Y10\0G01 X40 Y40\0G01 X60 Y40\0G01 X60 Y60\0End of File";

    char *gcode_file[80];

    gcode_file[0] = buf;
    gcode_file[1] = buf + 12;
    gcode_file[2] = buf + 24;
    gcode_file[3] = buf + 36;
    gcode_file[4] = buf + 48;

    int n = 0;

    queue pos_q;
    queue speed_q;
    init_q(&pos_q);
    init_q(&speed_q);

    //uint32_t loop_t = 0;
    //vector pos0 = {10, 10, 0};

    while (strcmp( gcode_file[n], "End of File" ) != 0) {
        // Initializations
        int g_code = -1;
        float f_val =   2, r_val = 0;
        vector victor = { 0, 0, 0 };
        //vector pos = pos0;

        // Populate Initializations from current GCode command
        read_line((char*)gcode_file[n], &g_code, &f_val, &r_val, &victor);

        switch(g_code){
            case 0: 
            case 1: 
                //process_linear(&pos_q, &speed_q, f_val, &victor, &pos);
                set_target(victor.x, victor.y, f_val/SPEED_DIV);
                break;
            case 2: 
            case 3: 
                break;
            default: 
                printf("Weird... I thought this would have been caught earlier.\n");
        }

        // process queue to get motion
        // while (!isEmpty(&pos_q)) {
        //     loop_t = CNT32();
        //     vector p = removeData(&pos_q);
        //     set_target(p.x, p.y, f_val/SPEED_DIV);

        //     printf("next step = (%f, %f)\n", p.x, p.y);
        //     waitcnt32(loop_t + time_to_cut*CLKFREQ);
        // }

        while(motion_active());

        //set_target(victor.x, victor.y, f_val/SPEED_DIV);
        //pos0.x = victor.x;
        //pos0.y = victor.y;

        printf("%s\n", gcode_file[n]); 
        n++;

        
    }
    printf("Done\n");

    while(1);
}

void __attribute__ ((naked)) mc_entry() {
    _init_sp((unsigned int *)CORE1_STACK);
    _init_core();

    mc_main();
}

void __attribute__ ((naked)) iface_entry() {
    _init_sp((unsigned int *)CORE2_STACK);
    _init_core();

    iface_main();
}

void __attribute__ ((naked)) kernel_main() {
    _init();

    ACT_LED_ON();

    start_core(mc_entry, CORE1_ADR);
    //start_core(iface_entry, CORE2_ADR);

    sd_main();

}