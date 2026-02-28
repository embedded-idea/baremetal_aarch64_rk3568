#include <stdint.h>

typedef signed char s8;
typedef unsigned char u8;

typedef signed short s16;
typedef unsigned short u16;

typedef signed int s32;
typedef unsigned int u32;

typedef signed long long s64;
typedef unsigned long long u64;

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
	struct rockchip_gpio_regs *regs = GPIO0_BASE;
	CLRBITS_LE32(&regs->swport_ddr, OFFSET_TO_BIT(offset));
	return 0;
}

static int rockchip_gpio_direction_output(unsigned offset, int value)
{
	struct rockchip_gpio_regs *regs = GPIO0_BASE;
	int mask = OFFSET_TO_BIT(offset);
	CLRSETBITS_LE32(&regs->swport_dr, mask, value ? mask : 0);
	SETBITS_LE32(&regs->swport_ddr, mask);
	return 0;
}

static int rockchip_gpio_get_value(unsigned offset)
{
	struct rockchip_gpio_regs *regs = GPIO0_BASE;
	return readl(&regs->ext_port) & OFFSET_TO_BIT(offset) ? 1 : 0;
}

static int rockchip_gpio_set_value(unsigned offset, int value)
{

	struct rockchip_gpio_regs *regs = GPIO0_BASE;
	int mask = OFFSET_TO_BIT(offset);
	CLRSETBITS_LE32(&regs->swport_dr, mask, value ? mask : 0);
	return 0;
}


// delay function match with rk3568 1.8GHz
// count 90000000 is about 500ms ?
static void delay(uint64_t count) {
    for (volatile uint64_t i = 0; i < count; i++);
}

int main(int argc, char *argv[])
{
    rockchip_gpio_direction_output(23,1);
    while (1) {
        delay(90000000);  // delay about 500ms
        rockchip_gpio_set_value(23, 0);
        delay(90000000);  // delay about 500ms
        rockchip_gpio_set_value(23, 1);
    }
    return 0;
}