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

class M3DCORE_API DataBuffer
{
public:
	typedef void (*DeallocDataFunc)(void *data);

private:
	// _data==_cdata
	// _data!=_cdata => _data==NULL
	uint8 *_data; 
	const uint8 *_cdata; // _cdata is ALWAYS present if not empty!
	size_t _size;
	DeallocDataFunc _dealloc;
	static void DeallocData(void *data);

public:
	DataBuffer() : _data(nullptr), _cdata(nullptr), _size(0), _dealloc(nullptr) {}
	DataBuffer(const DataBuffer &rhs);
	DataBuffer(DataBuffer &&rhs);
	explicit DataBuffer(size_t size);
	explicit DataBuffer(uint8 *data, size_t size, DeallocDataFunc dealloc = &DeallocData); // default third: copy data and be owner! dealloc may be NULL if data is not to be deleted.
	explicit DataBuffer(const uint8 *cdata, size_t size, DeallocDataFunc dealloc = &DeallocData); // default third: copy data and be owner! dealloc may be NULL if data is not to be deleted.
	~DataBuffer();

	DataBuffer &operator=(const DataBuffer &rhs);
	DataBuffer &operator=(DataBuffer &&rhs);

	inline bool operator==(const DataBuffer &rhs) const { return compare(rhs) == 0; }
	inline bool operator!=(const DataBuffer &rhs) const { return compare(rhs) != 0; }
	inline bool operator<(const DataBuffer &rhs) const { return compare(rhs) < 0; }
	inline bool operator>(const DataBuffer &rhs) const { return compare(rhs) > 0; }
	int32 compare(const DataBuffer &rhs) const;

	// These methods return a non-editable version of the data.
//	const uint8 *getBuffer() const { return _cdata; } // Commented to avoid unintentional data copies! Use getConstBuffer()!
	const uint8 *getConstBuffer() const { return _cdata; }
	// This method return an editable version of the data. 
	// NOTE: The compiler will prefer this method over the const version, if the object itself is not const. This may lead to an unintentional data-copy! 
	// Therefore: Use GetConstBuffer(), to get non-editable data!
	uint8 *getBuffer();
	// Data size in bytes.
	size_t getBufferSize() const { return _size; }
	// true if the data is non-editable and require an internal data-copy to get an editable version of the data.
	// This will only be true if this is not the data owner.
	bool isConstantBuffer() const { return _data != _cdata; }
	// true if the data is allocated inside this object.
	bool isDataOwner() const { return _dealloc == &DeallocData; }
	// Clears the buffer.
	void clear();
	// Makes us the owner of the data by doing a data copy.
	void makeDataOwner();
	// Set editable data to the buffer. If using standard deallocator, data-copy is performed.
	void setBufferData(uint8 *data, size_t size, DeallocDataFunc dealloc = &DeallocData); // default third: copy data and be owner!
	// Set non-editable data to the buffer. If using standard deallocator, data-copy is performed, and data will be editable. If not, data copy is performed when editable data is requested.
	void setBufferData(const uint8 *cdata, size_t size, DeallocDataFunc dealloc = &DeallocData); // default third: copy data and be owner!
	// Allocate memory to be able to keep size bytes of data.
	void realloc(size_t size, bool keepData = false);
};


}