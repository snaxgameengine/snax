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
#include "GraphicsResourceChip.h"
#include "M3DEngine/DocumentSaveLoadUtil.h"
#include "D3D12RenderWindow.h"
#include "D3D12Formats.h"

using namespace m3d;

CHIPDESCV1_DEF(GraphicsResourceChip, MTEXT("Graphics Resource"), GRAPHICSRESOURCECHIP_GUID, CHIP_GUID);


GraphicsResourceChip::GraphicsResourceChip()
{
	CREATE_CHILD(0, GRAPHICSRESOURCECHIP_GUID, false, UP, MTEXT("Resource"));	
}

bool GraphicsResourceChip::CopyChip(Chip* chip)
{
	GraphicsResourceChip* c = dynamic_cast<GraphicsResourceChip*>(chip);
	B_RETURN(Chip::CopyChip(c));

	ClearResource();

	_srvDesc = c->_srvDesc;
	_rtvDesc = c->_rtvDesc;
	_dsvDesc = c->_dsvDesc;
	_uavDesc = c->_uavDesc;

	SetUpdateStamp();
	return true;
}

bool GraphicsResourceChip::LoadChip(DocumentLoader& loader)
{
	B_RETURN(Chip::LoadChip(loader));

	ClearResource();

	LOADDEF("srv_format", _srvDesc.format, M3D_FORMAT_UNKNOWN);
	LOADDEF("srv_shader4ComponentMapping", _srvDesc.shader4ComponentMapping, M3D_DEFAULT_SHADER_4_COMPONENT_MAPPING);
	LOADDEF("srv_mostDetailedMip", _srvDesc.mostDetailedMip, 0);
	LOADDEF("srv_mipLevels", _srvDesc.mipLevels, -1);
	LOADDEF("srv_firstArraySlice", _srvDesc.firstArraySlice, 0);
	LOADDEF("srv_arraySize", _srvDesc.arraySize, -1);
	LOADDEF("srv_planeSlice", _srvDesc.planeSlice, 0);
	LOADDEF("srv_resourceMinLODClamp", _srvDesc.resourceMinLODClamp, 0.0f);
	LOADDEF("srv_flags", _srvDesc.flags, SRV_USE_CUBEMAP);

	LOADDEF("rtv_format", _rtvDesc.format, M3D_FORMAT_UNKNOWN);
	LOADDEF("rtv_mipSlice", _rtvDesc.mipSlice, 0);
	LOADDEF("rtv_firstArraySlice", _rtvDesc.firstArraySlice, 0);
	LOADDEF("rtv_arraySize", _rtvDesc.arraySize, -1);
	LOADDEF("rtv_planeSlice", _rtvDesc.planeSlice, 0);

	LOADDEF("dsv_format", _dsvDesc.format, M3D_FORMAT_UNKNOWN);
	LOADDEF("dsv_mipSlice", _dsvDesc.mipSlice, 0);
	LOADDEF("dsv_firstArraySlice", _dsvDesc.firstArraySlice, 0);
	LOADDEF("dsv_arraySize", _dsvDesc.arraySize, -1);
	LOADDEF("dsv_flags", _dsvDesc.flags, M3D_DSV_FLAG_NONE);

	LOADDEF("uav_format", _uavDesc.format, M3D_FORMAT_UNKNOWN);
	LOADDEF("uav_mipSlice", _uavDesc.mipSlice, 0);
	LOADDEF("uav_firstArraySlice", _uavDesc.firstArraySlice, 0);
	LOADDEF("uav_arraySize", _uavDesc.arraySize, -1);
	LOADDEF("uav_counterOffsetInBytes", _uavDesc.counterOffsetInBytes, 0);
	LOADDEF("uav_planeSlice", _uavDesc.planeSlice, 0);
	LOADDEF("uav_flags", _uavDesc.flags, M3D_BUFFER_UAV_FLAG_NONE);

	SetUpdateStamp();

	return true;
}

bool GraphicsResourceChip::SaveChip(DocumentSaver& saver) const
{
	B_RETURN(Chip::SaveChip(saver));

	SAVEDEF("srv_format", _srvDesc.format, M3D_FORMAT_UNKNOWN);
	SAVEDEF("srv_shader4ComponentMapping", _srvDesc.shader4ComponentMapping, M3D_DEFAULT_SHADER_4_COMPONENT_MAPPING);
	SAVEDEF("srv_mostDetailedMip", _srvDesc.mostDetailedMip, 0);
	SAVEDEF("srv_mipLevels", _srvDesc.mipLevels, -1);
	SAVEDEF("srv_firstArraySlice", _srvDesc.firstArraySlice, 0);
	SAVEDEF("srv_arraySize", _srvDesc.arraySize, -1);
	SAVEDEF("srv_planeSlice", _srvDesc.planeSlice, 0);
	SAVEDEF("srv_resourceMinLODClamp", _srvDesc.resourceMinLODClamp, 0.0f);
	SAVEDEF("srv_flags", _srvDesc.flags, SRV_USE_CUBEMAP);

	SAVEDEF("rtv_format", _rtvDesc.format, M3D_FORMAT_UNKNOWN);
	SAVEDEF("rtv_mipSlice", _rtvDesc.mipSlice, 0);
	SAVEDEF("rtv_firstArraySlice", _rtvDesc.firstArraySlice, 0);
	SAVEDEF("rtv_arraySize", _rtvDesc.arraySize, -1);
	SAVEDEF("rtv_planeSlice", _rtvDesc.planeSlice, 0);

	SAVEDEF("dsv_format", _dsvDesc.format, M3D_FORMAT_UNKNOWN);
	SAVEDEF("dsv_mipSlice", _dsvDesc.mipSlice, 0);
	SAVEDEF("dsv_firstArraySlice", _dsvDesc.firstArraySlice, 0);
	SAVEDEF("dsv_arraySize", _dsvDesc.arraySize, -1);
	SAVEDEF("dsv_flags", _dsvDesc.flags, M3D_DSV_FLAG_NONE);

	SAVEDEF("uav_format", _uavDesc.format, M3D_FORMAT_UNKNOWN);
	SAVEDEF("uav_mipSlice", _uavDesc.mipSlice, 0);
	SAVEDEF("uav_firstArraySlice", _uavDesc.firstArraySlice, 0);
	SAVEDEF("uav_arraySize", _uavDesc.arraySize, -1);
	SAVEDEF("uav_counterOffsetInBytes", _uavDesc.counterOffsetInBytes, 0);
	SAVEDEF("uav_planeSlice", _uavDesc.planeSlice, 0);
	SAVEDEF("uav_flags", _uavDesc.flags, M3D_BUFFER_UAV_FLAG_NONE);

	return true;
}


