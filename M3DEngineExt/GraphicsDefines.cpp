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
#include "GraphicsDefines.h"

using namespace m3d;


const ColorScheme m3d::StdColors = 
{
	Color(0.1f, 0.3f, 0.05f, 1.0f),
	Color(0.05f, 0.2f, 0.3f, 1.0f),
	Color(0.3f, 0.3f, 0.04f, 1.0f),
	Color(0.0f, 0.0f, 0.0f, 1.0f),
	Color(1.0f, 0.0f, 0.0f, 1.0f),
	Color(1.0f, 0.9f, 0.5f, 1.0f),
	Color(0.0f, 0.0f, 0.0f, 0.5f),
	Color(0.8f, 0.9f, 0.8f, 1.0f),
	Color(0.1f, 1.0f, 1.0f, 1.0f),
	Color(0.1f, 0.25f, 0.7f, 1.0f),
	Color(0.0f, 1.0f, 0.0f, 1.0f),
	Color(1.0f, 0.0f, 0.0f, 1.0f),
	Color(0.7f, 0.8f, 0.7f, 1.0f),
	Color(1.0f, 0.5f, 0.0f, 1.0f),
	Color(0.05f, 0.05f, 1.0f, 1.0f), // active
	Color(1.0f, 1.0f, 1.0f, 1.0f),
	Color(0.0f, 1.0f, 0.0f, 1.0f),
	Color(0.1f, 0.6f, 1.0f, 1.0f),
	Color(0.35f, 0.35f, 0.6f, 1.0f),
	Color(1.0f, 1.0f, 0.2f, 1.0f),
	Color(0.6f, 1.0f, 0.45f, 1.0f),
	Color(0.7f, 1.0f, 0.9f, 1.0f),
	Color(0.6f, 0.0f, 0.0f, 1.0f),
	Color(1.0f, 0.5f, 1.0f, 1.0f),
	Color(0.5f, 0.0f, 0.5f, 1.0f),
	Color(1.0f, 0.5f, 0.5f, 1.0f),
	Color(1.0f, 0.5f, 0.0f, 1.0f),
	Color(1.0f, 0.75f, 0.0f, 1.0f),
	Color(0.6f, 0.8f, 1.0f, 1.0f),
	Color(0.5f, 0.5f, 0.5f, 0.5f),
	Color(0.75f, 0.75f, 0.75f, 1.0f),
	Color(0.5f, 0.5f, 0.5f, 1.0f)
};

