#include "main.h"
#include "uart.h"

#define CONFIG_DEBUG_UART_BASE 0xfe660000
#define CONFIG_DEBUG_UART_CLOCK 24000000
#define CONFIG_BAUDRATE 1500000
#define CONFIG_SYS_NS16550_IER  0x00

#define UART_MCR_DTR	0x01		/* DTR   */
#define UART_MCR_RTS	0x02		/* RTS   */

/* useful defaults for LCR */
#define UART_LCR_8N1	0x03
#define UART_LCRVAL UART_LCR_8N1		/* 8 data, 1 stop, no parity */
#define UART_MCRVAL (UART_MCR_DTR | \
		     UART_MCR_RTS)		/* RTS/DTR */

/*
 * These are the definitions for the FIFO Control Register
 */
#define UART_FCR_FIFO_EN	0x01 /* Fifo enable */

#define UART_FCR_RXSR		0x02 /* Receiver soft reset */
#define UART_FCR_TXSR		0x04 /* Transmitter soft reset */

/* Clear & enable FIFOs */
#define UART_FCR_DEFVAL (UART_FCR_FIFO_EN | \
			UART_FCR_RXSR |	\
			UART_FCR_TXSR)
#define UART_LCR_BKSE	0x80		/* Bank select enable */
#define UART_LSR_THRE	0x20		/* Xmit holding register empty */
#define UART_LSR_DR	0x01		/* Data ready */

#define UART_FCR_CLEAR_RCVR	0x02 /* Clear the RCVR FIFO */
#define UART_FCR_CLEAR_XMIT	0x04 /* Clear the XMIT FIFO */




/* pin mux setting */
#define GRF_BASE		0xfdc60000
#define PMUGRF_BASE		0xfdc20000
#define rk_clrsetreg(addr, clr, set)	\
				writel((((clr) | (set)) << 16) | (set), addr)

#define BITS_PER_LONG 64
#define BITS_PER_LONG_LONG 64
/*
 * Create a contiguous bitmask starting at bit position @l and ending at
 * position @h. For example
 * GENMASK_ULL(39, 21) gives us the 64bit vector 0x000000ffffe00000.
 */
#define GENMASK(h, l) \
	(((~0UL) << (l)) & (~0UL >> (BITS_PER_LONG - 1 - (h))))

#define GENMASK_ULL(h, l) \
	(((~0ULL) << (l)) & (~0ULL >> (BITS_PER_LONG_LONG - 1 - (h))))

#define PMUGRF_BASE		0xfdc20000
#define GRF_BASE		0xfdc60000
#define GRF_GPIO1B_IOMUX_H	0x0C
#define GRF_GPIO1C_IOMUX_L	0x10
#define GRF_GPIO1C_IOMUX_H	0x14
#define GRF_GPIO1D_IOMUX_L	0x18
#define GRF_GPIO1D_IOMUX_H	0x1C
#define GRF_GPIO1B_DS_2		0x218
#define GRF_GPIO1B_DS_3		0x21c
#define GRF_GPIO1C_DS_0		0x220
#define GRF_GPIO1C_DS_1		0x224
#define GRF_GPIO1C_DS_2		0x228
#define GRF_GPIO1C_DS_3		0x22c
#define GRF_GPIO1D_DS_0		0x230
#define GRF_GPIO1D_DS_1		0x234
#define GRF_GPIO1D_DS_2		0x238
#define GRF_SOC_CON4		0x510
#define PMU_BASE_ADDR		0xfdd90000
#define PMU_NOC_AUTO_CON0	(0x70)
#define PMU_NOC_AUTO_CON1	(0x74)
#define CRU_BASE		0xfdd20000
#define CRU_SOFTRST_CON26	0x468
#define CRU_SOFTRST_CON28	0x470
#define SGRF_BASE		0xFDD18000
#define SGRF_SOC_CON3		0xC
#define SGRF_SOC_CON4		0x10
#define PMUGRF_SOC_CON15	0xfdc20100
#define CPU_GRF_BASE		0xfdc30000
#define GRF_CORE_PVTPLL_CON0	(0x10)
#define USBPHY_U3_GRF		0xfdca0000
#define USBPHY_U3_GRF_CON1	(USBPHY_U3_GRF + 0x04)
#define USBPHY_U2_GRF		0xfdca8000
#define USBPHY_U2_GRF_CON0	(USBPHY_U2_GRF + 0x00)
#define USBPHY_U2_GRF_CON1	(USBPHY_U2_GRF + 0x04)

#define PMU_PWR_GATE_SFTCON	(0xA0)
#define PMU_PWR_DWN_ST		(0x98)
#define PMU_BUS_IDLE_SFTCON0	(0x50)
#define PMU_BUS_IDLE_ST		(0x68)
#define PMU_BUS_IDLE_ACK	(0x60)

#define EBC_PRIORITY_REG	(0xfe158008)






#define CONFIG_DEBUG_UART_SHIFT 2

#define serial_dout(reg, value)	\
	serial_out_shift((char *)com_port + \
		((char *)reg - (char *)com_port) * \
			(1 << CONFIG_DEBUG_UART_SHIFT), \
		CONFIG_DEBUG_UART_SHIFT, value)
#define serial_din(reg) \
	serial_in_shift((char *)com_port + \
		((char *)reg - (char *)com_port) * \
			(1 << CONFIG_DEBUG_UART_SHIFT), \
		CONFIG_DEBUG_UART_SHIFT)

#define writeb(b,addr) (*(volatile unsigned char *) (addr) = (b))
// #define writew(b,addr) (*(volatile unsigned short *) (addr) = (b))
// #define writel(b,addr) (*(volatile unsigned int *) (addr) = (b))

#define readb(addr) (*(volatile unsigned char *) (addr))
// #define readw(addr) (*(volatile unsigned short *) (addr))
// #define readl(addr) (*(volatile unsigned int *) (addr))

