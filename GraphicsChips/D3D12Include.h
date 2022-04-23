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
#include "M3DEngine/GlobalDef.h"
#include "M3DCore/ComPtr.h"
#include "M3DCore/MMath.h"
#include <DXGI.h>
#include <DXGI1_3.h> // win8
#include <DXGI1_4.h> // win8
#include <dxgi1_5.h>
#include <d3d12.h>
#include "D3DX12.h"
#include <d3dcompiler.h>
#include <functional>
#include "M3DCore/Containers.h"


#include <d3d12sdklayers.h>

#define COMMAND_LIST_BUFFER_SIZE 4

namespace m3d
{
struct GRAPHICSCHIPS_API DelayedReleaseComPtrTraitsBase
{
	struct ResourceList
	{
		~ResourceList();
		List<IUnknown*> r;
	};

	static ResourceList resources; 
};

template<typename T>
struct DelayedReleaseComPtrTraits : public DelayedReleaseComPtrTraitsBase
{
	static void AddRef(T *t) { t->AddRef(); }
	static void Release(T *t) 
	{ 
		resources.r.push_back(t); 
	}
};
}


#define IPOINTERS(x)	typedef m3d::ComPtr<x> S##x; typedef m3d::ComPtr<x, m3d::DelayedReleaseComPtrTraits<x>> R##x; 


IPOINTERS(IUnknown)

IPOINTERS(IDXGIObject)
IPOINTERS(IDXGIFactory)
IPOINTERS(IDXGIFactory1)
IPOINTERS(IDXGIFactory2)
IPOINTERS(IDXGIAdapter)
IPOINTERS(IDXGIAdapter1)
IPOINTERS(IDXGIDevice)
IPOINTERS(IDXGIDevice1)
IPOINTERS(IDXGIOutput)
IPOINTERS(IDXGISwapChain)

// win8 (DXGI1_2.h)
IPOINTERS(IDXGIFactory2)
IPOINTERS(IDXGIAdapter2)
IPOINTERS(IDXGIOutput1)
IPOINTERS(IDXGISwapChain1)
IPOINTERS(IDXGIDevice2)

// DXGI1_3
IPOINTERS(IDXGIDecodeSwapChain)
IPOINTERS(IDXGIDevice3)
IPOINTERS(IDXGIFactory3)
IPOINTERS(IDXGIOutput2)
IPOINTERS(IDXGIOutput3)
IPOINTERS(IDXGISwapChain2)

// DXGI1_4
IPOINTERS(IDXGIAdapter3)
IPOINTERS(IDXGIFactory4)
IPOINTERS(IDXGIOutput4)
IPOINTERS(IDXGISwapChain3)

// DXGI1_5
IPOINTERS(IDXGIDevice4)
IPOINTERS(IDXGIFactory5)
IPOINTERS(IDXGIOutput5)
IPOINTERS(IDXGISwapChain4)

IPOINTERS(ID3DBlob)
IPOINTERS(ID3D12Debug)
IPOINTERS(ID3D12InfoQueue)
IPOINTERS(ID3D12DebugDevice)
IPOINTERS(ID3D12DebugCommandList)
IPOINTERS(ID3D12DebugCommandQueue)

IPOINTERS(ID3D12CommandAllocator)
IPOINTERS(ID3D12CommandQueue)
IPOINTERS(ID3D12DescriptorHeap)
IPOINTERS(ID3D12GraphicsCommandList)
IPOINTERS(ID3D12Fence)

IPOINTERS(ID3D12DeviceChild)
IPOINTERS(ID3D12Device)

IPOINTERS(ID3D12Pageable)
IPOINTERS(ID3D12Resource)
IPOINTERS(ID3D12PipelineState)
IPOINTERS(ID3D12RootSignature)

IPOINTERS(ID3D12ShaderReflection)


/*
IPOINTERS(ID3D12VertexShader)
IPOINTERS(ID3D12HullShader)
IPOINTERS(ID3D12DomainShader)
IPOINTERS(ID3D12GeometryShader)
IPOINTERS(ID3D12PixelShader)
IPOINTERS(ID3D12ComputeShader)

IPOINTERS(ID3D11Predicate)
IPOINTERS(ID3D11ClassInstance)
IPOINTERS(ID3D11ClassLinkage)

#ifdef WINDESKTOP
IPOINTERS(ID3D11ShaderReflection)
//IPOINTERS(ID3D11ShaderTrace)
//IPOINTERS(ID3D11ShaderTraceFactory)

#endif

IPOINTERS(ID3DBlob)
*/


