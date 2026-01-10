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

#include <lib/lib_vision_models_pp/Inc/od_st_yolox_pp_if.h>

#include "yolox.h"
#include "npu.h"

#define MAX(a,b) (((a)>(b))?(a):(b))

static od_pp_outBuffer_t out_detections[
	MAX(AI_OD_ST_YOLOX_PP_MAX_BOXES_LIMIT,
	    AI_OD_ST_YOLOX_PP_L_GRID_WIDTH * AI_OD_ST_YOLOX_PP_L_GRID_HEIGHT +
	    AI_OD_ST_YOLOX_PP_M_GRID_WIDTH * AI_OD_ST_YOLOX_PP_M_GRID_HEIGHT +
	    AI_OD_ST_YOLOX_PP_S_GRID_WIDTH * AI_OD_ST_YOLOX_PP_S_GRID_HEIGHT)];

static od_st_yolox_pp_static_param_t pp_params;

int
yolox_init(NN_Instance_TypeDef *NN_Instance)
{
	const LL_Buffer_InfoTypeDef *buffers_info;
	od_st_yolox_pp_static_param_t *params;
	int error;

	buffers_info = LL_ATON_Output_Buffers_Info(NN_Instance);

	params = &pp_params;
	bzero(params, sizeof(od_st_yolox_pp_static_param_t));
	params->raw_s_scale = *(buffers_info[0].scale);
	params->raw_s_zero_point = *(buffers_info[0].offset);
	params->raw_l_scale = *(buffers_info[1].scale);
	params->raw_l_zero_point = *(buffers_info[1].offset);
	params->raw_m_scale = *(buffers_info[2].scale);
	params->raw_m_zero_point = *(buffers_info[2].offset);
	params->nb_classes = AI_OD_ST_YOLOX_PP_NB_CLASSES;
	params->nb_anchors = AI_OD_ST_YOLOX_PP_NB_ANCHORS;
	params->grid_width_L = AI_OD_ST_YOLOX_PP_L_GRID_WIDTH;
	params->grid_height_L = AI_OD_ST_YOLOX_PP_L_GRID_HEIGHT;
	params->grid_width_M = AI_OD_ST_YOLOX_PP_M_GRID_WIDTH;
	params->grid_height_M = AI_OD_ST_YOLOX_PP_M_GRID_HEIGHT;
	params->grid_width_S = AI_OD_ST_YOLOX_PP_S_GRID_WIDTH;
	params->grid_height_S = AI_OD_ST_YOLOX_PP_S_GRID_HEIGHT;
	params->pAnchors_L = AI_OD_ST_YOLOX_PP_L_ANCHORS;
	params->pAnchors_M = AI_OD_ST_YOLOX_PP_M_ANCHORS;
	params->pAnchors_S = AI_OD_ST_YOLOX_PP_S_ANCHORS;
	params->max_boxes_limit = AI_OD_ST_YOLOX_PP_MAX_BOXES_LIMIT;
	params->conf_threshold = AI_OD_ST_YOLOX_PP_CONF_THRESHOLD;
	params->iou_threshold = AI_OD_ST_YOLOX_PP_IOU_THRESHOLD;

	error = od_st_yolox_pp_reset(params);

	return (error);
}

void
yolox_process(uint32_t **inputs)
{
	od_pp_outBuffer_t *rois;
	od_pp_out_t pp_output;
	int error;
	int i;

	pp_output.pOutBuff = out_detections;
	printf("%s\n", __func__);

	pp_params.nb_detect = 0;
	od_st_yolox_pp_in_t pp_input = {
		.pRaw_detections_S = (uint32_t *)inputs[0],
		.pRaw_detections_L = (uint32_t *)inputs[1],
		.pRaw_detections_M = (uint32_t *)inputs[2],
	};

	printf("%s: S %x\n", __func__, pp_input.pRaw_detections_S);

	error = od_st_yolox_pp_process_int8(&pp_input, &pp_output, &pp_params);
	printf("%s: error %d nb_detect %d\n", __func__, error,
	    pp_params.nb_detect);

	rois = pp_output.pOutBuff;

	uint32_t x0;
	uint32_t y0;
	uint32_t width;
	uint32_t height;
	float lcd_xsize = 480;
	float lcd_ysize = 480;
	uint32_t lcd_x0 = 0;

	layer2_clear();
	for (i = 0; i < pp_params.nb_detect; i++) {
		printf("%s: x_center %.08f width %.08f\n", __func__,
		    rois[i].x_center, rois[i].width);
		x0 = (uint32_t) ((rois[i].x_center - rois[i].width / 2) *
		    lcd_xsize) + lcd_x0;
		y0 = (uint32_t) ((rois[i].y_center - rois[i].height / 2) *
		    lcd_ysize);
		width = (uint32_t) (rois[i].width * lcd_xsize);
		height = (uint32_t) (rois[i].height * lcd_ysize);
		printf("Box%d: %d x %d, %d %d\n", i, x0, y0, width, height);
#if 0
		write_hline(x0, y0, width);
		write_hline(x0, y0 + height, width);
#endif

		x0 = (uint32_t) (rois[i].x_center * lcd_xsize);
		y0 = (uint32_t) (rois[i].y_center * lcd_ysize);
		write_hline(x0, y0, 5);
	}
}
