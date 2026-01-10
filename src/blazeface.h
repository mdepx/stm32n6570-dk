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

#ifndef _SRC_BLAZEFACE_H_
#define	_SRC_BLAZEFACE_H_

int blazeface_init(NN_Instance_TypeDef *);
void blazeface_process(uint32_t **);

#define POSTPROCESS_TYPE POSTPROCESS_OD_BLAZEFACE_UI

/* I/O configuration */
#define AI_OD_BLAZEFACE_PP_NB_KEYPOINTS              (6)
#define AI_OD_BLAZEFACE_PP_NB_CLASSES                (1)
#define AI_OD_BLAZEFACE_PP_IMG_SIZE                (128)

#define AI_OD_BLAZEFACE_PP_OUT_0_NB_BOXES          (512)
#define AI_OD_BLAZEFACE_PP_OUT_1_NB_BOXES          (384)

/* Postprocessing */
#define AI_OD_BLAZEFACE_PP_CONF_THRESHOLD    (0.6000000000f)
#define AI_OD_BLAZEFACE_PP_IOU_THRESHOLD     (0.3000000000f)
#define AI_OD_BLAZEFACE_PP_MAX_BOXES_LIMIT   (10)

#endif /* !_SRC_BLAZEFACE_H_ */
