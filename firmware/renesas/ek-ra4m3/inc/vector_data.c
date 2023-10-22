/* generated vector source file - do not edit */
#include "bsp_api.h"
/* Do not build these data structures if no interrupts are currently allocated
 * because IAR will have build errors. */
#if VECTOR_DATA_IRQ_COUNT > 0
BSP_DONT_REMOVE const fsp_vector_t
    g_vector_table[BSP_ICU_VECTOR_MAX_ENTRIES] BSP_PLACE_IN_SECTION(
        BSP_SECTION_APPLICATION_VECTORS) = {
        [0] = sci_uart_rxi_isr,         /* SCI0 RXI (Receive data full) */
        [1] = sci_uart_txi_isr,         /* SCI0 TXI (Transmit data empty) */
        [2] = sci_uart_tei_isr,         /* SCI0 TEI (Transmit end) */
        [3] = sci_uart_eri_isr,         /* SCI0 ERI (Receive error) */
        [4] = sci_spi_rxi_isr,          /* SCI2 RXI (Received data full) */
        [5] = sci_spi_txi_isr,          /* SCI2 TXI (Transmit data empty) */
        [6] = sci_spi_tei_isr,          /* SCI2 TEI (Transmit end) */
        [7] = sci_spi_eri_isr,          /* SCI2 ERI (Receive error) */
        [8] = gpt_counter_overflow_isr, /* GPT0 COUNTER OVERFLOW (Overflow) */
        [9] = gpt_counter_overflow_isr, /* GPT1 COUNTER OVERFLOW (Overflow) */
};
const bsp_interrupt_event_t
    g_interrupt_event_link_select[BSP_ICU_VECTOR_MAX_ENTRIES] = {
        [0] = BSP_PRV_IELS_ENUM(
            EVENT_SCI0_RXI), /* SCI0 RXI (Receive data full) */
        [1] = BSP_PRV_IELS_ENUM(
            EVENT_SCI0_TXI), /* SCI0 TXI (Transmit data empty) */
        [2] = BSP_PRV_IELS_ENUM(EVENT_SCI0_TEI), /* SCI0 TEI (Transmit end) */
        [3] = BSP_PRV_IELS_ENUM(EVENT_SCI0_ERI), /* SCI0 ERI (Receive error) */
        [4] = BSP_PRV_IELS_ENUM(
            EVENT_SCI2_RXI), /* SCI2 RXI (Received data full) */
        [5] = BSP_PRV_IELS_ENUM(
            EVENT_SCI2_TXI), /* SCI2 TXI (Transmit data empty) */
        [6] = BSP_PRV_IELS_ENUM(EVENT_SCI2_TEI), /* SCI2 TEI (Transmit end) */
        [7] = BSP_PRV_IELS_ENUM(EVENT_SCI2_ERI), /* SCI2 ERI (Receive error) */
        [8] = BSP_PRV_IELS_ENUM(
            EVENT_GPT0_COUNTER_OVERFLOW), /* GPT0 COUNTER OVERFLOW (Overflow) */
        [9] = BSP_PRV_IELS_ENUM(
            EVENT_GPT1_COUNTER_OVERFLOW), /* GPT1 COUNTER OVERFLOW (Overflow) */
};
#endif