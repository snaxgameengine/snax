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

#include <qstring.h>


namespace m3d
{

static QString NumberToString(float64 d)
{
	QString s = QString::number(d, 'f', 15);
	int32 n = s.lastIndexOf('.');
	if (n != -1) {
		for (int32 i = s.length() - 1; i >= n; i--) {
			if (s.at(i) != '0') {
				s = s.left(i == n ? i : i + 1);
				break;
			}
		}
	}
	return s;
}

static QString NumberToString(float32 d)
{
	QString s = QString::number(d, 'f', 7);
	int32 n = s.lastIndexOf('.');
	if (n != -1) {
		for (int32 i = s.length() - 1; i >= n; i--) {
			if (s.at(i) != '0') {
				s = s.left(i == n ? i : i + 1);
				break;
			}
		}
	}
	return s;
}

}