/*
 * Divide positive or negative dividend by positive divisor and round
 * to closest integer. Result is undefined for negative divisors and
 * for negative dividends if the divisor variable type is unsigned.
 */
#define DIV_ROUND_CLOSEST(x, divisor)(			\
{							\
	typeof(x) __x = x;				\
	typeof(divisor) __d = divisor;			\
	(((typeof(x))-1) > 0 ||				\
	 ((typeof(divisor))-1) > 0 || (__x) > 0) ?	\
		(((__x) + ((__d) / 2)) / (__d)) :	\
		(((__x) - ((__d) / 2)) / (__d));	\
}							\
)



enum {
	/* PMU_GRF_GPIO0C_IOMUX_L */
	GPIO0C1_SHIFT		= 4,
	GPIO0C1_MASK		= GENMASK(6, 4),
	GPIO0C1_GPIO		= 0,
	GPIO0C1_PWM2_M0,
	GPIO0C1_NPU_AVS,
	GPIO0C1_UART0_TX,
	GPIO0C1_MCU_JTAGTDI,

	GPIO0C0_SHIFT		= 0,
	GPIO0C0_MASK		= GENMASK(2, 0),
	GPIO0C0_GPIO		= 0,
	GPIO0C0_PWM1_M0,
	GPIO0C0_GPU_AVS,
	GPIO0C0_UART0_RX,

	/* PMU_GRF_GPIO0D_IOMUX_L */
	GPIO0D1_SHIFT		= 4,
	GPIO0D1_MASK		= GENMASK(6, 4),
	GPIO0D1_GPIO		= 0,
	GPIO0D1_UART2_TXM0,

	GPIO0D0_SHIFT		= 0,
	GPIO0D0_MASK		= GENMASK(2, 0),
	GPIO0D0_GPIO		= 0,
	GPIO0D0_UART2_RXM0,

	/* PMU_GRF_SOC_CON0 */
	UART0_IO_SEL_SHIFT	= 8,
	UART0_IO_SEL_MASK	= GENMASK(9, 8),
	UART0_IO_SEL_M0		= 0,
	UART0_IO_SEL_M1,
	UART0_IO_SEL_M2,
};
enum {
	/* GRF_GPIO1A_IOMUX_L */
	GPIO1A1_SHIFT		= 4,
	GPIO1A1_MASK		= GENMASK(6, 4),
	GPIO1A1_GPIO		= 0,
	GPIO1A1_I2C3_SCLM0,
	GPIO1A1_UART3_TXM0,
	GPIO1A1_CAN1_TXM0,
	GPIO1A1_AUDIOPWM_ROUT,
	GPIO1A1_ACODEC_ADCCLK,
	GPIO1A1_AUDIOPWM_LOUT,

	GPIO1A0_SHIFT		= 0,
	GPIO1A0_MASK		= GENMASK(2, 0),
	GPIO1A0_GPIO		= 0,
	GPIO1A0_I2C3_SDAM0,
	GPIO1A0_UART3_RXM0,
	GPIO1A0_CAN1_RXM0,
	GPIO1A0_AUDIOPWM_LOUT,
	GPIO1A0_ACODEC_ADCDATA,
	GPIO1A0_AUDIOPWM_LOUTP,

	/* GRF_GPIO1A_IOMUX_H */
	GPIO1A6_SHIFT		= 8,
	GPIO1A6_MASK		= GENMASK(10, 8),
	GPIO1A6_GPIO		= 0,
	GPIO1A6_I2S1_LRCKRXM0,
	GPIO1A6_UART4_TXM0,
	GPIO1A6_PDM_CLK0M0,
	GPIO1A6_AUDIOPWM_ROUTP,

	GPIO1A4_SHIFT		= 0,
	GPIO1A4_MASK		= GENMASK(2, 0),
	GPIO1A4_GPIO		= 0,
	GPIO1A4_I2S1_SCLKRXM0,
	GPIO1A4_UART4_RXM0,
	GPIO1A4_PDM_CLK1M0,
	GPIO1A4_SPDIF_TXM0,

	/* GRF_GPIO1D_IOMUX_H */
	GPIO1D6_SHIFT		= 8,
	GPIO1D6_MASK		= GENMASK(10, 8),
	GPIO1D6_GPIO		= 0,
	GPIO1D6_SDMMC0_D1,
	GPIO1D6_UART2_RXM1,
	GPIO1D6_UART6_RXM1,
	GPIO1D6_PWM9_M1,

	GPIO1D5_SHIFT		= 4,
	GPIO1D5_MASK		= GENMASK(6, 4),
	GPIO1D5_GPIO		= 0,
	GPIO1D5_SDMMC0_D0,
	GPIO1D5_UART2_TXM1,
	GPIO1D5_UART6_TXM1,
	GPIO1D5_PWM8_M1,

	/* GRF_GPIO2A_IOMUX_L */
	GPIO2A3_SHIFT		= 12,
	GPIO2A3_MASK		= GENMASK(14, 12),
	GPIO2A3_GPIO		= 0,
	GPIO2A3_SDMMC1_D0,
	GPIO2A3_GMAC0_RXD2,
	GPIO2A3_UART6_RXM0,

	GPIO2A2_SHIFT		= 8,
	GPIO2A2_MASK		= GENMASK(10, 8),
	GPIO2A2_GPIO		= 0,
	GPIO2A2_SDMMC0_CLK,
	GPIO2A2_TEST_CLKOUT,
	GPIO2A2_UART5_TXM0,
	GPIO2A2_CAN0_RXM1,

	GPIO2A1_SHIFT		= 4,
	GPIO2A1_MASK		= GENMASK(6, 4),
	GPIO2A1_GPIO		= 0,
	GPIO2A1_SDMMC0_CMD,
	GPIO2A1_PWM10_M1,
	GPIO2A1_UART5_RXM0,
	GPIO2A1_CAN0_TXM1,

