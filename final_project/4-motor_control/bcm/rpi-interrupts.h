#ifndef RPI_INTERRUPTS_H
#define RPI_INTERRUPTS_H

#include <stdint.h>

#include "rpi-base.h"

/** @brief See Section 7.5 of the BCM2836 ARM Peripherals documentation, the base
    address of the controller is actually xxxxB000, but there is a 0x200 offset
    to the first addressable register for the interrupt controller, so offset the
    base to the first register */
#define RPI_INTERRUPT_CONTROLLER_BASE   (PERIPHERAL_BASE + 0xB200)

/** @brief Bits in the Enable_Basic_IRQs register to enable various interrupts.
    See the BCM2835 ARM Peripherals manual, section 7.5 */
#define RPI_BASIC_ARM_TIMER_IRQ         (1 << 0)
#define RPI_BASIC_ARM_MAILBOX_IRQ       (1 << 1)
#define RPI_BASIC_ARM_DOORBELL_0_IRQ    (1 << 2)
#define RPI_BASIC_ARM_DOORBELL_1_IRQ    (1 << 3)
#define RPI_BASIC_GPU_0_HALTED_IRQ      (1 << 4)
#define RPI_BASIC_GPU_1_HALTED_IRQ      (1 << 5)
#define RPI_BASIC_ACCESS_ERROR_1_IRQ    (1 << 6)
#define RPI_BASIC_ACCESS_ERROR_0_IRQ    (1 << 7)

#define IRQ() void c_irq_handler()
#define HANG() void c_hang()

/** @brief The interrupt controller memory mapped register set */
typedef struct {
    volatile uint32_t IRQ_pending_basic;
    volatile uint32_t IRQ_pending_1;
    volatile uint32_t IRQ_pending_2;
    volatile uint32_t FIQ_control;
    volatile uint32_t enable_IRQ_1;
    volatile uint32_t enable_IRQ_2;
    volatile uint32_t enable_IRQ_basic;
    volatile uint32_t disable_IRQ_1;
    volatile uint32_t disable_IRQ_2;
    volatile uint32_t disable_IRQ_basic;
    } rpi_irq_controller_t;


extern rpi_irq_controller_t* _get_irq_controller(void);

#endif
