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
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libqrtr.h>

#include "qmi_ta227.h"
#include "qmi_ta228.h"
#include "qmi_svc229.h"
#include "ta.h"

#define MAX(x, y) ((x) > (y) ? (x) : (y))

extern char *__progname;

static int iterator = 0;

static int ta227_open(int sock, struct qrtr_packet *pkt)
{
	DEFINE_QRTR_PACKET(resp_buf, 32);
	struct ta227_open_resp resp;
	struct ta227_open_req req = {};
	unsigned int txn;
	int ret;

	ret = qmi_decode_message(&req, &txn, pkt, QMI_REQUEST, TA227_OPEN,
				 ta227_open_req_ei);
	if (ret < 0) {
		fprintf(stderr, "failed to decode TA227 open request\n");
		resp.result = 1;
	} else {
		resp.result = 0;

		/* Reset iterator */
		iterator = 0;
	}

	ret = qmi_encode_message(&resp_buf, QMI_RESPONSE, TA227_OPEN, txn,
				 &resp, ta227_open_resp_ei);
	if (ret < 0) {
		fprintf(stderr, "failed to encode TA227 open response\n");
		return ret;
	}

	ret = qrtr_sendto(sock, pkt->node, pkt->port, resp_buf.data,
			  resp_buf.data_len);
	if (ret < 0)
		fprintf(stderr, "failed to send TA227 open response\n");

	return 0;
}

static int ta227_close(int sock, struct qrtr_packet *pkt)
{
	DEFINE_QRTR_PACKET(resp_buf, 32);
	struct ta227_close_resp resp = { 0 };
	struct ta227_close_req req = {};
	unsigned int txn;
	int ret;

	ret = qmi_decode_message(&req, &txn, pkt, QMI_REQUEST, TA227_CLOSE,
				 ta227_close_req_ei);
	if (ret < 0) {
		fprintf(stderr, "failed to decode TA227 close request\n");
		resp.result = 1;
	} else {
		resp.result = 0;
	}

	ret = qmi_encode_message(&resp_buf, QMI_RESPONSE, TA227_CLOSE, txn,
				 &resp, ta227_close_resp_ei);
	if (ret < 0) {
		fprintf(stderr, "failed to encode TA227 close response\n");
		return ret;
	}

	ret = qrtr_sendto(sock, pkt->node, pkt->port, resp_buf.data,
			  resp_buf.data_len);
	if (ret < 0)
		fprintf(stderr, "failed to send TA227 close response\n");

	return ret;
}

static int ta227_read(int sock, struct qrtr_packet *pkt)
{
	DEFINE_QRTR_PACKET(resp_buf, 8192);
	struct ta227_read_resp resp = {};
	struct ta227_read_req req = {};
	unsigned int txn;
	size_t size;
	void *buf;;
	int ret;

	ret = qmi_decode_message(&req, &txn, pkt, QMI_REQUEST, TA227_READ,
				 ta227_read_req_ei);
	if (ret < 0) {
		fprintf(stderr, "[TA227] failed to decode read message\n");
		resp.result = 1;
	} else {
		buf = ta_get(req.unit, &size);

		/* XXX: Not sure what to do beyond SMD's maximum of 4k */
		if (size > 4096) {
			resp.result = 1;
		} else {
			resp.result = 0;
			resp.data_len = size;
			memcpy(resp.data, buf, size);
		}
	}

	ret = qmi_encode_message(&resp_buf, QMI_RESPONSE, TA227_READ, txn,
				 &resp, ta227_read_resp_ei);
	if (ret < 0) {
		fprintf(stderr, "[TA227] failed to encode read response\n");
		return ret;
	}

	ret = qrtr_sendto(sock, pkt->node, pkt->port, resp_buf.data,
			  resp_buf.data_len);
	if (ret < 0)
		printf("[TA227] failed to send read response\n");

	return  ret;
}

