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

#include "DataBuffer.h"


namespace m3d
{

class M3DCORE_API DataBufferOutputStream
{
public:
	DataBufferOutputStream(DataBuffer &db, size_t pos = 0);
	~DataBufferOutputStream() {}

	DataBuffer &GetDataBuffer() const { return _db; }
	size_t GetBufferPos() const { return _ptr; }
	bool IsError() const { return _error; }
	void SetBufferPos(size_t p) { _ptr = p; }
	void MoveBufferPos(SSIZE_T dp) { _ptr += dp; }

	DataBufferOutputStream &Write(const void *data, size_t size);

	template<typename T>
	DataBufferOutputStream &operator<<(const T &t)
	{
		return Write(&t, sizeof(t));
	}

private:
	DataBuffer &_db;
	size_t _ptr;
	bool _error;
};

class M3DCORE_API DataBufferInputStream
{
public:
	DataBufferInputStream(const DataBuffer &db, size_t pos = 0);
	~DataBufferInputStream() {}

	const DataBuffer &GetDataBuffer() const { return _db; }
	size_t GetBufferPos() const { return _ptr; }
	bool IsError() const { return _error; }
	void SetBufferPos(size_t p) { _ptr = p; }
	void MoveBufferPos(SSIZE_T dp) { _ptr += dp; }

	DataBufferInputStream &Read(void *data, size_t size);

	template<typename T>
	DataBufferInputStream &operator>>(T &t)
	{
		return Read(&t, sizeof(t));
	}

private:
	const DataBuffer &_db;
	size_t _ptr;
	bool _error;
};

}