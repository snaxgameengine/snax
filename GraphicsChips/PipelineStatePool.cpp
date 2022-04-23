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
#include "PipelineStatePool.h"
#include "StandardRootSignature.h"
#include "GraphicsChips/GraphicsException.h"

using namespace m3d;


Set<String> InputElementDesc::SemanticNamePool = Set<String>();



PipelineStatePool::PipelineStatePool() : _descCounter(0)
{
	assert(sizeof(PipelineStateID) == sizeof(uint32));
}

PipelineStatePool::~PipelineStatePool()
{
}

void PipelineStatePool::SetDevice(ID3D12Device *device)
{
	_device = device;
}

void PipelineStatePool::OnDestroyDevice()
{
	_device = nullptr;
	_states.clear();
	for (size_t i = 0; i < _RootSignaturesByID.size(); i++)
		const_cast<_RootSignature*>(_RootSignaturesByID[i].get())->signature = nullptr;
}

RootSignatureID PipelineStatePool::RegisterRootSignatureDesc(const RootSignatureDesc &desc, String *msg)
{
	SID3DBlob signature, error;
	HRESULT hr = D3D12SerializeRootSignature(&desc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error);
	if (FAILED(hr)) {
		String msg(MTEXT("D3D12SerializeRootSignature(...) failed."));
		if (error) {
			const Char *str = (const Char*)error->GetBufferPointer();
			if (str)
				msg = String(str, error->GetBufferSize());
		}
		throw GraphicsException(msg, FATAL);
	}
	return _registerRootSignature(signature, desc);
}

RootSignatureID PipelineStatePool::_registerRootSignature(ID3DBlob *signature, const RootSignatureDesc &desc)
{
	std::shared_ptr<_RootSignature> s = std::make_shared<_RootSignature>(signature, RootSignatureID(_RootSignaturesByID.size() + 1), RootSignatureDesc(desc), SID3D12RootSignature(), false);

	auto n = _rootSignatures.insert(s);
	if (n.second)
		_RootSignaturesByID.push_back(s);
	return n.first->get()->id; // If it already exist, the id of the existing one is returned!
}

ID3D12RootSignature *PipelineStatePool::GetRootSignature(RootSignatureID id)
{
	if (!_device)
		throw GraphicsException(MTEXT("No device"));
	if (id == 0 || id > _RootSignaturesByID.size())
		throw GraphicsException(MTEXT("Invalid root signature ID."));// return nullptr;
	const _RootSignature *signature = _RootSignaturesByID[id - 1].get();
	if (signature->failed)
		throw GraphicsException(MTEXT("Could not create root signature."));//return nullptr; // Could not create the root signature!

	if (!signature->signature) {
		_RootSignature *s = const_cast<_RootSignature*>(signature);
		HRESULT hr = _device->CreateRootSignature(0, signature->serialized->GetBufferPointer(), signature->serialized->GetBufferSize(), IID_PPV_ARGS(s->signature.fill(1)));
		if (FAILED(hr)) {
			s->failed = true; // Avoid to make the same mistake again!
			throw GraphicsException(MTEXT("Could not create root signature."));//return nullptr; // Could not create the root signature!
//			return nullptr; // Could not create the root signature!
		}
	}
	return signature->signature;
}

const RootSignatureDesc *PipelineStatePool::GetRootSignatureDesc(RootSignatureID id)
{
	if (id == 0 || id > _RootSignaturesByID.size())
		return nullptr;
	const _RootSignature *signature = _RootSignaturesByID[id - 1].get();
	return &signature->desc;
}

PipelineStateDescID PipelineStatePool::RegisterPipelineStateDesc(const PipelineStateDesc &desc)
{
	std::shared_ptr<_PipelineStateDesc> d = std::make_shared<_PipelineStateDesc>(desc, _descCounter + 1);
	auto n = _descs.insert(d);
	if (n.second) {
		_descsByID.insert(std::make_pair(n.first->get()->id, d));
		_descCounter++;
	}
	else 
		n.first->get()->refCount++; // This state does already exist. Increment reference count!
	return n.first->get()->id;
}

bool PipelineStatePool::UnregisterPipelineStateDesc(PipelineStateDescID id)
{
	auto n = _descsByID.find(id);
	if (n != _descsByID.end()) {
		if (--n->second->refCount == 0) {
			_descs.erase(n->second);
			_descsByID.erase(n);
			auto m = _states.upper_bound(PipelineStateID(id, 0, 0, 0));
			while (m != _states.end() && m->first.psID == id) {
				m = _states.erase(m);
			}
		}
		return true;
	}
	return false;
}

PipelineStateDescID PipelineStatePool::RegisterComputePipelineStateDesc(const ComputePipelineStateDesc &desc)
{
	std::shared_ptr<_ComputePipelineStateDesc> d = std::make_shared<_ComputePipelineStateDesc>(desc, _descCounter + 1);
	auto n = _computeDescs.insert(d);
	if (n.second) {
		_computeDescsByID.insert(std::make_pair(n.first->get()->id, d));
		_descCounter++;
	}
	else
		n.first->get()->refCount++; // This state does already exist. Increment reference count!
	return n.first->get()->id;
}