void GraphicsResourceChip::OnReleasingBackBuffer(RenderWindow* rw)
{
	if (_res == ((D3D12RenderWindow*)rw)->GetBackBuffer())
		ClearResource();
}

void GraphicsResourceChip::OnDestroyDevice()
{
	ClearResource();
}

void GraphicsResourceChip::UpdateChip(BufferLayoutID layoutID)
{
	RefreshT refresh(Refresh);
	if (!refresh) {
		if (!_res)
			throw ReinitPreventedException(this);
		return;
	}

	try
	{
		ID3D12Resource* r = nullptr;

		ChildPtr<GraphicsResourceChip> ch0 = GetChild(0);
		CHECK_CHILD(ch0, 0);

		ch0->UpdateChip(layoutID);

		r = ch0->GetResource(); // throws! never nullptr!

		if (r != _res) { // New resource?
			ClearResource();
			_res = r;
			_layoutID = ch0->GetBufferLayoutID();
		}
	}
	catch (const ChipException&)
	{
		ClearResource();
		throw;
	}
}

ID3D12Resource* GraphicsResourceChip::GetResource()
{
	if (!_res)
		throw GraphicsException(this, MTEXT("No resource. Call Updatechip()!"));
	return _res;
}

void GraphicsResourceChip::ClearResource()
{
	_res = nullptr;
	_srv = nullptr;
	_rtv = nullptr;
	_dsv = nullptr;
	_layoutID = InvalidBufferLayoutID;
	SetUpdateStamp();
}

ResourceStateTracker* GraphicsResourceChip::GetResourceStateTracker()
{
	ID3D12Resource* res = GetResource();
	SResourceStateTracker rst;
	::GetResourceStateTracker(res, &rst);
	return rst; // can be nullptr
}

const Descriptor& GraphicsResourceChip::GetShaderResourceViewDescriptor()
{
	ID3D12Resource* res = GetResource(); // Get/Create resource. Never nullptr!

	if (_srv)
		return _srv->GetDescriptor();

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;

	GetShaderResourceViewDesc(srvDesc);

	GetHeapManager()->GetDescriptorTable(1, &_srv);

	Descriptor& d = _srv->InitDescriptor(0, DescriptorType::SRV);
	d.resource = res;
	*d.srv = srvDesc;

	_srv->CreateDescriptors();

	return _srv->GetDescriptor();
}

const Descriptor& GraphicsResourceChip::GetRenderTargetViewDescriptor()
{
	ID3D12Resource* res = GetResource(); // Get/Create resource. Never nullptr!

	if (_rtv)
		return _rtv->GetDescriptor();

	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc;

	GetRenderTargetViewDesc(rtvDesc);

	GetRTVHeapManager()->GetDescriptorTable(1, &_rtv);

	Descriptor& d = _rtv->InitDescriptor(0, DescriptorType::RTV);
	d.resource = res;
	*d.rtv = rtvDesc;

	_rtv->CreateDescriptors();

	return _rtv->GetDescriptor();
}

const Descriptor& GraphicsResourceChip::GetDepthStencilViewDescriptor()
{
	ID3D12Resource* res = GetResource(); // Get/Create resource. Never nullptr!

	if (_dsv)
		return _dsv->GetDescriptor();

	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;

	GetDepthStencilViewDesc(dsvDesc);

	GetDSVHeapManager()->GetDescriptorTable(1, &_dsv);

	Descriptor& d = _dsv->InitDescriptor(0, DescriptorType::DSV);
	d.resource = res;
	*d.dsv = dsvDesc;

	_dsv->CreateDescriptors();

	return _dsv->GetDescriptor();
}

void GraphicsResourceChip::GetShaderResourceViewDesc(D3D12_SHADER_RESOURCE_VIEW_DESC& srvDesc)
{
	ID3D12Resource* res = GetResource();

	D3D12_RESOURCE_DESC rDesc = res->GetDesc();

	srvDesc.Format = _srvDesc.format == DXGI_FORMAT_UNKNOWN ? rDesc.Format : (DXGI_FORMAT)_srvDesc.format;
	srvDesc.Shader4ComponentMapping = _srvDesc.shader4ComponentMapping;

#ifdef DEVCHECKS
	if (srvDesc.Format != DXGI_FORMAT_UNKNOWN && rDesc.Dimension != D3D12_RESOURCE_DIMENSION_BUFFER) {
		D3D12_FEATURE_DATA_FORMAT_SUPPORT support{ srvDesc.Format, D3D12_FORMAT_SUPPORT1_NONE, D3D12_FORMAT_SUPPORT2_NONE };
		HRESULT hr = device()->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT, &support, sizeof(support));
		if (FAILED(hr) || (support.Support1 & (D3D12_FORMAT_SUPPORT1_SHADER_LOAD | D3D12_FORMAT_SUPPORT1_SHADER_SAMPLE | D3D12_FORMAT_SUPPORT1_SHADER_SAMPLE_COMPARISON)) == 0)
			throw GraphicsException(this, MTEXT("The format for this resource is not supported for a SRV."));

		if (dxgiformat::IsTypeless(srvDesc.Format) || dxgiformat::IsTypeless(rDesc.Format) && rDesc.Format != dxgiformat::MakeTypeless(srvDesc.Format))
			throw GraphicsException(this, MTEXT("Can not set a typeless format as a SRV."));

		if (dxgiformat::MakeTypeless(srvDesc.Format) != dxgiformat::MakeTypeless(rDesc.Format))
			throw GraphicsException(this, MTEXT("The format for this SRV is incompatible with the resource."));
	}

	if (rDesc.Flags & D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE)
		throw GraphicsException(this, MTEXT("This resource does not support SRV."));

