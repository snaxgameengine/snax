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


//#define  XM_NO_OPERATOR_OVERLOADS
#include "Exports.h"
#include "MTypes.h"
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include "Containers.h"

using namespace DirectX;
using namespace DirectX::PackedVector;

namespace m3d
{

static bool flteql(float32 A, float32 B, float32 maxDiff = 1.0e-10f, float32 maxRelDiff = 1.0e-5f)
{
    float32 diff = fabs(A - B);
    return diff <= maxDiff || diff <= (((B = fabs(B)) > (A = fabs(A)) ? B : A) * maxRelDiff); 
}

const XMUBYTE4 BLACK = XMUBYTE4(0xFF000000);
const XMUBYTE4 GRAY25 = XMUBYTE4(0xFF404040);
const XMUBYTE4 GRAY = XMUBYTE4(0xFF808080);
const XMUBYTE4 GRAY75 = XMUBYTE4(0xFFC0C0C0);
const XMUBYTE4 WHITE = XMUBYTE4(0xFFFFFFFF);
const XMUBYTE4 RED = XMUBYTE4(0xFF0000FF);
const XMUBYTE4 GREEN = XMUBYTE4(0xFF00FF00);
const XMUBYTE4 BLUE = XMUBYTE4(0xFFFF0000);
const XMUBYTE4 YELLOW = XMUBYTE4(0xFF00FFFF);
const XMUBYTE4 MAGENTA = XMUBYTE4(0xFFFF00FF);
const XMUBYTE4 CYAN = XMUBYTE4(0xFFFFFF00);




}
