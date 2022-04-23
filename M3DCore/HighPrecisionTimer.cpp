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
#include "HighPrecisionTimer.h"
#include <algorithm>


using namespace m3d;

int64 queryFreq() { int64 i = 0; QueryPerformanceFrequency((LARGE_INTEGER*)&i); return i; }

const int64 HighPrecisionTimer::_freq = queryFreq();


HighPrecisionTimer::HighPrecisionTimer()
{
	_count = 0;
	_dt = 0.0;
	_time = 0.0;
	_dt_us = 0;
	_time_us = 0;
}

HighPrecisionTimer::~HighPrecisionTimer()
{
}

int64 _gettime_us(int64 t, int64 f) { return int64(t / f) * int64(1000000) + int64(t % f) * int64(1000000) / f; }
float64 _gettime_s(int64 t, int64 f) { return float64(t / f) + float64(t % f) / f; }

void HighPrecisionTimer::Tick()
{
	int64 count;
//	DWORD_PTR threadAffMask = SetThreadAffinityMask(GetCurrentThread(), 1);
	QueryPerformanceCounter((LARGE_INTEGER*)&count);
//	SetThreadAffinityMask(GetCurrentThread(),threadAffMask );

	if (_count != 0) {
		_dt = _gettime_s(count - _count, _freq);
		_dt_us = _gettime_us(count - _count, _freq);
	}

	_time = _gettime_s(count, _freq);
	_time_us = _gettime_us(count, _freq);
	_count = count;
}


FPS::FPS() : _fps(0), _c(0) 
{
}

FPS::~FPS() 
{
}

void FPS::NewFrame(int64 currentTimeInUS)
{
	_t[_c++ % 20] = currentTimeInUS;
	if (_c > 1) {
		int64 d = (_t[(_c - 1) % 20] - _t[(_c - std::min(_c, 20u)) % 20]);
		_fps = d > 0 ? (1000000.0 * (std::min(_c, 20u) - 1.0) / d) : 0.0;
	}
}