#endif

	switch (rDesc.Dimension)
	{
	case D3D12_RESOURCE_DIMENSION_BUFFER:
	{
		const BufferLayout* bl = BufferLayoutManager::GetInstance().GetLayout(_layoutID);
		if (!bl)
			throw GraphicsException(this, MTEXT("Buffer is missing layout!"));
		UINT64 bufferSize = bl->GetBufferSize(), numElements = (bufferSize / (sizeof(float) * 4));
		if (bufferSize == 0 || bufferSize % (sizeof(float) * 4) != 0)
			throw GraphicsException(this, MTEXT("Buffer has an invalid size. It is either 0 er not a multiple of 4 bytes."));
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
		srvDesc.Buffer.FirstElement = _srvDesc.mostDetailedMip;
		srvDesc.Buffer.NumElements = _srvDesc.mipLevels == -1 ? UINT(numElements - srvDesc.Buffer.FirstElement) : _srvDesc.mipLevels;
		srvDesc.Buffer.StructureByteStride = _srvDesc.planeSlice;
		srvDesc.Buffer.Flags = _srvDesc.flags & D3D12_BUFFER_SRV_FLAG_RAW ? D3D12_BUFFER_SRV_FLAG_RAW : D3D12_BUFFER_SRV_FLAG_NONE;
		srvDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
#ifdef DEVCHECKS
		if (srvDesc.Buffer.FirstElement >= numElements || srvDesc.Buffer.NumElements > numElements || (srvDesc.Buffer.FirstElement + srvDesc.Buffer.NumElements) > numElements)
			throw GraphicsException(this, MTEXT("The buffer specification for this SRV is invalid for the given resource."));
#endif
		break;
	}
	case D3D12_RESOURCE_DIMENSION_TEXTURE1D:
		if (rDesc.DepthOrArraySize == 1) {
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1D;
			srvDesc.Texture1D.MipLevels = _srvDesc.mipLevels == -1 ? (rDesc.MipLevels - (UINT)_srvDesc.mostDetailedMip) : _srvDesc.mipLevels;
			srvDesc.Texture1D.MostDetailedMip = (UINT)_srvDesc.mostDetailedMip;
			srvDesc.Texture1D.ResourceMinLODClamp = _srvDesc.resourceMinLODClamp;
#ifdef DEVCHECKS
			if (srvDesc.Texture1D.MipLevels == 0 || srvDesc.Texture1D.MostDetailedMip >= rDesc.MipLevels || srvDesc.Texture1D.MipLevels > rDesc.MipLevels || (srvDesc.Texture1D.MostDetailedMip + srvDesc.Texture1D.MipLevels) > rDesc.MipLevels)
				throw GraphicsException(this, MTEXT("The mip level specified for this SRV is invalid for the given resource."));
#endif
		}
		else {
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1DARRAY;
			srvDesc.Texture1DArray.ArraySize = _srvDesc.arraySize == -1 ? (rDesc.DepthOrArraySize - _srvDesc.firstArraySlice) : _srvDesc.arraySize;
			srvDesc.Texture1DArray.FirstArraySlice = _srvDesc.firstArraySlice;
			srvDesc.Texture1DArray.MipLevels = _srvDesc.mipLevels == -1 ? (rDesc.MipLevels - (UINT)_srvDesc.mostDetailedMip) : _srvDesc.mipLevels;
			srvDesc.Texture1DArray.MostDetailedMip = (UINT)_srvDesc.mostDetailedMip;
			srvDesc.Texture1DArray.ResourceMinLODClamp = _srvDesc.resourceMinLODClamp;
#ifdef DEVCHECKS
			if (srvDesc.Texture1DArray.MipLevels == 0 || srvDesc.Texture1DArray.MostDetailedMip >= rDesc.MipLevels || srvDesc.Texture1DArray.MipLevels > rDesc.MipLevels || (srvDesc.Texture1DArray.MostDetailedMip + srvDesc.Texture1DArray.MipLevels) > rDesc.MipLevels)
				throw GraphicsException(this, MTEXT("The mip level specified for this SRV is invalid for the given resource."));
			if (srvDesc.Texture1DArray.ArraySize == 0 || srvDesc.Texture1DArray.FirstArraySlice >= rDesc.DepthOrArraySize || srvDesc.Texture1DArray.ArraySize > rDesc.DepthOrArraySize || (srvDesc.Texture1DArray.FirstArraySlice + srvDesc.Texture1DArray.ArraySize) > rDesc.DepthOrArraySize)
				throw GraphicsException(this, MTEXT("The array specification for this SRV is invalid for the given resource."));
#endif
		}
		break;
	case D3D12_RESOURCE_DIMENSION_TEXTURE2D:
		if (rDesc.SampleDesc.Count == 1) {
			if (rDesc.DepthOrArraySize == 1) {
				srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
				srvDesc.Texture2D.MipLevels = _srvDesc.mipLevels == -1 ? (rDesc.MipLevels - (UINT)_srvDesc.mostDetailedMip) : _srvDesc.mipLevels;
				srvDesc.Texture2D.MostDetailedMip = (UINT)_srvDesc.mostDetailedMip;
				srvDesc.Texture2D.PlaneSlice = _srvDesc.planeSlice;
				srvDesc.Texture2D.ResourceMinLODClamp = _srvDesc.resourceMinLODClamp;
#ifdef DEVCHECKS
				if (srvDesc.Texture2D.MipLevels == 0 || srvDesc.Texture2D.MostDetailedMip >= rDesc.MipLevels || srvDesc.Texture2D.MipLevels > rDesc.MipLevels || (srvDesc.Texture2D.MostDetailedMip + srvDesc.Texture2D.MipLevels) > rDesc.MipLevels)
					throw GraphicsException(this, MTEXT("The mip level specified for this SRV is invalid for the given resource."));
#endif
			}
			else {
				if (_srvDesc.flags & SRV_USE_CUBEMAP) {
					if (rDesc.DepthOrArraySize == 6) {
						srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
						srvDesc.TextureCube.MipLevels = _srvDesc.mipLevels == -1 ? (rDesc.MipLevels - (UINT)_srvDesc.mostDetailedMip) : _srvDesc.mipLevels;
						srvDesc.TextureCube.MostDetailedMip = (UINT)_srvDesc.mostDetailedMip;
						srvDesc.TextureCube.ResourceMinLODClamp = _srvDesc.resourceMinLODClamp;
#ifdef DEVCHECKS
						if (srvDesc.TextureCube.MipLevels == 0 || srvDesc.TextureCube.MostDetailedMip >= rDesc.MipLevels || srvDesc.TextureCube.MipLevels > rDesc.MipLevels || (srvDesc.TextureCube.MostDetailedMip + srvDesc.TextureCube.MipLevels) > rDesc.MipLevels)
							throw GraphicsException(this, MTEXT("The mip level specified for this SRV is invalid for the given resource."));
#endif
					}
					else {
						srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBEARRAY;
						srvDesc.TextureCubeArray.First2DArrayFace = _srvDesc.firstArraySlice;
						srvDesc.TextureCubeArray.MipLevels = _srvDesc.mipLevels == -1 ? (rDesc.MipLevels - (UINT)_srvDesc.mostDetailedMip) : _srvDesc.mipLevels;
						srvDesc.TextureCubeArray.MostDetailedMip = (UINT)_srvDesc.mostDetailedMip;
						srvDesc.TextureCubeArray.NumCubes = _srvDesc.arraySize == -1 ? (rDesc.DepthOrArraySize - _srvDesc.firstArraySlice) / 6u : _srvDesc.arraySize;
						srvDesc.TextureCubeArray.ResourceMinLODClamp = _srvDesc.resourceMinLODClamp;
#ifdef DEVCHECKS
						if (srvDesc.TextureCubeArray.MipLevels == 0 || srvDesc.TextureCubeArray.MostDetailedMip >= rDesc.MipLevels || srvDesc.TextureCubeArray.MipLevels > rDesc.MipLevels || (srvDesc.TextureCubeArray.MostDetailedMip + srvDesc.TextureCubeArray.MipLevels) > rDesc.MipLevels)
							throw GraphicsException(this, MTEXT("The mip level specified for this SRV is invalid for the given resource."));
						if (srvDesc.TextureCubeArray.NumCubes == 0 || srvDesc.TextureCubeArray.First2DArrayFace >= rDesc.DepthOrArraySize || (srvDesc.TextureCubeArray.NumCubes * 6u) > rDesc.DepthOrArraySize || (srvDesc.TextureCubeArray.First2DArrayFace + srvDesc.TextureCubeArray.NumCubes * 6u) > rDesc.DepthOrArraySize)
							throw GraphicsException(this, MTEXT("The array specification for this SRV is invalid for the given resource."));
#endif
					}
				}
				else {
					srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
					srvDesc.Texture2DArray.ArraySize = _srvDesc.arraySize == -1 ? (rDesc.DepthOrArraySize - _srvDesc.firstArraySlice) : _srvDesc.arraySize;
					srvDesc.Texture2DArray.FirstArraySlice = _srvDesc.firstArraySlice;
					srvDesc.Texture2DArray.MipLevels = _srvDesc.mipLevels == -1 ? (rDesc.MipLevels - (UINT)_srvDesc.mostDetailedMip) : _srvDesc.mipLevels;
					srvDesc.Texture2DArray.MostDetailedMip = (UINT)_srvDesc.mostDetailedMip;
					srvDesc.Texture2DArray.PlaneSlice = _srvDesc.planeSlice;
					srvDesc.Texture2DArray.ResourceMinLODClamp = _srvDesc.resourceMinLODClamp;
#ifdef DEVCHECKS
					if (srvDesc.Texture2DArray.MipLevels == 0 || srvDesc.Texture2DArray.MostDetailedMip >= rDesc.MipLevels || srvDesc.Texture2DArray.MipLevels > rDesc.MipLevels || (srvDesc.Texture2DArray.MostDetailedMip + srvDesc.Texture2DArray.MipLevels) > rDesc.MipLevels)
						throw GraphicsException(this, MTEXT("The mip level specified for this SRV is invalid for the given resource."));
					if (srvDesc.Texture2DArray.ArraySize == 0 || srvDesc.Texture2DArray.FirstArraySlice >= rDesc.DepthOrArraySize || srvDesc.Texture2DArray.ArraySize > rDesc.DepthOrArraySize || (srvDesc.Texture2DArray.FirstArraySlice + srvDesc.Texture2DArray.ArraySize) > rDesc.DepthOrArraySize)
						throw GraphicsException(this, MTEXT("The array specification for this SRV is invalid for the given resource."));
#endif
				}
			}
		}
		else {
			if (rDesc.DepthOrArraySize == 1) {
				srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMS;
			}
			else {
				srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMSARRAY;
				srvDesc.Texture2DMSArray.ArraySize = _srvDesc.arraySize == -1 ? (rDesc.DepthOrArraySize - _srvDesc.firstArraySlice) : _srvDesc.arraySize;
				srvDesc.Texture2DMSArray.FirstArraySlice = _srvDesc.firstArraySlice;
#ifdef DEVCHECKS
				if (srvDesc.Texture2DMSArray.ArraySize == 0 || srvDesc.Texture2DMSArray.FirstArraySlice >= rDesc.DepthOrArraySize || srvDesc.Texture2DMSArray.ArraySize > rDesc.DepthOrArraySize || (srvDesc.Texture2DMSArray.FirstArraySlice + srvDesc.Texture2DMSArray.ArraySize) > rDesc.DepthOrArraySize)
					throw GraphicsException(this, MTEXT("The array specification for this SRV is invalid for the given resource."));
#endif
			}
		}
		break;
	case D3D12_RESOURCE_DIMENSION_TEXTURE3D:
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
		srvDesc.Texture3D.MipLevels = _srvDesc.mipLevels == -1 ? (rDesc.MipLevels - (UINT)_srvDesc.mostDetailedMip) : _srvDesc.mipLevels;
		srvDesc.Texture3D.MostDetailedMip = (UINT)_srvDesc.mostDetailedMip;
		srvDesc.Texture3D.ResourceMinLODClamp = _srvDesc.resourceMinLODClamp;
#ifdef DEVCHECKS
		if (srvDesc.Texture3D.MipLevels == 0 || srvDesc.Texture3D.MostDetailedMip >= rDesc.MipLevels || srvDesc.Texture3D.MipLevels > rDesc.MipLevels || (srvDesc.Texture3D.MostDetailedMip + srvDesc.Texture3D.MipLevels) > rDesc.MipLevels)
			throw GraphicsException(this, MTEXT("The mip level specified for this SRV is invalid for the given resource."));
#endif
		break;
	default:
		throw GraphicsException(this, MTEXT("Invalid SRV descriptor!"));
	}
}

