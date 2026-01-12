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

#include <arm/stm/stm32n6_dcmipp.h>

#include <lib/stnpu/ll_aton/ll_aton_runtime.h>
#include <lib/stnpu/ll_aton/ll_aton_platform.h>

#include <lib/stnpu/ll_aton/ll_aton_rt_user_api.h>
LL_ATON_DECLARE_NAMED_NN_INSTANCE_AND_INTERFACE(network);

#include "npu.h"
#include "yolox.h"
#include "blazeface.h"

static uint8_t *nn_in;

extern struct stm32n6_dcmipp_softc dcmipp_sc;

#define	MAX_NUMBER_OUTPUT	5

static void
nn_init(uint32_t *nnin_length, uint32_t *nn_out[], int *number_output,
    uint32_t nn_out_len[])
{
	const LL_Buffer_InfoTypeDef *nn_in_info;
	const LL_Buffer_InfoTypeDef *nn_out_info;
	int i;

	nn_in_info = LL_ATON_Input_Buffers_Info(&NN_Instance_network);
	nn_out_info = LL_ATON_Output_Buffers_Info(&NN_Instance_network);

	/* Input address. */
	nn_in = (uint8_t *)LL_Buffer_addr_start(&nn_in_info[0]);

	printf("nn in %p\n", nn_in);

	while (nn_out_info[*number_output].name != NULL)
		(*number_output)++;

	assert(*number_output <= MAX_NUMBER_OUTPUT);

	for (i = 0; i < *number_output; i++) {
		/* Output address. */
		nn_out[i] = (uint32_t *)LL_Buffer_addr_start(&nn_out_info[i]);
		nn_out_len[i] = LL_Buffer_len(&nn_out_info[i]);
		printf("%s: out %d: %p len %d\n", __func__, i, nn_out[i],
		    nn_out_len[i]);
#if 1
		uint8_t *addr = (uint8_t *)nn_out[i];
		int j;
		for (j = 0; j < nn_out_len[i]; j++)
			addr[j] = 0;
#endif
	}

	*nnin_length = LL_Buffer_len(&nn_in_info[0]);

	LL_ATON_RT_RuntimeInit();
	LL_ATON_RT_Init_Network(&NN_Instance_network);
}

void
nn_pass(void)
{
	LL_ATON_RT_RetValues_t ll_aton_rt_ret;

	do {
		ll_aton_rt_ret = \
		    LL_ATON_RT_RunEpochBlock(&NN_Instance_network);
		if (ll_aton_rt_ret == LL_ATON_RT_WFE) {
			/* Wait for event here. */
		}
	} while (ll_aton_rt_ret != LL_ATON_RT_DONE);

	LL_ATON_RT_Reset_Network(&NN_Instance_network);
}

int
npu_test(void)
{
	uint32_t *nn_out[MAX_NUMBER_OUTPUT] = {0};
	uint32_t nn_out_len[MAX_NUMBER_OUTPUT] = {0};
	uint32_t nn_in_len;
	int number_output;
	int j;

	nn_in_len = 0;
	number_output = 0;

	nn_init(&nn_in_len, nn_out, &number_output, nn_out_len);
#if 1
	yolox_init(&NN_Instance_network);
#else
	blazeface_init(&NN_Instance_network);
#endif

	nn_in = (void *)0x34200000;
	LL_ATON_Set_User_Input_Buffer_network(0, nn_in, nn_in_len);
	printf("%s: nn in %p len %d\n", __func__, nn_in, nn_in_len);

	while (1) {
		stm32n6_dcmipp_capture_req(&dcmipp_sc, 2);

		SCB_InvalidateDCache_by_Addr(nn_in, nn_in_len);

		nn_pass();

		for (j = 0; j < number_output; j++)
			SCB_InvalidateDCache_by_Addr(nn_out[j], nn_out_len[j]);

#if 1
		yolox_process(nn_out);
#else
		blazeface_process(nn_out);
#endif
	}

	return (0);
}
