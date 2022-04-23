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


#include "M3DEngine/GlobalDef.h"
#include "M3DCore/ComPtr.h"
#include "M3DCore/MMath.h"
#include <DXGI.h>
#include <DXGI1_2.h> // win8
#include <d3d11.h>


#include <d3d11sdklayers.h>


#define IPOINTERS(x)	typedef m3d::ComPtr<x> S##x; 
//typedef m3d::ComPtr<const x> cS##x;


IPOINTERS(IUnknown)

IPOINTERS(IDXGIObject)
IPOINTERS(IDXGIFactory)
IPOINTERS(IDXGIAdapter)
IPOINTERS(IDXGIDevice)
IPOINTERS(IDXGIFactory1)
IPOINTERS(IDXGIAdapter1)
IPOINTERS(IDXGIDevice1)
IPOINTERS(IDXGIOutput)
IPOINTERS(IDXGISwapChain)

// win8 (DXGI1_2.h)
IPOINTERS(IDXGIFactory2)
IPOINTERS(IDXGIAdapter2)
IPOINTERS(IDXGIOutput1)
IPOINTERS(IDXGISwapChain1)
IPOINTERS(IDXGIDevice2)
	
IPOINTERS(ID3D11Debug)
IPOINTERS(ID3D11InfoQueue)

IPOINTERS(ID3D11DeviceChild)
IPOINTERS(ID3D11Device)
IPOINTERS(ID3D11DeviceContext)

IPOINTERS(ID3D11InputLayout)

IPOINTERS(ID3D11Resource)
IPOINTERS(ID3D11Texture1D)
IPOINTERS(ID3D11Texture2D)
IPOINTERS(ID3D11Texture3D)
IPOINTERS(ID3D11Buffer)

IPOINTERS(ID3D11View)
IPOINTERS(ID3D11RenderTargetView)
IPOINTERS(ID3D11DepthStencilView)
IPOINTERS(ID3D11ShaderResourceView)
IPOINTERS(ID3D11UnorderedAccessView)

IPOINTERS(ID3D11BlendState)
IPOINTERS(ID3D11DepthStencilState)
IPOINTERS(ID3D11RasterizerState)
IPOINTERS(ID3D11SamplerState)

IPOINTERS(ID3D11VertexShader)
IPOINTERS(ID3D11HullShader)
IPOINTERS(ID3D11DomainShader)
IPOINTERS(ID3D11GeometryShader)
IPOINTERS(ID3D11PixelShader)
IPOINTERS(ID3D11ComputeShader)

IPOINTERS(ID3D11Predicate)


