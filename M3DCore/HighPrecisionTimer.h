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

namespace m3d
{

class M3DCORE_API HighPrecisionTimer
{
private:
	int64 _count;
	int64 _dt_us;
	int64 _time_us;
	float64 _dt;
	float64 _time;

	static const int64 _freq;

public:
	HighPrecisionTimer();
	~HighPrecisionTimer();

	void Tick();

	__declspec(deprecated) float64 GetDt() const { return _dt; }
	__declspec(deprecated) float64 GetTime() const { return _time; }

	int64 GetDt_us() const { return _dt_us; }
	int64 GetTime_us() const { return _time_us; }
};

class M3DCORE_API FPS
{
	int64 _t[20];
	float64 _fps;
	uint32 _c;

public:
	FPS();
	~FPS();

	void NewFrame(int64 currentTimeInUS);

	float64 GetFPS() const { return _fps; }
	uint32 GetFrameCount() const { return _c; }

};


}