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
#include "M3DEngine/Chip.h"
#include "GraphicsDefines.h"
#include "D3D12Formats.h"
#include "Graphics.h"
#include "ResourceStateTracker.h"
#include "DescriptorHeapManager.h"



namespace m3d
{

static const Guid GRAPHICSRESOURCECHIP_GUID = { 0x84d0e4fc, 0x8b2a, 0x4283, { 0x80, 0x1d, 0x2d, 0xa1, 0xf9, 0xae, 0x18, 0x55 } };


#define SRV_USE_CUBEMAP 0x100000

struct SRVDesc
{
	M3D_FORMAT format;
	UINT shader4ComponentMapping;
	UINT64 mostDetailedMip; // FirstElement for BUFFER (should be 64bit)
	UINT mipLevels; // NumElements for BUFFER
	UINT firstArraySlice; // First2DArrayFace for TEXTURECUBEARRAY
	UINT arraySize; // NumCubes for TEXTURECUBEARRAY
	UINT planeSlice; // StructureByteStride for structured buffers
	FLOAT resourceMinLODClamp;
	UINT flags; // Buffer only (and SRV_USE_CUBEMAP for cubemap)
	SRVDesc(M3D_FORMAT format = M3D_FORMAT_UNKNOWN, UINT shader4ComponentMapping = M3D_DEFAULT_SHADER_4_COMPONENT_MAPPING, UINT64 mostDetailedMip = 0,
		UINT mipLevels = -1, UINT firstArraySlice = 0, UINT arraySize = -1, UINT planeSlice = 0, FLOAT resourceMinLODClamp = 0.0f, UINT flags = SRV_USE_CUBEMAP)
		: format(format), shader4ComponentMapping(shader4ComponentMapping), mostDetailedMip(mostDetailedMip), mipLevels(mipLevels), firstArraySlice(firstArraySlice),
		arraySize(arraySize), planeSlice(planeSlice), resourceMinLODClamp(resourceMinLODClamp), flags(flags) {}

	bool operator==(const SRVDesc& rhs) const { return format == rhs.format && shader4ComponentMapping == rhs.shader4ComponentMapping && mostDetailedMip == rhs.mostDetailedMip && mipLevels == rhs.mipLevels && firstArraySlice == rhs.firstArraySlice && arraySize == rhs.arraySize && planeSlice == rhs.planeSlice && resourceMinLODClamp == rhs.resourceMinLODClamp && flags == rhs.flags; }
	bool operator!=(const SRVDesc& rhs) const { return !(*this == rhs); }
};


struct RTVDesc
{
	M3D_FORMAT format;
	UINT64 mipSlice; // FirstElement for BUFFER (should be 64bit)
	UINT firstArraySlice; // NumElements for BUFFER, FirstWSlice for TEXTURE3D
	UINT arraySize; // WSize for TEXTURE3D
	UINT planeSlice;
	RTVDesc(M3D_FORMAT format = M3D_FORMAT_UNKNOWN, UINT64 mipSlice = 0, UINT firstArraySlice = 0, UINT arraySize = -1, UINT planeSlice = 0)
		: format(format), mipSlice(mipSlice), firstArraySlice(firstArraySlice), arraySize(arraySize), planeSlice(planeSlice) {}

	bool operator==(const RTVDesc& rhs) const { return format == rhs.format && mipSlice == rhs.mipSlice && firstArraySlice == rhs.firstArraySlice && arraySize == rhs.arraySize && planeSlice == rhs.planeSlice; }
	bool operator!=(const RTVDesc& rhs) const { return !(*this == rhs); }
};


struct DSVDesc
{
	M3D_FORMAT format;
	UINT mipSlice;
	UINT firstArraySlice;
	UINT arraySize;
	M3D_DSV_FLAGS flags;
	DSVDesc(M3D_FORMAT format = M3D_FORMAT_UNKNOWN, UINT mipSlice = 0, UINT firstArraySlice = 0, UINT arraySize = -1, M3D_DSV_FLAGS flags = M3D_DSV_FLAG_NONE)
		: format(format), mipSlice(mipSlice), firstArraySlice(firstArraySlice), arraySize(arraySize), flags(flags) {}

