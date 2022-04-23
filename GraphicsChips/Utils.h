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

#include "D3D12Include.h"

namespace m3d
{

static void CalculateMipLevelSize(UINT &width, UINT &height, UINT &depth, UINT mipSlice)
{
	// I've not confirmed that this calculation is correct in every case...
	UINT d = 0x1 << mipSlice;
	width = std::max(1u, width / d);
	height = std::max(1u, height / d);
	depth = std::max(1u, depth / d);
}

static void CalculateMipLevelSize(UINT &width, UINT &height, UINT &depth, UINT mipSlice, ID3D12Resource *res)
{
	D3D12_RESOURCE_DESC desc = res->GetDesc();
	UINT m = 0;
	width = height = depth = 0;

	switch (desc.Dimension) {
	case D3D12_RESOURCE_DIMENSION_BUFFER:
	{
		width = (UINT)desc.Width;
		m = 1;
	}
	break;
	case D3D12_RESOURCE_DIMENSION_TEXTURE1D:
	{
		width = (UINT)desc.Width;
		m = desc.MipLevels;
	}
	break;
	case D3D12_RESOURCE_DIMENSION_TEXTURE2D:
	{
		width = (UINT)desc.Width;
		height = desc.Height;
		m = desc.MipLevels;
	}
	break;
	case D3D12_RESOURCE_DIMENSION_TEXTURE3D:
	{
		width = (UINT)desc.Width;
		height = desc.Height;
		depth = desc.DepthOrArraySize;
		m = desc.MipLevels;
	}
	break;
	default:
		break;
	}
	if (mipSlice >= m)
		return;

	CalculateMipLevelSize(width, height, depth, mipSlice);
}

static void CalculateShaderResourceSize(UINT &width, UINT &height, UINT &depth, UINT mipSlice, ID3D12Resource *resource, D3D12_SHADER_RESOURCE_VIEW_DESC *srv)
{
	if (srv)
	{
		switch (srv->ViewDimension) {
		case D3D12_SRV_DIMENSION_TEXTURE1D:
			mipSlice += srv->Texture1D.MostDetailedMip;
			break;
		case D3D12_SRV_DIMENSION_TEXTURE1DARRAY:
			mipSlice += srv->Texture1DArray.MostDetailedMip;
			break;
		case D3D12_SRV_DIMENSION_TEXTURE2D:
			mipSlice += srv->Texture2D.MostDetailedMip;
			break;
		case D3D12_SRV_DIMENSION_TEXTURE2DARRAY:
			mipSlice += srv->Texture2DArray.MostDetailedMip;
			break;
		case D3D12_SRV_DIMENSION_TEXTURE3D:
			mipSlice += srv->Texture3D.MostDetailedMip;
			break;
		case D3D12_RTV_DIMENSION_BUFFER:
		case D3D12_RTV_DIMENSION_TEXTURE2DMS:
		case D3D12_RTV_DIMENSION_TEXTURE2DMSARRAY:
		default:
			mipSlice = 0;
		}
	}
	return CalculateMipLevelSize(width, height, depth, mipSlice, resource);
}

static void CalculateRenderTargetSize(UINT &width, UINT &height, ID3D12Resource *resource, D3D12_RENDER_TARGET_VIEW_DESC *rtv)
{
	UINT mipSlice = 0;

	if (rtv) // If no descriptor, assume mip level 0!
	{
		switch (rtv->ViewDimension) {
		case D3D12_RTV_DIMENSION_TEXTURE1D:
			mipSlice = rtv->Texture1D.MipSlice;
			break;
		case D3D12_RTV_DIMENSION_TEXTURE1DARRAY:
			mipSlice = rtv->Texture1DArray.MipSlice;
			break;
		case D3D12_RTV_DIMENSION_TEXTURE2D:
			mipSlice = rtv->Texture2D.MipSlice;
			break;
		case D3D12_RTV_DIMENSION_TEXTURE2DARRAY:
			mipSlice = rtv->Texture2DArray.MipSlice;
			break;
		case D3D12_RTV_DIMENSION_TEXTURE3D:
			mipSlice = rtv->Texture3D.MipSlice;
			break;
		case D3D12_RTV_DIMENSION_BUFFER:
		case D3D12_RTV_DIMENSION_TEXTURE2DMS:
		case D3D12_RTV_DIMENSION_TEXTURE2DMSARRAY:
		default:
			mipSlice = 0;
		}
	}
	UINT depth = 0;
	return CalculateMipLevelSize(width, height, depth, mipSlice, resource);
}

static void CalculateDepthStencilSize(UINT &width, UINT &height, ID3D12Resource *resource, D3D12_DEPTH_STENCIL_VIEW_DESC *dsv)
{
	UINT mipSlice = 0;

	if (dsv) // if no descriptor, assume mip level 0!
	{
		switch (dsv->ViewDimension)
		{
		case D3D12_DSV_DIMENSION_TEXTURE1D:
			mipSlice = dsv->Texture1D.MipSlice;
			break;
		case D3D12_DSV_DIMENSION_TEXTURE1DARRAY:
			mipSlice = dsv->Texture1DArray.MipSlice;
			break;
		case D3D12_DSV_DIMENSION_TEXTURE2D:
			mipSlice = dsv->Texture2D.MipSlice;
			break;
		case D3D12_DSV_DIMENSION_TEXTURE2DARRAY:
			mipSlice = dsv->Texture2DArray.MipSlice;
			break;
		case D3D12_DSV_DIMENSION_TEXTURE2DMS:
		case D3D12_DSV_DIMENSION_TEXTURE2DMSARRAY:
		default:
			mipSlice = 0;
		}
	}
	UINT depth = 0;
	return CalculateMipLevelSize(width, height, depth, mipSlice, resource);
}

static void CalculateUnorderedAccessSize(UINT &width, UINT &height, ID3D12Resource *resource, D3D12_UNORDERED_ACCESS_VIEW_DESC *uav)
{
	UINT mipSlice = 0;

	if (uav)
	{
		switch (uav->ViewDimension)
		{
		case D3D12_UAV_DIMENSION_BUFFER:
			mipSlice = 0;
			break;
		case D3D12_UAV_DIMENSION_TEXTURE1D:
			mipSlice = uav->Texture1D.MipSlice;
			break;
		case D3D12_UAV_DIMENSION_TEXTURE1DARRAY:
			mipSlice = uav->Texture1DArray.MipSlice;
			break;
		case D3D12_UAV_DIMENSION_TEXTURE2D:
			mipSlice = uav->Texture2D.MipSlice;
			break;
		case D3D12_UAV_DIMENSION_TEXTURE2DARRAY:
			mipSlice = uav->Texture2DArray.MipSlice;
			break;
		case D3D12_UAV_DIMENSION_TEXTURE3D:
			mipSlice = uav->Texture3D.MipSlice;
		default:
			mipSlice = 0;
		}
	}
	UINT depth = 0;
	return CalculateMipLevelSize(width, height, depth, mipSlice, resource);

}





}