struct CD3DX12_CONSTANT_BUFFER_VIEW_DESC : public D3D12_CONSTANT_BUFFER_VIEW_DESC
{
	CD3DX12_CONSTANT_BUFFER_VIEW_DESC() {}
	explicit CD3DX12_CONSTANT_BUFFER_VIEW_DESC(const D3D12_CONSTANT_BUFFER_VIEW_DESC &r) : D3D12_CONSTANT_BUFFER_VIEW_DESC(r) {}
	CD3DX12_CONSTANT_BUFFER_VIEW_DESC(D3D12_GPU_VIRTUAL_ADDRESS bufferLocation, UINT sizeInBytes) 
	{
		BufferLocation = bufferLocation;
		SizeInBytes = sizeInBytes;
	}
};

struct CD3DX12_SHADER_RESOURCE_VIEW_DESC : public D3D12_SHADER_RESOURCE_VIEW_DESC
{
	CD3DX12_SHADER_RESOURCE_VIEW_DESC() {}
	explicit CD3DX12_SHADER_RESOURCE_VIEW_DESC(const D3D12_SHADER_RESOURCE_VIEW_DESC &r) : D3D12_SHADER_RESOURCE_VIEW_DESC(r) {}
	CD3DX12_SHADER_RESOURCE_VIEW_DESC(D3D12_SRV_DIMENSION viewDimension, DXGI_FORMAT format, UINT shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING)
	{
		Format = format;
		ViewDimension = viewDimension;
		Shader4ComponentMapping = shader4ComponentMapping;
		switch (viewDimension)
		{
		case D3D12_SRV_DIMENSION_BUFFER:
			Buffer.FirstElement = 0;
			Buffer.NumElements = 0;
			Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
			Buffer.StructureByteStride = 0;
			break;
		case D3D12_SRV_DIMENSION_TEXTURE1D:
			Texture1D.MostDetailedMip = 0;
			Texture1D.MipLevels = -1;
			Texture1D.ResourceMinLODClamp = 0.0f;
			break;
		case D3D12_SRV_DIMENSION_TEXTURE1DARRAY:
			Texture1DArray.MostDetailedMip = 0;
			Texture1DArray.MipLevels = -1;
			Texture1DArray.FirstArraySlice = 0;
			Texture1DArray.ArraySize = 0;
			break;
		case D3D12_SRV_DIMENSION_TEXTURE2D:
			Texture2D.MostDetailedMip = 0;
			Texture2D.MipLevels = -1;
			Texture2D.PlaneSlice = 0;
			Texture2D.ResourceMinLODClamp = 0.0f;
			break;
		case D3D12_SRV_DIMENSION_TEXTURE2DARRAY:
			Texture2DArray.MostDetailedMip = 0;
			Texture2DArray.MipLevels = -1;
			Texture2DArray.FirstArraySlice = 0;
			Texture2DArray.ArraySize = 0;
			Texture2DArray.PlaneSlice = 0;
			Texture2DArray.ResourceMinLODClamp = 0.0f;
			break;
		case D3D12_SRV_DIMENSION_TEXTURE2DMS:
			break;
		case D3D12_SRV_DIMENSION_TEXTURE2DMSARRAY:
			Texture2DMSArray.FirstArraySlice = 0;
			Texture2DMSArray.ArraySize = 0;
			break;
		case D3D12_SRV_DIMENSION_TEXTURE3D:
			Texture3D.MostDetailedMip = 0;
			Texture3D.MipLevels = -1;
			Texture3D.ResourceMinLODClamp = 0.0f;
			break;
		case D3D12_SRV_DIMENSION_TEXTURECUBE:
			TextureCube.MostDetailedMip = 0;
			TextureCube.MipLevels = -1;
			TextureCube.ResourceMinLODClamp = 0.0f;
			break;
		case D3D12_SRV_DIMENSION_TEXTURECUBEARRAY:
			TextureCubeArray.MostDetailedMip = 0;
			TextureCubeArray.MipLevels = -1;
			TextureCubeArray.First2DArrayFace = 0;
			TextureCubeArray.NumCubes = 0;
			TextureCubeArray.ResourceMinLODClamp = 0.0f;
			break;
		}
	}
};

