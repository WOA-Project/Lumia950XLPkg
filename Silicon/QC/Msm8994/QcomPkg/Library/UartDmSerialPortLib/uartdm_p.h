#ifndef _UARTDM_PRIVATE_H
#define _UARTDM_PRIVATE_H

#include "uart_dm.h"

extern uint32_t g_uart_dm_base;

void uart_dm_init(uint8_t id, uint32_t gsbi_base, uint32_t uart_dm_base);
int  uart_putc(char c);
int  uart_getc(uint8_t *byte, bool wait);
int  uart_tstc(void);

#endif // _UARTDM_PRIVATE_H
