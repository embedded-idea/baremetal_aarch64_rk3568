#include <stdint.h>
#include "start_main.h"
#include "uart.h"
#include "minixs_head.h"

#define GPIO0_BASE        0xfdd60000  // GPIO0 物理基地址

#define OFFSET_TO_BIT(bit)	(1UL << (bit))

#define REG_L(R)	(R##_l)
#define REG_H(R)	(R##_h)
#define READ_REG(REG)	((readl(REG_L(REG)) & 0xFFFF) | \
			((readl(REG_H(REG)) & 0xFFFF) << 16))
#define WRITE_REG(REG, VAL)	\
{\
	writel(((VAL) & 0xFFFF) | 0xFFFF0000, REG_L(REG)); \
	writel((((VAL) & 0xFFFF0000) >> 16) | 0xFFFF0000, REG_H(REG));\
}
#define CLRBITS_LE32(REG, MASK)	WRITE_REG(REG, READ_REG(REG) & ~(MASK))
#define SETBITS_LE32(REG, MASK)	WRITE_REG(REG, READ_REG(REG) | (MASK))
#define CLRSETBITS_LE32(REG, MASK, VAL)	WRITE_REG(REG, \
				(READ_REG(REG) & ~(MASK)) | (VAL))

struct rockchip_gpio_regs {
	u32 swport_dr_l;                        /* ADDRESS OFFSET: 0x0000 */
	u32 swport_dr_h;                        /* ADDRESS OFFSET: 0x0004 */
	u32 swport_ddr_l;                       /* ADDRESS OFFSET: 0x0008 */
	u32 swport_ddr_h;                       /* ADDRESS OFFSET: 0x000c */
	u32 int_en_l;                           /* ADDRESS OFFSET: 0x0010 */
	u32 int_en_h;                           /* ADDRESS OFFSET: 0x0014 */
	u32 int_mask_l;                         /* ADDRESS OFFSET: 0x0018 */
	u32 int_mask_h;                         /* ADDRESS OFFSET: 0x001c */
	u32 int_type_l;                         /* ADDRESS OFFSET: 0x0020 */
	u32 int_type_h;                         /* ADDRESS OFFSET: 0x0024 */
	u32 int_polarity_l;                     /* ADDRESS OFFSET: 0x0028 */
	u32 int_polarity_h;                     /* ADDRESS OFFSET: 0x002c */
	u32 int_bothedge_l;                     /* ADDRESS OFFSET: 0x0030 */
	u32 int_bothedge_h;                     /* ADDRESS OFFSET: 0x0034 */
	u32 debounce_l;                         /* ADDRESS OFFSET: 0x0038 */
	u32 debounce_h;                         /* ADDRESS OFFSET: 0x003c */
	u32 dbclk_div_en_l;                     /* ADDRESS OFFSET: 0x0040 */
	u32 dbclk_div_en_h;                     /* ADDRESS OFFSET: 0x0044 */
	u32 dbclk_div_con;                      /* ADDRESS OFFSET: 0x0048 */
	u32 reserved004c;                       /* ADDRESS OFFSET: 0x004c */
	u32 int_status;                         /* ADDRESS OFFSET: 0x0050 */
	u32 reserved0054;                       /* ADDRESS OFFSET: 0x0054 */
	u32 int_rawstatus;                      /* ADDRESS OFFSET: 0x0058 */
	u32 reserved005c;                       /* ADDRESS OFFSET: 0x005c */
	u32 port_eoi_l;                         /* ADDRESS OFFSET: 0x0060 */
	u32 port_eoi_h;                         /* ADDRESS OFFSET: 0x0064 */
	u32 reserved0068[2];                    /* ADDRESS OFFSET: 0x0068 */
	u32 ext_port;                           /* ADDRESS OFFSET: 0x0070 */
	u32 reserved0074;                       /* ADDRESS OFFSET: 0x0074 */
	u32 ver_id;                             /* ADDRESS OFFSET: 0x0078 */
};
static inline void writel(u32 val, void *addr)
{
	*(volatile u32 *)addr = val;
}

static inline u32 readl(void *addr)
{
	return *(volatile u32 *)addr;
}
//offset is really confusion.
//GPIO0_A will be 0-7, GPIO0_B will be 8-15, GPIO0_C will be 16-23, GPIO0_D will be 24-31
static int rockchip_gpio_direction_input(unsigned offset)
{
	struct rockchip_gpio_regs *regs = (struct rockchip_gpio_regs *)GPIO0_BASE;
	CLRBITS_LE32(&regs->swport_ddr, OFFSET_TO_BIT(offset));
	return 0;
}

static int rockchip_gpio_direction_output(unsigned offset, int value)
{
	struct rockchip_gpio_regs *regs = (struct rockchip_gpio_regs *)GPIO0_BASE;
	int mask = OFFSET_TO_BIT(offset);
	CLRSETBITS_LE32(&regs->swport_dr, mask, value ? mask : 0);
	SETBITS_LE32(&regs->swport_ddr, mask);
	return 0;
}

static int rockchip_gpio_get_value(unsigned offset)
{
	struct rockchip_gpio_regs *regs = (struct rockchip_gpio_regs *)GPIO0_BASE;
	return readl(&regs->ext_port) & OFFSET_TO_BIT(offset) ? 1 : 0;
}

static int rockchip_gpio_set_value(unsigned offset, int value)
{

	struct rockchip_gpio_regs *regs = (struct rockchip_gpio_regs *)GPIO0_BASE;
	int mask = OFFSET_TO_BIT(offset);
	CLRSETBITS_LE32(&regs->swport_dr, mask, value ? mask : 0);
	return 0;
}


// delay function match with rk3568 1.8GHz
// count 90000000 is about 500ms ?
static void delay(uint64_t count) {
    for (volatile uint64_t i = 0; i < count; i++);
}

static void welcome()
{
    _debug_uart_putc('H');
    _debug_uart_putc('e');
    _debug_uart_putc('l');
    _debug_uart_putc('l');
    _debug_uart_putc('o');
    _debug_uart_putc('-');
    uart_puts("ARMv8-A (AArch64) Baremetal Program\n");
    uart_puts("UART: 1500000 8N1\n");
    uart_puts("press any key and enter to continue\n");
}
int main(int argc, char *argv[])
{
    int ch=0,ret=-5;
    rockchip_gpio_direction_output(23,1);
    board_debug_uart_init();
    _debug_uart_init();
    welcome();
    ch = _debug_uart_getc();
    // Echo the received character back to the UART
    _debug_uart_putc(ch);

    //call minix_init to jump to minix kernel
    ret = minix_init();
    if(ret != 0){
        uart_puts("fake minix_init failed, going to next while loop and halting.\n");
    }  
    else{
        uart_puts("fake minix_init returned successfully, but we are still in main.c, going to next while loop and halting.\n");
    }

    while (1) {
        delay(90000000);  // delay about 500ms
        rockchip_gpio_set_value(23, 0);
        delay(90000000);  // delay about 500ms
        rockchip_gpio_set_value(23, 1);
        _debug_uart_putc('o');
    }
    return 0;
}