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

#pragma once

#include "Exports.h"
#include "MTypes.h"
#include "MString.h"

namespace m3d
{

struct M3DCORE_API Base64
{
	// targetlen must be >= (sourcelen + 2 - ((sourcelen + 2) % 3)) / 3 * 4 + 1
	static bool Encode(uint8 const *source, size_t sourcelen, Char *target, size_t targetlen);
	static size_t CalculateTargetLengthForEncoding(size_t sourcelen) { return (sourcelen + 2 - ((sourcelen + 2) % 3)) / 3 * 4 + 1; }

	// if *target != 0 => targetlen must be >= strlen(source)/4*3 where strlen(source)%4==0 (must be multiply of 4)
	static bool Decode(Char const *source, uint8 **target, size_t &targetlen);
	static size_t CalculateTargetLengthForDecoding(Char const *source);
};

}