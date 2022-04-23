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
#include "Sampler.h"
#include "M3DEngine/DocumentSaveLoadUtil.h"

using namespace m3d;

CHIPDESCV1_DEF(Sampler, MTEXT("Sampler"), SAMPLER_GUID, CHIP_GUID);

namespace DirectX
{
	extern bool operator!=(const XMFLOAT4& l, const XMFLOAT4& r);
}

namespace m3d
{

bool SerializeDocumentData(DocumentSaver& saver, const M3D_SAMPLER_DESC& data)
{
	SAVEDEF("addressU", data.AddressU, M3D_TEXTURE_ADDRESS_MODE_WRAP);
	SAVEDEF("addressV", data.AddressV, M3D_TEXTURE_ADDRESS_MODE_WRAP);
	SAVEDEF("addressW", data.AddressW, M3D_TEXTURE_ADDRESS_MODE_WRAP);
	SAVEDEF("borderColor", *(const DirectX::XMFLOAT4*)data.BorderColor, DirectX::XMFLOAT4(0, 0, 0, 0));
	SAVEDEF("comparisonFunc", data.ComparisonFunc, M3D_COMPARISON_FUNC_LESS_EQUAL);
	SAVEDEF("filter", data.Filter, M3D_FILTER_ANISOTROPIC);
	SAVEDEF("maxAnisotropy", data.MaxAnisotropy, 16);
	SAVEDEF("maxLOD", data.MaxLOD, std::numeric_limits<float32>::max());
	SAVEDEF("minLOD", data.MinLOD, -std::numeric_limits<float32>::max());
	SAVEDEF("mipLODBias", data.MipLODBias, 0);
	return true;
}

bool DeserializeDocumentData(DocumentLoader& loader, M3D_SAMPLER_DESC& data)
{
	LOADDEF("addressU|AddressU", data.AddressU, M3D_TEXTURE_ADDRESS_MODE_WRAP);
	LOADDEF("addressV|AddressV", data.AddressV, M3D_TEXTURE_ADDRESS_MODE_WRAP);
	LOADDEF("addressW|AddressW", data.AddressW, M3D_TEXTURE_ADDRESS_MODE_WRAP);
	LOADDEF("borderColor|BorderColor", *(DirectX::XMFLOAT4*)data.BorderColor, DirectX::XMFLOAT4(0, 0, 0, 0));
	LOADDEF("comparisonFunc|ComparisonFunc", data.ComparisonFunc, M3D_COMPARISON_FUNC_LESS_EQUAL);
	LOADDEF("filter|Filter", data.Filter, M3D_FILTER_ANISOTROPIC);
	LOADDEF("maxAnisotropy|MaxAnisotropy", data.MaxAnisotropy, 16);
	LOADDEF("maxLOD|MaxLOD", data.MaxLOD, std::numeric_limits<float32>::max());
	LOADDEF("minLOD|MinLOD", data.MinLOD, -std::numeric_limits<float32>::max());
	LOADDEF("mipLODBias|MipLODBias", data.MipLODBias, 0);
	return true;
}

}

Sampler::Sampler()
{
}

Sampler::~Sampler()
{
}

bool Sampler::CopyChip(Chip *chip)
{
	Sampler *c = dynamic_cast<Sampler*>(chip);
	B_RETURN(Chip::CopyChip(c));
	_samplerDesc = c->_samplerDesc;
	SetUpdateStamp();
	return true;
}

bool Sampler::LoadChip(DocumentLoader &loader)
{
	B_RETURN(Chip::LoadChip(loader));
	LOAD("sampler|Sampler", _samplerDesc);
	SetUpdateStamp();
	return true;
}


bool Sampler::SaveChip(DocumentSaver &saver) const
{
	B_RETURN(Chip::SaveChip(saver));
	SAVE("sampler", _samplerDesc);
	return true;
}

M3D_STATIC_SAMPLER_DESC Sampler::GetStaticSamplerDesc(UINT ShaderRegister, UINT RegisterSpace) const
{
	M3D_STATIC_SAMPLER_DESC sSampler;
	sSampler.AddressU = _samplerDesc.AddressU;
	sSampler.AddressV = _samplerDesc.AddressV;
	sSampler.AddressW = _samplerDesc.AddressW;
	sSampler.ComparisonFunc = _samplerDesc.ComparisonFunc;
	sSampler.Filter = _samplerDesc.Filter;
	sSampler.MaxAnisotropy = _samplerDesc.MaxAnisotropy;
	sSampler.MaxLOD= _samplerDesc.MaxLOD;
	sSampler.MinLOD = _samplerDesc.MinLOD;
	sSampler.MipLODBias = _samplerDesc.MipLODBias;
	XMFLOAT4 bc(_samplerDesc.BorderColor);
	if (bc.x == 1.0f && bc.y == 1.0f && bc.z == 1.0f && bc.w == 1.0f)
		sSampler.BorderColor = M3D_STATIC_BORDER_COLOR_OPAQUE_WHITE;
	else if (bc.x == 0.0f && bc.y == 0.0f && bc.z == 0.0f && bc.w == 1.0f)
		sSampler.BorderColor = M3D_STATIC_BORDER_COLOR_OPAQUE_BLACK;
	else {
		if (!(bc.x == 0.0f && bc.y == 0.0f && bc.z == 0.0f && bc.w == 0.0f)) {
			const_cast<Sampler*>(this)->AddMessage(ChipMessage(MTEXT("Invalid border color"), MTEXT("Static sampler's border color can only be white/black opaque or black transparent. Selecting black transparent!"), WARN));
		}
		sSampler.BorderColor = M3D_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
	}
	sSampler.ShaderVisibility = M3D_SHADER_VISIBILITY_ALL;
	sSampler.ShaderRegister = ShaderRegister;
	sSampler.RegisterSpace = RegisterSpace;
	return sSampler;
}

void Sampler::SetSamplerDesc(const M3D_SAMPLER_DESC &desc)
{
	if (std::memcmp(&_samplerDesc, &desc, sizeof(M3D_SAMPLER_DESC)) != 0) {
		_samplerDesc = desc;
		SetUpdateStamp();
	}
}