struct CD3DX12_UNORDERED_ACCESS_VIEW_DESC : public D3D12_UNORDERED_ACCESS_VIEW_DESC
{
	CD3DX12_UNORDERED_ACCESS_VIEW_DESC() {}
	explicit CD3DX12_UNORDERED_ACCESS_VIEW_DESC(const D3D12_UNORDERED_ACCESS_VIEW_DESC &r) : D3D12_UNORDERED_ACCESS_VIEW_DESC(r) {}
	CD3DX12_UNORDERED_ACCESS_VIEW_DESC(D3D12_UAV_DIMENSION viewDimension, DXGI_FORMAT format)
	{
		Format = format;
		ViewDimension = viewDimension;
		switch (viewDimension)
		{
		case D3D12_SRV_DIMENSION_BUFFER:
			Buffer.CounterOffsetInBytes = 0;
			Buffer.FirstElement = 0;
			Buffer.NumElements = 0;
			Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;
			Buffer.StructureByteStride = 0;
			break;
		case D3D12_SRV_DIMENSION_TEXTURE1D:
			Texture1D.MipSlice = 0;
			break;
		case D3D12_SRV_DIMENSION_TEXTURE1DARRAY:
			Texture1DArray.MipSlice = 0;
			Texture1DArray.FirstArraySlice = 0;
			Texture1DArray.ArraySize = 0;
			break;
		case D3D12_SRV_DIMENSION_TEXTURE2D:
			Texture2D.MipSlice = 0;
			Texture2D.PlaneSlice = 0;
			break;
		case D3D12_SRV_DIMENSION_TEXTURE2DARRAY:
			Texture2DArray.MipSlice = 0;
			Texture2DArray.FirstArraySlice = 0;
			Texture2DArray.ArraySize = 0;
			Texture2DArray.PlaneSlice = 0;
			break;
		case D3D12_SRV_DIMENSION_TEXTURE3D:
			Texture3D.MipSlice = 0;
			Texture3D.FirstWSlice = 0;
			Texture3D.WSize = 0;
			break;
		}
	}
};

struct CD3DX12_SAMPLER_DESC : public D3D12_SAMPLER_DESC
{
	CD3DX12_SAMPLER_DESC() {}
	explicit CD3DX12_SAMPLER_DESC(const D3D12_SAMPLER_DESC &r) : D3D12_SAMPLER_DESC(r) {}
	CD3DX12_SAMPLER_DESC(CD3DX12_DEFAULT)
	{
		Filter = D3D12_FILTER_ANISOTROPIC;
		AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		MipLODBias = 0;
		MaxAnisotropy = 16;
		ComparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
		BorderColor[0] = 0.0f;
		BorderColor[1] = 0.0f;
		BorderColor[2] = 0.0f;
		BorderColor[3] = 0.0f;
		MinLOD = -D3D12_FLOAT32_MAX;
		MaxLOD = D3D12_FLOAT32_MAX;
	}
};

static bool operator==(const D3D12_STATIC_SAMPLER_DESC &l, const D3D12_STATIC_SAMPLER_DESC &r)
{
	return l.Filter == r.Filter &&
		l.AddressU == r.AddressU &&
		l.AddressV == r.AddressV &&
		l.AddressW == r.AddressW && 
		m3d::flteql(l.MipLODBias, r.MipLODBias) && 
		l.MaxAnisotropy == r.MaxAnisotropy &&
		l.ComparisonFunc == r.ComparisonFunc &&
		l.BorderColor == r.BorderColor &&
		m3d::flteql(l.MinLOD, r.MinLOD) &&
		m3d::flteql(l.MaxLOD, r.MaxLOD) &&
		l.ShaderRegister == r.ShaderRegister &&
		l.RegisterSpace == r.RegisterSpace &&
		l.ShaderVisibility == r.ShaderVisibility;
}
static bool operator!=(const D3D12_STATIC_SAMPLER_DESC &l, const D3D12_STATIC_SAMPLER_DESC &r) { return !(l == r); }

static bool operator==(const D3D12_CONSTANT_BUFFER_VIEW_DESC &l, const D3D12_CONSTANT_BUFFER_VIEW_DESC &r)
{
	return l.BufferLocation == r.BufferLocation && l.SizeInBytes == r.SizeInBytes;
}
static bool operator!=(const D3D12_CONSTANT_BUFFER_VIEW_DESC &l, const D3D12_CONSTANT_BUFFER_VIEW_DESC &r) { return !(l == r); }

