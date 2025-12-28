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

#include <lib/stnpu/ll_aton/ll_aton_runtime.h>
#include <lib/stnpu/ll_aton/ll_aton_platform.h>
#include <lib/stnpu/ll_aton/ll_aton_reloc_network.h>

#include <lib/stnpu/ll_aton/ll_aton_rt_user_api.h>
LL_ATON_DECLARE_NAMED_NN_INSTANCE_AND_INTERFACE(Default);

#include "npu.h"

static uint8_t *nn_in;

#define	MAX_NUMBER_OUTPUT	5

static void
nn_init(uint32_t *nnin_length, float *nn_out[], int *number_output,
    int32_t nn_out_len[])
{
	const LL_Buffer_InfoTypeDef *nn_in_info;
	const LL_Buffer_InfoTypeDef *nn_out_info;

	nn_in_info = LL_ATON_Input_Buffers_Info(&NN_Instance_Default);
	nn_out_info = LL_ATON_Output_Buffers_Info(&NN_Instance_Default);

	/* Input address. */
	nn_in = (uint8_t *) LL_Buffer_addr_start(&nn_in_info[0]);

	while (nn_out_info[*number_output].name != NULL)
		(*number_output)++;

	assert(*number_output <= MAX_NUMBER_OUTPUT);

	for (int i = 0; i < *number_output; i++) {
		/* Output address. */
		nn_out[i] = (float *)LL_Buffer_addr_start(&nn_out_info[i]);
		nn_out_len[i] = LL_Buffer_len(&nn_out_info[i]);
	}

	*nnin_length = LL_Buffer_len(&nn_in_info[0]);

	LL_ATON_RT_RuntimeInit();
	LL_ATON_RT_Init_Network(&NN_Instance_Default);
}

void
nn_pass(void)
{
	LL_ATON_RT_RetValues_t ll_aton_rt_ret;

	do {
		ll_aton_rt_ret = LL_ATON_RT_RunEpochBlock(&NN_Instance_Default);
		if (ll_aton_rt_ret == LL_ATON_RT_WFE)
			LL_ATON_OSAL_WFE();
	} while (ll_aton_rt_ret != LL_ATON_RT_DONE);
}

int
npu_test(void)
{
	float *nn_out[MAX_NUMBER_OUTPUT] = {0};
	int32_t nn_out_len[MAX_NUMBER_OUTPUT] = {0};
	uint32_t nn_in_len;
	int number_output;

	printf("%s\n", __func__);

	nn_in_len = 0;
	number_output = 0;

	nn_init(&nn_in_len, nn_out, &number_output, nn_out_len);
	if (1 == 0)
		nn_pass();

	return (0);
}