	/* GRF_GPIO2A_IOMUX_H */
	GPIO2A7_SHIFT		= 12,
	GPIO2A7_MASK		= GENMASK(14, 12),
	GPIO2A7_GPIO		= 0,
	GPIO2A7_SDMMC1_CMD,
	GPIO2A7_GMAC0_TXD3,
	GPIO2A7_UART9_RXM0,

	GPIO2A6_SHIFT		= 8,
	GPIO2A6_MASK		= GENMASK(10, 8),
	GPIO2A6_GPIO		= 0,
	GPIO2A6_SDMMC1_D3,
	GPIO2A6_GMAC0_TXD2,
	GPIO2A6_UART7_TXM0,

	GPIO2A5_SHIFT		= 4,
	GPIO2A5_MASK		= GENMASK(6, 4),
	GPIO2A5_GPIO		= 0,
	GPIO2A5_SDMMC1_D2,
	GPIO2A5_GMAC0_RXCLK,
	GPIO2A5_UART7_RXM0,

	GPIO2A4_SHIFT		= 0,
	GPIO2A4_MASK		= GENMASK(2, 0),
	GPIO2A4_GPIO		= 0,
	GPIO2A4_SDMMC1_D1,
	GPIO2A4_GMAC0_RXD3,
	GPIO2A4_UART6_TXM0,

	/* GRF_GPIO2B_IOMUX_L */
	GPIO2B3_SHIFT		= 12,
	GPIO2B3_MASK		= GENMASK(14, 12),
	GPIO2B3_GPIO		= 0,
	GPIO2B3_GMAC0_TXD0,
	GPIO2B3_UART1_RXM0,

	GPIO2B0_SHIFT		= 0,
	GPIO2B0_MASK		= GENMASK(2, 0),
	GPIO2B0_GPIO		= 0,
	GPIO2B0_SDMMC1_CLK,
	GPIO2B0_GMAC0_TXCLK,
	GPIO2B0_UART9_TXM0,

	/* GRF_GPIO2B_IOMUX_H */
	GPIO2B4_SHIFT		= 0,
	GPIO2B4_MASK		= GENMASK(2, 0),
	GPIO2B4_GPIO		= 0,
	GPIO2B4_GMAC0_TXD1,
	GPIO2B4_UART1_TXM0,

	/* GRF_GPIO2C_IOMUX_L */
	GPIO2C2_SHIFT		= 8,
	GPIO2C2_MASK		= GENMASK(10, 8),
	GPIO2C2_GPIO		= 0,
	GPIO2C2_GMAC0_MCLKINOUT	= 2,

	/* GRF_GPIO2C_IOMUX_H */
	GPIO2C6_SHIFT		= 8,
	GPIO2C6_MASK		= GENMASK(10, 8),
	GPIO2C6_GPIO		= 0,
	GPIO2C6_CLK32K_OUT1,
	GPIO2C6_UART8_RXM0,
	GPIO2C6_SPI1_CS1M0,

	GPIO2C5_SHIFT		= 4,
	GPIO2C5_MASK		= GENMASK(6, 4),
	GPIO2C5_GPIO		= 0,
	GPIO2C5_I2S2_SDIM0,
	GPIO2C5_GMAC0_RXER,
	GPIO2C5_UART8_TXM0,
	GPIO2C5_SPI2_CS1M0,

	/* GRF_GPIO2D_IOMUX_H */
	GPIO2D7_SHIFT		= 12,
	GPIO2D7_MASK		= GENMASK(14, 12),
	GPIO2D7_GPIO		= 0,
	GPIO2D7_LCDC_D7,
	GPIO2D7_BT656_D7M0,
	GPIO2D7_SPI2_MISOM1,
	GPIO2D7_UART8_TXM1,
	GPIO2D7_I2S1_SDO0M2,

	/* GRF_GPIO3A_IOMUX_L */
	GPIO3A0_SHIFT		= 0,
	GPIO3A0_MASK		= GENMASK(2, 0),
	GPIO3A0_GPIO		= 0,
	GPIO3A0_LCDC_CLK,
	GPIO3A0_BT656_CLKM0,
	GPIO3A0_SPI2_CLKM1,
	GPIO3A0_UART8_RXM1,
	GPIO3A0_I2S1_SDO1M2,

	/* GRF_GPIO3B_IOMUX_L */
	GPIO3B2_SHIFT		= 8,
	GPIO3B2_MASK		= GENMASK(10, 8),
	GPIO3B2_GPIO		= 0,
	GPIO3B2_LCDC_D17,
	GPIO3B2_BT1120_D8,
	GPIO3B2_GMAC1_RXD1M0,
	GPIO3B2_UART4_TXM1,
	GPIO3B2_PWM9_M0,

	GPIO3B1_SHIFT		= 4,
	GPIO3B1_MASK		= GENMASK(6, 4),
	GPIO3B1_GPIO		= 0,
	GPIO3B1_LCDC_D16,
	GPIO3B1_BT1120_D7,
	GPIO3B1_GMAC1_RXD0M0,
	GPIO3B1_UART4_RXM1,
	GPIO3B1_PWM8_M0,

	/* GRF_GPIO3B_IOMUX_H */
	GPIO3B7_SHIFT		= 12,
	GPIO3B7_MASK		= GENMASK(14, 12),
	GPIO3B7_GPIO		= 0,
	GPIO3B7_LCDC_D22,
	GPIO3B7_PWM12_M0,
	GPIO3B7_GMAC1_TXENM0,
	GPIO3B7_UART3_TXM1,
	GPIO3B7_PDM_SDI2M2,

	/* GRF_GPIO3C_IOMUX_L */
	GPIO3C3_SHIFT		= 12,
	GPIO3C3_MASK		= GENMASK(14, 12),
	GPIO3C3_GPIO		= 0,
	GPIO3C3_LCDC_DEN,
	GPIO3C3_BT1120_D15,
	GPIO3C3_SPI1_CLKM1,
	GPIO3C3_UART5_RXM1,
	GPIO3C3_I2S1_SCLKRXM,

