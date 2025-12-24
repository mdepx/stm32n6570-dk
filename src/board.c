/*-
 * Copyright (c) 2025 Ruslan Bukin <br@bsdpad.com>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <sys/cdefs.h>
#include <sys/console.h>
#include <sys/systm.h>
#include <sys/malloc.h>
#include <sys/thread.h>

#include <machine/cpufunc.h>
#include <machine/cpuregs.h>

#include <dev/intc/intc.h>
#include <dev/uart/uart.h>

#include <arm/stm/stm32n6.h>
#include <arm/arm/nvic.h>

#include <dev/mx66uw/mx66uw.h>

static struct stm32n6_rcc_softc rcc_sc;
static struct stm32l4_usart_softc usart_sc;
static struct stm32f4_timer_softc timer_sc;
static struct stm32n6_xspi_softc xspi1_sc;	/* PSRAM */
static struct stm32n6_xspi_softc xspi2_sc;	/* NOR flash */
static struct stm32n6_pwr_softc pwr_sc;
static struct stm32f4_i2c_softc i2c1_sc;

struct stm32f4_gpio_softc gpio_sc;
static struct stm32n6_ltdc_softc ltdc_sc;
static struct stm32n6_ramcfg_softc ramcfg_sc;
static struct stm32n6_risaf_softc risaf11_sc;	/* xspi 1 */
static struct stm32n6_risaf_softc risaf12_sc;	/* xspi 2 */
static struct stm32n6_risaf_softc risaf6_sc;
struct stm32n6_dcmipp_softc dcmipp_sc;
struct stm32n6_csi_softc csi_sc;

static struct arm_nvic_softc nvic_sc;
static struct mdx_device dev_nvic = { .sc = &nvic_sc };
struct mdx_device dev_i2c1 = { .sc = &i2c1_sc };
static struct mx66uw_softc mx66uw_sc;

static struct layer_info info;

#define	APS256XX_READ_CMD		0x00
#define	APS256XX_READ_LINEAR_BURST_CMD	0x20
#define	APS256XX_WRITE_CMD		0x80
#define	APS256XX_WRITE_LINEAR_BURST_CMD	0xA0
#define	APS256XX_RESET_CMD		0xFF
#define	APS256XX_READ_REG_CMD		0x40
#define	APS256XX_WRITE_REG_CMD		0xC0

void
udelay(uint32_t usec)
{
	int i;

	/* TODO: implement me */

	for (i = 0; i < usec * 100; i++)
		;
}

static void
uart_putchar(int c, void *arg)
{
	struct stm32l4_usart_softc *sc;

	sc = arg;

	if (c == '\n')
		stm32l4_usart_putc(sc, '\r');

	stm32l4_usart_putc(sc, c);
}