bool PipelineStatePool::UnregisterComputePipelineStateDesc(PipelineStateDescID id)
{
	return false;
}

PipelineStateDescID PipelineStatePool::RegisterPipelineInputStateDesc(const PipelineInputStateDesc &desc)
{
	auto n = _mapInDescToID.find(desc);
	if (n != _mapInDescToID.end())
		return n->second;
	n = _mapInDescToID.insert(std::make_pair(desc, PipelineStateDescID(_inIDToMapNode.size() + 1))).first;
	_inIDToMapNode.push_back(n);
	return n->second;
}

const PipelineInputStateDesc *PipelineStatePool::GetPipelineInputStateDesc(PipelineStateDescID id) const
{
	if (id == 0 || id > _inIDToMapNode.size())
		return nullptr;
	return &_inIDToMapNode[id - 1]->first;
}

const PipelineStateDesc* PipelineStatePool::GetPipelineStateDesc(PipelineStateDescID id) const
{
	auto n = _descsByID.find(id);
	if (n != _descsByID.end()) {
		return &n->second->desc;
	}
	return nullptr;
}

const ComputePipelineStateDesc* PipelineStatePool::GetComputePipelineStateDesc(PipelineStateDescID id) const
{
	auto n = _computeDescsByID.find(id);
	if (n != _computeDescsByID.end()) {
		return &n->second->desc;
	}
	return nullptr;
}

PipelineStateDescID PipelineStatePool::RegisterPipelineOutputStateDesc(const PipelineOutputStateDesc &desc)
{
	auto n = _mapOutDescToID.find(desc);
	if (n != _mapOutDescToID.end())
		return n->second;
	n = _mapOutDescToID.insert(std::make_pair(desc, PipelineStateDescID(_outIDToMapNode.size() + 1))).first;
	_outIDToMapNode.push_back(n);
	return n->second;
}


const PipelineState &PipelineStatePool::GetState(PipelineStateID id)
{
	//static const PipelineState INVALID;

	auto n = _states.find(id);
	if (n != _states.end()) {
		if (!n->second.IsValid())
			throw GraphicsException(MTEXT("Creating this state failed earlier!"));
		return n->second; // State found!
	}

	auto m = _descsByID.find(id.psID);
	if (m == _descsByID.end())
		throw GraphicsException(MTEXT("Invalid pipeline state ID."));//return INVALID; // Invalid id!

	if (id.pisID == 0 || id.pisID > _inIDToMapNode.size() || id.posID == 0 || id.posID > _outIDToMapNode.size())
		throw GraphicsException(MTEXT("Invalid pipeline state ID."));//return INVALID; // Invalid id!

	const PipelineStateDesc &desc = m->second->desc;
	const PipelineInputStateDesc &descIn = _inIDToMapNode[id.pisID - 1]->first;
	const PipelineOutputStateDesc &descOut = _outIDToMapNode[id.posID - 1]->first;

	SID3D12RootSignature signature = GetRootSignature(id.rsID); // throws!

	SID3D12PipelineState s;

	// We must have a root signature!! (signature should never be nullptr...)
	D3D12_GRAPHICS_PIPELINE_STATE_DESC d { };
	d.pRootSignature =  signature;
	if (desc.VS) d.VS = { desc.VS->GetBufferPointer(), desc.VS->GetBufferSize() };
	if (desc.PS) d.PS = { desc.PS->GetBufferPointer(), desc.PS->GetBufferSize() };
	if (desc.DS) d.DS = { desc.DS->GetBufferPointer(), desc.DS->GetBufferSize() };
	if (desc.HS) d.HS = { desc.HS->GetBufferPointer(), desc.HS->GetBufferSize() };
	if (desc.GS) d.GS = { desc.GS->GetBufferPointer(), desc.GS->GetBufferSize() };
	d.StreamOutput = desc.StreamOutput;
	d.BlendState = desc.BlendState;
	d.SampleMask = desc.SampleMask;
	d.RasterizerState = desc.RasterizerState;
	d.DepthStencilState = desc.DepthStencilState; 
	d.InputLayout = descIn.InputLayout;
	d.IBStripCutValue = descIn.IBStripCutValue;
	d.PrimitiveTopologyType = descIn.PrimitiveTopologyType;
	d.NumRenderTargets = descOut.NumRenderTargets;
	std::memcpy(d.RTVFormats, descOut.RTVFormats, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC::RTVFormats));
	d.DSVFormat = descOut.DSVFormat;
	d.SampleDesc = descOut.SampleDesc;
	d.NodeMask = desc.NodeMask;
	d.CachedPSO = desc.CachedPSO;
	d.Flags = D3D12_PIPELINE_STATE_FLAG_NONE        ;//desc.Flags;

	HRESULT hr = _device->CreateGraphicsPipelineState(&d, IID_PPV_ARGS(s.fill(1)));
	try {
		if (FAILED(hr))
			throw GraphicsException(MTEXT("Failed to create graphics pipeline state!"));

		PipelineState ps;
		
		/* THIS DOES NOT SEEM TO WORK? PSO MUST USE THE SAME RS?
		PipelineStateID tmp1 = id, tmp2 = id;
		tmp1.rsID = 0x00;
		tmp2.rsID = 0xFF;
		for (auto a = _states.findFirstAfter(tmp1), b = _states.findFirstAfter(tmp2); a != b && (a->first.id & 0xFFFFFF00) == (id.id & 0xFFFFFF00); a++) {
			if (a->second.pso != nullptr) {
				s = a->second.pso; // We did already create a PSO with the same settings, but different signature - Just use that old PSO instead, as the signature does not matter! (Signature is just to confirm shader inputs are valid...)
				break;
			}
		}
		*/

		const RootSignatureDesc *rsDesc = GetRootSignatureDesc(id.rsID); // Never nullptr at this point!

		ps.pso = s;
		ps.rootSignature = signature;

		if (rsDesc->StandardLayout)
			ps.standardRootSignature.reset(rsDesc->StandardLayout->GetStandardGraphicsRootSignature(rsDesc->NumStaticSamplers, desc.Pipeline.uniforms)); // Throws! Never nullptr!

		n = _states.insert(std::make_pair(id, ps)).first;
	}
	catch (const GraphicsException &e) {
		n = _states.insert(std::make_pair(id, PipelineState())).first; // Add invalid state! Early throw next time!
		throw e; // Rethrow!
	}
	return n->second; // Return new state. Next time we have an early return from this func! :)
}