void GraphicsResourceChip::GetRenderTargetViewDesc(D3D12_RENDER_TARGET_VIEW_DESC& rtvDesc)
{
	ID3D12Resource* res = GetResource();

	D3D12_RESOURCE_DESC rDesc = res->GetDesc();

	rtvDesc.Format = _rtvDesc.format == DXGI_FORMAT_UNKNOWN ? rDesc.Format : (DXGI_FORMAT)_rtvDesc.format;

#ifdef DEVCHECKS
	if (!(rDesc.Flags & D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET))
		throw GraphicsException(this, MTEXT("This resource does not support RTV."));

	if (dxgiformat::IsTypeless(rtvDesc.Format) || dxgiformat::IsTypeless(rDesc.Format) && rDesc.Format != dxgiformat::MakeTypeless(rtvDesc.Format))
		throw GraphicsException(this, MTEXT("Can not set a typeless format as a RTV."));

	if (dxgiformat::MakeTypeless(rtvDesc.Format) != dxgiformat::MakeTypeless(rDesc.Format))
		throw GraphicsException(this, MTEXT("The format for this RTV is incompatible with the resource."));
#endif

	switch (rDesc.Dimension)
	{
	case D3D12_RESOURCE_DIMENSION_BUFFER:
		rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_BUFFER;
		rtvDesc.Buffer.FirstElement = _rtvDesc.mipSlice;
		rtvDesc.Buffer.NumElements = _rtvDesc.firstArraySlice;
#ifdef DEVCHECKS
		if (rtvDesc.Buffer.FirstElement >= rDesc.Width || rtvDesc.Buffer.NumElements > rDesc.Width || (rtvDesc.Buffer.FirstElement + rtvDesc.Buffer.NumElements) > rDesc.Width)
			throw GraphicsException(this, MTEXT("The buffer specification for this RTV is invalid for the given resource."));
#endif
		break;
	case D3D12_RESOURCE_DIMENSION_TEXTURE1D:
		if (rDesc.DepthOrArraySize == 1) {
			rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE1D;
			rtvDesc.Texture1D.MipSlice = (UINT)_rtvDesc.mipSlice;
#ifdef DEVCHECKS
			if (rtvDesc.Texture1D.MipSlice >= rDesc.MipLevels)
				throw GraphicsException(this, MTEXT("The mip level specified for this RTV is invalid for the given resource."));
#endif
		}
		else {
			rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE1DARRAY;
			rtvDesc.Texture1DArray.MipSlice = (UINT)_rtvDesc.mipSlice;
			rtvDesc.Texture1DArray.FirstArraySlice = _rtvDesc.firstArraySlice;
			rtvDesc.Texture1DArray.ArraySize = _rtvDesc.arraySize == -1 ? (rDesc.DepthOrArraySize - _rtvDesc.firstArraySlice) : _rtvDesc.arraySize;
#ifdef DEVCHECKS
			if (rtvDesc.Texture1DArray.MipSlice >= rDesc.MipLevels)
				throw GraphicsException(this, MTEXT("The mip level specified for this RTV is invalid for the given resource."));
			if (rtvDesc.Texture1DArray.ArraySize == 0 || rtvDesc.Texture1DArray.FirstArraySlice >= rDesc.DepthOrArraySize || rtvDesc.Texture1DArray.ArraySize > rDesc.DepthOrArraySize || (rtvDesc.Texture1DArray.FirstArraySlice + rtvDesc.Texture1DArray.ArraySize) > rDesc.DepthOrArraySize)
				throw GraphicsException(this, MTEXT("The array specification for this RTV is invalid for the given resource."));
#endif
		}
		break;
	case D3D12_RESOURCE_DIMENSION_TEXTURE2D:
		if (rDesc.SampleDesc.Count == 1) {
			if (rDesc.DepthOrArraySize == 1) {
				rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
				rtvDesc.Texture2D.MipSlice = (UINT)_rtvDesc.mipSlice;
				rtvDesc.Texture2D.PlaneSlice = 0;
				if (rtvDesc.Texture2D.MipSlice >= rDesc.MipLevels)
					throw GraphicsException(this, MTEXT("The mip level specified for this RTV is invalid for the given resource."));
			}
			else {
				rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
				rtvDesc.Texture2DArray.MipSlice = (UINT)_rtvDesc.mipSlice;
				rtvDesc.Texture2DArray.FirstArraySlice = _rtvDesc.firstArraySlice;
				rtvDesc.Texture2DArray.ArraySize = _rtvDesc.arraySize == -1 ? (rDesc.DepthOrArraySize - _rtvDesc.firstArraySlice) : _rtvDesc.arraySize;
				rtvDesc.Texture2DArray.PlaneSlice = 0;
#ifdef DEVCHECKS
				if (rtvDesc.Texture2DArray.MipSlice >= rDesc.MipLevels)
					throw GraphicsException(this, MTEXT("The mip level specified for this RTV is invalid for the given resource."));
				if (rtvDesc.Texture2DArray.ArraySize == 0 || rtvDesc.Texture2DArray.FirstArraySlice >= rDesc.DepthOrArraySize || rtvDesc.Texture2DArray.ArraySize > rDesc.DepthOrArraySize || (rtvDesc.Texture2DArray.FirstArraySlice + rtvDesc.Texture2DArray.ArraySize) > rDesc.DepthOrArraySize)
					throw GraphicsException(this, MTEXT("The array specification for this RTV is invalid for the given resource."));
#endif
			}
		}
		else {
			if (rDesc.DepthOrArraySize == 1) {
				rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DMS;
			}
			else {
				rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DMSARRAY;
				rtvDesc.Texture2DMSArray.FirstArraySlice = _rtvDesc.firstArraySlice;
				rtvDesc.Texture2DMSArray.ArraySize = _rtvDesc.arraySize == -1 ? (rDesc.DepthOrArraySize - _rtvDesc.firstArraySlice) : _rtvDesc.arraySize;
#ifdef DEVCHECKS
				if (rtvDesc.Texture2DMSArray.ArraySize == 0 || rtvDesc.Texture2DMSArray.FirstArraySlice >= rDesc.DepthOrArraySize || rtvDesc.Texture2DMSArray.ArraySize > rDesc.DepthOrArraySize || (rtvDesc.Texture2DMSArray.FirstArraySlice + rtvDesc.Texture2DMSArray.ArraySize) > rDesc.DepthOrArraySize)
					throw GraphicsException(this, MTEXT("The array specification for this RTV is invalid for the given resource."));
#endif
			}
		}
		break;
	case D3D12_RESOURCE_DIMENSION_TEXTURE3D:
	{
		UINT mipLevelWSize = std::max(1u, rDesc.DepthOrArraySize / (1u << (UINT)_rtvDesc.mipSlice));
		rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE3D;
		rtvDesc.Texture3D.MipSlice = (UINT)_rtvDesc.mipSlice;
		rtvDesc.Texture3D.FirstWSlice = _rtvDesc.firstArraySlice;
		rtvDesc.Texture3D.WSize = _rtvDesc.arraySize == -1 ? (mipLevelWSize - _rtvDesc.firstArraySlice) : _rtvDesc.arraySize;
#ifdef DEVCHECKS
		if (rtvDesc.Texture3D.MipSlice >= rDesc.MipLevels)
			throw GraphicsException(this, MTEXT("The mip level specified for this RTV is invalid for the given resource."));
		if (rtvDesc.Texture3D.WSize == 0 || rtvDesc.Texture3D.FirstWSlice >= mipLevelWSize || rtvDesc.Texture3D.WSize > mipLevelWSize || (rtvDesc.Texture3D.FirstWSlice + rtvDesc.Texture3D.WSize) > mipLevelWSize)
			throw GraphicsException(this, MTEXT("The array specification for this RTV is invalid for the given resource."));
#endif
	}
	break;
	default:
		throw GraphicsException(this, MTEXT("Invalid RTV descriptor!"));
	}
}