static bool operator==(const D3D12_SHADER_RESOURCE_VIEW_DESC &l, const D3D12_SHADER_RESOURCE_VIEW_DESC &r)
{
	if (!(l.Format == r.Format && l.Shader4ComponentMapping == r.Shader4ComponentMapping && l.ViewDimension == r.ViewDimension))
		return false;
	switch (l.ViewDimension)
	{
	case D3D12_SRV_DIMENSION_BUFFER: return 
		l.Buffer.FirstElement == r.Buffer.FirstElement && 
		l.Buffer.Flags == r.Buffer.Flags && 
		l.Buffer.NumElements == r.Buffer.NumElements && 
		l.Buffer.StructureByteStride == r.Buffer.StructureByteStride;
	case D3D12_SRV_DIMENSION_TEXTURE1D: return 
		l.Texture1D.MostDetailedMip == r.Texture1D.MostDetailedMip && 
		l.Texture1D.MipLevels == r.Texture1D.MipLevels && 
		m3d::flteql(l.Texture1D.ResourceMinLODClamp, r.Texture1D.ResourceMinLODClamp);
	case D3D12_SRV_DIMENSION_TEXTURE1DARRAY: return 
		l.Texture1DArray.MostDetailedMip == r.Texture1DArray.MostDetailedMip && 
		l.Texture1DArray.MipLevels == r.Texture1DArray.MipLevels && 
		l.Texture1DArray.FirstArraySlice == r.Texture1DArray.FirstArraySlice && 
		l.Texture1DArray.ArraySize == r.Texture1DArray.ArraySize && 
		m3d::flteql(l.Texture1DArray.ResourceMinLODClamp, r.Texture1DArray.ResourceMinLODClamp);
	case D3D12_SRV_DIMENSION_TEXTURE2D: return 
		l.Texture2D.MostDetailedMip == r.Texture2D.MostDetailedMip && 
		l.Texture2D.MipLevels == r.Texture2D.MipLevels && 
		l.Texture2D.PlaneSlice == r.Texture2D.PlaneSlice && 
		m3d::flteql(l.Texture2D.ResourceMinLODClamp, r.Texture2D.ResourceMinLODClamp);
	case D3D12_SRV_DIMENSION_TEXTURE2DARRAY: return  
		l.Texture2DArray.MostDetailedMip == r.Texture2DArray.MostDetailedMip && 
		l.Texture2DArray.MipLevels == r.Texture2DArray.MipLevels && 
		l.Texture2DArray.FirstArraySlice == r.Texture2DArray.FirstArraySlice && 
		l.Texture2DArray.ArraySize == r.Texture2DArray.ArraySize && 
		l.Texture2DArray.PlaneSlice == r.Texture2DArray.PlaneSlice && 
		m3d::flteql(l.Texture2DArray.ResourceMinLODClamp, r.Texture2DArray.ResourceMinLODClamp);
	case D3D12_SRV_DIMENSION_TEXTURE2DMS: return true;
	case D3D12_SRV_DIMENSION_TEXTURE2DMSARRAY: return 
		l.Texture2DMSArray.FirstArraySlice == r.Texture2DMSArray.FirstArraySlice && 
		l.Texture2DMSArray.ArraySize == r.Texture2DMSArray.ArraySize;
	case D3D12_SRV_DIMENSION_TEXTURE3D: return 
		l.Texture3D.MostDetailedMip == r.Texture3D.MostDetailedMip && 
		l.Texture3D.MipLevels == r.Texture3D.MipLevels && 
		m3d::flteql(l.Texture3D.ResourceMinLODClamp, r.Texture3D.ResourceMinLODClamp);
	case D3D12_SRV_DIMENSION_TEXTURECUBE: return
		l.TextureCube.MostDetailedMip == r.TextureCube.MostDetailedMip && 
		l.TextureCube.MipLevels == r.TextureCube.MipLevels && 
		m3d::flteql(l.TextureCube.ResourceMinLODClamp, r.TextureCube.ResourceMinLODClamp);
	case D3D12_SRV_DIMENSION_TEXTURECUBEARRAY: return 
		l.TextureCubeArray.MostDetailedMip == r.TextureCubeArray.MostDetailedMip && 
		l.TextureCubeArray.MipLevels == r.TextureCubeArray.MipLevels &&
		l.TextureCubeArray.First2DArrayFace == r.TextureCubeArray.First2DArrayFace && 
		l.TextureCubeArray.NumCubes == r.TextureCubeArray.NumCubes &&
		m3d::flteql(l.TextureCubeArray.ResourceMinLODClamp, r.TextureCubeArray.ResourceMinLODClamp);
	}
	return true;
}
static bool operator!=(const D3D12_SHADER_RESOURCE_VIEW_DESC &l, const D3D12_SHADER_RESOURCE_VIEW_DESC &r) { return !(l == r); }


