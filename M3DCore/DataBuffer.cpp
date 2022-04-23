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
#include "DataBuffer.h"
#include "MemoryManager.h"
#include <cstring>


using namespace m3d;


void DataBuffer::DeallocData(void *data) { mmfree(data); }

DataBuffer::DataBuffer(size_t size) : _data(nullptr), _cdata(nullptr), _size(0), _dealloc(nullptr)
{
	realloc(size);
}

DataBuffer::DataBuffer(const DataBuffer &rhs) : _data(nullptr), _cdata(nullptr), _size(0), _dealloc(nullptr)
{
	*this = rhs;
}

DataBuffer::DataBuffer(DataBuffer &&rhs) : _data(rhs._data), _cdata(rhs._cdata), _size(rhs._size), _dealloc(rhs._dealloc)
{
	rhs._data = nullptr;
	rhs._cdata = nullptr;
	rhs._size = 0;
	rhs._dealloc = nullptr;
}

DataBuffer::DataBuffer(uint8 *data, size_t size, DeallocDataFunc dealloc) : _data(nullptr), _cdata(nullptr), _size(0), _dealloc(nullptr)
{
	setBufferData(data, size, dealloc);
}

DataBuffer::DataBuffer(const uint8 *cdata, size_t size, DeallocDataFunc dealloc) : _data(nullptr), _cdata(nullptr), _size(0), _dealloc(nullptr)
{
	setBufferData(cdata, size, dealloc);
}

DataBuffer::~DataBuffer()
{
	clear();
}

DataBuffer &DataBuffer::operator=(const DataBuffer &rhs)
{
	if (this == &rhs)
		return *this;
	setBufferData(rhs._cdata, rhs._size, rhs._dealloc);
	return *this;
}

DataBuffer &DataBuffer::operator=(DataBuffer &&rhs)
{
	if (this == &rhs)
		return *this;
	clear();
	_data = rhs._data;
	_cdata = rhs._cdata;
	_size = rhs._size;
	_dealloc = rhs._dealloc;
	rhs._data = nullptr;
	rhs._cdata = nullptr;
	rhs._size = 0;
	rhs._dealloc = nullptr;
	return *this;
}

int32 DataBuffer::compare(const DataBuffer &rhs) const
{
	if (!_cdata)
		return rhs._cdata ? 1 : 0;
	if (!rhs._cdata)
		return -1;
	if (_size != rhs._size)
		return _size < rhs._size;
	return std::memcmp(_cdata, rhs._cdata, _size);
}

void DataBuffer::clear()
{
	if (_dealloc && _cdata)
		(*_dealloc)((void*)_cdata);
	_data = nullptr;
	_cdata = nullptr;
	_size = 0;
	_dealloc = nullptr;
}

void DataBuffer::makeDataOwner()
{
	if (isDataOwner())
		return;
	if (_cdata) {
		uint8 *data = (uint8*)mmalloc(_size);
		std::memcpy(data, _cdata, _size);
		if (_dealloc)
			(*_dealloc)((void*)_cdata);
		_cdata = _data = data;
		_dealloc = &DeallocData;
	}
}

uint8 *DataBuffer::getBuffer() 
{ 
	if (_data)
		return _data; 
	if (_cdata) {
		_data = (uint8*)mmalloc(_size);
		std::memcpy(_data, _cdata, _size);
		if (_dealloc)
			(*_dealloc)((void*)_cdata);
		_cdata = _data;
		_dealloc = &DeallocData;
	}
	return _data;
}


void DataBuffer::setBufferData(uint8 *data, size_t size, DeallocDataFunc dealloc)
{
	clear();
	if (size == 0)
		return;
	if (dealloc == &DeallocData) { // do a copy
		_cdata = _data = (uint8*)mmalloc(size);
		_size = size;
		std::memcpy(_data, data, size);
	}
	else {
		_cdata = _data = data;
		_size = size;
	}
	_dealloc = dealloc;
}

void DataBuffer::setBufferData(const uint8 *cdata, size_t size, DeallocDataFunc dealloc)
{
	clear();
	if (size == 0)
		return;
	if (dealloc == &DeallocData) { // do a copy
		_cdata = _data = (uint8*)mmalloc(size);
		_size = size;
		std::memcpy(_data, cdata, size);
	}
	else {
		//_data will be NULL. If editable version is needed, a data copy is performed.
		_cdata = cdata;
		_size = size;
	}
	_dealloc = dealloc;
}


void DataBuffer::realloc(size_t size, bool keepData)
{
	if (!keepData)
		clear();
	uint8 *data = 0;
	if (size > 0)
		data = (uint8*)mmalloc(size);
	if (keepData) {
		if (_size > 0 && size > 0)
			std::memcpy(data, _cdata, _size < size ? _size : size);
		clear();
	}
	_cdata = _data = data;
	_size = size;
	_dealloc = &DeallocData;
}

