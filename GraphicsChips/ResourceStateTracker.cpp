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
#include "ResourceStateTracker.h"
#include "D3DX12.h"


namespace m3d
{

String _concatStateNames(D3D12_RESOURCE_STATES s)
{
	if (s == D3D12_RESOURCE_STATE_COMMON)
		return  MTEXT("COMMON|PRESENT");

	String r;

	const D3D12_RESOURCE_STATES states[] = {
		D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
		D3D12_RESOURCE_STATE_INDEX_BUFFER,
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		D3D12_RESOURCE_STATE_DEPTH_READ,
		D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		D3D12_RESOURCE_STATE_STREAM_OUT,
		D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT,
		D3D12_RESOURCE_STATE_COPY_DEST,
		D3D12_RESOURCE_STATE_COPY_SOURCE,
		D3D12_RESOURCE_STATE_RESOLVE_DEST,
		D3D12_RESOURCE_STATE_RESOLVE_SOURCE };

	const String stateNames[] = {
		MTEXT("VERTEX_AND_CONSTANT_BUFFER"),
		MTEXT("INDEX_BUFFER"),
		MTEXT("RENDER_TARGET"),
		MTEXT("UNORDERED_ACCESS"),
		MTEXT("DEPTH_WRITE"),
		MTEXT("DEPTH_READ"),
		MTEXT("NON_PIXEL_SHADER_RESOURCE"),
		MTEXT("PIXEL_SHADER_RESOURCE"),
		MTEXT("STREAM_OUT"),
		MTEXT("INDIRECT_ARGUMENT|PREDICATION"),
		MTEXT("COPY_DEST"),
		MTEXT("COPY_SOURCE"),
		MTEXT("RESOLVE_DEST"),
		MTEXT("RESOLVE_SOURCE") };

	for (uint32 i = 0; i < _countof(states); i++) {
		if (s & states[i]) {
			if (!r.empty())
				r += MTEXT("|");
			r += stateNames[i];
		}
	}

	return r;
}

String _getResourceStateTrackerExceptionString(ID3D12Resource *res, UINT subresource, D3D12_RESOURCE_STATES currentState, D3D12_RESOURCE_STATES requestedState)
{
	UINT m, a, p;
	auto d = res->GetDesc();
	D3D12DecomposeSubresource(subresource, d.MipLevels, d.DepthOrArraySize, m, a, p);

	Char c[256];
	UINT aSize = 256;
	if (FAILED(res->GetPrivateData(WKPDID_D3DDebugObjectName, &aSize, c)))
		c[0] = '\0';
	return strUtils::ConstructString(MTEXT("Subresource %1 (Mip: %2, ArraySlice: %3) of resource named \'%4\' was locked in state %5. Can not enter state %6.")).arg(subresource).arg(m).arg(a).arg(String(c, aSize)).arg(_concatStateNames(currentState)).arg(_concatStateNames(requestedState));
}

ResourceStateTrackerException::ResourceStateTrackerException(ID3D12Resource *res, UINT subresource, D3D12_RESOURCE_STATES currentState, D3D12_RESOURCE_STATES requestedState) : GraphicsException(_getResourceStateTrackerExceptionString(res, subresource, currentState, requestedState))
{
}


static const GUID RESOURCE_STATE_TRACKER_GUID = { 0x3124d440, 0xd1d9, 0x653,{ 0x90, 0xc4, 0x6a, 0x9e, 0xc4, 0xb2, 0xed, 0x2d } };

struct __ResourceStateTrackerInternal : public IUnknown
{
	ResourceStateTracker *stateTracker;
	ULONG refCount;
	__ResourceStateTrackerInternal() : stateTracker(nullptr), refCount(0) {}

