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
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>

#define TA_MAGIC	0x3bf8e9c1
#define TA_BLOCK_SIZE	0x20000

typedef uint32_t __le32;

struct unit {
	struct unit *next;

	unsigned id;
	size_t len;

	uint8_t data[];
};

struct phys_unit {
	__le32 id;
	__le32 len;
	__le32 magic;
	__le32 reserved;
	uint8_t data[];
};

struct phys_block {
	__le32 magic;
	__le32 unknown[2];
};

static struct unit *units;

static void ta_parse_block(void *ptr)
{
	struct phys_unit *phys_unit;
	struct unit *unit;

	for (;;) {
		phys_unit = ptr;
		if (phys_unit->magic != TA_MAGIC)
			break;

		unit = malloc(sizeof(struct unit) + phys_unit->len);
		unit->id = phys_unit->id;
		unit->len = phys_unit->len;
		memcpy(unit->data, phys_unit->data, phys_unit->len);

		unit->next = units;
		units = unit;

		ptr += sizeof(struct phys_unit) + ((phys_unit->len + 3) & ~3);
	}
}

int ta_load(const char *path)
{
	struct phys_block *phys_block;
	off_t offset;
	void *mem;
	int fd;
	int n;

	mem = malloc(TA_BLOCK_SIZE);
	if (!mem) {
		fprintf(stderr, "failed to allocate scratch buffer");
		exit(1);
	}

	fd = open(path, O_RDONLY);
	if (fd < 0) {
		fprintf(stderr, "failed to open %s", path);
		exit(1);
	}

	for (offset = 0; ; offset += TA_BLOCK_SIZE) {
		n = pread(fd, mem, sizeof(struct phys_block), offset);
		if (n != sizeof(struct phys_block))
			break;

		phys_block = mem;
		if (phys_block->magic == TA_MAGIC) {
			n = pread(fd, mem, TA_BLOCK_SIZE, offset);
			if (n < 0) {
				fprintf(stderr, "failed to read ta phys_block");
				exit(1);
			}

			ta_parse_block(mem + sizeof(struct phys_block));

			break;
		}
	}

	close(fd);
	free(mem);

	return 0;
}

void *ta_get(unsigned id, size_t *len)
{
	struct unit *unit;

	for (unit = units; unit; unit = unit->next) {
		if (unit->id == id) {
			*len = unit->len;
			return unit->data;
		}
	}

	return NULL;
}

int ta_get_next(int id, size_t *len)
{
	struct unit *unit;

	if (!units)
		return -1;

	if (!id) {
		*len = units->len;
		return units->id;
	}

	for (unit = units; unit; unit = unit->next) {
		if (unit->id == id)
			break;
	}

	if (!unit || !unit->next)
		return -1;

	unit = unit->next;

	*len = unit->len;
	return unit->id;
}