static const struct stm32_gpio_pin uart_pins[] = {
	{ PORT_E, 5, MODE_ALT, 7, OT_PP, OS_H, FLOAT }, /* VCP USART1_TX */
	{ PORT_E, 6, MODE_ALT, 7, OT_PP, OS_H, FLOAT }, /* VCP USART1_RX */
	{ PORT_D, 5, MODE_ALT, 7, OT_PP, OS_H, FLOAT }, /* D1 USART2_TX */
	{ PORT_F, 6, MODE_ALT, 7, OT_PP, OS_H, FLOAT }, /* D0 USART2_RX */

	/* ROCKTEK RK050HR18-CTG */
	{ PORT_G,  0, MODE_ALT, 14, OT_PP, OS_VH, FLOAT }, /* R0 */
	{ PORT_D,  9, MODE_ALT, 14, OT_PP, OS_VH, FLOAT }, /* R1 */
	{ PORT_D, 15, MODE_ALT, 14, OT_PP, OS_VH, FLOAT }, /* R2 */
	{ PORT_B,  4, MODE_ALT, 14, OT_PP, OS_VH, FLOAT }, /* R3 */
	{ PORT_H,  4, MODE_ALT, 14, OT_PP, OS_VH, FLOAT }, /* R4 */
	{ PORT_A, 15, MODE_ALT, 14, OT_PP, OS_VH, FLOAT }, /* R5 */
	{ PORT_G, 11, MODE_ALT, 14, OT_PP, OS_VH, FLOAT }, /* R6 */
	{ PORT_D,  8, MODE_ALT, 14, OT_PP, OS_VH, FLOAT }, /* R7 */

	{ PORT_G, 12, MODE_ALT, 14, OT_PP, OS_VH, FLOAT }, /* G0 */
	{ PORT_G,  1, MODE_ALT, 14, OT_PP, OS_VH, FLOAT }, /* G1 */
	{ PORT_A,  1, MODE_ALT, 14, OT_PP, OS_VH, FLOAT }, /* G2 */
	{ PORT_A,  0, MODE_ALT, 14, OT_PP, OS_VH, FLOAT }, /* G3 */
	{ PORT_B, 15, MODE_ALT, 14, OT_PP, OS_VH, FLOAT }, /* G4 */
	{ PORT_B, 12, MODE_ALT, 14, OT_PP, OS_VH, FLOAT }, /* G5 */
	{ PORT_B, 11, MODE_ALT, 14, OT_PP, OS_VH, FLOAT }, /* G6 */
	{ PORT_G,  8, MODE_ALT, 14, OT_PP, OS_VH, FLOAT }, /* G7 */

	{ PORT_G, 15, MODE_ALT, 14, OT_PP, OS_VH, FLOAT }, /* B0 */
	{ PORT_A,  7, MODE_ALT, 14, OT_PP, OS_VH, FLOAT }, /* B1 */
	{ PORT_B,  2, MODE_ALT, 14, OT_PP, OS_VH, FLOAT }, /* B2 */
	{ PORT_G,  6, MODE_ALT, 14, OT_PP, OS_VH, FLOAT }, /* B3 */
	{ PORT_H,  3, MODE_ALT, 14, OT_PP, OS_VH, FLOAT }, /* B4 */
	{ PORT_H,  6, MODE_ALT, 14, OT_PP, OS_VH, FLOAT }, /* B5 */
	{ PORT_A,  8, MODE_ALT, 14, OT_PP, OS_VH, FLOAT }, /* B6 */
	{ PORT_A,  2, MODE_ALT, 14, OT_PP, OS_VH, FLOAT }, /* B7 */

	{ PORT_G, 13, MODE_OUT,  0, OT_PP, OS_VH, FLOAT }, /* LCD_DE */
	{ PORT_Q,  3, MODE_OUT,  0, OT_PP, OS_VH, FLOAT }, /* LCD_ON/OFF */
	{ PORT_B, 14, MODE_ALT, 14, OT_PP, OS_VH, FLOAT }, /* LCD_HSYNC */
	{ PORT_E, 11, MODE_ALT, 14, OT_PP, OS_VH, FLOAT }, /* LCD_VSYNC */
	{ PORT_B, 13, MODE_ALT, 14, OT_PP, OS_VH, FLOAT }, /* LCD_CLK */
	{ PORT_Q,  4, MODE_INP,  0, OT_PP, OS_VH, PULLUP }, /* LCD_INT */
	{ PORT_Q,  6, MODE_OUT,  0, OT_PP, OS_VH, FLOAT }, /* LCD_BL_CTRL */
	{ PORT_E,  1, MODE_OUT,  0, OT_PP, OS_VH, FLOAT }, /* NRST */

	/* XSPIM_P1 APS256XXN-OBR-BG */
	{ PORT_O,  0, MODE_ALT, 9, OT_PP, OS_VH, PULLUP }, /* NCS1 */
	{ PORT_O,  2, MODE_ALT, 9, OT_PP, OS_VH, PULLUP }, /* DQS0 */
	{ PORT_O,  3, MODE_ALT, 9, OT_PP, OS_VH, PULLUP }, /* DQS1 */
	{ PORT_O,  4, MODE_ALT, 9, OT_PP, OS_VH, PULLUP }, /* CLK */

	{ PORT_P,  0, MODE_ALT, 9, OT_PP, OS_VH, FLOAT }, /* IO0 */
	{ PORT_P,  1, MODE_ALT, 9, OT_PP, OS_VH, FLOAT }, /* IO1 */
	{ PORT_P,  2, MODE_ALT, 9, OT_PP, OS_VH, FLOAT }, /* IO2 */
	{ PORT_P,  3, MODE_ALT, 9, OT_PP, OS_VH, FLOAT }, /* IO3 */
	{ PORT_P,  4, MODE_ALT, 9, OT_PP, OS_VH, FLOAT }, /* IO4 */
	{ PORT_P,  5, MODE_ALT, 9, OT_PP, OS_VH, FLOAT }, /* IO5 */
	{ PORT_P,  6, MODE_ALT, 9, OT_PP, OS_VH, FLOAT }, /* IO6 */
	{ PORT_P,  7, MODE_ALT, 9, OT_PP, OS_VH, FLOAT }, /* IO7 */
	{ PORT_P,  8, MODE_ALT, 9, OT_PP, OS_VH, FLOAT }, /* IO8 */
	{ PORT_P,  9, MODE_ALT, 9, OT_PP, OS_VH, FLOAT }, /* IO9 */
	{ PORT_P, 10, MODE_ALT, 9, OT_PP, OS_VH, FLOAT }, /* IO10 */
	{ PORT_P, 11, MODE_ALT, 9, OT_PP, OS_VH, FLOAT }, /* IO11 */
	{ PORT_P, 12, MODE_ALT, 9, OT_PP, OS_VH, FLOAT }, /* IO12 */
	{ PORT_P, 13, MODE_ALT, 9, OT_PP, OS_VH, FLOAT }, /* IO13 */
	{ PORT_P, 14, MODE_ALT, 9, OT_PP, OS_VH, FLOAT }, /* IO14 */
	{ PORT_P, 15, MODE_ALT, 9, OT_PP, OS_VH, FLOAT }, /* IO15 */

	/* XSPIM_P2 MX66UW1G45G */
	{ PORT_N,  0, MODE_ALT, 9, OT_PP, OS_H, PULLUP }, /* DQS0 */
	{ PORT_N,  1, MODE_ALT, 9, OT_PP, OS_H, PULLUP }, /* NCS1 */
	{ PORT_N,  6, MODE_ALT, 9, OT_PP, OS_H, PULLUP }, /* CLK */
	{ PORT_N,  7, MODE_ALT, 9, OT_PP, OS_H, FLOAT }, /* NCLK (unused) */
	{ PORT_N,  2, MODE_ALT, 9, OT_PP, OS_H, FLOAT }, /* D0 */
	{ PORT_N,  3, MODE_ALT, 9, OT_PP, OS_H, FLOAT }, /* D1 */
	{ PORT_N,  4, MODE_ALT, 9, OT_PP, OS_H, FLOAT }, /* D2 */
	{ PORT_N,  5, MODE_ALT, 9, OT_PP, OS_H, FLOAT }, /* D3 */
	{ PORT_N,  8, MODE_ALT, 9, OT_PP, OS_H, FLOAT }, /* D4 */
	{ PORT_N,  9, MODE_ALT, 9, OT_PP, OS_H, FLOAT }, /* D5 */
	{ PORT_N, 10, MODE_ALT, 9, OT_PP, OS_H, FLOAT }, /* D6 */
	{ PORT_N, 11, MODE_ALT, 9, OT_PP, OS_H, FLOAT }, /* D7 */

	/* imx335 camera module */
	/*
	 * I2C address of MB1854-B01:
	 * Camera: Read 0x35 / Write 0x34
	 * TOF: Read 0x53 / Write 0x52
	 * IMU: Read 0xD6 / Write 0xD5
	 * PQ5 TOF_LPn
	 * PQ0 TOF_INT
	 * PQ1 IMU_INT1
	 * PQ2 IMU_INT2
	 * PH9 I2C_SCL
	 * PC1 I2C_SDA
	 */
	{ PORT_C, 8, MODE_OUT, 0, OT_PP, OS_VH, FLOAT }, /* NRST_CAM */
	{ PORT_D, 2, MODE_OUT, 0, OT_PP, OS_VH, FLOAT }, /* EN_MODULE */
	{ PORT_H, 9, MODE_ALT, 4, OT_OD, OS_VH, FLOAT }, /* I2C1_SCL */
	{ PORT_C, 1, MODE_ALT, 4, OT_OD, OS_VH, FLOAT }, /* I2C1_SDA */

	PINS_END
};