static int ta227_iterate(int sock, struct qrtr_packet *pkt)
{
	DEFINE_QRTR_PACKET(resp_buf, 64);
	struct ta227_iterate_resp resp = { 0 };
	struct ta227_iterate_req req = {};
	unsigned int txn;
	size_t size;
	int ret;

	ret = qmi_decode_message(&req, &txn, pkt, QMI_REQUEST, TA227_ITERATE,
				 ta227_iterate_req_ei);
	if (ret < 0) {
		fprintf(stderr, "failed to decode TA227 iterate request\n");
		resp.result = 1;
	} else {
		iterator = ta_get_next(iterator, &size);
		if (iterator < 0) {
			resp.result = 1;
		} else {
			resp.result = 0;
			resp.unit_valid = true;
			resp.unit = iterator;

			resp.size_valid = true;
			resp.size = size;
		}
	}

	ret = qmi_encode_message(&resp_buf, QMI_RESPONSE, TA227_ITERATE, txn,
				 &resp, ta227_iterate_resp_ei);
	if (ret < 0) {
		fprintf(stderr, "failed to encode TA227 iterate response\n");
		return ret;
	}

	ret = qrtr_sendto(sock, pkt->node, pkt->port, resp_buf.data,
			  resp_buf.data_len);
	if (ret < 0)
		fprintf(stderr, "failed to send TA227 iterate response\n");


	return ret;
}

static int handle_ta227(int sock, struct qrtr_packet *pkt)
{
	unsigned int msg_id;
	int ret;

	switch (pkt->type) {
	case QRTR_TYPE_DATA:
		ret = qmi_decode_header(pkt, &msg_id);
		if (ret < 0)
			return ret;

		switch (msg_id) {
		case TA227_OPEN:
			ta227_open(sock, pkt);
			break;
		case TA227_CLOSE:
			ta227_close(sock, pkt);
			break;
		case TA227_READ:
			ta227_read(sock, pkt);
			break;
		case TA227_ITERATE:
			ta227_iterate(sock, pkt);
			break;
		default:
			fprintf(stderr, "Unhandled TA227 message: %d\n",
				msg_id);
			break;
		}
		break;
	}

	return 0;
}

static int ta228_get_size(int sock, struct qrtr_packet *pkt)
{
	DEFINE_QRTR_PACKET(resp_buf, 32);
	struct ta228_get_size_resp resp = {};
	struct ta228_get_size_req req = {};
	unsigned int txn;
	size_t size;
	void *buf;;
	int ret;

	ret = qmi_decode_message(&req, &txn, pkt, QMI_REQUEST, TA228_GET_SIZE,
				 ta228_get_size_req_ei);
	if (ret < 0) {
		fprintf(stderr, "[TA228] failed to decode get_size message\n");
		resp.result = 1;
	} else {
		buf = ta_get(req.unit, &size);

		if (!buf) {
			resp.result = 1;
		} else {
			resp.result = 0;
			resp.size_valid = true;
			resp.size = size;;
		}
	}

	ret = qmi_encode_message(&resp_buf, QMI_RESPONSE, TA228_GET_SIZE, txn,
				 &resp, ta228_get_size_resp_ei);
	if (ret < 0) {
		fprintf(stderr, "[TA228] failed to encode get_size response\n");
		return ret;
	}

	ret = qrtr_sendto(sock, pkt->node, pkt->port, resp_buf.data,
			  resp_buf.data_len);
	if (ret < 0)
		fprintf(stderr, "[TA228] failed to send response\n");

	return ret;
}

static int ta228_read(int sock, struct qrtr_packet *pkt)
{
	DEFINE_QRTR_PACKET(resp_buf, 8192);
	struct ta228_read_resp resp = {};
	struct ta228_read_req req = {};
	unsigned int txn;
	size_t size;
	void *buf;
	int ret;

	ret = qmi_decode_message(&req, &txn, pkt, QMI_REQUEST, TA228_READ,
				 ta228_read_req_ei);
	if (ret < 0) {
		fprintf(stderr, "[TA228] failed to decode message\n");
		resp.result = 1;
	} else {
		buf = ta_get(req.unit, &size);

		resp.result = 0;
		resp.data_len = size;
		memcpy(resp.data, buf, size);
	}

	ret = qmi_encode_message(&resp_buf, QMI_RESPONSE, TA228_READ, txn,
				 &resp, ta228_read_resp_ei);
	if (ret < 0) {
		fprintf(stderr, "[TA228] failed to encode response\n");
		return ret;
	}

	ret = qrtr_sendto(sock, pkt->node, pkt->port, resp_buf.data,
			  resp_buf.data_len);
	if (ret < 0)
		printf("[TA228] failed to send response\n");

	return ret;
}