static bool operator==(const D3D12_UNORDERED_ACCESS_VIEW_DESC &l, const D3D12_UNORDERED_ACCESS_VIEW_DESC &r)
{
	if (!(l.Format == r.Format && l.ViewDimension == r.ViewDimension))
		return false;
	switch (l.ViewDimension)
	{
	case D3D12_UAV_DIMENSION_BUFFER: return
		l.Buffer.CounterOffsetInBytes == r.Buffer.CounterOffsetInBytes &&
		l.Buffer.FirstElement == r.Buffer.FirstElement &&
		l.Buffer.Flags == r.Buffer.Flags &&
		l.Buffer.NumElements == r.Buffer.NumElements &&
		l.Buffer.StructureByteStride == r.Buffer.StructureByteStride;
	case D3D12_UAV_DIMENSION_TEXTURE1D: return
		l.Texture1D.MipSlice == r.Texture1D.MipSlice;
	case D3D12_UAV_DIMENSION_TEXTURE1DARRAY: return
		l.Texture1DArray.MipSlice == r.Texture1DArray.MipSlice &&
		l.Texture1DArray.FirstArraySlice == r.Texture1DArray.FirstArraySlice &&
		l.Texture1DArray.ArraySize == r.Texture1DArray.ArraySize;
	case D3D12_UAV_DIMENSION_TEXTURE2D: return
		l.Texture2D.MipSlice == r.Texture2D.MipSlice &&
		l.Texture2D.PlaneSlice == r.Texture2D.PlaneSlice;
	case D3D12_UAV_DIMENSION_TEXTURE2DARRAY: return
		l.Texture2DArray.MipSlice == r.Texture2DArray.MipSlice &&
		l.Texture2DArray.FirstArraySlice == r.Texture2DArray.FirstArraySlice &&
		l.Texture2DArray.ArraySize == r.Texture2DArray.ArraySize &&
		l.Texture2DArray.PlaneSlice == r.Texture2DArray.PlaneSlice;
	case D3D12_UAV_DIMENSION_TEXTURE3D: return
		l.Texture3D.FirstWSlice == r.Texture3D.FirstWSlice &&
		l.Texture3D.MipSlice == r.Texture3D.MipSlice &&
		l.Texture3D.WSize == r.Texture3D.WSize;
	}
	return true;
}
static bool operator!=(const D3D12_UNORDERED_ACCESS_VIEW_DESC &l, const D3D12_UNORDERED_ACCESS_VIEW_DESC &r) { return !(l == r); }



static bool operator==(const D3D12_SAMPLER_DESC &l, const D3D12_SAMPLER_DESC &r)
{
	return	l.Filter == r.Filter && 
		l.AddressU == r.AddressU && 
		l.AddressV == r.AddressV && 
		l.AddressW == r.AddressW && 
		l.MipLODBias == r.MipLODBias && 
		l.MaxAnisotropy == r.MaxAnisotropy && 
		l.ComparisonFunc == r.ComparisonFunc && 
		m3d::flteql(l.BorderColor[0], r.BorderColor[0]) && 
		m3d::flteql(l.BorderColor[1], r.BorderColor[1]) && 
		m3d::flteql(l.BorderColor[2], r.BorderColor[2]) && 
		m3d::flteql(l.BorderColor[3], r.BorderColor[3]) && 
		m3d::flteql(l.MinLOD, r.MinLOD) && 
		m3d::flteql(l.MaxLOD, r.MaxLOD);
}
static bool operator!=(const D3D12_SAMPLER_DESC &l, const D3D12_SAMPLER_DESC &r) { return !(l == r); }


struct SubresourceRange
{
	UINT MostDetailedMip;
	UINT MipLevels;
	UINT FirstArraySlice;
	UINT ArraySize;
	UINT PlaneSlice;
	UINT PlaneSliceCount; // Can be -1 if unknown/all slices (normally just 1 slice, but it depends on the format)
};