	// IUnknown
	virtual ULONG AddRef() override { return ++refCount; }
	virtual ULONG Release() override 
	{ 
		if (--refCount == 0) { 
			stateTracker->_invalidate(); 
			stateTracker->Release(); 
			delete this; 
			return 0; 
		} 
		return refCount; 
	}
	virtual HRESULT QueryInterface(REFIID riid, void **ppvObject) override { return E_NOTIMPL; }
};

HRESULT CreateResourceStateTracker(ID3D12Resource *resource, D3D12_RESOURCE_STATES initState, ResourceStateTracker **stateTracker)
{
	if (resource == nullptr)
		return E_INVALIDARG;
	if (SUCCEEDED(GetResourceStateTracker(resource, stateTracker)))
		return S_FALSE; // Returning old state tracker.
	__ResourceStateTrackerInternal *st = new __ResourceStateTrackerInternal();
	HRESULT hr = resource->SetPrivateDataInterface(RESOURCE_STATE_TRACKER_GUID, st);
	st->stateTracker = new ResourceStateTracker(resource, initState);
	if (stateTracker) {
		st->stateTracker->AddRef();
		*stateTracker = st->stateTracker;
	}
	return S_OK;
}

HRESULT GetResourceStateTracker(ID3D12Resource *resource, ResourceStateTracker **stateTracker)
{
	if (resource == nullptr)
		return E_INVALIDARG;
	__ResourceStateTrackerInternal *st = nullptr;
	UINT size = sizeof(st);
	HRESULT hr = resource->GetPrivateData(RESOURCE_STATE_TRACKER_GUID, &size, &st);
	if (FAILED(hr))
		return hr;
	assert(st && st->stateTracker);
	if (stateTracker) {
		st->stateTracker->AddRef();
		*stateTracker = st->stateTracker;
	}
	return S_OK;
}

} // namespace m3d

using namespace m3d;

ResourceStateLock::~ResourceStateLock() { if (_unlock) _rst->Unlock(_srr); }

ResourceStateTracker::ResourceStateTracker(ID3D12Resource *resource, D3D12_RESOURCE_STATES initState) : _resource(resource), _refCount(1)
{
	SID3D12Device device;

	resource->GetDevice(__uuidof(*device), reinterpret_cast<void**>(&device));

	D3D12_RESOURCE_DESC desc = _resource->GetDesc();
	_mipLevels = desc.MipLevels;
	_arraySize = desc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE3D ? 1 : desc.DepthOrArraySize;
	_planeSliceCount = D3D12GetFormatPlaneCount(device, desc.Format);
	_states = new State[_mipLevels * _arraySize * _planeSliceCount];
	for (uint32 i = 0; i < _mipLevels * _arraySize * _planeSliceCount; i++)
		_states[i] = {initState, false};
}

ResourceStateTracker::~ResourceStateTracker()
{
	delete[] _states;
}

void ResourceStateTracker::_invalidate()
{
	_resource = nullptr;
	delete[] _states;
	_states = nullptr;
	_mipLevels = 0;
	_arraySize = 0;
	_planeSliceCount = 0;
}

D3D12_RESOURCE_STATES ResourceStateTracker::GetCurrentState(UINT subresource) const
{
	return _states[subresource].state;
}

ResourceStateLock ResourceStateTracker::EnterState(D3D12_RESOURCE_BARRIER **barrierArray, const D3D12_SHADER_RESOURCE_VIEW_DESC *srv, D3D12_RESOURCE_STATES afterState)
{
	if (!srv)
		return EnterState(barrierArray, afterState);

	return EnterState(barrierArray, GetSubresourceRange(srv), afterState);
}

ResourceStateLock ResourceStateTracker::EnterState(D3D12_RESOURCE_BARRIER **barrierArray, const D3D12_RENDER_TARGET_VIEW_DESC *rtv, D3D12_RESOURCE_STATES afterState)
{
	if (!rtv)
		return EnterState(barrierArray, afterState);

	return EnterState(barrierArray, GetSubresourceRange(rtv), afterState);
}

