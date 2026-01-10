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
#include <sys/systm.h>
#include <sys/thread.h>
#include <sys/spinlock.h>

#include <arm/stm/stm32n6.h>

#include <dev/imx335/imx335.h>
#include <dev/i2c/i2c.h>

#include "npu.h"

extern struct stm32n6_dcmipp_softc dcmipp_sc;
extern struct stm32n6_csi_softc csi_sc;
extern struct stm32f4_gpio_softc gpio_sc;
extern struct mdx_device dev_i2c1;

#define	dprintf(fmt, ...)

static void __unused
memzero(uint32_t addr, uint32_t len)
{
	int i;

	for (i = 0; i < len; i += 4)
		*(uint32_t *)(addr + i) = 0;
}

static void __unused
memtest(uint32_t addr, uint32_t len)
{
	uint32_t data;
	int i;

	printf("Starting memory test for %x\n", addr);

	printf("Writing...\n");
	for (i = 0; i < len; i += 4)
		*(uint32_t *)(addr + i) = i;

	printf("Reading...\n");
	for (i = (len - 4); i >= 0; i -= 4) {
		printf("%d %d\n", i, data);
		data = *(uint32_t *)(addr + i);
		if (i != data)
			panic("test failed: %d != %d", i, data);
	}

	printf("Test OK\n");
}

static void
flashtest(uint32_t addr, uint32_t len)
{
	int i;

	printf("Accessing flash on read: ");

	for (i = 0; i < len; i++)
		printf("%x", *(uint8_t *)(addr + i));

	printf("\n");
}

static void __unused
memfill(uint32_t base)
{
	int flag;
	int i;

	flag = 0;

	printf("LTDC test for %x\n", base);

	while (1) {
		if (flag == 1)
			flag = 0;
		else
			flag = 1;

		for (i = 0; i < (480 * 800 * 4); i += 4)
			if (flag)
				*(uint32_t *)(base + i) = 0xdddddddd;
			else
				*(uint32_t *)(base + i) = 0x0;
	}
}

void
layer2_clear(void)
{
	int i;

	for (i = 0; i < 480 * 480 * 2; i += 4)
		*(uint32_t *)(0x91100000 + i) = 0;
}

void
write_hline(int x, int y, int len)
{
	uint32_t addr;
	int i;

	addr = (0x91100000 + (x * 2 + y * 480 * 2));

	for (i = 0; i < len * 2; i += 2)
		*(uint16_t *)(addr + i) = 0xffff;
}

void
write_vline(int x, int y, int len)
{
	uint32_t addr;
	int i;

	addr = (0x91100000 + (x * 2 + y * 480 * 2));

	for (i = 0; i < len * 480 * 2; i += 480 * 2)
		*(uint16_t *)(addr + i) = 0xffff;
}

int mcu_cache_enable(void);

int
main(void)
{
	uint8_t val;
	int error;

#if 1
	/* TODO */
	*(uint32_t *)(0xE001E000) |= 1 << 12; //dcache power
	*(uint32_t *)(0xE001E000) |= 1 << 13; //icache power
	*(uint32_t *)0x480DFC00 = 1; //cacheaxi
	*(uint32_t *)0x48035000 = 1; //icache
	mcu_cache_enable();
#endif

#if 0
	memzero(0x90000000, 1024 * 1024 * 32);
	memtest(0x90000000, 1024 * 1024 * 1);
	memtest(0x34200000, 2048);
#endif
	flashtest(0x70000000, 16);

	error = imx335_init(&dev_i2c1, 0x34);
	if (error)
		panic("could not init imx335");
	/* Verify */
	error = imx335_read_data(&dev_i2c1, 0x34, IMX335_STANDBY, 1, &val);
	if (error != 0 || val != 0)
		panic("could not verify imx335 init");

	error = imx335_set_exposure(&dev_i2c1, 0x34, 250);
	if (error != 0)
		panic("could not set exposure");

	error = imx335_set_gain(&dev_i2c1, 0x34, 40);
	if (error != 0)
		panic("could not set gain");

#if 0
	/* Camera Test Pattern */
	error = imx335_test_pattern_enable(&dev_i2c1, 0x34, 10);
	if (error)
		panic("could not enable pattern");
	/* Verify */
	error = imx335_read_data(&dev_i2c1, 0x34, IMX335_TPG, 1, &val);
	if (error != 0 || val != 10)
		panic("could not verify imx335 tpg");
#endif

	npu_test();

	while (1) {
		printf("%s: Hello World from n6\n", __func__);
		stm32n6_dcmipp_status(&dcmipp_sc);
		stm32n6_csi_status(&csi_sc);
		mdx_usleep(500000);
		mdx_usleep(500000);
	}

	return (0);
}
