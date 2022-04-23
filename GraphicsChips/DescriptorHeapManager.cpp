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
#include "DescriptorHeapManager.h"
#include "GraphicsChips/GraphicsException.h"

using namespace m3d;




DescriptorHeapManager::DescriptorHeapManager() : _oldestFrame(0), _currentFrame(-1), _handleIncrementSize(0), _cpuHeapStart(CD3DX12_CPU_DESCRIPTOR_HANDLE(CD3DX12_DEFAULT())), _gpuHeapStart(CD3DX12_GPU_DESCRIPTOR_HANDLE(CD3DX12_DEFAULT()))
{
	_desc = { };
}

DescriptorHeapManager::~DescriptorHeapManager()
{
}

HRESULT DescriptorHeapManager::Init(ID3D12Device *device, D3D12_DESCRIPTOR_HEAP_TYPE type, UINT size, BOOL shaderVisible)
{
	HRESULT hr;

	if (shaderVisible && (type != D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV && type != D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER))
		return E_INVALIDARG;

	_device = device;

	_desc.Type = type;
	_desc.NumDescriptors = size;
	_desc.NodeMask = 0;
	_desc.Flags = shaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	V_RETURN(_device->CreateDescriptorHeap(&_desc, IID_PPV_ARGS(&_heap)));

	_handleIncrementSize =  _device->GetDescriptorHandleIncrementSize(_desc.Type);

	_cpuHeapStart = _heap->GetCPUDescriptorHandleForHeapStart();
	_gpuHeapStart = _heap->GetGPUDescriptorHandleForHeapStart();

	_descriptors.reserve(std::min(1024u, _desc.NumDescriptors)); // Reserve size for up to 1024 descriptors!

	_oldestFrame = _currentFrame = 1;

	return S_OK;
}

void DescriptorHeapManager::Clear()
{
	_device = nullptr;
	_heap = nullptr;
}

void DescriptorHeapManager::GetDescriptorTable(UINT count, DescriptorTable **dt)
{
	*dt = nullptr;

	if (count == 0)
		return;

	auto n = _reusables.upper_bound(count - 1);
	if (n != _reusables.end()) {
		DescriptorTable *range = new DescriptorTable(this, n->second, count);
		if (count < n->first)
			_reusables.insert(std::make_pair(n->first - count, n->second + count));
		_reusables.erase(n);
		*dt = range;
		(*dt)->AddRef();
		return;
	}

	if (_descriptors.size() + count > _desc.NumDescriptors)
		return; // Not enough size on heap!

	_descriptors.resize(_descriptors.size() + count);

	DescriptorTable *range = new DescriptorTable(this, (UINT)_descriptors.size() - count, count);
	*dt = range;
	(*dt)->AddRef();
}

void DescriptorHeapManager::CreateDescriptors(const DescriptorTable *table)
{
	assert(table);
	CD3DX12_CPU_DESCRIPTOR_HANDLE cpuHandle(GetCPUHandle(table->offset));
	for (UINT i = 0; i < table->count; i++) {
		const Descriptor &d = table->GetDescriptor(i);
		switch (d.type)
		{
		case DescriptorType::CBV:
			_device->CreateConstantBufferView(d.cbv, cpuHandle);
			break;
		case DescriptorType::SRV:
			_device->CreateShaderResourceView(d.resource, d.srv, cpuHandle);
			break;
		case DescriptorType::UAV:
			_device->CreateUnorderedAccessView(d.resource, nullptr, d.uav, cpuHandle); // TODO: Counter is not yet implemented!
			break;
		case DescriptorType::RTV:
			_device->CreateRenderTargetView(d.resource, d.rtv, cpuHandle);
			break;
		case DescriptorType::DSV:
			_device->CreateDepthStencilView(d.resource, d.dsv, cpuHandle);
			break;
		case DescriptorType::SAMPLER:
			_device->CreateSampler(d.sampler, cpuHandle);
		default: // We skip type NONE. It's no problem if type NONE is among valid descriptor types, as long as it is not used by shaders.
			break;
		};
		cpuHandle.Offset(1, _handleIncrementSize);
	}
}

void DescriptorHeapManager::FreeDescriptorTable(DescriptorTable *table)
{
	assert(_currentFrame != -1);
	_trash[_currentFrame % _countof(_trash)].descriptors.push_back(table);
}

