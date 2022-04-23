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
#include "GraphicsChips/GraphicsException.h"

namespace m3d
{

struct __ResourceStateTrackerInternal;
class ResourceStateTracker;

HRESULT GRAPHICSCHIPS_API CreateResourceStateTracker(ID3D12Resource *resource, D3D12_RESOURCE_STATES initState, ResourceStateTracker **stateTracker);
HRESULT GRAPHICSCHIPS_API GetResourceStateTracker(ID3D12Resource *resource, ResourceStateTracker **stateTracker);

struct ResourceStateTrackerException : public GraphicsException
{
	ResourceStateTrackerException(ID3D12Resource *res, UINT subresource, D3D12_RESOURCE_STATES currentState, D3D12_RESOURCE_STATES requestedState);
};

class GRAPHICSCHIPS_API ResourceStateLock
{
private:
	ResourceStateTracker *_rst;
	SubresourceRange _srr;
	mutable bool _unlock;

public:
	ResourceStateLock() : _unlock(false) {}
	ResourceStateLock(ResourceStateTracker *rst, SubresourceRange srr) : _rst(rst), _srr(srr), _unlock(true) {}
	ResourceStateLock(const ResourceStateLock &rhs) : _rst(rhs._rst), _srr(rhs._srr), _unlock(rhs._unlock) { rhs._unlock = false; }
	~ResourceStateLock();

	const ResourceStateLock &operator=(const ResourceStateLock &rhs) = delete;
};

class GRAPHICSCHIPS_API ResourceStateTracker
{
	friend HRESULT GRAPHICSCHIPS_API CreateResourceStateTracker(ID3D12Resource*, D3D12_RESOURCE_STATES, ResourceStateTracker**);
	friend struct __ResourceStateTrackerInternal;
public:
	D3D12_RESOURCE_STATES GetCurrentState(UINT subresource = 0) const;

	// Caller must make sure the array is large enough to hold the maximum number of barriers that can be created. Returns the number of barriers created.
	// These functions can throw ResourceStateTracker if a subresource is currently locked.
	ResourceStateLock EnterState(D3D12_RESOURCE_BARRIER **barrierArray, const D3D12_SHADER_RESOURCE_VIEW_DESC *srv, D3D12_RESOURCE_STATES afterState = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	ResourceStateLock EnterState(D3D12_RESOURCE_BARRIER **barrierArray, const D3D12_RENDER_TARGET_VIEW_DESC *rtv, D3D12_RESOURCE_STATES afterState = D3D12_RESOURCE_STATE_RENDER_TARGET);
	ResourceStateLock EnterState(D3D12_RESOURCE_BARRIER **barrierArray, const D3D12_DEPTH_STENCIL_VIEW_DESC *dsv, D3D12_RESOURCE_STATES afterState = D3D12_RESOURCE_STATE_DEPTH_WRITE);
	ResourceStateLock EnterState(D3D12_RESOURCE_BARRIER **barrierArray, const D3D12_UNORDERED_ACCESS_VIEW_DESC *uav, D3D12_RESOURCE_STATES afterState = D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	ResourceStateLock EnterState(D3D12_RESOURCE_BARRIER **barrierArray, const SubresourceRange &srr, D3D12_RESOURCE_STATES afterState);
	ResourceStateLock EnterState(D3D12_RESOURCE_BARRIER **barrierArray, UINT subresource, D3D12_RESOURCE_STATES afterState);
	ResourceStateLock EnterState(D3D12_RESOURCE_BARRIER **barrierArray, D3D12_RESOURCE_STATES afterState);

	void Unlock(const SubresourceRange &srr);

	UINT GetSubresourceCount() const { return _mipLevels * _arraySize * _planeSliceCount; }

	// We have reference counting built-in.
	ULONG AddRef() { return ++_refCount; }
	ULONG Release();

	// If resource is released or state tracker removed from the resource, IsValid() can return false! Do not call other methods then!
	BOOL IsValid() const { return _resource != nullptr; }

private:
	ResourceStateTracker(ID3D12Resource *resource, D3D12_RESOURCE_STATES initState = D3D12_RESOURCE_STATE_COMMON);
	~ResourceStateTracker();

	void _invalidate();
	void _enterState(D3D12_RESOURCE_BARRIER **barrierArray, UINT subresource, D3D12_RESOURCE_STATES afterState);

	struct State
	{
		D3D12_RESOURCE_STATES state;
		UINT locked;
	};

	ID3D12Resource *_resource;
	State *_states;

	UINT _mipLevels;
	UINT _arraySize;
	UINT _planeSliceCount;

	ULONG _refCount;
};

IPOINTERS(ResourceStateTracker)



}


/*
// ID3D12Object
virtual HRESULT GetPrivateData(REFGUID guid, UINT *pDataSize, void *pData) override { return _resource->GetPrivateData(guid, pDataSize, pData); }
virtual HRESULT SetName(LPCWSTR Name) override { return _resource->SetName(Name); }
virtual HRESULT SetPrivateData(REFGUID guid, UINT DataSize, const void *pData) override { return _resource->SetPrivateData(guid, DataSize, pData); }
virtual HRESULT SetPrivateDataInterface(REFGUID  guid, const IUnknown *pData) override { return _resource->SetPrivateDataInterface(guid, pData); }

// ID3D12DeviceChild
virtual HRESULT GetDevice(REFIID riid, void **ppvDevice) override { return _resource->GetDevice(riid, ppvDevice); }

// ID3D12Resource
virtual D3D12_RESOURCE_DESC GetDesc() override { return _resource->GetDesc(); }
virtual D3D12_GPU_VIRTUAL_ADDRESS GetGPUVirtualAddress() override { return _resource->GetGPUVirtualAddress(); }
virtual HRESULT GetHeapProperties(D3D12_HEAP_PROPERTIES *pHeapProperties, D3D12_HEAP_FLAGS *pHeapFlags) override { return _resource->GetHeapProperties(pHeapProperties, pHeapFlags); }
virtual HRESULT Map(UINT Subresource, const D3D12_RANGE *pReadRange, void **ppData) override { return _resource->Map(Subresource, pReadRange, ppData); }
virtual HRESULT ReadFromSubresource(void *pDstData, UINT DstRowPitch, UINT DstDepthPitch, UINT SrcSubresource, const D3D12_BOX *pSrcBox) override { return _resource->ReadFromSubresource(pDstData, DstRowPitch, DstDepthPitch, SrcSubresource, pSrcBox); }
virtual void Unmap(UINT Subresource, const D3D12_RANGE *pWrittenRange) override { return _resource->Unmap(Subresource, pWrittenRange); }
virtual HRESULT WriteToSubresource(UINT DstSubresource, const D3D12_BOX *pDstBox, const void *pSrcData, UINT SrcRowPitch, UINT SrcDepthPitch) override { return _resource->WriteToSubresource(DstSubresource, pDstBox, pSrcData, SrcRowPitch, SrcDepthPitch); }
*/