	GPIO3C2_SHIFT		= 8,
	GPIO3C2_MASK		= GENMASK(10, 8),
	GPIO3C2_GPIO		= 0,
	GPIO3C2_LCDC_VSYNC,
	GPIO3C2_BT1120_D14,
	GPIO3C2_SPI1_MISOM1,
	GPIO3C2_UART5_TXM1,
	GPIO3C2_I2S1_SDO3M2,

	GPIO3C0_SHIFT		= 0,
	GPIO3C0_MASK		= GENMASK(2, 0),
	GPIO3C0_GPIO		= 0,
	GPIO3C0_LCDC_D23,
	GPIO3C0_PWM13_M0,
	GPIO3C0_GMAC1_MCLKINOUTM0,
	GPIO3C0_UART3_RXM1,
	GPIO3C0_PDM_SDI3M2,

	/* GRF_GPIO3C_IOMUX_H */
	GPIO3C5_SHIFT		= 4,
	GPIO3C5_MASK		= GENMASK(6, 4),
	GPIO3C5_GPIO		= 0,
	GPIO3C5_PWM15_IRM0,
	GPIO3C5_SPDIF_TXM1,
	GPIO3C5_GMAC1_MDIOM0,
	GPIO3C5_UART7_RXM1,
	GPIO3C5_I2S1_LRCKRXM2,

	GPIO3C4_SHIFT		= 0,
	GPIO3C4_MASK		= GENMASK(2, 0),
	GPIO3C4_GPIO		= 0,
	GPIO3C4_PWM14_M0,
	GPIO3C4_VOP_PWMM1,
	GPIO3C4_GMAC1_MDCM0,
	GPIO3C4_UART7_TXM1,
	GPIO3C4_PDM_CLK1M2,

	/* GRF_GPIO3D_IOMUX_H */
	GPIO3D7_SHIFT		= 12,
	GPIO3D7_MASK		= GENMASK(14, 12),
	GPIO3D7_GPIO		= 0,
	GPIO3D7_CIF_D9,
	GPIO3D7_EBC_SDDO9,
	GPIO3D7_GMAC1_TXD3M1,
	GPIO3D7_UART1_RXM1,
	GPIO3D7_PDM_SDI0M1,

	GPIO3D6_SHIFT		= 8,
	GPIO3D6_MASK		= GENMASK(10, 8),
	GPIO3D6_GPIO		= 0,
	GPIO3D6_CIF_D8,
	GPIO3D6_EBC_SDDO8,
	GPIO3D6_GMAC1_TXD2M1,
	GPIO3D6_UART1_TXM1,
	GPIO3D6_PDM_CLK0M1,

	/* GRF_GPIO4A_IOMUX_L */
	GPIO4A3_SHIFT		= 12,
	GPIO4A3_MASK		= GENMASK(14, 12),
	GPIO4A3_GPIO		= 0,
	GPIO4A3_CIF_D13,
	GPIO4A3_EBC_SDDO13,
	GPIO4A3_GMAC1_RXCLKM1,
	GPIO4A3_UART7_RXM2,
	GPIO4A3_PDM_SDI3M1,

	GPIO4A2_SHIFT		= 8,
	GPIO4A2_MASK		= GENMASK(10, 8),
	GPIO4A2_GPIO		= 0,
	GPIO4A2_CIF_D12,
	GPIO4A2_EBC_SDDO12,
	GPIO4A2_GMAC1_RXD3M1,
	GPIO4A2_UART7_TXM2,
	GPIO4A2_PDM_SDI2M1,

	/* GRF_GPIO4A_IOMUX_H */
	GPIO4A5_SHIFT		= 4,
	GPIO4A5_MASK		= GENMASK(6, 4),
	GPIO4A5_GPIO		= 0,
	GPIO4A5_CIF_D15,
	GPIO4A5_EBC_SDDO15,
	GPIO4A5_GMAC1_TXD1M1,
	GPIO4A5_UART9_RXM2,
	GPIO4A5_I2S2_LRCKRXM1,

	GPIO4A4_SHIFT		= 0,
	GPIO4A4_MASK		= GENMASK(2, 0),
	GPIO4A4_GPIO		= 0,
	GPIO4A4_CIF_D14,
	GPIO4A4_EBC_SDDO14,
	GPIO4A4_GMAC1_TXD0M1,
	GPIO4A4_UART9_TXM2,
	GPIO4A4_I2S2_LRCKTXM1,

	/* GRF_GPIO4C_IOMUX_L */
	GPIO4C1_SHIFT		= 4,
	GPIO4C1_MASK		= GENMASK(6, 4),
	GPIO4C1_GPIO		= 0,
	GPIO4C1_CIF_CLKIN,
	GPIO4C1_EBC_SDCLK,
	GPIO4C1_GMAC1_MCLKINOUTM1,

	/* GRF_GPIO4C_IOMUX_H */
	GPIO4C6_SHIFT		= 8,
	GPIO4C6_MASK		= GENMASK(10, 8),
	GPIO4C6_GPIO		= 0,
	GPIO4C6_PWM13_M1,
	GPIO4C6_SPI3_CS0M1,
	GPIO4C6_SATA0_ACTLED,
	GPIO4C6_UART9_RXM1,
	GPIO4C6_I2S3_SDIM1,

	GPIO4C5_SHIFT		= 4,
	GPIO4C5_MASK		= GENMASK(6, 4),
	GPIO4C5_GPIO		= 0,
	GPIO4C5_PWM12_M1,
	GPIO4C5_SPI3_MISOM1,
	GPIO4C5_SATA1_ACTLED,
	GPIO4C5_UART9_TXM1,
	GPIO4C5_I2S3_SDOM1,