static SubresourceRange GetSubresourceRange(const D3D12_SHADER_RESOURCE_VIEW_DESC *srv)
{
	switch (srv->ViewDimension)
	{
	case D3D12_SRV_DIMENSION_BUFFER:
		return { 0, 1, 0, 1, 0, 0xffffffff };
	case D3D12_SRV_DIMENSION_TEXTURE1D:
		return { srv->Texture1D.MostDetailedMip, srv->Texture1D.MipLevels, 0, 1, 0, 0xffffffff };
	case D3D12_SRV_DIMENSION_TEXTURE1DARRAY:
		return { srv->Texture1DArray.MostDetailedMip, srv->Texture1DArray.MipLevels, srv->Texture1DArray.FirstArraySlice, srv->Texture1DArray.ArraySize, 0, 0xffffffff };
	case D3D12_SRV_DIMENSION_TEXTURE2D: 
		return { srv->Texture2D.MostDetailedMip, srv->Texture2D.MipLevels, 0, 1, srv->Texture2D.PlaneSlice, 1 };
	case D3D12_SRV_DIMENSION_TEXTURE2DARRAY: 
		return { srv->Texture2DArray.MostDetailedMip, srv->Texture2DArray.MipLevels, srv->Texture2DArray.FirstArraySlice, srv->Texture2DArray.ArraySize, srv->Texture2DArray.PlaneSlice, 1 };
	case D3D12_SRV_DIMENSION_TEXTURE2DMS:
		return { 0, 1, 0, 1, 0, 0xffffffff };
	case D3D12_SRV_DIMENSION_TEXTURE2DMSARRAY:
		return { 0, 1, srv->Texture2DMSArray.FirstArraySlice, srv->Texture2DMSArray.ArraySize, 0, 0xffffffff };
	case D3D12_SRV_DIMENSION_TEXTURE3D:
		return { srv->Texture3D.MostDetailedMip, srv->Texture3D.MipLevels, 0, 1, 0, 0xffffffff };
	case D3D12_SRV_DIMENSION_TEXTURECUBE:
		return { srv->TextureCube.MostDetailedMip, srv->TextureCube.MipLevels, 0, 6, 0, 0xffffffff };
	case D3D12_SRV_DIMENSION_TEXTURECUBEARRAY:
		return { srv->TextureCubeArray.MostDetailedMip, srv->TextureCubeArray.MipLevels, srv->TextureCubeArray.First2DArrayFace, srv->TextureCubeArray.NumCubes * 6, 0, 0xffffffff };
	}
	return { 0, 1, 0, 1, 0, 0xffffffff };
}

static SubresourceRange GetSubresourceRange(const D3D12_RENDER_TARGET_VIEW_DESC *rtv)
{
	switch (rtv->ViewDimension)
	{
	case D3D12_RTV_DIMENSION_BUFFER:
		return { 0, 1, 0, 1, 0, 0xffffffff };
	case D3D12_RTV_DIMENSION_TEXTURE1D:
		return { rtv->Texture1D.MipSlice, 1, 0, 1, 0, 0xffffffff };
	case D3D12_RTV_DIMENSION_TEXTURE1DARRAY:
		return { rtv->Texture1DArray.MipSlice, 1, rtv->Texture1DArray.FirstArraySlice, rtv->Texture1DArray.ArraySize, 0, 0xffffffff };
	case D3D12_RTV_DIMENSION_TEXTURE2D: 
		return { rtv->Texture2D.MipSlice, 1, 0, 1, rtv->Texture2D.PlaneSlice, 1 };
	case D3D12_RTV_DIMENSION_TEXTURE2DARRAY: 
		return { rtv->Texture2DArray.MipSlice, 1, rtv->Texture2DArray.FirstArraySlice, rtv->Texture2DArray.ArraySize, rtv->Texture2DArray.PlaneSlice, 1 };
	case D3D12_RTV_DIMENSION_TEXTURE2DMS:
		return { 0, 1, 0, 1, 0, 0xffffffff };
	case D3D12_RTV_DIMENSION_TEXTURE2DMSARRAY:
		return { 0, 1, rtv->Texture2DMSArray.FirstArraySlice, rtv->Texture2DMSArray.ArraySize, 0, 0xffffffff };
	case D3D12_RTV_DIMENSION_TEXTURE3D:
		return { rtv->Texture3D.MipSlice, 1, 0, 1, 0, 0xffffffff };
	}
	return { 0, 1, 0, 1, 0, 0xffffffff };
}

