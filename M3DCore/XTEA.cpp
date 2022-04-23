// SnaX Game Engine - https://github.com/snaxgameengine/snax
// Licensed under the MIT License <http://opensource.org/licenses/MIT>.
// SPDX-License-Identifier: MIT
// Copyright (c) 2013 - 2022 Frank-Vegar Mortensen <franksvm(at)outlook(dot)com>.
//
// Permission is hereby  granted, free of charge, to any  person obtaining a copy
// of this software and associated  documentation files (the "Software"), to deal
// in the Software  without restriction, including without  limitation the rights
// to  use, copy,  modify, merge,  publish, distribute,  sublicense, and/or  sell
// copies  of  the Software,  and  to  permit persons  to  whom  the Software  is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE  IS PROVIDED "AS  IS", WITHOUT WARRANTY  OF ANY KIND,  EXPRESS OR
// IMPLIED,  INCLUDING BUT  NOT  LIMITED TO  THE  WARRANTIES OF  MERCHANTABILITY,
// FITNESS FOR  A PARTICULAR PURPOSE AND  NONINFRINGEMENT. IN NO EVENT  SHALL THE
// AUTHORS  OR COPYRIGHT  HOLDERS  BE  LIABLE FOR  ANY  CLAIM,  DAMAGES OR  OTHER
// LIABILITY, WHETHER IN AN ACTION OF  CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE  OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "pch.h"
#include "XTEA.h"
#include "PlatformDef.h"
#include <ppl.h>
#include <algorithm>

using namespace m3d;


static const uint32 delta = 0x9E3779B9;
static const uint32 rounds = 32 * 2;
static const uint32 jobsize = 16 * 1024 * 1024;

typedef void (*job_func)(void *data, uint64 size, const uint32 *X);

static void _enc(void *data, uint64 size, const uint32 *X)
{
	for (uint64 i = 0, j = size / 8; i < j; i++) {
		uint32 &v0 = *((uint32*)data + i * 2), &v1 = *((uint32*)data + i * 2 + 1);
		for (uint32 k = 0; k < rounds;) {
			v0 += (((v1 << 4) ^ (v1 >> 5)) + v1) ^ X[k++];
			v1 += (((v0 << 4) ^ (v0 >> 5)) + v0) ^ X[k++];
		}
	}
}

static void _dec(void *data, uint64 size, const uint32 *X)
{
	for (uint64 i = 0, j = size / 8; i < j; i++) {
		uint32 &v0 = *((uint32*)data + i * 2), &v1 = *((uint32*)data + i * 2 + 1);
		for (uint32 k = 0; k < rounds;) {
			v1 -= (((v0 << 4) ^ (v0 >> 5)) + v0) ^ X[k++];
			v0 -= (((v1 << 4) ^ (v1 >> 5)) + v1) ^ X[k++];
		}
	}
}


static void _startJobs(void *data, uint64 size, const uint32 *X, job_func func)
{
	if (size < jobsize)
		return (*func)(data, size, X);
	else {
#if 1 // Lets go multithreaded!
		uint32 v = (uint32)((size + jobsize - 1) / jobsize);

		concurrency::parallel_for(uint32(0), v, [&](uint32 i) 
		{
			(*func)((uint32*)data + (i * jobsize) / sizeof(uint32), std::min(size - i * jobsize, (uint64)jobsize), X);
		});
#else
		return (*func)(data, size, X);
#endif
	}
}

void XTEA::enc(void *data, uint64 size, const uint32 key[4])
{
	uint32 X[64];
	uint32 sum = 0;
	for (uint32 i = 0; i < rounds;) {
		X[i++] = sum + key[sum & 3];
		sum += delta;
		X[i++] = sum + key[(sum>>11) & 3];
	}

	_startJobs(data, size, X, _enc);
}


void XTEA::dec(void *data, uint64 size, const uint32 key[4])
{
	uint32 X[64];
	uint32 sum = delta * (rounds / 2); // Ignore C4307
	for (uint32 i = 0; i < rounds;) {
		X[i++] = sum + key[(sum>>11) & 3];
		sum -= delta;
		X[i++] = sum + key[sum & 3];
	}

	_startJobs(data, size, X, _dec);
}