void
board_init(void)
{
	struct rcc_config cfg;
	struct xspi_config conf;
	struct risaf_config rconf;

	bzero(&cfg, sizeof(struct rcc_config));
	cfg.ahb4enr = AHB4ENSR_GPIOAEN | AHB4ENSR_GPIOBEN | AHB4ENSR_GPIOCEN |
	    AHB4ENSR_GPIODEN | AHB4ENSR_GPIOEEN | AHB4ENSR_GPIOFEN |
	    AHB4ENSR_GPIOGEN | AHB4ENSR_GPIOHEN | AHB4ENSR_GPIONEN |
	    AHB4ENSR_GPIOOEN | AHB4ENSR_GPIOPEN | AHB4ENSR_GPIOQEN |
	    AHB4ENSR_PWREN;
	cfg.ahb5enr = AHB5ENSR_NPUENS | AHB5ENSR_CACHEAXIENS |
	    AHB5ENSR_GPU2DENS | AHB5ENSR_GFXMMUENS | AHB5ENSR_XSPI3ENS |
	    AHB5ENSR_XSPIMENS | AHB5ENSR_XSPI2ENS | AHB5ENSR_XSPI1ENS |
	    AHB5ENSR_FMCENS | AHB5ENSR_JPEGENS | AHB5ENSR_DMA2DENS;
	cfg.apb1lenr = APB1LENSR_UART5ENS | APB1LENSR_USART2ENS |
	    APB1LENSR_TIM2ENS | APB1LENSR_I2C1ENS;
	cfg.apb2enr = APB2ENSR_USART1EN | APB2ENSR_TIM1ENS;
	cfg.apb5enr = APB5ENSR_LTDCEN | APB5ENSR_DCMIPPENS |
	    APB5ENSR_GFXTIMENS | APB5ENSR_VENCENS | APB5ENSR_CSIENS;
	cfg.memenr = MEMENSR_BOOTROMENS | MEMENSR_CACHEAXIRAMENS |
	    MEMENSR_FLEXRAMENS | MEMENSR_AXISRAM2ENS | MEMENSR_AXISRAM1ENS |
	    MEMENSR_BKPSRAMENS | MEMENSR_AHBSRAM2ENS | MEMENSR_AHBSRAM1ENS |
	    MEMENSR_AXISRAM6ENS | MEMENSR_AXISRAM5ENS | MEMENSR_AXISRAM4ENS |
	    MEMENSR_AXISRAM3ENS;

	/* RCC */
	stm32n6_rcc_init(&rcc_sc, RCC_BASE);
	stm32n6_rcc_setup(&rcc_sc, &cfg);
	stm32n6_rcc_pll1(&rcc_sc);

	/* GPIO */
	stm32f4_gpio_init(&gpio_sc, GPIO_BASE);
	pin_configure(&gpio_sc, uart_pins);

	/* USART */
	stm32l4_usart_init(&usart_sc, USART1_BASE, 150000000, 115200);
	mdx_console_register(uart_putchar, (void *)&usart_sc);

	/* NVIC */
	arm_nvic_init(&dev_nvic, NVIC_BASE);
	*(volatile uint32_t *)0xe000ed08 = 0x34120000; /* Setup VTOR. */

	/* TIMER */
	stm32f4_timer_init(&timer_sc, TIM1_BASE, 150000000);
	mdx_intc_setup(&dev_nvic, 115, stm32f4_timer_intr, &timer_sc);
	mdx_intc_enable(&dev_nvic, 115);

	/* Enable 1v8 power for PSRAM and NOR. */
	stm32n6_pwr_init(&pwr_sc, PWR_BASE);
	stm32n6_pwr_setup_vddio23_1v8(&pwr_sc);

	/* PSRAM */
	bzero(&conf, sizeof(struct xspi_config));
	conf.dummy_cycles = 0;
	conf.prescaler = 0;
	conf.dqs_en = 0;
	conf.mem_type = DCR1_MTYP_AP;
	conf.dev_size = DCR1_DEVSIZE_256M;
	conf.cs_cycles = 6;
	conf.data_dtr = 1;
	conf.data_lines = 8;
	conf.address_dtr = 1;
	conf.address_lines = 8;
	conf.address_size = 32;
	conf.instruction_dtr = 0;
	conf.instruction_lines = 8;
	conf.instruction_size = 8;
	conf.instruction = APS256XX_WRITE_REG_CMD;
	conf.mode = XSPI_MODE_INDIRECT_WRITE;

	stm32n6_xspi_init(&xspi1_sc, XSPI1_BASE);
	/* Read latency 7, up to 200MHz. */
	stm32n6_xspi_setup(&xspi1_sc, &conf);
	uint8_t val[2];
	val[0] = 0x30;
	stm32n6_xspi_transmit(&xspi1_sc, 0, val, 2);
	/* Write latency 7, up to 200MHz. */
	stm32n6_xspi_setup(&xspi1_sc, &conf);
	val[0] = 0x20;
	stm32n6_xspi_transmit(&xspi1_sc, 4, val, 2);
	/* Switch to 16 data lines mode. */
	stm32n6_xspi_setup(&xspi1_sc, &conf);
	val[0] = 0x40;
	stm32n6_xspi_transmit(&xspi1_sc, 8, val, 2);

	/* Reconfigure XSPI1 for memory-mapped mode. */
	conf.instruction = 0;
	conf.dqs_en = 1;
	conf.wdqs_en = 1;
	conf.dummy_cycles = 6; /* nb: 4 for 8 data lines */
	conf.wdummy_cycles = 6;
	conf.data_lines = 16;
	conf.mode = XSPI_MODE_MEMORY_MAPPED;
	conf.instruction_read = APS256XX_READ_LINEAR_BURST_CMD;
	conf.instruction_write = APS256XX_WRITE_LINEAR_BURST_CMD;
	stm32n6_xspi_setup(&xspi1_sc, &conf);

	/* NOR Flash (Macronix MX66UW1G45G) */
	stm32n6_xspi_init(&xspi2_sc, XSPI2_BASE);
	mx66uw_init(&mx66uw_sc, &xspi2_sc);

	/* AXISRAM unshutdown */
	stm32n6_ramcfg_init(&ramcfg_sc, RAMCFG_BASE);
	stm32n6_ramcfg_shutdown(&ramcfg_sc, 3, 0);
	stm32n6_ramcfg_shutdown(&ramcfg_sc, 4, 0);
	stm32n6_ramcfg_shutdown(&ramcfg_sc, 5, 0);
	stm32n6_ramcfg_shutdown(&ramcfg_sc, 6, 0);

	/* RISAFs Configuration */
	stm32n6_risaf_init(&risaf11_sc, RISAF11_BASE);
	stm32n6_risaf_init(&risaf12_sc, RISAF12_BASE);
	stm32n6_risaf_init(&risaf6_sc, RISAF6_BASE);

	/* RISAF11: peripherals to XSPI1 access (256mb) */
	rconf.base_start = 0x90000000;
	rconf.base_end = 0xa0000000;
	rconf.base_cid_write = 0xff;
	rconf.base_cid_read = 0xff;
	rconf.base_sec = 0;
	rconf.suba_start = 0x90000000;
	rconf.suba_end = 0xa0000000;
	rconf.suba_rd = 1;
	rconf.suba_wr = 1;
	rconf.suba_cid = 0; /* All peripherals */
	rconf.suba_sec = 0;
	stm32n6_risaf_setup(&risaf11_sc, 1, &rconf);

	/* RISAF11: CPU to XSPI1 access */
	rconf.base_sec = 1;
	rconf.suba_cid = 1;
	rconf.suba_sec = 1;
	stm32n6_risaf_setup(&risaf11_sc, 2, &rconf);

	/* RISAF12: peripherals to XSPI2 access (128mb) */
	rconf.base_start = 0x70000000;
	rconf.base_end = 0x78000000;
	rconf.base_cid_write = 0xff;
	rconf.base_cid_read = 0xff;
	rconf.base_sec = 0;
	rconf.suba_start = 0x70000000;
	rconf.suba_end = 0x78000000;
	rconf.suba_rd = 1;
	rconf.suba_wr = 1;
	rconf.suba_cid = 0; /* All peripherals */
	rconf.suba_sec = 0;
	stm32n6_risaf_setup(&risaf12_sc, 1, &rconf);

	/* RISAF12: CPU to XSPI2 access */
	rconf.base_sec = 1;
	rconf.suba_cid = 1;
	rconf.suba_sec = 1;
	stm32n6_risaf_setup(&risaf12_sc, 2, &rconf);

	/* RISAF6: peripherals to AXISRAM3/4/5/6 access. */
	rconf.base_start = 0x34200000;
	rconf.base_end = 0x343c0000;
	rconf.base_cid_write = 0xff;
	rconf.base_cid_read = 0xff;
	rconf.base_sec = 0;
	rconf.suba_start = 0x34200000;
	rconf.suba_end = 0x343c0000;
	rconf.suba_rd = 1;
	rconf.suba_wr = 1;
	rconf.suba_cid = 0; /* All peripherals */
	rconf.suba_sec = 0;
	stm32n6_risaf_setup(&risaf6_sc, 1, &rconf);

	/* RISAF6: CPU to AXISRAM3/4/5/6 access. */
	rconf.base_sec = 1;
	rconf.suba_cid = 1;
	rconf.suba_sec = 1;
	stm32n6_risaf_setup(&risaf6_sc, 2, &rconf);

	/* LTDC layer. */
	info.width = 800;
	info.height = 480;
	info.hsync = 4;
	info.vsync = 4;
	info.vfp = 8;
	info.hfp = 8;
	info.vbp = 8;
	info.hbp = 8;
	info.bpp = 16;
	info.base = 0x90000000; /* External PSRAM */
	info.base = 0x34200000;	/* AXISRAM3 */

	pin_set(&gpio_sc, PORT_E,  1, 1); /* NRST */
	pin_set(&gpio_sc, PORT_Q,  3, 1); /* LCD_ON/OFF */
	pin_set(&gpio_sc, PORT_Q,  6, 1); /* LCD_BL_CTRL */
	pin_set(&gpio_sc, PORT_G, 13, 1); /* LCD_DE */

	stm32n6_ltdc_init(&ltdc_sc, LTDC_BASE);
	stm32n6_ltdc_setup(&ltdc_sc, &info, 1);

	/* Reset & Enable camera module */
	pin_set(&gpio_sc, PORT_C, 8, 0);
	pin_set(&gpio_sc, PORT_D, 2, 0);
	udelay(10000);
	pin_set(&gpio_sc, PORT_C, 8, 1);
	pin_set(&gpio_sc, PORT_D, 2, 1);

	stm32n6_pwr_setup_vddio4_3v3(&pwr_sc, 1);
	stm32f4_i2c_init(&dev_i2c1, I2C1_BASE);	/* imx335 */
	mdx_intc_setup(&dev_nvic, 100, stm32f4_i2c_intr, &i2c1_sc);
	mdx_intc_enable(&dev_nvic, 100);

	/* DCMIPP and CSI-2 */
	struct stm32n6_dcmipp_downsize_config dconf;
	dconf.hratio		= 25656;
	dconf.vratio		= 33161;
	dconf.hsize		= 800;
	dconf.vsize		= 480;
	dconf.hdivfactor	= 316;
	dconf.vdivfactor	= 253;

	stm32n6_dcmipp_init(&dcmipp_sc, DCMIPP_BASE);
	stm32n6_dcmipp_setup_downsize(&dcmipp_sc, &dconf);
	stm32n6_dcmipp_setup(&dcmipp_sc);
	stm32n6_csi_init(&csi_sc, CSI_BASE);

#if 0
	malloc_init();
	malloc_add_region((void *)0x20020000, 0x20000);
#endif
}