void GraphicsResourceChip::GetDepthStencilViewDesc(D3D12_DEPTH_STENCIL_VIEW_DESC& dsvDesc)
{
	ID3D12Resource* res = GetResource();

	D3D12_RESOURCE_DESC rDesc = res->GetDesc();

	dsvDesc.Format = _dsvDesc.format == DXGI_FORMAT_UNKNOWN ? rDesc.Format : (DXGI_FORMAT)_dsvDesc.format;
	dsvDesc.Flags = (D3D12_DSV_FLAGS)_dsvDesc.flags;

#ifdef DEVCHECKS
	if (!(rDesc.Flags & D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL))
		throw GraphicsException(this, MTEXT("This resource does not support DSV."));

	if (dxgiformat::IsTypeless(dsvDesc.Format) || dxgiformat::IsTypeless(rDesc.Format) && rDesc.Format != dxgiformat::MakeTypeless(dsvDesc.Format))
		throw GraphicsException(this, MTEXT("Can not set a typeless format as a DSV."));

	if (dxgiformat::MakeTypeless(dsvDesc.Format) != dxgiformat::MakeTypeless(rDesc.Format))
		throw GraphicsException(this, MTEXT("The format for this DSV is incompatible with the resource."));
#endif

	switch (rDesc.Dimension)
	{
	case D3D12_RESOURCE_DIMENSION_BUFFER:
		throw GraphicsException(this, MTEXT("This resource does not support DSV."));
	case D3D12_RESOURCE_DIMENSION_TEXTURE1D:
		if (rDesc.DepthOrArraySize == 1) {
			dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE1D;
			dsvDesc.Texture1D.MipSlice = _dsvDesc.mipSlice;
#ifdef DEVCHECKS
			if (dsvDesc.Texture1D.MipSlice >= rDesc.MipLevels)
				throw GraphicsException(this, MTEXT("The mip level specified for this DSV is invalid for the given resource."));
#endif
		}
		else {
			dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE1DARRAY;
			dsvDesc.Texture1DArray.MipSlice = _dsvDesc.mipSlice;
			dsvDesc.Texture1DArray.FirstArraySlice = _dsvDesc.firstArraySlice;
			dsvDesc.Texture1DArray.ArraySize = _dsvDesc.arraySize == -1 ? (rDesc.DepthOrArraySize - _dsvDesc.firstArraySlice) : _dsvDesc.arraySize;
#ifdef DEVCHECKS
			if (dsvDesc.Texture1DArray.MipSlice >= rDesc.MipLevels)
				throw GraphicsException(this, MTEXT("The mip level specified for this DSV is invalid for the given resource."));
			if (dsvDesc.Texture1DArray.ArraySize == 0 || dsvDesc.Texture1DArray.FirstArraySlice >= rDesc.DepthOrArraySize || dsvDesc.Texture1DArray.ArraySize > rDesc.DepthOrArraySize || (dsvDesc.Texture1DArray.FirstArraySlice + dsvDesc.Texture1DArray.ArraySize) > rDesc.DepthOrArraySize)
				throw GraphicsException(this, MTEXT("The array specification for this DSV is invalid for the given resource."));
#endif
		}
		break;
	case D3D12_RESOURCE_DIMENSION_TEXTURE2D:
		if (rDesc.SampleDesc.Count == 1) {
			if (rDesc.DepthOrArraySize == 1) {
				dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
				dsvDesc.Texture2D.MipSlice = _dsvDesc.mipSlice;
#ifdef DEVCHECKS
				if (dsvDesc.Texture2D.MipSlice >= rDesc.MipLevels)
					throw GraphicsException(this, MTEXT("The mip level specified for this DSV is invalid for the given resource."));
#endif
			}
			else {
				dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
				dsvDesc.Texture2DArray.MipSlice = _dsvDesc.mipSlice;
				dsvDesc.Texture2DArray.FirstArraySlice = _dsvDesc.firstArraySlice;
				dsvDesc.Texture2DArray.ArraySize = _dsvDesc.arraySize == -1 ? (rDesc.DepthOrArraySize - _dsvDesc.firstArraySlice) : _dsvDesc.arraySize;
#ifdef DEVCHECKS
				if (dsvDesc.Texture2DArray.MipSlice >= rDesc.MipLevels)
					throw GraphicsException(this, MTEXT("The mip level specified for this DSV is invalid for the given resource."));
				if (dsvDesc.Texture2DArray.ArraySize == 0 || dsvDesc.Texture2DArray.FirstArraySlice >= rDesc.DepthOrArraySize || dsvDesc.Texture2DArray.ArraySize > rDesc.DepthOrArraySize || (dsvDesc.Texture2DArray.FirstArraySlice + dsvDesc.Texture2DArray.ArraySize) > rDesc.DepthOrArraySize)
					throw GraphicsException(this, MTEXT("The array specification for this DSV is invalid for the given resource."));
#endif
			}
		}
		else {
			if (rDesc.DepthOrArraySize == 1) {
				dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DMS;
			}
			else {
				dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DMSARRAY;
				dsvDesc.Texture2DMSArray.FirstArraySlice = _dsvDesc.firstArraySlice;
				dsvDesc.Texture2DMSArray.ArraySize = _dsvDesc.arraySize == -1 ? (rDesc.DepthOrArraySize - _dsvDesc.firstArraySlice) : _dsvDesc.arraySize;
#ifdef DEVCHECKS
				if (dsvDesc.Texture2DMSArray.ArraySize == 0 || dsvDesc.Texture2DMSArray.FirstArraySlice >= rDesc.DepthOrArraySize || dsvDesc.Texture2DMSArray.ArraySize > rDesc.DepthOrArraySize || (dsvDesc.Texture2DMSArray.FirstArraySlice + dsvDesc.Texture2DMSArray.ArraySize) > rDesc.DepthOrArraySize)
					throw GraphicsException(this, MTEXT("The array specification for this DSV is invalid for the given resource."));
#endif
			}
		}
		break;
	case D3D12_RESOURCE_DIMENSION_TEXTURE3D:
		throw GraphicsException(this, MTEXT("This resource does not support DSV."));
	default:
		throw GraphicsException(this, MTEXT("Invalid DSV descriptor!"));
	}
}

