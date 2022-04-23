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
#include "D3DSaveLoadUtil.h"
//#include "InputElementDescs.h"


using namespace m3d;


bool m3d::SerializeDocumentData(DocumentSaver &saver, const ID3DBlob *data)
{
	ID3DBlob *d = (ID3DBlob*)data;
	
	SAVE("size", (uint32)d->GetBufferSize());
	SAVEARRAY("data", d->GetBufferPointer(), (uint32)d->GetBufferSize());
	return true;
}

bool m3d::DeserializeDocumentData(DocumentLoader &loader, ID3DBlob *&data)
{
	uint32 size = 0;
	LOAD("size", size);
	B_RETURN(SUCCEEDED(D3DCreateBlob(size, &data)));
	LOADARRAY("data", data->GetBufferPointer(), size);
	return true;
}


namespace DirectX
{
	bool operator!=(const XMFLOAT4& l, const XMFLOAT4& r) { return l.x != r.x || l.y != r.y || l.z != r.z || l.w != r.w; }
}

bool m3d::SerializeDocumentData(DocumentSaver &saver, const D3D12_SAMPLER_DESC &data)
{
	SAVEDEF("AddressU", (uint32)data.AddressU, D3D12_TEXTURE_ADDRESS_MODE_WRAP);
	SAVEDEF("AddressV", (uint32)data.AddressV, D3D12_TEXTURE_ADDRESS_MODE_WRAP);
	SAVEDEF("AddressW", (uint32)data.AddressW, D3D12_TEXTURE_ADDRESS_MODE_WRAP);
	SAVEDEF("BorderColor", *(const DirectX::XMFLOAT4*)data.BorderColor, DirectX::XMFLOAT4(0,0,0,0));
	SAVEDEF("ComparisonFunc", (uint32)data.ComparisonFunc, D3D12_COMPARISON_FUNC_LESS_EQUAL);
	SAVEDEF("Filter", (uint32)data.Filter, D3D12_FILTER_ANISOTROPIC);
	SAVEDEF("MaxAnisotropy", data.MaxAnisotropy, 16);
	SAVEDEF("MaxLOD", data.MaxLOD, D3D12_FLOAT32_MAX);
	SAVEDEF("MinLOD", data.MinLOD, -D3D12_FLOAT32_MAX);
	SAVEDEF("MipLODBias", data.MipLODBias, 0);
	return true;
}

bool m3d::DeserializeDocumentData(DocumentLoader &loader, D3D12_SAMPLER_DESC &data)
{
	LOADDEF("AddressU", (uint32&)data.AddressU, D3D12_TEXTURE_ADDRESS_MODE_WRAP);
	LOADDEF("AddressV", (uint32&)data.AddressV, D3D12_TEXTURE_ADDRESS_MODE_WRAP);
	LOADDEF("AddressW", (uint32&)data.AddressW, D3D12_TEXTURE_ADDRESS_MODE_WRAP);
	LOADDEF("BorderColor", *(DirectX::XMFLOAT4*)data.BorderColor, DirectX::XMFLOAT4(0,0,0,0));
	LOADDEF("ComparisonFunc", (uint32&)data.ComparisonFunc, D3D12_COMPARISON_FUNC_LESS_EQUAL);
	LOADDEF("Filter", (uint32&)data.Filter, D3D12_FILTER_ANISOTROPIC);
	LOADDEF("MaxAnisotropy", data.MaxAnisotropy, 16);
	LOADDEF("MaxLOD", data.MaxLOD, D3D12_FLOAT32_MAX);
	LOADDEF("MinLOD", data.MinLOD, -D3D12_FLOAT32_MAX);
	LOADDEF("MipLODBias", data.MipLODBias, 0);
	return true;
}
