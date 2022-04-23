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
#include "RingBuffer.h"

using namespace m3d;



RingBuffer::RingBuffer() : _oldestFrame(0), _currentFrame(0), _dataBegin(0), _dataEnd(0), _dataCurrent(0)
{
}

RingBuffer::~RingBuffer()
{
}


HRESULT RingBuffer::Init(ID3D12Device *device, UINT64 size)
{
	HRESULT hr;
	 
	CD3DX12_HEAP_PROPERTIES hp(D3D12_HEAP_TYPE_UPLOAD);
	CD3DX12_RESOURCE_DESC b = CD3DX12_RESOURCE_DESC::Buffer(size);
	V_RETURN(device->CreateCommittedResource( 
		&hp, 	
		D3D12_HEAP_FLAG_NONE, 
		&b, 
		D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, 	
		IID_PPV_ARGS( &_buffer )));
	
	void *data = nullptr;
	V_RETURN(_buffer->Map(0, nullptr, (void**)&data)); 

	_dataBegin = _dataCurrent = reinterpret_cast<SIZE_T>(data);
	_dataEnd = _dataBegin + size;

	_oldestFrame = 1;
	_currentFrame = 1;

	String s = strUtils::ConstructString("Ring buffer (%1 bytes)").arg(size);
	_buffer->SetPrivateData(WKPDID_D3DDebugObjectName, (UINT)s.size(), s.c_str());
	return S_OK;
}

void RingBuffer::Clear()
{
	for (uint32 i = 0; i < COMMAND_LIST_BUFFER_SIZE; i++)
		_frames[i] = Frame();
	_buffer = nullptr;
	_oldestFrame = 0;
	_currentFrame = 0;
	_dataBegin = 0;
	_dataEnd = 0;
	_dataCurrent = 0;
}

HRESULT RingBuffer::Allocate(SIZE_T size, UINT alignment, BYTE **mappedPtr, D3D12_GPU_VIRTUAL_ADDRESS *gpuAddress, UINT64 *offset)
{
	assert(_currentFrame != -1);
	if (size == 0 || alignment == 0 || (alignment & (alignment - 1)))
		return E_INVALIDARG;

	SIZE_T dataCurrent = _align(_dataCurrent, alignment);
	
	if (_currentFrame > _oldestFrame) { // Do we have a queue of frames?
		SIZE_T dataEnd = _frames[_oldestFrame % _countof(_frames)].address;
		if (dataEnd > _dataCurrent) {
			if (dataEnd < dataCurrent || dataEnd - dataCurrent < size)
				return E_NOT_SUFFICIENT_BUFFER;
		}
		else { // There is space until the end of the buffer...
			if (_dataEnd < dataCurrent || _dataEnd - dataCurrent < size) { // Not enough space at the end of the buffer?
				if (_dataCurrent == _dataBegin)
					return E_NOT_SUFFICIENT_BUFFER;
				dataCurrent = _align(_dataBegin, alignment);
				if (_dataEnd < dataCurrent || _dataEnd - dataCurrent < size)
					return E_NOT_SUFFICIENT_BUFFER;
			}
		}
	}
	else {
		if (_dataEnd < dataCurrent || _dataEnd - dataCurrent < size) {
			if (_dataCurrent == _dataBegin)
				return E_NOT_SUFFICIENT_BUFFER;
			dataCurrent = _align(_dataBegin, alignment);
			if (_dataEnd < dataCurrent || _dataEnd - dataCurrent < size)
				return E_NOT_SUFFICIENT_BUFFER;
		}
	}

	_dataCurrent = dataCurrent + size;
	if (mappedPtr)
		*mappedPtr = reinterpret_cast<BYTE*>(dataCurrent);
	if (gpuAddress)
		*gpuAddress = _buffer->GetGPUVirtualAddress() + (dataCurrent - _dataBegin);
	if (offset)
		*offset = UINT64(dataCurrent - _dataBegin);
	return S_OK;
}

void RingBuffer::UpdateFrame(UINT64 lastCompletedFrame, UINT64 currentFrame)
{
	assert(currentFrame - lastCompletedFrame <= _countof(_frames));
	assert(currentFrame >= _currentFrame);

	while (_oldestFrame <= lastCompletedFrame)
		_frames[_oldestFrame++ % _countof(_frames)] = Frame();

	if (currentFrame > _currentFrame)
		_frames[(_currentFrame = currentFrame) % _countof(_frames)] = Frame(_dataCurrent, _buffer);


}

SIZE_T RingBuffer::_align(SIZE_T location, UINT alignment)
{
	return (location + alignment - 1) & ~UINT64(alignment - 1);
}
