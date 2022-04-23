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
#include "RootSignature.h"
#include "GraphicsChips/Sampler.h"
#include "M3DEngine/DocumentSaveLoadUtil.h"
#include "StandardRootSignature.h"

using namespace m3d;


CHIPDESCV1_DEF(RootSignature, MTEXT("Root Signature"), ROOTSIGNATURE_GUID, CHIP_GUID);



RootSignature::RootSignature()
{
	CREATE_CHILD(0, SAMPLER_GUID, true, UP, MTEXT("Static Samplers"));

	_localCBVCount = 4;
	_localSRVCount = 8;
	_localUAVCount = 4;
	_localSamplerCount = 4;

	_rootSignatureID = 0;
}

RootSignature::~RootSignature()
{
}

bool RootSignature::CopyChip(Chip* chip)
{
	RootSignature* c = dynamic_cast<RootSignature*>(chip);
	B_RETURN(Chip::CopyChip(c));
	_localCBVCount = c->_localCBVCount;
	_localSRVCount = c->_localSRVCount;
	_localUAVCount = c->_localUAVCount;
	_localSamplerCount = c->_localSamplerCount;
	return true;
}

bool RootSignature::LoadChip(DocumentLoader& loader)
{
	B_RETURN(Chip::LoadChip(loader));
	LOADDEF("localCBVCount|LocalCBVCount", _localCBVCount, 1);
	LOADDEF("localSRVCount|LocalSRVCount", _localSRVCount, 8);
	LOADDEF("localUAVCount|LocalUAVCount", _localUAVCount, 0);
	LOADDEF("localSamplerCount|LocalSamplerCount", _localSamplerCount, 4);
	return true;
}

bool RootSignature::SaveChip(DocumentSaver& saver) const
{
	B_RETURN(Chip::SaveChip(saver));
	SAVEDEF("localCBVCount", _localCBVCount, 1);
	SAVEDEF("localSRVCount", _localSRVCount, 8);
	SAVEDEF("localUAVCount", _localUAVCount, 0);
	SAVEDEF("localSamplerCount", _localSamplerCount, 4);
	return true;
}

RootSignatureID RootSignature::GetRootSignatureID()
{
	D3D_DEBUG_REPORTER_BLOCK // D3D12SerializeRootSignature is called...

	ChipExceptionScope ces(this); // Callee not a chip throws

	RefreshT refresh(Refresh);
	if (!refresh) {
		if (_rootSignatureID == 0)
			throw ReinitPreventedException(this);
		return _rootSignatureID;
	}

	List<D3D12_STATIC_SAMPLER_DESC> sSamplers;

	for (uint32 i = 0, j = std::min(StandardRootSignatureLayout::MaxStaticSamplers, GetSubConnectionCount(0)); i < j; i++) {
		ChildPtr<Sampler> ch0 = GetChild(0, i);
		D3D12_STATIC_SAMPLER_DESC sSampler;
		if (ch0)
			sSampler = (const D3D12_STATIC_SAMPLER_DESC&)ch0->GetStaticSamplerDesc((UINT)sSamplers.size() + StandardRootSignatureLayout::StaticSamplerRegister.Register, StandardRootSignatureLayout::StaticSamplerRegister.Space); // (Can not be register 0?!)
		else {
			sSampler = CD3DX12_STATIC_SAMPLER_DESC((UINT)sSamplers.size() + StandardRootSignatureLayout::StaticSamplerRegister.Register);
			sSampler.RegisterSpace = StandardRootSignatureLayout::StaticSamplerRegister.Space;
		}
		sSamplers.push_back(sSampler);
	}

	{
		bool equal = false;

		if (_rootSignatureID != 0) {
			const RootSignatureDesc* oldDesc = GetPipelineStatePool()->GetRootSignatureDesc(_rootSignatureID);
			if (oldDesc) {
				if (oldDesc->NumStaticSamplers == sSamplers.size()) {
					if (oldDesc->NumStaticSamplers == 0)
						equal = true;
					else if (oldDesc->pStaticSamplers) {
						equal = true;
						for (UINT i = 0; i < oldDesc->NumStaticSamplers; i++) {
							if (oldDesc->pStaticSamplers[i] != sSamplers[i]) {
								equal = false;
								break;
							}
						}
					}
				}
				if (equal) {
					if (oldDesc->StandardLayout->MaterialCBVCount != GetLocalCBVCount() || oldDesc->StandardLayout->MaterialSRVCount != GetLocalSRVCount() ||
						oldDesc->StandardLayout->MaterialUAVCount != GetLocalUAVCount() || oldDesc->StandardLayout->MaterialSamplerCount != GetLocalSamplerCount())
						equal = false;
				}
			}
		}

		if (!equal) {
			String msg;
			RootSignatureDesc desc = StandardRootSignatureLayout::CreateRootSignatureDesc(graphics()->GetResourceBindingTier(), GetLocalCBVCount(), GetLocalSRVCount(), GetLocalUAVCount(), GetLocalSamplerCount(), sSamplers);
			RootSignatureID rsid = GetPipelineStatePool()->RegisterRootSignatureDesc(desc); // throws
			_rootSignatureID = rsid;
			SetUpdateStamp();
		}
	}
	return _rootSignatureID;
}
