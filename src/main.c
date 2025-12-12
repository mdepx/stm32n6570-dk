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

#include <dev/i2c/i2c.h>

extern struct stm32f4_gpio_softc gpio_sc;

#define	dprintf(fmt, ...)

int
main(void)
{
	int i __unused;

#if 1
	uint32_t addr;
	int k;
	printf("reading xspi1 val\n");

	k = 64;
	k = 2048;
	addr = 0x34200000;
	addr = 0x90000000;

	for (i = 0; i <= k; i += 4)
		*(uint32_t *)(addr + i) = i;
	for (i = k; i >= 0; i -= 4)
		printf("xspi1 %d val %d\n", i, *(uint32_t *)(addr + i));

	printf("ok\n");
#endif

#if 1
	uint32_t base;
	base = 0x34200000;
	base = 0x90000000;
#if 0
	for (i = 0; i < (480 * 800 * 3); i += 3) {
		*(uint8_t *)(base + i + 0) = 0xff;
		*(uint8_t *)(base + i + 1) = 0x00;
		*(uint8_t *)(base + i + 2) = 0x00;
	}
#else
	int flag = 0;

	while (1) {
		if (flag == 0xff)
			flag = 0;
		else
			flag = 0xff;
#if 1
		for (i = 0; i < (480 * 800 * 4); i += 4)
			if (flag)
				*(uint32_t *)(base + i) = 0xdddddddd;
			else
				*(uint32_t *)(base + i) = 0x0;
#else
		for (i = 0; i < (480 * 800 * 3); i += 3) {
			*(uint8_t *)(base + i + 0) = flag;
			*(uint8_t *)(base + i + 1) = 0x00;
			*(uint8_t *)(base + i + 2) = 0x00;
		}
#endif
	}
#endif
#endif

	pin_set(&gpio_sc, PORT_E,  1, 1); /* NRST */
	pin_set(&gpio_sc, PORT_Q,  3, 1); /* LCD_ON/OFF */
	pin_set(&gpio_sc, PORT_Q,  6, 1); /* LCD_BL_CTRL */
	pin_set(&gpio_sc, PORT_G, 13, 1); /* LCD_DE */

	while (1) {
		printf("%s: Hello World from n6\n", __func__);
		mdx_usleep(500000);
		mdx_usleep(500000);
		printf("risaf11 st %x\n", *(uint32_t *)(0x54030000 + 0x8));
		printf("risaf11 addr %x\n", *(uint32_t *)(0x54030000 + 0x24));
		printf("risaf11 err st %x\n", *(uint32_t *)(0x54030000 + 0x20));
		printf("risaf6 st %x\n", *(uint32_t *)(0x5402b000 + 0x8));
		printf("risaf5 st %x\n", *(uint32_t *)(0x5402a000 + 0x8));
	}

	return (0);
}