static int handle_ta228(int sock, struct qrtr_packet *pkt)
{
	unsigned int msg_id;
	int ret;

	switch (pkt->type) {
	case QRTR_TYPE_DATA:
		ret = qmi_decode_header(pkt, &msg_id);
		if (ret < 0)
			return ret;

		switch (msg_id) {
		case TA228_GET_SIZE:
			ta228_get_size(sock, pkt);
			break;
		case TA228_READ:
			ta228_read(sock, pkt);
			break;
		default:
			fprintf(stderr, "Unhandled TA228 message: %d\n", msg_id);
			break;
		}
		break;
	}

	return 0;
}

static int svc229_handle_1(int sock, struct qrtr_packet *pkt)
{
	DEFINE_QRTR_PACKET(resp_buf, 8192);
	struct svc229_resp resp = {};
	struct svc229_req req = {};
	unsigned int txn;
	int ret;

	ret = qmi_decode_message(&req, &txn, pkt, QMI_REQUEST, SVC229_MSG1,
				 svc229_req_ei);
	if (ret < 0) {
		fprintf(stderr, "[SVC229] failed to decode message\n");
		resp.result = 1;
	} else {
		resp.result = 0;
		resp.data_len = 1;
		resp.data[0] = 0;
	}

	ret = qmi_encode_message(&resp_buf, QMI_RESPONSE, SVC229_MSG1, txn,
				 &resp, svc229_resp_ei);
	if (ret < 0) {
		fprintf(stderr, "[SVC229] failed to encode response\n");
		return ret;
	}

	ret = qrtr_sendto(sock, pkt->node, pkt->port, resp_buf.data, resp_buf.data_len);
	if (ret < 0)
		fprintf(stderr, "[SVC229] failed to send response\n");

	return ret;
}

static int handle_svc229(int sock, struct qrtr_packet *pkt)
{
	unsigned int msg_id;
	int ret;

	switch (pkt->type) {
	case QRTR_TYPE_DATA:
		ret = qmi_decode_header(pkt, &msg_id);
		if (ret < 0)
			return ret;

		switch (msg_id) {
		case 1:
			svc229_handle_1(sock, pkt);
			break;
		default:
			fprintf(stderr, "Unhandled SVC229 message: %d\n", msg_id);
			break;
		}
	}

	return 0;
}

int main(int argc, char **argv)
{
	struct sockaddr_qrtr sq;
	struct qrtr_packet pkt;
	socklen_t sl;
	char buf[4096];
	fd_set rfds;
	int fds[3];
	int sock;
	int nfds;
	int ret;
	int i;

	if (argc != 2) {
		fprintf(stderr, "%s <partition>\n", __progname);
		exit(1);
	}

	ret = ta_load(argv[1]);
	if (ret < 0)
		exit(1);

	for (i = 0; i < 3; i++) {
		fds[i] = qrtr_open(0);
		if (fds[i] < 0) {
			fprintf(stderr, "failed to create qrtr socket");
			exit(1);
		}

		ret = qrtr_publish(fds[i], 227 + i, 1, 0);
		if (ret < 0) {
			fprintf(stderr, "failed to publish service %d", 227 + i);
			exit(1);
		}
	}

	for (;;) {
		FD_ZERO(&rfds);
		nfds = 0;

		for (i = 0; i < 3; i++) {
			FD_SET(fds[i], &rfds);
			nfds = MAX(nfds, fds[i]);
		}

		ret = select(nfds + 1, &rfds, NULL, NULL, NULL);
		if (ret < 0) {
			fprintf(stderr, "select failed: %d\n", ret);
			break;
		} else if (ret == 0) {
			continue;
		}

		for (i = 0; i < 3; i++) {
			sock = fds[i];

			if (!FD_ISSET(sock, &rfds))
				continue;

			sl = sizeof(sq);
			ret = recvfrom(sock, buf, sizeof(buf), 0,
				       (struct sockaddr *)&sq, &sl);
			if (ret < 0) {
				fprintf(stderr, "recvfrom failed: %d\n", ret);
				return ret;
			}

			ret = qrtr_decode(&pkt, buf, ret, &sq);
			if (ret < 0) {
				fprintf(stderr, "failed to decode message\n");
				return ret;
			}

			switch (i) {
			case 0:
				handle_ta227(sock, &pkt);
				break;
			case 1:
				handle_ta228(sock, &pkt);
				break;
			case 2:
				handle_svc229(sock, &pkt);
				break;
			}
		}
	}

	return 0;
}
