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
#include "Base64.h"
#include "MemoryManager.h"
#include "MString.h"

using namespace m3d;


const Char *BASE64_CHARS = MTEXT("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/");
const uint8 BASE64_CHARS_INV[256] = 
{
	255,255,255,255,255,255,255,255,
	255,255,255,255,255,255,255,255,
	255,255,255,255,255,255,255,255,
	255,255,255,255,255,255,255,255,
	255,255,255,255,255,255,255,255,
	255,255,255,62,255,255,255,63,
	52,53,54,55,56,57,58,59,
	60,61,255,255,255,254,255,255,
	255,0,1,2,3,4,5,6,
	7,8,9,10,11,12,13,14,
	15,16,17,18,19,20,21,22,
	23,24,25,255,255,255,255,255,
	255,26,27,28,29,30,31,32,
	33,34,35,36,37,38,39,40,
	41,42,43,44,45,46,47,48,
	49,50,51,255,255,255,255,255,
	255,255,255,255,255,255,255,255,
	255,255,255,255,255,255,255,255,
	255,255,255,255,255,255,255,255,
	255,255,255,255,255,255,255,255,
	255,255,255,255,255,255,255,255,
	255,255,255,255,255,255,255,255,
	255,255,255,255,255,255,255,255,
	255,255,255,255,255,255,255,255,
	255,255,255,255,255,255,255,255,
	255,255,255,255,255,255,255,255,
	255,255,255,255,255,255,255,255,
	255,255,255,255,255,255,255,255,
	255,255,255,255,255,255,255,255,
	255,255,255,255,255,255,255,255,
	255,255,255,255,255,255,255,255,
	255,255,255,255,255,255,255,255
};


void __base64_encode_triple(uint8 const triple[3], Char result[4])
{
	result[0] = BASE64_CHARS[(triple[0] & 0xFC) >> 2];
	result[1] = BASE64_CHARS[((triple[0] & 0x03) << 4) | ((triple[1] & 0xF0) >> 4)];
	result[2] = BASE64_CHARS[((triple[1] & 0x0F) << 2) | ((triple[2] & 0xC0) >> 6)];
	result[3] = BASE64_CHARS[(triple[2] & 0x3F)];
} 

void __base64_decode_triple(Char const quadruple[4], uint8 result[3])
{
	result[0] = (BASE64_CHARS_INV[quadruple[0]] << 2) | ((BASE64_CHARS_INV[quadruple[1]] & 0x30) >> 4);
	result[1] = ((BASE64_CHARS_INV[quadruple[1]] & 0x0F) << 4) | ((BASE64_CHARS_INV[quadruple[2]] & 0x3C) >> 2);
	result[2] = ((BASE64_CHARS_INV[quadruple[2]] & 0x03) << 6) | (BASE64_CHARS_INV[quadruple[3]]);
}

void __base64_decode_double(Char const quadruple[4], uint8 result[3])
{
	result[0] = (BASE64_CHARS_INV[quadruple[0]] << 2) | ((BASE64_CHARS_INV[quadruple[1]] & 0x30) >> 4);
	result[1] = ((BASE64_CHARS_INV[quadruple[1]] & 0x0F) << 4) | ((BASE64_CHARS_INV[quadruple[2]] & 0x3C) >> 2);
}

void __base64_decode_single(Char const quadruple[4], uint8 result[3])
{
	result[0] = (BASE64_CHARS_INV[quadruple[0]] << 2) | ((BASE64_CHARS_INV[quadruple[1]] & 0x30) >> 4);
}

bool Base64::Encode(uint8 const *source, size_t sourcelen, Char*target, size_t targetlen)
{
	if ((sourcelen+2)/3*4 > targetlen-1)
		return false;

	for (; sourcelen >= 3; sourcelen -= 3, source += 3, target += 4)
		__base64_encode_triple(source, target);

	if (sourcelen > 0) {
		uint8 temp[3] = {source[0], sourcelen > 1 ? source[1] : uint8('\0'), uint8('\0') };
		__base64_encode_triple(temp, target);
		target[3] = '=';
		if (sourcelen == 1)
			 target[2] = '=';
		target += 4;
	}

	target[0] = 0;

	return true;
} 

size_t Base64::CalculateTargetLengthForDecoding(Char const *source)
{
	size_t sourcelen = 0;
	size_t g = 0;
	for (; true; sourcelen++) {
		if (source[sourcelen] == MCHAR('\0'))
			break;
		if (g != 0 && BASE64_CHARS_INV[source[sourcelen]] != 254)
			return false; // Invalid input
		if (BASE64_CHARS_INV[source[sourcelen]] == 255)
			return false; // Invalid input
		if (BASE64_CHARS_INV[source[sourcelen]] == 254)
			g++;
	}

	if (g > 2 || sourcelen % 4)
		return -1; // Invalid input

	g = sourcelen / 4 * 3 - g;

	return g;
}

bool Base64::Decode(Char const *source, uint8 **target, size_t &targetlen)
{
	size_t g = CalculateTargetLengthForDecoding(source);
	if (g == -1)
		return false; // Invalid input

	if (g == 0)
		return true;

	if (*target && targetlen < g)
		return false; // Target too small

	targetlen = g;

	if (!*target)
		*target = (uint8*)mmalloc(targetlen);

	uint8 *t = *target;

	for (; g >= 3; g -= 3, source += 4, t += 3)
		__base64_decode_triple(source, t);

	if (g == 1)
		__base64_decode_single(source, t);
	else if (g == 2)
		__base64_decode_double(source, t);

	return true;
} 