void DescriptorHeapManager::UpdateFrame(UINT64 lastCompletedFrame, UINT64 currentFrame)
{
	assert(currentFrame > lastCompletedFrame);
	assert(_currentFrame == -1 || _currentFrame - _oldestFrame < _countof(_trash) - 1);

	for (; _oldestFrame <= lastCompletedFrame; _oldestFrame++) {// _trash[_oldestFrame % _countof(_trash)].frameNr <= lastCompletedFrameNr; _oldestFrame++) {
		Trash &t = _trash[_oldestFrame % _countof(_trash)];
		for (size_t i = 0; i < t.descriptors.size(); i++) {
			DescriptorTable *dt = t.descriptors[i];
			_reusables.insert(std::make_pair(dt->count, dt->offset));
			for (UINT j = 0; j < dt->count; j++)
				_clearDescriptor(dt->GetDescriptor(j));
			delete dt;
		}
		t.descriptors.clear();
	}

	_currentFrame = currentFrame;

//	if (currentFrame > _currentFrame)
//		_trash[(_currentFrame = currentFrame) % _countof(_trash)].frameNr = currentFrameNr;
}

const Descriptor &DescriptorHeapManager::GetDescriptor(D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle) const
{
	SIZE_T index = (cpuHandle.ptr - _cpuHeapStart.ptr) / _handleIncrementSize;
	if (cpuHandle.ptr < _cpuHeapStart.ptr || index >= _descriptors.size())
		throw GraphicsException(MTEXT("Failed to get descriptor from cpu-handle."));
	return _descriptors[index];
}

const Descriptor &DescriptorHeapManager::GetDescriptor(D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle) const
{
	SIZE_T index = (gpuHandle.ptr - _gpuHeapStart.ptr) / _handleIncrementSize;
	if (gpuHandle.ptr < _gpuHeapStart.ptr || index >= _descriptors.size())
		throw GraphicsException(MTEXT("Failed to get descriptor from gpu-handle."));
	return _descriptors[index];
}

Descriptor &DescriptorHeapManager::InitDescriptor(DescriptorTable *dt, UINT index, DescriptorType type, BOOL createDesc)
{
	assert(type != DescriptorType::NONE);

	Descriptor &d = GetDescriptor(index);

	d.dt = dt;
	d.type = type;

	if (createDesc) 
	{
		switch (type)
		{
		case DescriptorType::CBV: d.cbv = new D3D12_CONSTANT_BUFFER_VIEW_DESC(); break;
		case DescriptorType::SRV: d.srv = new D3D12_SHADER_RESOURCE_VIEW_DESC(); break;
		case DescriptorType::UAV: d.uav = new D3D12_UNORDERED_ACCESS_VIEW_DESC(); break;
		case DescriptorType::RTV: d.rtv = new D3D12_RENDER_TARGET_VIEW_DESC(); break;
		case DescriptorType::DSV: d.dsv = new D3D12_DEPTH_STENCIL_VIEW_DESC(); break;
		case DescriptorType::SAMPLER: d.sampler = new D3D12_SAMPLER_DESC(); break;
		default: d.srv = nullptr; break;
		}
	}

	return d;
}

UINT DescriptorHeapManager::GetIndex(const Descriptor &desc) const 
{ 
	INT64 i = &desc - &_descriptors.front(); 
	if (i < 0 || i >= (INT)_descriptors.size())
		throw GraphicsException(MTEXT("Unknown Descriptor given to DescriptorHeapManager."));
	return (UINT)i; 
}

void DescriptorHeapManager::_clearDescriptor(Descriptor &d)
{
	switch (d.type)
	{
	case DescriptorType::CBV: delete d.cbv; break;
	case DescriptorType::SRV: delete d.srv; break;
	case DescriptorType::UAV: delete d.uav; break;
	case DescriptorType::RTV: delete d.rtv; break;
	case DescriptorType::DSV: delete d.dsv; break;
	case DescriptorType::SAMPLER: delete d.sampler; break;
	}

	d.resource = nullptr;
	d.type = DescriptorType::NONE;
	d.bufferLayoutID = InvalidBufferLayoutID;
	d.cbv = nullptr;
}