	/* GRF_IOFUNC_SEL3 */
	UART4_IO_SEL_SHIFT	= 14,
	UART4_IO_SEL_MASK	= GENMASK(14, 14),
	UART4_IO_SEL_M0		= 0,
	UART4_IO_SEL_M1,

	UART3_IO_SEL_SHIFT	= 12,
	UART3_IO_SEL_MASK	= GENMASK(12, 12),
	UART3_IO_SEL_M0		= 0,
	UART3_IO_SEL_M1,

	UART2_IO_SEL_SHIFT	= 10,
	UART2_IO_SEL_MASK	= GENMASK(11, 10),
	UART2_IO_SEL_M0		= 0,
	UART2_IO_SEL_M1,

	UART1_IO_SEL_SHIFT	= 8,
	UART1_IO_SEL_MASK	= GENMASK(8, 8),
	UART1_IO_SEL_M0		= 0,
	UART1_IO_SEL_M1,

	/* GRF_IOFUNC_SEL4 */
	UART9_IO_SEL_SHIFT	= 8,
	UART9_IO_SEL_MASK	= GENMASK(9, 8),
	UART9_IO_SEL_M0		= 0,
	UART9_IO_SEL_M1,
	UART9_IO_SEL_M2,

	UART8_IO_SEL_SHIFT	= 6,
	UART8_IO_SEL_MASK	= GENMASK(6, 6),
	UART8_IO_SEL_M0		= 0,
	UART8_IO_SEL_M1,

	UART7_IO_SEL_SHIFT	= 4,
	UART7_IO_SEL_MASK	= GENMASK(5, 4),
	UART7_IO_SEL_M0		= 0,
	UART7_IO_SEL_M1,
	UART7_IO_SEL_M2,

	UART6_IO_SEL_SHIFT	= 2,
	UART6_IO_SEL_MASK	= GENMASK(2, 2),
	UART6_IO_SEL_M0		= 0,
	UART6_IO_SEL_M1,

	UART5_IO_SEL_SHIFT	= 0,
	UART5_IO_SEL_MASK	= GENMASK(0, 0),
	UART5_IO_SEL_M0		= 0,
	UART5_IO_SEL_M1,
};

