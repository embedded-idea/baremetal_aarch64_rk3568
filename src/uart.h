#ifndef _UART_H
#define _UART_H
/*
 * For driver model we always use one byte per register, and sort out the
 * differences in the driver
 */
#define CONFIG_SYS_NS16550_REG_SIZE (-1)

#define UART_REG(x)							\
	unsigned char x;						\
	unsigned char postpad_##x[-CONFIG_SYS_NS16550_REG_SIZE - 1];




#define thr rbr
#define iir fcr
#define dll rbr
#define dlm ier

struct NS16550 {
	UART_REG(rbr);		/* 0 */
	UART_REG(ier);		/* 1 */
	UART_REG(fcr);		/* 2 */
	UART_REG(lcr);		/* 3 */
	UART_REG(mcr);		/* 4 */
	UART_REG(lsr);		/* 5 */
	UART_REG(msr);		/* 6 */
	UART_REG(spr);		/* 7 */
	UART_REG(mdr1);		/* 8 */
	UART_REG(reg9);		/* 9 */
	UART_REG(regA);		/* A */
	UART_REG(regB);		/* B */
	UART_REG(regC);		/* C */
	UART_REG(regD);		/* D */
	UART_REG(regE);		/* E */
	UART_REG(uasr);		/* F */
	UART_REG(scr);		/* 10*/
	UART_REG(ssr);		/* 11*/

};
typedef struct NS16550 *NS16550_t;

void board_debug_uart_init(void);
void _debug_uart_init(void);
void _debug_uart_putc(int ch);


int _debug_uart_getc(void);
int _debug_uart_tstc(int input);
int _debug_uart_clrc(void);
int _debug_uart_flushc(void);
/* baudrate*/
int _debug_uart_setbrg(void);
void uart_puts(const char *str);
#endif