void GraphicsResourceChip::GetUnorderedAccessViewDesc(D3D12_UNORDERED_ACCESS_VIEW_DESC& uavDesc)
{
	ID3D12Resource* res = GetResource();

	D3D12_RESOURCE_DESC rDesc = res->GetDesc();

	uavDesc.Format = _uavDesc.format == DXGI_FORMAT_UNKNOWN ? rDesc.Format : (DXGI_FORMAT)_uavDesc.format;

#ifdef DEVCHECKS
	if (!(rDesc.Flags & D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS))
		throw GraphicsException(this, MTEXT("This resource does not support UAV."));

	if (dxgiformat::IsTypeless(uavDesc.Format) || dxgiformat::IsTypeless(rDesc.Format) && rDesc.Format != dxgiformat::MakeTypeless(uavDesc.Format))
		throw GraphicsException(this, MTEXT("Can not set a typeless format as a UAV."));

	if (dxgiformat::MakeTypeless(uavDesc.Format) != dxgiformat::MakeTypeless(rDesc.Format))
		throw GraphicsException(this, MTEXT("The format for this UAV is incompatible with the resource."));
#endif

	switch (rDesc.Dimension)
	{
	case D3D12_RESOURCE_DIMENSION_BUFFER:
		uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
		uavDesc.Buffer.FirstElement = _uavDesc.mipSlice;
		uavDesc.Buffer.NumElements = _uavDesc.firstArraySlice;
		uavDesc.Buffer.StructureByteStride = _uavDesc.arraySize;
		uavDesc.Buffer.CounterOffsetInBytes = _uavDesc.counterOffsetInBytes;
		uavDesc.Buffer.Flags = (D3D12_BUFFER_UAV_FLAGS)_uavDesc.flags;
#ifdef DEVCHECKS
		if (uavDesc.Format != DXGI_FORMAT_UNKNOWN)
			throw GraphicsException(this, MTEXT("Format for buffer UAVs must be set to Unknown."));
		if (uavDesc.Buffer.FirstElement > rDesc.Width || uavDesc.Buffer.NumElements > rDesc.Width || uavDesc.Buffer.StructureByteStride > rDesc.Width || (uavDesc.Buffer.NumElements + uavDesc.Buffer.FirstElement) * uavDesc.Buffer.StructureByteStride > rDesc.Width)
			throw GraphicsException(this, MTEXT("First element plus number of elements multipled by structure byte stride can not be greater than size of buffer in bytes."));
#endif
		break;
	case D3D12_RESOURCE_DIMENSION_TEXTURE1D:
		if (rDesc.DepthOrArraySize == 1) {
			uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE1D;
			uavDesc.Texture1D.MipSlice = (UINT)_uavDesc.mipSlice;
#ifdef DEVCHECKS
			if (uavDesc.Texture1D.MipSlice >= rDesc.MipLevels)
				throw GraphicsException(this, MTEXT("The mip level specified for this UAV is invalid for the given resource."));
#endif
		}
		else {
			uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE1DARRAY;
			uavDesc.Texture1DArray.MipSlice = (UINT)_uavDesc.mipSlice;
			uavDesc.Texture1DArray.FirstArraySlice = _uavDesc.firstArraySlice;
			uavDesc.Texture1DArray.ArraySize = _uavDesc.arraySize == -1 ? (rDesc.DepthOrArraySize - _uavDesc.firstArraySlice) : _uavDesc.arraySize;
#ifdef DEVCHECKS
			if (uavDesc.Texture1DArray.MipSlice >= rDesc.MipLevels)
				throw GraphicsException(this, MTEXT("The mip level specified for this UAV is invalid for the given resource."));
			if (uavDesc.Texture1DArray.ArraySize == 0 || uavDesc.Texture1DArray.FirstArraySlice >= rDesc.DepthOrArraySize || uavDesc.Texture1DArray.ArraySize > rDesc.DepthOrArraySize || (uavDesc.Texture1DArray.FirstArraySlice + uavDesc.Texture1DArray.ArraySize) > rDesc.DepthOrArraySize)
				throw GraphicsException(this, MTEXT("The array specification for this UAV is invalid for the given resource."));
#endif
		}
		break;
	case D3D12_RESOURCE_DIMENSION_TEXTURE2D:
		if (rDesc.DepthOrArraySize == 1) {
			uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
			uavDesc.Texture2D.MipSlice = (UINT)_uavDesc.mipSlice;
			uavDesc.Texture2D.PlaneSlice = 0;
#ifdef DEVCHECKS
			if (uavDesc.Texture2D.MipSlice >= rDesc.MipLevels)
				throw GraphicsException(this, MTEXT("The mip level specified for this UAV is invalid for the given resource."));
#endif
		}
		else {
			uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
			uavDesc.Texture2DArray.MipSlice = (UINT)_uavDesc.mipSlice;
			uavDesc.Texture2DArray.FirstArraySlice = _uavDesc.firstArraySlice;
			uavDesc.Texture2DArray.ArraySize = _uavDesc.arraySize == -1 ? (rDesc.DepthOrArraySize - _uavDesc.firstArraySlice) : _uavDesc.arraySize;
			uavDesc.Texture2DArray.PlaneSlice = 0;
#ifdef DEVCHECKS
			if (uavDesc.Texture2DArray.MipSlice >= rDesc.MipLevels)
				throw GraphicsException(this, MTEXT("The mip level specified for this UAV is invalid for the given resource."));
			if (uavDesc.Texture2DArray.ArraySize == 0 || uavDesc.Texture2DArray.FirstArraySlice >= rDesc.DepthOrArraySize || uavDesc.Texture2DArray.ArraySize > rDesc.DepthOrArraySize || (uavDesc.Texture2DArray.FirstArraySlice + uavDesc.Texture2DArray.ArraySize) > rDesc.DepthOrArraySize)
				throw GraphicsException(this, MTEXT("The array specification for this UAV is invalid for the given resource."));
#endif
		}
		break;
	case D3D12_RESOURCE_DIMENSION_TEXTURE3D:
	{
		UINT mipLevelWSize = std::max(1u, rDesc.DepthOrArraySize / (1u << (UINT)_rtvDesc.mipSlice));
		uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE3D;
		uavDesc.Texture3D.MipSlice = (UINT)_uavDesc.mipSlice;
		uavDesc.Texture3D.FirstWSlice = _uavDesc.firstArraySlice;
		uavDesc.Texture3D.WSize = _uavDesc.arraySize == -1 ? (mipLevelWSize - _uavDesc.firstArraySlice) : _uavDesc.arraySize;
#ifdef DEVCHECKS
		if (uavDesc.Texture3D.MipSlice >= rDesc.MipLevels)
			throw GraphicsException(this, MTEXT("The mip level specified for this UAV is invalid for the given resource."));
		if (uavDesc.Texture3D.WSize == 0 || uavDesc.Texture3D.FirstWSlice >= mipLevelWSize || uavDesc.Texture3D.WSize > mipLevelWSize || (uavDesc.Texture3D.FirstWSlice + uavDesc.Texture3D.WSize) > mipLevelWSize)
			throw GraphicsException(this, MTEXT("The array specification for this UAV is invalid for the given resource."));
#endif
	}
	break;
	default:
		throw GraphicsException(this, MTEXT("Invalid UAV descriptor!"));
	}
}

