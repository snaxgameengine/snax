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
#include "D3D12Include.h"
#include "Graphics.h"
#include "StandardRootSignature.h"

namespace m3d
{

static const Guid MATERIAL_GUID = { 0xffd3f5e6, 0x9c54, 0x4fbf, { 0xa0, 0x3e, 0x64, 0x2, 0x9, 0xf0, 0x9b, 0xeb } };


class GRAPHICSCHIPS_API Material : public Chip, public GraphicsUsage
{
	CHIPDESC_DECL;
public:
	Material();

	void UpdateChip();
	void SetGraphicsStates();

protected:
	// These are all set in UpdateChip().
	XMFLOAT4 _blendFactor = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	UINT _stencilRef = 0;
	PipelineStateDescID _psID = 0;
	RootSignatureID _rsID = 0;

	// Our constant buffers.
	StandardRootSignature::CBVVector _cbvList;
	// Our texture resources.
	StandardRootSignature::SRVVector _srvList;
	// Our UAVs.
	StandardRootSignature::UAVVector _uavList;
	// The decriptor table with our constant buffer and textuers.
	SDescriptorTable _resourcesDT;
	// Our samplers.
	List<D3D12_SAMPLER_DESC> _samplers;
	// Descriptor table for samplers.
	SDescriptorTable _samplersDT;
	// Caching for verification of shader resources. These PipelineStateIDs are confirmed valid with our descriptor tables.
	PipelineStateID _resourceVerification[4];

};

}