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

#ifndef _SRC_YOLOX_H_
#define	_SRC_YOLOX_H_

int yolox_init(NN_Instance_TypeDef *);
void yolox_process(uint32_t **);

/* I/O configuration */
/* Postprocessing ST_YOLO_X configuration */
#define AI_OD_ST_YOLOX_PP_NB_CLASSES        (1)
#define AI_OD_ST_YOLOX_PP_L_GRID_WIDTH      (60)
#define AI_OD_ST_YOLOX_PP_L_GRID_HEIGHT     (60)
#define AI_OD_ST_YOLOX_PP_L_NB_INPUT_BOXES  (AI_OD_ST_YOLOX_PP_L_GRID_WIDTH * AI_OD_ST_YOLOX_PP_L_GRID_HEIGHT)
#define AI_OD_ST_YOLOX_PP_M_GRID_WIDTH      (30)
#define AI_OD_ST_YOLOX_PP_M_GRID_HEIGHT     (30)
#define AI_OD_ST_YOLOX_PP_M_NB_INPUT_BOXES  (AI_OD_ST_YOLOX_PP_M_GRID_WIDTH * AI_OD_ST_YOLOX_PP_M_GRID_HEIGHT)
#define AI_OD_ST_YOLOX_PP_S_GRID_WIDTH      (15)
#define AI_OD_ST_YOLOX_PP_S_GRID_HEIGHT     (15)
#define AI_OD_ST_YOLOX_PP_S_NB_INPUT_BOXES  (AI_OD_ST_YOLOX_PP_S_GRID_WIDTH * AI_OD_ST_YOLOX_PP_S_GRID_HEIGHT)
#define AI_OD_ST_YOLOX_PP_NB_ANCHORS        (3)

static const float AI_OD_ST_YOLOX_PP_L_ANCHORS[2*AI_OD_ST_YOLOX_PP_NB_ANCHORS] = {30.000000, 30.000000, 4.200000, 15.000000, 13.800000, 41.999999};
static const float AI_OD_ST_YOLOX_PP_M_ANCHORS[2*AI_OD_ST_YOLOX_PP_NB_ANCHORS] = {15.000000, 15.000000, 2.100000, 7.500000, 6.900000, 21.000000};
static const float AI_OD_ST_YOLOX_PP_S_ANCHORS[2*AI_OD_ST_YOLOX_PP_NB_ANCHORS] = {7.500000, 7.500000, 1.050000, 3.750000, 3.450000, 10.500000};

#define AI_OD_ST_YOLOX_PP_IOU_THRESHOLD		(0.5)
#define AI_OD_ST_YOLOX_PP_CONF_THRESHOLD	(0.6)
#define AI_OD_ST_YOLOX_PP_MAX_BOXES_LIMIT	(100)

#endif /* !_SRC_YOLOX_H_ */
