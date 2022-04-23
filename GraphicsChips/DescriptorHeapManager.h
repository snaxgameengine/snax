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
#include "BufferLayout.h"

namespace m3d
{

enum struct DescriptorType 
{ 
	SRV = D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 
	UAV = D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 
	CBV = D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 
	SAMPLER = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER,
	RTV = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER + 1,
	DSV = RTV + 2, 
	NONE = DSV + 1
};

struct DescriptorTable;

struct Descriptor
{
	DescriptorTable *dt; // The descriptor table we belong to!
	SID3D12Resource resource; // Not used for CBV and SAMPLER.
	DescriptorType type; // Do not change manually. Use DescriptorTable::Init().
	BufferLayoutID bufferLayoutID; // For buffers/tbuffers to compare againt shader signature.
	union
	{
		// Note: sometimes we can do without these!
		D3D12_SHADER_RESOURCE_VIEW_DESC *srv;
		D3D12_UNORDERED_ACCESS_VIEW_DESC *uav;
		D3D12_CONSTANT_BUFFER_VIEW_DESC *cbv;
		D3D12_SAMPLER_DESC *sampler;
		D3D12_RENDER_TARGET_VIEW_DESC *rtv;
		D3D12_DEPTH_STENCIL_VIEW_DESC *dsv;
	};
	Descriptor() : dt(nullptr), type(DescriptorType::NONE), bufferLayoutID(InvalidBufferLayoutID), srv(nullptr) {}

	inline D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandle() const;
	inline D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle() const;
};


// The DescriptorHeapManager wraps a ID3D12DescriptorHeap. This is a memory pool for descriptors of a given type.
// Call GetDescriptorTable(...) to get a DescriptorTable that is an array of Descriptors. 
// Fill the Descriptors in the table, and call CreateDescriptors(...). The DescriptorTable is reference counted,
// and will clean up its memory by calling FreeDecriptorTable(...). UpdateFrame(...) will recycle descriptors
// when the last frame they where potentially used is finished. We can not reuse a descriptor before the GPU is done!
class GRAPHICSCHIPS_API DescriptorHeapManager
{
public:
	DescriptorHeapManager();
	~DescriptorHeapManager();

	HRESULT Init(ID3D12Device *device, D3D12_DESCRIPTOR_HEAP_TYPE type, UINT size, BOOL shaderVisible = FALSE);
	void Clear();

	ID3D12Device *GetDevice() const { return _device; }
	ID3D12DescriptorHeap *GetHeap() const { return _heap; }

	// Get a table of descriptors.
	void GetDescriptorTable(UINT count, DescriptorTable **dt);
	// After describing the table, call CreateDescriptors().
	void CreateDescriptors(const DescriptorTable *table);
	// Free the table after finish using it. 
	void FreeDescriptorTable(DescriptorTable *table);

	// UpdateFrame() should be called before the manager is used!!
	void UpdateFrame(UINT64 lastCompletedFrame, UINT64 currentFrame);

	// You can get the descriptor if you have the cpu/gpu-handle. Throws - Always valid.
	const Descriptor &GetDescriptor(D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle) const;
	const Descriptor &GetDescriptor(D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle) const;

	// To be used by DescriptorTable only!
	Descriptor &InitDescriptor(DescriptorTable *dt, UINT index, DescriptorType type, BOOL createDesc);
	inline Descriptor &GetDescriptor(UINT index) { return _descriptors[index]; }
	UINT GetIndex(const Descriptor &desc) const;
	inline D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandle(UINT index) const { return CD3DX12_CPU_DESCRIPTOR_HANDLE(_cpuHeapStart, index, _handleIncrementSize); }
	inline D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle(UINT index) const { return CD3DX12_GPU_DESCRIPTOR_HANDLE(_gpuHeapStart, index, _handleIncrementSize); }

private:
	void _clearDescriptor(Descriptor &d);

	struct Trash
	{
		List<DescriptorTable*> descriptors;
	};
	Trash _trash[16];
	UINT64 _oldestFrame;
	UINT64 _currentFrame;

	SID3D12Device _device;
	SID3D12DescriptorHeap _heap;

	// The descriptor used creating the heap.
	D3D12_DESCRIPTOR_HEAP_DESC _desc;

	UINT _handleIncrementSize;
	D3D12_CPU_DESCRIPTOR_HANDLE _cpuHeapStart;
	D3D12_GPU_DESCRIPTOR_HANDLE _gpuHeapStart;

	// This is the array of descriptors. Size may increment up to _size.
	List<Descriptor> _descriptors;

	// When FreeDescriptorTable() are called, the descriptors are moved to trash, before they are moved to _reusables.
	// It contains all the 'holes' in the heap, ordered by the number of descriptors available, with the offset as the mapped value.
	MultiMap<UINT, UINT> _reusables;
};

struct DescriptorTable
{
	DescriptorHeapManager *const heapManager;
	const UINT offset;
	const UINT count;

	DescriptorTable(DescriptorHeapManager *heapManager, UINT offset, UINT count) : heapManager(heapManager), offset(offset), count(count), _refCount(0) {}

	// Note: We do not store a raw Descriptor-pointer in us, because the raw array in the manager can change (reallocate) during the lifetime of the DescriptorTable.
	Descriptor &InitDescriptor(UINT index, DescriptorType type, BOOL createDesc = TRUE) { return heapManager->InitDescriptor(this, offset + index, type, createDesc); }
	Descriptor &GetDescriptor(UINT index = 0) { return heapManager->GetDescriptor(offset + index); }
	const Descriptor &GetDescriptor(UINT index = 0) const { return heapManager->GetDescriptor(offset + index); }
	UINT GetIndex(const Descriptor &desc) const { return heapManager->GetIndex(desc) - offset; }

	inline D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandle(UINT index = 0) const { return heapManager->GetCPUHandle(offset + index); }
	inline D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle(UINT index = 0) const { return heapManager->GetGPUHandle(offset + index); }

	void CreateDescriptors() { return heapManager->CreateDescriptors(this); }

	void AddRef() { ++_refCount; }
	void Release() { if (--_refCount == 0) heapManager->FreeDescriptorTable(this); }

private:
	UINT _refCount;
};

IPOINTERS(DescriptorTable);

D3D12_CPU_DESCRIPTOR_HANDLE Descriptor::GetCPUHandle() const { return dt->GetCPUHandle(dt->GetIndex(*this)); }
D3D12_GPU_DESCRIPTOR_HANDLE Descriptor::GetGPUHandle() const { return dt->GetGPUHandle(dt->GetIndex(*this)); }


}