ResourceStateLock ResourceStateTracker::EnterState(D3D12_RESOURCE_BARRIER **barrierArray, const D3D12_DEPTH_STENCIL_VIEW_DESC *dsv, D3D12_RESOURCE_STATES afterState)
{
	if (!dsv)
		return EnterState(barrierArray, afterState);

	return EnterState(barrierArray, GetSubresourceRange(dsv), afterState);
}

ResourceStateLock ResourceStateTracker::EnterState(D3D12_RESOURCE_BARRIER **barrierArray, const D3D12_UNORDERED_ACCESS_VIEW_DESC *uav, D3D12_RESOURCE_STATES afterState)
{
	if (!uav)
		return EnterState(barrierArray, afterState);

	return EnterState(barrierArray, GetSubresourceRange(uav), afterState);
}

ResourceStateLock ResourceStateTracker::EnterState(D3D12_RESOURCE_BARRIER **barrierArray, const SubresourceRange &srr, D3D12_RESOURCE_STATES afterState)
{
	// Whole resource?
	if (srr.MostDetailedMip == 0 && srr.MipLevels == _mipLevels && srr.FirstArraySlice == 0 && srr.ArraySize == _arraySize && srr.PlaneSlice == 0 && (srr.PlaneSliceCount == -1 || srr.PlaneSliceCount == _planeSliceCount))
		return EnterState(barrierArray, afterState);

	UINT i = srr.MostDetailedMip, j = srr.FirstArraySlice, k = srr.PlaneSlice, a = std::min(_mipLevels, i + srr.MipLevels), b = std::min(_arraySize, j + srr.ArraySize), c = srr.PlaneSliceCount == -1 ? _planeSliceCount : std::min(_planeSliceCount, k + srr.PlaneSliceCount);
	try {
		for (; k < c; k++)
			for (j = srr.FirstArraySlice; j < b; j++)
				for (i = srr.MostDetailedMip; i < a; i++)
					_enterState(barrierArray, D3D12CalcSubresource(i, j, k, _mipLevels, _arraySize), afterState);
	}
	catch (const ResourceStateTrackerException&) 
	{
		// Subresource(s) was locked! Do cleanup; unlock and revert state!
		/*for (UINT m = 0, r = 0, s = 0, R = 0, t = firstPlanceSlice; t <= k; t++) {
			for (s = firstArraySlice; s <= j; s++) {
				for (r = mostDetailedMip, R = (s == j && t == k) ? i : a; r < R; r++) {
					UINT subresource = D3D12CalcSubresource(r, s, t, _mipLevels, _arraySize);
					State &s = _states[subresource];
					s.locked = false;
					if (m < n && subresource == barrierArray[m].Transition.Subresource)
						s.state = barrierArray[m++].Transition.StateBefore;
				}
			}
		}*/
		throw; // rethrow!
	}
	return ResourceStateLock(this, srr);
}