const PipelineState &PipelineStatePool::GetComputeState(PipelineStateID id)
{
	auto n = _states.find(id);
	if (n != _states.end()) {
		if (!n->second.IsValid())
			throw GraphicsException(MTEXT("Creating this state failed earlier!"));
		return n->second; // State found!
	}

	auto m = _computeDescsByID.find(id.psID);
	if (m == _computeDescsByID.end())
		throw GraphicsException(MTEXT("Invalid pipeline state ID."));//return INVALID; // Invalid id!

	if (id.pisID != 0 || id.posID != 0)
		throw GraphicsException(MTEXT("Invalid pipeline state ID."));//return INVALID; // Invalid id!

	const ComputePipelineStateDesc &desc = m->second->desc;

	SID3D12RootSignature signature = GetRootSignature(id.rsID); // throws!

	SID3D12PipelineState s;

	// We must have a root signature!! (signature should never be nullptr...)
	D3D12_COMPUTE_PIPELINE_STATE_DESC d{};
	d.pRootSignature = signature;
	if (desc.CS) d.CS = { desc.CS->GetBufferPointer(), desc.CS->GetBufferSize() };
	d.NodeMask = desc.NodeMask;
	d.CachedPSO = desc.CachedPSO;
	d.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;//desc.Flags;

	HRESULT hr = _device->CreateComputePipelineState(&d, IID_PPV_ARGS(s.fill(1)));
	try {
		if (FAILED(hr))
			throw GraphicsException(MTEXT("Failed to create compute pipeline state!"));

		PipelineState ps;

		/* THIS DOES NOT SEEM TO WORK? PSO MUST USE THE SAME RS?
		PipelineStateID tmp1 = id, tmp2 = id;
		tmp1.rsID = 0x00;
		tmp2.rsID = 0xFF;
		for (auto a = _states.findFirstAfter(tmp1), b = _states.findFirstAfter(tmp2); a != b && (a->first.id & 0xFFFFFF00) == (id.id & 0xFFFFFF00); a++) {
		if (a->second.pso != nullptr) {
		s = a->second.pso; // We did already create a PSO with the same settings, but different signature - Just use that old PSO instead, as the signature does not matter! (Signature is just to confirm shader inputs are valid...)
		break;
		}
		}
		*/

		const RootSignatureDesc *rsDesc = GetRootSignatureDesc(id.rsID); // Never nullptr at this point!

		ps.pso = s;
		ps.rootSignature = signature;

		if (rsDesc->StandardLayout)
			ps.standardRootSignature.reset(rsDesc->StandardLayout->GetStandardComputeRootSignature(rsDesc->NumStaticSamplers, desc.uniforms)); // Throws! Never nullptr!

		n = _states.insert(std::make_pair(id, ps)).first;
	}
	catch (const GraphicsException &e) {
		n = _states.insert(std::make_pair(id, PipelineState())).first; // Add invalid state! Early throw next time!
		throw e; // Rethrow!
	}
	return n->second; // Return new state. Next time we have an early return from this func! :)
}