static SubresourceRange GetSubresourceRange(const D3D12_DEPTH_STENCIL_VIEW_DESC *dsv)
{
	switch (dsv->ViewDimension)
	{
	case D3D12_DSV_DIMENSION_TEXTURE1D:
		return { dsv->Texture1D.MipSlice, 1, 0, 1, 0, 0xffffffff };
	case D3D12_DSV_DIMENSION_TEXTURE1DARRAY:
		return { dsv->Texture1DArray.MipSlice, 1, dsv->Texture1DArray.FirstArraySlice, dsv->Texture1DArray.ArraySize, 0, 0xffffffff };
	case D3D12_DSV_DIMENSION_TEXTURE2D: 
		return { dsv->Texture2D.MipSlice, 1, 0, 1, 0, 0xffffffff };
	case D3D12_DSV_DIMENSION_TEXTURE2DARRAY: 
		return { dsv->Texture2DArray.MipSlice, 1, dsv->Texture2DArray.FirstArraySlice, dsv->Texture2DArray.ArraySize, 0, 0xffffffff };
	case D3D12_DSV_DIMENSION_TEXTURE2DMS:
		return { 0, 1, 0, 1, 0, 0xffffffff };
	case D3D12_DSV_DIMENSION_TEXTURE2DMSARRAY:
		return { 0, 1, dsv->Texture2DMSArray.FirstArraySlice, dsv->Texture2DMSArray.ArraySize, 0, 0xffffffff };
	}
	return { 0, 1, 0, 1, 0, 0xffffffff };
}

static SubresourceRange GetSubresourceRange(const D3D12_UNORDERED_ACCESS_VIEW_DESC *uav)
{
	switch (uav->ViewDimension)
	{
	case D3D12_UAV_DIMENSION_BUFFER:
		return { 0, 1, 0, 1, 0, 0xffffffff };
	case D3D12_UAV_DIMENSION_TEXTURE1D:
		return { uav->Texture1D.MipSlice, 1, 0, 1, 0, 0xffffffff };
	case D3D12_UAV_DIMENSION_TEXTURE1DARRAY:
		return { uav->Texture1DArray.MipSlice, 1, uav->Texture1DArray.FirstArraySlice, uav->Texture1DArray.ArraySize, 0, 0xffffffff };
	case D3D12_UAV_DIMENSION_TEXTURE2D:
		return { uav->Texture2D.MipSlice, 1, 0, 1, uav->Texture2D.PlaneSlice, 1 };
	case D3D12_UAV_DIMENSION_TEXTURE2DARRAY:
		return { uav->Texture2DArray.MipSlice, 1, uav->Texture2DArray.FirstArraySlice, uav->Texture2DArray.ArraySize, uav->Texture2DArray.PlaneSlice, 1 };
	case D3D12_UAV_DIMENSION_TEXTURE3D:
		return { uav->Texture3D.MipSlice, 1, 0, 1, 0, 0xffffffff };
	}
	return { 0, 1, 0, 1, 0, 0xffffffff };
}


constexpr bool operator==(const D3D12_RENDER_TARGET_BLEND_DESC& l, const D3D12_RENDER_TARGET_BLEND_DESC& r)
{
	if (l.BlendEnable != r.BlendEnable)
		return false;
	if (l.BlendEnable)
		return std::make_tuple(l.SrcBlend, l.DestBlend, l.BlendOp, l.SrcBlendAlpha, l.DestBlendAlpha, l.BlendOpAlpha, l.RenderTargetWriteMask) ==
		std::make_tuple(r.SrcBlend, r.DestBlend, r.BlendOp, r.SrcBlendAlpha, r.DestBlendAlpha, r.BlendOpAlpha, r.RenderTargetWriteMask);
	if (l.LogicOpEnable != r.LogicOpEnable)
		return false;
	if (l.LogicOpEnable)
		return std::make_tuple(l.LogicOp, l.RenderTargetWriteMask) ==
		std::make_tuple(r.LogicOp, r.RenderTargetWriteMask);
	return l.RenderTargetWriteMask == r.RenderTargetWriteMask;
}

constexpr bool operator==(const D3D12_BLEND_DESC& l, const D3D12_BLEND_DESC& r)
{
	if (l.IndependentBlendEnable != r.IndependentBlendEnable)
		return false;
	if (l.IndependentBlendEnable)
		return std::make_tuple(l.AlphaToCoverageEnable, (const m3d::Array<D3D12_RENDER_TARGET_BLEND_DESC, 8>&)l.RenderTarget) ==
		std::make_tuple(r.AlphaToCoverageEnable, (const m3d::Array<D3D12_RENDER_TARGET_BLEND_DESC, 8>&)r.RenderTarget);
	return std::make_tuple(l.AlphaToCoverageEnable, l.RenderTarget[0]) ==
		std::make_tuple(r.AlphaToCoverageEnable, r.RenderTarget[0]);
}