struct rk3568_grf {
	unsigned int gpio1a_iomux_l;
	unsigned int gpio1a_iomux_h;
	unsigned int gpio1b_iomux_l;
	unsigned int gpio1b_iomux_h;
	unsigned int gpio1c_iomux_l;
	unsigned int gpio1c_iomux_h;
	unsigned int gpio1d_iomux_l;
	unsigned int gpio1d_iomux_h;
	unsigned int gpio2a_iomux_l;
	unsigned int gpio2a_iomux_h;
	unsigned int gpio2b_iomux_l;
	unsigned int gpio2b_iomux_h;
	unsigned int gpio2c_iomux_l;
	unsigned int gpio2c_iomux_h;
	unsigned int gpio2d_iomux_l;
	unsigned int gpio2d_iomux_h;
	unsigned int gpio3a_iomux_l;
	unsigned int gpio3a_iomux_h;
	unsigned int gpio3b_iomux_l;
	unsigned int gpio3b_iomux_h;
	unsigned int gpio3c_iomux_l;
	unsigned int gpio3c_iomux_h;
	unsigned int gpio3d_iomux_l;
	unsigned int gpio3d_iomux_h;
	unsigned int gpio4a_iomux_l;
	unsigned int gpio4a_iomux_h;
	unsigned int gpio4b_iomux_l;
	unsigned int gpio4b_iomux_h;
	unsigned int gpio4c_iomux_l;
	unsigned int gpio4c_iomux_h;
	unsigned int gpio4d_iomux_l;
	unsigned int reserved0[(0x0080 - 0x0078) / 4 - 1];
	unsigned int gpio1a_p;
	unsigned int gpio1b_p;
	unsigned int gpio1c_p;
	unsigned int gpio1d_p;
	unsigned int gpio2a_p;
	unsigned int gpio2b_p;
	unsigned int gpio2c_p;
	unsigned int gpio2d_p;
	unsigned int gpio3a_p;
	unsigned int gpio3b_p;
	unsigned int gpio3c_p;
	unsigned int gpio3d_p;
	unsigned int gpio4a_p;
	unsigned int gpio4b_p;
	unsigned int gpio4c_p;
	unsigned int gpio4d_p;
	unsigned int gpio1a_ie;
	unsigned int gpio1b_ie;
	unsigned int gpio1c_ie;
	unsigned int gpio1d_ie;
	unsigned int gpio2a_ie;
	unsigned int gpio2b_ie;
	unsigned int gpio2c_ie;
	unsigned int gpio2d_ie;
	unsigned int gpio3a_ie;
	unsigned int gpio3b_ie;
	unsigned int gpio3c_ie;
	unsigned int gpio3d_ie;
	unsigned int gpio4a_ie;
	unsigned int gpio4b_ie;
	unsigned int gpio4c_ie;
	unsigned int gpio4d_ie;
	unsigned int gpio1a_opd;
	unsigned int gpio1b_opd;
	unsigned int gpio1c_opd;
	unsigned int gpio1d_opd;
	unsigned int gpio2a_opd;
	unsigned int gpio2b_opd;
	unsigned int gpio2c_opd;
	unsigned int gpio2d_opd;
	unsigned int gpio3a_opd;
	unsigned int gpio3b_opd;
	unsigned int gpio3c_opd;
	unsigned int gpio3d_opd;
	unsigned int gpio4a_opd;
	unsigned int gpio4b_opd;
	unsigned int gpio4c_opd;
	unsigned int gpio4d_opd;
	unsigned int gpio1a_sus;
	unsigned int gpio1b_sus;
	unsigned int gpio1c_sus;
	unsigned int gpio1d_sus;
	unsigned int gpio2a_sus;
	unsigned int gpio2b_sus;
	unsigned int gpio2c_sus;
	unsigned int gpio2d_sus;
	unsigned int gpio3a_sus;
	unsigned int gpio3b_sus;
	unsigned int gpio3c_sus;
	unsigned int gpio3d_sus;
	unsigned int gpio4a_sus;
	unsigned int gpio4b_sus;
	unsigned int gpio4c_sus;
	unsigned int gpio4d_sus;
	unsigned int gpio1a_sl;
	unsigned int gpio1b_sl;
	unsigned int gpio1c_sl;
	unsigned int gpio1d_sl;
	unsigned int gpio2a_sl;
	unsigned int gpio2b_sl;
	unsigned int gpio2c_sl;
	unsigned int gpio2d_sl;
	unsigned int gpio3a_sl;
	unsigned int gpio3b_sl;
	unsigned int gpio3c_sl;
	unsigned int gpio3d_sl;
	unsigned int gpio4a_sl;
	unsigned int gpio4b_sl;
	unsigned int gpio4c_sl;
	unsigned int gpio4d_sl;
	unsigned int reserved1[(0x0200 - 0x01bc) / 4 - 1];
	unsigned int gpio1a_ds_0;
	unsigned int gpio1a_ds_1;
	unsigned int gpio1a_ds_2;
	unsigned int gpio1a_ds_3;
	unsigned int gpio1b_ds_0;
	unsigned int gpio1b_ds_1;
	unsigned int gpio1b_ds_2;
	unsigned int gpio1b_ds_3;
	unsigned int gpio1c_ds_0;
	unsigned int gpio1c_ds_1;
	unsigned int gpio1c_ds_2;
	unsigned int gpio1c_ds_3;
	unsigned int gpio1d_ds_0;
	unsigned int gpio1d_ds_1;
	unsigned int gpio1d_ds_2;
	unsigned int gpio1d_ds_3;
	unsigned int gpio2a_ds_0;
	unsigned int gpio2a_ds_1;
	unsigned int gpio2a_ds_2;
	unsigned int gpio2a_ds_3;
	unsigned int gpio2b_ds_0;
	unsigned int gpio2b_ds_1;
	unsigned int gpio2b_ds_2;
	unsigned int gpio2b_ds_3;
	unsigned int gpio2c_ds_0;
	unsigned int gpio2c_ds_1;
	unsigned int gpio2c_ds_2;
	unsigned int gpio2c_ds_3;
	unsigned int gpio2d_ds_0;
	unsigned int gpio2d_ds_1;
	unsigned int gpio2d_ds_2;
	unsigned int gpio2d_ds_3;
	unsigned int gpio3a_ds_0;
	unsigned int gpio3a_ds_1;
	unsigned int gpio3a_ds_2;
	unsigned int gpio3a_ds_3;
	unsigned int gpio3b_ds_0;
	unsigned int gpio3b_ds_1;
	unsigned int gpio3b_ds_2;
	unsigned int gpio3b_ds_3;
	unsigned int gpio3c_ds_0;
	unsigned int gpio3c_ds_1;
	unsigned int gpio3c_ds_2;
	unsigned int gpio3c_ds_3;
	unsigned int gpio3d_ds_0;
	unsigned int gpio3d_ds_1;
	unsigned int gpio3d_ds_2;
	unsigned int gpio3d_ds_3;
	unsigned int gpio4a_ds_0;
	unsigned int gpio4a_ds_1;
	unsigned int gpio4a_ds_2;
	unsigned int gpio4a_ds_3;
	unsigned int gpio4b_ds_0;
	unsigned int gpio4b_ds_1;
	unsigned int gpio4b_ds_2;
	unsigned int gpio4b_ds_3;
	unsigned int gpio4c_ds_0;
	unsigned int gpio4c_ds_1;
	unsigned int gpio4c_ds_2;
	unsigned int gpio4c_ds_3;
	unsigned int gpio4d_ds_0;
	unsigned int gpio4d_ds_1;
	unsigned int gpio4d_ds_2;
	unsigned int gpio4d_ds_3;
	unsigned int iofunc_sel0;
	unsigned int iofunc_sel1;
	unsigned int iofunc_sel2;
	unsigned int iofunc_sel3;
	unsigned int iofunc_sel4;
	unsigned int iofunc_sel5;
	unsigned int reserved2[(0x0340 - 0x0314) / 4 - 1];
	unsigned int vi_con0;
	unsigned int vi_con1;
	unsigned int vi_status0;
	unsigned int reserved3[(0x0360 - 0x0348) / 4 - 1];
	unsigned int vo_con0;
	unsigned int vo_con1;
	unsigned int vo_con2;
	unsigned int vo_con3;
	unsigned int reserved4[(0x0380 - 0x036c) / 4 - 1];
	unsigned int mac0_con0;
	unsigned int mac0_con1;
	unsigned int mac1_con0;
	unsigned int mac1_con1;
	unsigned int reserved5[(0x03a0 - 0x038c) / 4 - 1];
	unsigned int biu_con0;
	unsigned int biu_con1;
	unsigned int biu_con2;
	unsigned int reserved6[(0x03c0 - 0x03a8) / 4 - 1];
	unsigned int gic_con0;
	unsigned int gic_con1;
	unsigned int gic_con2;
	unsigned int reserved7[(0x03f0 - 0x03c8) / 4 - 1];
	unsigned int gpu_con0;
	unsigned int gpu_con1;
	unsigned int reserved8[(0x0400 - 0x03f4) / 4 - 1];
	unsigned int cpu_con0;
	unsigned int reserved9[(0x0420 - 0x0400) / 4 - 1];
	unsigned int cpu_status0;
	unsigned int reserved10[(0x0500 - 0x0420) / 4 - 1];
	unsigned int soc_con0;
	unsigned int soc_con1;
	unsigned int soc_con2;
	unsigned int soc_con3;
	unsigned int reserved11[(0x0514 - 0x050c) / 4 - 1];
	unsigned int soc_con5;
	unsigned int soc_con6;
	unsigned int reserved12[(0x0580 - 0x0518) / 4 - 1];
	unsigned int soc_status0;
	unsigned int reserved13[(0x05c0 - 0x0580) / 4 - 1];
	unsigned int ram_con;
	unsigned int core_ram_con;
	unsigned int reserved14[(0x0600 - 0x05c4) / 4 - 1];
	unsigned int tsadc_con;
	unsigned int reserved15[(0x0610 - 0x0600) / 4 - 1];
	unsigned int saradc_con;
	unsigned int reserved16[(0x0700 - 0x0610) / 4 - 1];
	unsigned int gpupvtpll_con0;
	unsigned int gpupvtpll_con1;
	unsigned int gpupvtpll_con2;
	unsigned int gpupvtpll_con3;
	unsigned int reserved17[(0x0740 - 0x070c) / 4 - 1];
	unsigned int npupvtpll_con0;
	unsigned int npupvtpll_con1;
	unsigned int npupvtpll_con2;
	unsigned int npupvtpll_con3;
	unsigned int reserved18[(0x0800 - 0x074c) / 4 - 1];
	unsigned int chip_id;
	unsigned int reserved19[(0x0840 - 0x0800) / 4 - 1];
	unsigned int gpio1c5_ds;
	unsigned int gpio2a2_ds;
	unsigned int gpio2b0_ds;
	unsigned int gpio3a0_ds;
	unsigned int gpio3a6_ds;
	unsigned int gpio4a0_ds;
	unsigned int reserved20[(0x0900 - 0x0854) / 4 - 1];
	unsigned int dmac0_con0;
	unsigned int dmac0_con1;
	unsigned int dmac0_con2;
	unsigned int dmac0_con3;
	unsigned int dmac0_con4;
	unsigned int dmac0_con5;
	unsigned int dmac0_con6;
	unsigned int dmac0_con7;
	unsigned int dmac0_con8;
	unsigned int dmac0_con9;
	unsigned int reserved21[(0x0940 - 0x0924) / 4 - 1];
	unsigned int dmac1_con0;
	unsigned int dmac1_con1;
	unsigned int dmac1_con2;
	unsigned int dmac1_con3;
	unsigned int dmac1_con4;
	unsigned int dmac1_con5;
	unsigned int dmac1_con6;
	unsigned int dmac1_con7;
	unsigned int dmac1_con8;
	unsigned int dmac1_con9;
};