void GraphicsResourceChip::GetConstantBufferViewDesc(D3D12_CONSTANT_BUFFER_VIEW_DESC& cbvDesc)
{
	ID3D12Resource* res = GetResource();

	D3D12_RESOURCE_DESC rDesc = res->GetDesc();

	cbvDesc.BufferLocation = res->GetGPUVirtualAddress();
	cbvDesc.SizeInBytes = (UINT)rDesc.Width;
}

void GraphicsResourceChip::SetSRVDesc(const SRVDesc& srvDesc)
{
	auto a = GetUpdateStamp();

	if (srvDesc == _srvDesc)
		return;
	_srvDesc = srvDesc;
	SetUpdateStamp();

	if (a != GetUpdateStamp())
		_srv = nullptr;
}

void GraphicsResourceChip::SetRTVDesc(const RTVDesc& rtvDesc)
{
	auto a = GetUpdateStamp();

	if (rtvDesc == _rtvDesc)
		return;
	_rtvDesc = rtvDesc;
	SetUpdateStamp();

	if (a != GetUpdateStamp())
		_rtv = nullptr;
}

void GraphicsResourceChip::SetDSVDesc(const DSVDesc& dsvDesc)
{
	auto a = GetUpdateStamp();

	if (dsvDesc == _dsvDesc)
		return;
	_dsvDesc = dsvDesc;
	SetUpdateStamp();

	if (a != GetUpdateStamp())
		_dsv = nullptr;
}

void GraphicsResourceChip::SetUAVDesc(const UAVDesc& uavDesc)
{
	if (uavDesc == _uavDesc)
		return;
	_uavDesc = uavDesc;
	SetUpdateStamp();
}