ResourceStateLock ResourceStateTracker::EnterState(D3D12_RESOURCE_BARRIER **barrierArray, UINT subresource, D3D12_RESOURCE_STATES afterState)
{
	if (subresource == D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES) { // All subresources?
		D3D12_RESOURCE_STATES s = _states[0].state;
		UINT i = 0, j = _mipLevels * _arraySize * _planeSliceCount;
		UINT locked = -1;
		for (; i < j; i++) {
			if (_states[i].state != s)
				break;
			if (locked == -1 && _states[i].locked > 0)
				locked = i;
		}
		if (i == j) { // All current states are equal
			if (s != afterState) { // State change?
				if (locked != -1) // Was a subresource locked?
					throw ResourceStateTrackerException(_resource, locked, s, afterState);
				D3D12_RESOURCE_BARRIER *ba = *barrierArray;
				ba->Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
				ba->Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
				ba->Transition.pResource = _resource;
				ba->Transition.StateBefore = s;
				ba->Transition.StateAfter = afterState;
				ba->Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
				++(*barrierArray);
				for (i = 0; i < j; i++) {
					_states[i].state = afterState;
					++_states[i].locked;
				}
			}
			else {
				// IDEA: Maybe if UAV->UAV, we need a D3D12_RESOURCE_BARRIER_TYPE_UAV barrier instead?
//				if (s == D3D12_RESOURCE_STATE_UNORDERED_ACCESS) {
//					D3D12_RESOURCE_BARRIER* ba = *barrierArray;
//					ba->Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
//					ba->Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
//					ba->UAV.pResource = _resource;
//					++(*barrierArray);
//				}

				for (i = 0; i < j; i++)
					++_states[i].locked; // Lock must still be incremented!
			}
		}
		else {
			// Subresources have different current states..
			try {
				for (i = 0; i < j; i++)
					_enterState(barrierArray, i, afterState);
			}
			catch (const ResourceStateTrackerException &) {
				for (j = 0; j < i; j++)
					--_states[j].locked; // Revert lock
				throw;
			}
		}
		SubresourceRange srr = { 0, _mipLevels, 0, _arraySize, 0, _planeSliceCount };
		return ResourceStateLock(this, srr);
	}

	_enterState(barrierArray, subresource, afterState);

	SubresourceRange srr = { 0, 1, 0, 1, 0, 1 };
	D3D12DecomposeSubresource(subresource, _mipLevels, _arraySize, srr.MostDetailedMip, srr.ArraySize, srr.PlaneSlice);
	return ResourceStateLock(this, srr);
}

ResourceStateLock ResourceStateTracker::EnterState(D3D12_RESOURCE_BARRIER **barrierArray, D3D12_RESOURCE_STATES afterState)
{
	return EnterState(barrierArray, D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, afterState);
}

void ResourceStateTracker::_enterState(D3D12_RESOURCE_BARRIER **barrierArray, UINT subresource, D3D12_RESOURCE_STATES afterState)
{
	assert(subresource <  _mipLevels * _arraySize * _planeSliceCount);

	State &s = _states[subresource];

	if (s.state == afterState) {
		// IDEA: Maybe if UAV->UAV, we need a D3D12_RESOURCE_BARRIER_TYPE_UAV barrier instead?
//		if (s.state == D3D12_RESOURCE_STATE_UNORDERED_ACCESS) {
//			D3D12_RESOURCE_BARRIER* ba = *barrierArray;
//			ba->Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
//			ba->Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
//			ba->UAV.pResource = _resource;
//			++(*barrierArray);
//		}
		++s.locked;
		return;
	}

	if (s.locked > 0)
		throw ResourceStateTrackerException(_resource, subresource, s.state, afterState);

	++s.locked;

	D3D12_RESOURCE_BARRIER *ba = *barrierArray;

	ba->Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	ba->Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	ba->Transition.pResource = _resource;
	ba->Transition.StateBefore = s.state;
	ba->Transition.StateAfter = s.state = afterState;
	ba->Transition.Subresource = subresource;

	++(*barrierArray);
}

void ResourceStateTracker::Unlock(const SubresourceRange &srr)
{
	UINT i = srr.MostDetailedMip, j = srr.FirstArraySlice, k = srr.PlaneSlice, I = std::min(_mipLevels, i + srr.MipLevels), J = std::min(_arraySize, j + srr.ArraySize), K = srr.PlaneSliceCount == -1 ? _planeSliceCount : std::min(_planeSliceCount, k + srr.PlaneSliceCount);
	for (; k < K; k++) {
		for (j = srr.FirstArraySlice; j < J; j++) {
			for (i = srr.MostDetailedMip; i < I; i++) {
				UINT subresource = D3D12CalcSubresource(i, j, k, _mipLevels, _arraySize);
				State &s = _states[subresource];
				assert(s.locked > 0);
				--s.locked;
			}
		}
	}
}

ULONG ResourceStateTracker::Release()
{
	if (--_refCount > 0)
		return _refCount;
	delete this;
	return 0;
}