struct rk3568_pmugrf {
	unsigned int pmu_gpio0a_iomux_l;
	unsigned int pmu_gpio0a_iomux_h;
	unsigned int pmu_gpio0b_iomux_l;
	unsigned int pmu_gpio0b_iomux_h;
	unsigned int pmu_gpio0c_iomux_l;
	unsigned int pmu_gpio0c_iomux_h;
	unsigned int pmu_gpio0d_iomux_l;
	unsigned int reserved0[(0x0020 - 0x0018) / 4 - 1];
	unsigned int pmu_gpio0a_p;
	unsigned int pmu_gpio0b_p;
	unsigned int pmu_gpio0c_p;
	unsigned int pmu_gpio0d_p;
	unsigned int pmu_gpio0a_ie;
	unsigned int pmu_gpio0b_ie;
	unsigned int pmu_gpio0c_ie;
	unsigned int pmu_gpio0d_ie;
	unsigned int pmu_gpio0a_opd;
	unsigned int pmu_gpio0b_opd;
	unsigned int pmu_gpio0c_opd;
	unsigned int pmu_gpio0d_opd;
	unsigned int pmu_gpio0a_sus;
	unsigned int pmu_gpio0b_sus;
	unsigned int pmu_gpio0c_sus;
	unsigned int pmu_gpio0d_sus;
	unsigned int pmu_gpio0a_sl;
	unsigned int pmu_gpio0b_sl;
	unsigned int pmu_gpio0c_sl;
	unsigned int pmu_gpio0d_sl;
	unsigned int pmu_gpio0a_ds_0;
	unsigned int pmu_gpio0a_ds_1;
	unsigned int pmu_gpio0a_ds_2;
	unsigned int pmu_gpio0a_ds_3;
	unsigned int pmu_gpio0b_ds_0;
	unsigned int pmu_gpio0b_ds_1;
	unsigned int pmu_gpio0b_ds_2;
	unsigned int pmu_gpio0b_ds_3;
	unsigned int pmu_gpio0c_ds_0;
	unsigned int pmu_gpio0c_ds_1;
	unsigned int pmu_gpio0c_ds_2;
	unsigned int pmu_gpio0c_ds_3;
	unsigned int pmu_gpio0d_ds_0;
	unsigned int pmu_gpio0d_ds_1;
	unsigned int pmu_gpio0d_ds_2;
	unsigned int pmu_gpio0d_ds_3;
	unsigned int reserved1[(0x0100 - 0x00ac) / 4 - 1];
	unsigned int pmu_soc_con0;
	unsigned int pmu_soc_con1;
	unsigned int pmu_soc_con2;
	unsigned int pmu_soc_con3;
	unsigned int pmu_soc_con4;
	unsigned int pmu_soc_con5;
	unsigned int reserved2[(0x0124 - 0x0114) / 4 - 1];
	unsigned int pmu_io_vsel0;
	unsigned int pmu_io_vsel1;
	unsigned int pmu_io_vsel2;
	unsigned int reserved3[(0x0180 - 0x012c) / 4 - 1];
	unsigned int pmu_dll_con0;
	unsigned int reserved4[(0x0200 - 0x0180) / 4 - 1];
	unsigned int pmu_os_reg0;
	unsigned int pmu_os_reg1;
	unsigned int pmu_os_reg2;
	unsigned int pmu_os_reg3;
	unsigned int pmu_os_reg4;
	unsigned int pmu_os_reg5;
	unsigned int pmu_os_reg6;
	unsigned int pmu_os_reg7;
	unsigned int pmu_os_reg8;
	unsigned int pmu_os_reg9;
	unsigned int pmu_os_reg10;
	unsigned int pmu_os_reg11;
	unsigned int pmu_reset_function_status;
	unsigned int pmu_reset_function_clr;
	unsigned int reserved5[(0x0380 - 0x0234) / 4 - 1];
	unsigned int pmu_sig_detect_con;
	unsigned int reserved6[(0x0390 - 0x0380) / 4 - 1];
	unsigned int pmu_sig_detect_status;
	unsigned int reserved7[(0x03a0 - 0x0390) / 4 - 1];
	unsigned int pmu_sig_detect_status_clear;
	unsigned int reserved8[(0x03b0 - 0x03a0) / 4 - 1];
	unsigned int pmu_sdmmc_det_counter;
};


