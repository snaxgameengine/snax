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
#include "D3D12Include.h"


namespace m3d
{



class GRAPHICSCHIPS_API RingBuffer
{
public:
	RingBuffer();
	~RingBuffer();

	// Allocates the buffer with the given size. Size is automatically incremented if neccessary (not implemented!!)
	HRESULT Init(ID3D12Device *device, UINT64 size);
	
	void Clear();

	// The memory returned is only valid until UpdateFrame() is called!
	HRESULT Allocate(SIZE_T size, UINT alignment, BYTE **mappedPtr, D3D12_GPU_VIRTUAL_ADDRESS *gpuAddress, UINT64 *offset = nullptr);

	// UpdateFrame() must be called before Allocate()!!
	void UpdateFrame(UINT64 lastCompletedFrame, UINT64 currentFrame);

	ID3D12Resource *GetBufferResource() { return _buffer; }

private:
	SIZE_T _align(SIZE_T location, UINT alignment);

	struct Frame
	{
		SIZE_T address;
		SID3D12Resource buffer;

		Frame() : address(0) {}
		Frame(SIZE_T address, SID3D12Resource buffer) : address(address), buffer(buffer) {}
	};
	Frame _frames[COMMAND_LIST_BUFFER_SIZE];
	UINT64 _oldestFrame;
	UINT64 _currentFrame;

	SID3D12Resource _buffer; // Not need for this to be R-pointer!!
	SIZE_T _dataBegin;
	SIZE_T _dataEnd;
	SIZE_T _dataCurrent;

};

}
