/* generated vector header file - do not edit */
#ifndef VECTOR_DATA_H
#define VECTOR_DATA_H
#ifdef __cplusplus
extern "C" {
#endif
/* Number of interrupts allocated */
#ifndef VECTOR_DATA_IRQ_COUNT
#define VECTOR_DATA_IRQ_COUNT (10)
#endif
/* ISR prototypes */
void sci_uart_rxi_isr(void);
void sci_uart_txi_isr(void);
void sci_uart_tei_isr(void);
void sci_uart_eri_isr(void);
void sci_spi_rxi_isr(void);
void sci_spi_txi_isr(void);
void sci_spi_tei_isr(void);
void sci_spi_eri_isr(void);
void gpt_counter_overflow_isr(void);

/* Vector table allocations */
#define VECTOR_NUMBER_SCI0_RXI                                                 \
    ((IRQn_Type)0) /* SCI0 RXI (Receive data full)                             \
                    */
#define SCI0_RXI_IRQn ((IRQn_Type)0) /* SCI0 RXI (Receive data full) */
#define VECTOR_NUMBER_SCI0_TXI                                                 \
    ((IRQn_Type)1) /* SCI0 TXI (Transmit data empty) */
#define SCI0_TXI_IRQn ((IRQn_Type)1) /* SCI0 TXI (Transmit data empty) */
#define VECTOR_NUMBER_SCI0_TEI ((IRQn_Type)2) /* SCI0 TEI (Transmit end) */
#define SCI0_TEI_IRQn ((IRQn_Type)2) /* SCI0 TEI (Transmit end) */
#define VECTOR_NUMBER_SCI0_ERI ((IRQn_Type)3) /* SCI0 ERI (Receive error) */
#define SCI0_ERI_IRQn ((IRQn_Type)3) /* SCI0 ERI (Receive error) */
#define VECTOR_NUMBER_SCI2_RXI                                                 \
    ((IRQn_Type)4) /* SCI2 RXI (Received data full) */
#define SCI2_RXI_IRQn ((IRQn_Type)4) /* SCI2 RXI (Received data full) */
#define VECTOR_NUMBER_SCI2_TXI                                                 \
    ((IRQn_Type)5) /* SCI2 TXI (Transmit data empty) */
#define SCI2_TXI_IRQn ((IRQn_Type)5) /* SCI2 TXI (Transmit data empty) */
#define VECTOR_NUMBER_SCI2_TEI ((IRQn_Type)6) /* SCI2 TEI (Transmit end) */
#define SCI2_TEI_IRQn ((IRQn_Type)6) /* SCI2 TEI (Transmit end) */
#define VECTOR_NUMBER_SCI2_ERI ((IRQn_Type)7) /* SCI2 ERI (Receive error) */
#define SCI2_ERI_IRQn ((IRQn_Type)7) /* SCI2 ERI (Receive error) */
#define VECTOR_NUMBER_GPT0_COUNTER_OVERFLOW                                    \
    ((IRQn_Type)8) /* GPT0 COUNTER OVERFLOW (Overflow) */
#define GPT0_COUNTER_OVERFLOW_IRQn                                             \
    ((IRQn_Type)8) /* GPT0 COUNTER OVERFLOW (Overflow) */
#define VECTOR_NUMBER_GPT1_COUNTER_OVERFLOW                                    \
    ((IRQn_Type)9) /* GPT1 COUNTER OVERFLOW (Overflow) */
#define GPT1_COUNTER_OVERFLOW_IRQn                                             \
    ((IRQn_Type)9) /* GPT1 COUNTER OVERFLOW (Overflow) */
#ifdef __cplusplus
}
#endif
#endif /* VECTOR_DATA_H */