static inline void writel(u32 val, void *addr)
{
	*(volatile u32 *)addr = val;
}

static inline u32 readl(void *addr)
{
	return *(volatile u32 *)addr;
}



void board_debug_uart_init(void)
{

	static struct rk3568_grf * const grf = (void *)GRF_BASE;

	static struct rk3568_pmugrf * const pmugrf = (void *)PMUGRF_BASE;
	/* UART2 M0 */
	rk_clrsetreg(&grf->iofunc_sel3, UART2_IO_SEL_MASK,
		     UART2_IO_SEL_M0 << UART2_IO_SEL_SHIFT);

	/* Switch iomux */
	rk_clrsetreg(&pmugrf->pmu_gpio0d_iomux_l,
		     GPIO0D1_MASK | GPIO0D0_MASK,
		     GPIO0D1_UART2_TXM0 << GPIO0D1_SHIFT |
		     GPIO0D0_UART2_RXM0 << GPIO0D0_SHIFT);

}


static inline int serial_in_shift(void *addr, int shift)
{
	return readb(addr);
}


static inline void serial_out_shift(void *addr, int shift, int value)
{
	writeb(value, addr);
}

int ns16550_calc_divisor(NS16550_t port, int clock, int baudrate)
{
	const unsigned int mode_x_div = 16;

	return DIV_ROUND_CLOSEST(clock, mode_x_div * baudrate);
}

void _debug_uart_init(void)
{
	struct NS16550 *com_port = (struct NS16550 *)CONFIG_DEBUG_UART_BASE;
	int baud_divisor;

	baud_divisor = ns16550_calc_divisor(com_port, CONFIG_DEBUG_UART_CLOCK,
					    CONFIG_BAUDRATE);
	serial_dout(&com_port->ier, CONFIG_SYS_NS16550_IER);
	serial_dout(&com_port->mcr, UART_MCRVAL);
	serial_dout(&com_port->fcr, UART_FCR_DEFVAL);

	serial_dout(&com_port->lcr, UART_LCR_BKSE | UART_LCRVAL);
	serial_dout(&com_port->dll, baud_divisor & 0xff);
	serial_dout(&com_port->dlm, (baud_divisor >> 8) & 0xff);
	serial_dout(&com_port->lcr, UART_LCRVAL);
}

void _debug_uart_putc(int ch)
{
	struct NS16550 *com_port;
    com_port = (struct NS16550 *)CONFIG_DEBUG_UART_BASE;

	while (!(serial_din(&com_port->lsr) & UART_LSR_THRE))
		;
	serial_dout(&com_port->thr, ch);
}


// UART send string
void uart_puts(const char *str) {
    char c;
    while ((c = *str++) != '\0') {
        // manual add \r for \n
        if (c == '\n' ) {
            _debug_uart_putc('\r');
        }
        _debug_uart_putc(c);
    }
}


int _debug_uart_getc(void)
{
	struct NS16550 *com_port;

	com_port = (struct NS16550 *)CONFIG_DEBUG_UART_BASE;

	while (!(serial_din(&com_port->lsr) & UART_LSR_DR))
		;

	return serial_din(&com_port->rbr);
}

int _debug_uart_tstc(int input)
{
	struct NS16550 *com_port;

	com_port = (struct NS16550 *)CONFIG_DEBUG_UART_BASE;

	if (input)
		return serial_din(&com_port->lsr) & UART_LSR_DR ? 1 : 0;
	else
		return serial_din(&com_port->lsr) & UART_LSR_THRE ? 0 : 1;
}

int _debug_uart_clrc(void)
{
	struct NS16550 *com_port;
	com_port = (struct NS16550 *)CONFIG_DEBUG_UART_BASE;

	serial_dout(&com_port->fcr, UART_FCR_CLEAR_RCVR | UART_FCR_CLEAR_XMIT);

	return 0;
}

int _debug_uart_flushc(void)
{
	struct NS16550 *com_port;
    com_port = (struct NS16550 *)CONFIG_DEBUG_UART_BASE;

	/*
	 * Wait fifo flush.
	 *
	 * UART_USR: bit2 trans_fifo_empty:
	 *	0 = Transmit FIFO is not empty
	 *	1 = Transmit FIFO is empty
	 */
	while (!(serial_din(&com_port->rbr + 0x1f) & 0x04))
		;

	return 0;
}

/* baudrate*/
int _debug_uart_setbrg(void)
{
	struct NS16550 *com_port;
	int baud_divisor;

	com_port = (struct NS16550 *)CONFIG_DEBUG_UART_BASE;
	baud_divisor = ns16550_calc_divisor(com_port,CONFIG_DEBUG_UART_CLOCK, CONFIG_BAUDRATE);
	
	serial_dout(&com_port->lcr, UART_LCR_BKSE | UART_LCRVAL);
	serial_dout(&com_port->dll, baud_divisor & 0xff);
	serial_dout(&com_port->dlm, (baud_divisor >> 8) & 0xff);
	serial_dout(&com_port->lcr, UART_LCRVAL);

	return 0;
}