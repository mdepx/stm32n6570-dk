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

extern struct stm32f4_gpio_softc gpio_sc;
extern struct mdx_device dev_i2c1;

#define	dprintf(fmt, ...)

static void
memtest(uint32_t addr, uint32_t len)
{
	uint32_t data;
	int i;

	printf("Starting memory test for %x\n", addr);

	printf("Writing...\n");
	for (i = 0; i <= len; i += 4)
		*(uint32_t *)(addr + i) = i;

	printf("Reading...\n");
	for (i = len; i >= 0; i -= 4) {
		data = *(uint32_t *)(addr + i);
		if (i != data)
			panic("test failed");
	}

	printf("Test OK\n");
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

int
main(void)
{
	uint8_t val;
	int error;

	memtest(0x90000000, 2048);
	memtest(0x34200000, 2048);

	error = imx335_init(&dev_i2c1, 0x34);
	if (error)
		panic("could not init imx335");

	/* Verify */
	error = imx335_read_data(&dev_i2c1, 0x34, IMX335_STANDBY, 1, &val);
	if (error != 0 || val != 0)
		panic("could not verify imx335 init");

	/* Note: switch LTDC layer address in src/board.c */
	memfill(0x34200000);
	memfill(0x90000000);

	while (1) {
		printf("%s: Hello World from n6\n", __func__);
		mdx_usleep(500000);
		mdx_usleep(500000);
	}

	return (0);
}