	bool operator==(const DSVDesc& rhs) const { return format == rhs.format && mipSlice == rhs.mipSlice && firstArraySlice == rhs.firstArraySlice && arraySize == rhs.arraySize && flags == rhs.flags; }
	bool operator!=(const DSVDesc& rhs) const { return !(*this == rhs); }
};

struct UAVDesc
{
	M3D_FORMAT format;
	UINT64 mipSlice; // FirstElement for BUFFER (should be 64bit)
	UINT firstArraySlice; // NumElements for BUFFER, FirstWSlice for TEXTURE3D
	UINT arraySize; // WSize for TEXTURE3D, StructureByteStride for BUFFER
	UINT64 counterOffsetInBytes; // for BUFFER only
	UINT planeSlice;
	M3D_BUFFER_UAV_FLAGS flags; // BUFFER only
	UAVDesc(M3D_FORMAT format = M3D_FORMAT_UNKNOWN, UINT64 mipSlice = 0, UINT firstArraySlice = 0, UINT arraySize = -1, UINT64 counterOffsetInBytes = 0, UINT planeSlice = 0, M3D_BUFFER_UAV_FLAGS flags = M3D_BUFFER_UAV_FLAG_NONE)
		: format(format), mipSlice(mipSlice), firstArraySlice(firstArraySlice), arraySize(arraySize), counterOffsetInBytes(counterOffsetInBytes), planeSlice(planeSlice), flags(flags) {}

	bool operator==(const UAVDesc& rhs) const { return format == rhs.format && mipSlice == rhs.mipSlice && firstArraySlice == rhs.firstArraySlice && arraySize == rhs.arraySize && counterOffsetInBytes == rhs.counterOffsetInBytes && planeSlice == rhs.planeSlice && flags == rhs.flags; }
	bool operator!=(const UAVDesc& rhs) const { return !(*this == rhs); }
};



class GRAPHICSCHIPS_API GraphicsResourceChip : public Chip, public GraphicsUsage
{
	CHIPDESC_DECL; 
public:
	GraphicsResourceChip();

	bool CopyChip(Chip* chip) override;
	bool LoadChip(DocumentLoader& loader) override;
	bool SaveChip(DocumentSaver& saver) const override;

	// Get/Set descriptors for the different view types.
	virtual void SetSRVDesc(const SRVDesc& srvDesc);
	const SRVDesc& GetSRVDesc() const { return _srvDesc; }
	virtual void SetRTVDesc(const RTVDesc& rtvDesc);
	const RTVDesc& GetRTVDesc() const { return _rtvDesc; }
	virtual void SetDSVDesc(const DSVDesc& dsvDesc);
	const DSVDesc& GetDSVDesc() const { return _dsvDesc; }
	virtual void SetUAVDesc(const UAVDesc& uavDesc);
	const UAVDesc& GetUAVDesc() const { return _uavDesc; }

	void OnReleasingBackBuffer(RenderWindow* rw) override;
	void OnDestroyDevice() override;

	// Note: Most of these functions can throw an exception!

	virtual void UpdateChip(BufferLayoutID layoutID = InvalidBufferLayoutID);

	virtual void ClearResource();

	virtual ID3D12Resource* GetResource();
	virtual ResourceStateTracker* GetResourceStateTracker();

	virtual BufferLayoutID GetBufferLayoutID() const { return _layoutID; }

	virtual const Descriptor& GetShaderResourceViewDescriptor();
	virtual const Descriptor& GetRenderTargetViewDescriptor();
	virtual const Descriptor& GetDepthStencilViewDescriptor();

	virtual void GetShaderResourceViewDesc(D3D12_SHADER_RESOURCE_VIEW_DESC& srvDesc);
	virtual void GetRenderTargetViewDesc(D3D12_RENDER_TARGET_VIEW_DESC& rtvDesc);
	virtual void GetDepthStencilViewDesc(D3D12_DEPTH_STENCIL_VIEW_DESC& dsvDesc);
	virtual void GetUnorderedAccessViewDesc(D3D12_UNORDERED_ACCESS_VIEW_DESC& uavDesc);
	virtual void GetConstantBufferViewDesc(D3D12_CONSTANT_BUFFER_VIEW_DESC& cbvDesc);

	// Returns true if this chip has created a resource.
	virtual bool HasResource() const { return _res != nullptr; }

protected:
	SRVDesc _srvDesc;
	RTVDesc _rtvDesc;
	DSVDesc _dsvDesc;
	UAVDesc _uavDesc;

	RID3D12Resource _res;
	SDescriptorTable _srv;
	SDescriptorTable _rtv;
	SDescriptorTable _dsv;
	BufferLayoutID _layoutID = InvalidBufferLayoutID;
};

}
