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
#include "Util.h"
#include <fstream>


using namespace m3d;



bool m3d::LoadDataBuffer(Path filename, DataBuffer &db)
{
	db.clear();
	std::ifstream is(filename.AsString().c_str(), std::ios::binary | std::ios::ate);
	if (!is.is_open())
		return false;
	size_t size = (size_t)is.tellg();
	if (size) {
		db.realloc(size);
		is.seekg(0, std::ios::beg);
		is.read((char*)db.getBuffer(), size);
	}
	is.close();
	return true;
}

bool m3d::SaveDataBuffer(Path filename, const DataBuffer &db)
{
	std::ofstream os(filename.AsString().c_str(), std::ios::binary);
	if (!os.is_open())
		return false;
	os.write((const char*)db.getConstBuffer(), db.getBufferSize());
	os.close();
	return true;
}