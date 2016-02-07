/*
 * Copyright (c) 2018, Bjorn Andersson <bjorn@kryo.se>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef __QMI_TA227_H__
#define __QMI_TA227_H__

#include <stdint.h>
#include <stdbool.h>

#include "libqrtr.h"

#define TA227_OPEN 1
#define TA227_CLOSE 2
#define TA227_READ 4
#define TA227_ITERATE 7

struct ta227_open_req {
	uint32_t unknown1;
	uint32_t unknown2;
};

struct ta227_open_resp {
	uint32_t result;
};

struct ta227_close_req {
};

struct ta227_close_resp {
	uint32_t result;
};

struct ta227_read_req {
	uint32_t unit;
	uint32_t size;
};

struct ta227_read_resp {
	uint32_t result;
	uint32_t data_len;
	uint8_t data[65535];
};

struct ta227_iterate_req {
};

struct ta227_iterate_resp {
	uint32_t result;
	bool unit_valid;
	uint32_t unit;
	bool size_valid;
	uint32_t size;
};

extern struct qmi_elem_info ta227_open_req_ei[];
extern struct qmi_elem_info ta227_open_resp_ei[];
extern struct qmi_elem_info ta227_close_req_ei[];
extern struct qmi_elem_info ta227_close_resp_ei[];
extern struct qmi_elem_info ta227_read_req_ei[];
extern struct qmi_elem_info ta227_read_resp_ei[];
extern struct qmi_elem_info ta227_iterate_req_ei[];
extern struct qmi_elem_info ta227_iterate_resp_ei[];

#endif