constexpr bool operator==(const D3D12_RASTERIZER_DESC& l, const D3D12_RASTERIZER_DESC& r)
{
	return  std::make_tuple(l.FillMode, l.CullMode, l.FrontCounterClockwise, l.DepthBias, l.DepthBiasClamp, l.SlopeScaledDepthBias, l.DepthClipEnable, l.MultisampleEnable, l.AntialiasedLineEnable, l.ForcedSampleCount, l.ConservativeRaster) ==
		std::make_tuple(r.FillMode, r.CullMode, r.FrontCounterClockwise, r.DepthBias, r.DepthBiasClamp, r.SlopeScaledDepthBias, r.DepthClipEnable, r.MultisampleEnable, r.AntialiasedLineEnable, r.ForcedSampleCount, r.ConservativeRaster);
}

constexpr bool operator==(const D3D12_DEPTH_STENCILOP_DESC& l, const D3D12_DEPTH_STENCILOP_DESC& r)
{
	return  std::make_tuple(l.StencilFailOp, l.StencilDepthFailOp, l.StencilPassOp, l.StencilFunc) ==
		std::make_tuple(r.StencilFailOp, r.StencilDepthFailOp, r.StencilPassOp, r.StencilFunc);
}

constexpr bool operator==(const D3D12_DEPTH_STENCIL_DESC& l, const D3D12_DEPTH_STENCIL_DESC& r)
{
	return  std::make_tuple(l.DepthEnable, l.DepthWriteMask, l.DepthFunc, l.StencilEnable, l.StencilReadMask, l.StencilWriteMask, l.FrontFace, l.BackFace) ==
		std::make_tuple(r.DepthEnable, r.DepthWriteMask, r.DepthFunc, r.StencilEnable, r.StencilReadMask, r.StencilWriteMask, r.FrontFace, r.BackFace);
}


namespace m3d
{
	template<>
	struct Hash<D3D12_RENDER_TARGET_BLEND_DESC>
	{
		std::size_t operator()(D3D12_RENDER_TARGET_BLEND_DESC const& s) const noexcept
		{
			if (s.BlendEnable)
				return m3d::hash_combine_simple(s.SrcBlend, s.DestBlend, s.BlendOp, s.SrcBlendAlpha, s.DestBlendAlpha, s.BlendOpAlpha, s.RenderTargetWriteMask);
			else if (s.LogicOpEnable)
				return m3d::hash_combine_simple(s.LogicOp, s.RenderTargetWriteMask);
			return m3d::hash_combine_simple(s.RenderTargetWriteMask);
		}
	};

	template<>
	struct Hash<D3D12_BLEND_DESC>
	{
		std::size_t operator()(D3D12_BLEND_DESC const& s) const noexcept
		{
			if (s.IndependentBlendEnable)
				return m3d::hash_combine_simple<m3d::Hash>(s.AlphaToCoverageEnable, (const Array<D3D12_RENDER_TARGET_BLEND_DESC, 8>&)s.RenderTarget);
			else
				return m3d::hash_combine_simple<m3d::Hash>(s.AlphaToCoverageEnable, s.RenderTarget[0]);
		}
	};

	template<>
	struct Hash<D3D12_RASTERIZER_DESC>
	{
		std::size_t operator()(D3D12_RASTERIZER_DESC const& s) const noexcept
		{
			return m3d::hash_combine_simple(s.FillMode, s.CullMode, s.FrontCounterClockwise, s.DepthBias, s.DepthBiasClamp, s.SlopeScaledDepthBias, s.DepthClipEnable, s.MultisampleEnable, s.AntialiasedLineEnable, s.ForcedSampleCount, s.ConservativeRaster);
		}
	};

	template<>
	struct Hash<D3D12_DEPTH_STENCILOP_DESC>
	{
		std::size_t operator()(D3D12_DEPTH_STENCILOP_DESC const& s) const noexcept
		{
			return m3d::hash_combine_simple(s.StencilFailOp, s.StencilDepthFailOp, s.StencilPassOp, s.StencilFunc);
		}
	};



	template<>
	struct Hash<D3D12_DEPTH_STENCIL_DESC>
	{
		std::size_t operator()(D3D12_DEPTH_STENCIL_DESC const& s) const noexcept
		{
			return m3d::hash_combine_simple<m3d::Hash>(s.DepthEnable, s.DepthWriteMask, s.DepthFunc, s.StencilEnable, s.StencilReadMask, s.StencilWriteMask, s.FrontFace, s.BackFace);
		}
	};


}