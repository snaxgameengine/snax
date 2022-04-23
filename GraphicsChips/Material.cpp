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
#include "Material.h"
#include "GraphicsState.h"
#include "StdChips/VectorChip.h"
#include "StdChips/Value.h"
#include "GraphicsResourceChip.h"
#include "Sampler.h"
#include "GraphicsState.h"
#include "RenderSettings.h"
#include "GraphicsChips/Sampler.h"
#include "StandardRootSignature.h"
#include "GraphicsChips/GraphicsException.h"

using namespace m3d;


CHIPDESCV1_DEF(Material, MTEXT("Material"), MATERIAL_GUID, CHIP_GUID);


// [X] IASetIndexBuffer:		Geometry::Prepare()
// [X] IASetPrimitiveTopology:	Renderable::Render(...)
// [X] IASetVertexBuffers:		Geometry::Prepare()
// [X] OMSetBlendFactor:		Material::Render(...)
// [X] OMSetRenderTargets:		GraphicsCommand::CallChip()
// [X] OMSetStencilRef:			Material::Render(...)
// [X] RSSetScissorRects		GraphicsCommand::CallChip()
// [X] RSSetViewports			GraphicsCommand::CallChip()
// [X] SetDescriptorHeaps		Graphics::_prepareFrame()
// [ ] SetGraphicsRoot*
// [X] SetPipelineState			CommandList::PrepareDraw()
//	X	- Input State			Geometry::Prepare()
//	X	- Main State			Material::Render(...)/GraphicsState::CallChip()
//	X	- Output State			GraphicsCommand::CallChip() (OMSetRenderTargets)
//  X   - Root Signature		Material::Render(...)/GraphicsState::CallChip()
// [ ] SetPredication
// [ ] SOSetTargets


Material::Material()
{
	CREATE_CHILD(0, GRAPHICSSTATE_GUID, false, UP, MTEXT("Graphics State"));
	CREATE_CHILD(1, VECTORCHIP_GUID, false, UP, MTEXT("Blend Factor"));
	CREATE_CHILD(2, VALUE_GUID, false, UP, MTEXT("Stencil Reference Value"));
	CREATE_CHILD(3, GRAPHICSRESOURCECHIP_GUID, true, UP, MTEXT("Constant Buffer View"));
	CREATE_CHILD(4, GRAPHICSRESOURCECHIP_GUID, true, UP, MTEXT("Shader Resource Views"));
	CREATE_CHILD(5, SAMPLER_GUID, true, UP, MTEXT("Samplers"));
}

void Material::UpdateChip()
{
	RefreshT refresh(Refresh);
	if (!refresh) {
		if (!(_psID && _rsID))
			throw ReinitPreventedException(this);
		return;
	}

	_psID = 0;
	_rsID = 0;

	// The pipeline state can be inherited. It does not need to be set to the material!
	PipelineStateDescID psID = rs()->GetPipelineStateID().psID;
	RootSignatureID rsID = rs()->GetPipelineStateID().rsID;

	ChildPtr<GraphicsState> ch0 = GetChild(0);
	if (ch0) {
		// These can throw! Propagate - We can not continue without these states.
		ch0->UpdateChip();
		psID = ch0->GetPipelineStateID();
		rsID = ch0->GetRootSignatureID();
	}

	if (psID == 0)
		throw GraphicsException(this, MTEXT("No pipeline state is set."), FATAL);
	if (rsID == 0)
		throw GraphicsException(this, MTEXT("No root signature is set."), FATAL);

	ChildPtr<VectorChip> ch1 = GetChild(1);
	_blendFactor = ch1 ? ch1->GetVector() : XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	ChildPtr<Value> ch2 = GetChild(2);
	_stencilRef = ch2 ? (UINT)ch2->GetValue() : 0;

	const PipelineStateDesc* psDesc = rs()->GetPipelineStatePool()->GetPipelineStateDesc(psID);
	assert(psDesc != nullptr);

	StandardRootSignature::CBVVector cbvList;
	StandardRootSignature::SRVVector srvList;

	{
		ShaderInputBindDescMap::const_iterator n = psDesc->Pipeline.uniforms.end();

		for (uint32 i = 0, j = GetSubConnectionCount(3); i < j; i++) {
			if (n == psDesc->Pipeline.uniforms.end() || (StandardRootSignatureLayout::MaterialCBVRegister.Register + i) >= (n->second.BindPoint + n->second.BindCount))
				n = psDesc->Pipeline.uniforms.find(ShaderInputBindDescKey(ShaderInputBindDescKey::CBV, StandardRootSignatureLayout::MaterialCBVRegister.Register + i, StandardRootSignatureLayout::MaterialCBVRegister.Space));
			if (n != psDesc->Pipeline.uniforms.end())
			{
				try
				{
					ChildPtr<GraphicsResourceChip> ch3 = GetChild(3, i);
					if (ch3) {
						ch3->UpdateChip(n->second.bufferLayoutID);
						StandardRootSignature::CBV cbv;
						cbv.resource = ch3->GetResource();
						cbv.bufferLayoutID = ch3->GetBufferLayoutID();
						ch3->GetConstantBufferViewDesc(cbv.desc);
						cbvList.push_back(cbv);
					}
					else
						cbvList.push_back(StandardRootSignature::CBV()); // Add an empty CBV in this case!
				}
				catch (const ChipException& exp)
				{
					cbvList.push_back(StandardRootSignature::CBV()); // Add an empty CBV in this case!
					AddException(exp); // Report and continue!
				}
			}
			else
				cbvList.push_back(StandardRootSignature::CBV()); // Add an empty CBV in this case!
		}
	}
	{
		ShaderInputBindDescMap::const_iterator n = psDesc->Pipeline.uniforms.end();

		for (uint32 i = 0, j = GetSubConnectionCount(4); i < j; i++) {
			if (n == psDesc->Pipeline.uniforms.end() || (StandardRootSignatureLayout::MaterialSRVRegister.Register + i) >= (n->second.BindPoint + n->second.BindCount))
				n = psDesc->Pipeline.uniforms.find(ShaderInputBindDescKey(ShaderInputBindDescKey::SRV, StandardRootSignatureLayout::MaterialSRVRegister.Register + i, StandardRootSignatureLayout::MaterialSRVRegister.Space));
			if (n != psDesc->Pipeline.uniforms.end())
			{
				try
				{
					ChildPtr<GraphicsResourceChip> ch4 = GetChild(4, i);
					if (ch4) {
						ch4->UpdateChip(n->second.bufferLayoutID);
						StandardRootSignature::SRV srv;
						srv.resource = ch4->GetResource();
						ch4->GetShaderResourceViewDesc(srv.desc);
						srv.bufferLayoutID = ch4->GetBufferLayoutID();
						srvList.push_back(srv);
					}
					else
						srvList.push_back(StandardRootSignature::SRV()); // Add an empty SRV in this case!
				}
				catch (const ChipException& exp)
				{
					srvList.push_back(StandardRootSignature::SRV()); // Add an empty SRV in this case!
					AddException(exp); // Report and continue!
				}
			}
			else
				srvList.push_back(StandardRootSignature::SRV()); // Add an empty SRV in this case!
		}
	}

	// If resources changed, or the root signature* changed, we must recreate the descriptor table.
	// *The decriptor table must be compatible with the root signature!
	if (cbvList != _cbvList || srvList != _srvList || _rsID != rsID) {
		_cbvList = cbvList;
		_srvList = srvList;
		_resourcesDT = nullptr; // Clear the descriptor table. We create it when rendering!
		memset(_resourceVerification, 0, sizeof(_resourceVerification)); // Clear the verification cache!
	}

	List<D3D12_SAMPLER_DESC> samplers;
	for (uint32 i = 0, j = GetSubConnectionCount(5); i < j; i++) {
		ChildPtr<Sampler> ch5 = GetChild(5, i);
		if (ch5)
			samplers.push_back((const D3D12_SAMPLER_DESC&)ch5->GetSamplerDesc());
		else
			samplers.push_back(CD3DX12_SAMPLER_DESC(CD3DX12_DEFAULT()));
	}

	// Samplers changed, recreate DT. Same if root signature changed.
	if (samplers != _samplers || _rsID != rsID) {
		_samplers = samplers;
		_samplersDT = nullptr; // Clear the descriptor table. We create it when rendering!
		memset(_resourceVerification, 0, sizeof(_resourceVerification)); // Clear the verification cache!
	}

	_psID = psID;
	_rsID = rsID;
}

void Material::SetGraphicsStates()
{
	D3D_DEBUG_REPORTER_BLOCK

		ChipExceptionScope ces(this);

	RenderSettings* rs = this->rs();

	rs->OMSetBlendFactor(_blendFactor);
	rs->OMSetStencilRef(_stencilRef);

	// Set the pipeline state.
	rs->SetPipelineStateDesc(_psID);
	rs->SetGraphicsRootSignature(_rsID);

	const PipelineState& pso = rs->GetPipelineState(); // Throws! Always returns a valid pso!

	if (!pso.IsValid() || pso.standardRootSignature == nullptr)
		throw GraphicsException(this, MTEXT("No valid pipeline state object is set."), FATAL);

	rs->CommitGraphicsRootSignatureAndPipelineState();

	if (pso.standardRootSignature->NeedsMaterialResources()) {
		if (_resourcesDT == nullptr)
			pso.standardRootSignature->CreateMaterialResources(this, _cbvList, _srvList, StandardRootSignature::UAVVector(), graphics()->GetHeapManager(), &_resourcesDT);

		if (_resourcesDT == nullptr)
			throw GraphicsException(this, MTEXT("Failed to create material resource descriptor table."), FATAL); // Could not create the descriptor for some reason...
	}

	if (pso.standardRootSignature->NeedsMaterialSamplers()) {
		if (_samplersDT == nullptr)
			pso.standardRootSignature->CreateMaterialSamplers(this, _samplers, graphics()->GetSamplerHeapManager(), &_samplersDT);

		if (_samplersDT == nullptr)
			throw GraphicsException(this, MTEXT("Failed to create material sampler descriptor."), FATAL); // Could not create the descriptor for some reason...
	}

	// Check if the resources matches the shaders.
	{
		uint32 i = 0;
		const uint32 verifications = _countof(_resourceVerification);
		for (; i < verifications; i++)
			if (_resourceVerification[i] == rs->GetPipelineStateID())
				break;
		if (i == verifications) { // No verification found!
			if (_resourcesDT)
				pso.standardRootSignature->VerifyMaterialResources(_resourcesDT); // throws
			if (_samplersDT)
				pso.standardRootSignature->VerifyMaterialSamplers(_samplersDT); // throws
			i = verifications - 1;
		}

		for (uint32 j = 0; j < i; j++)
			_resourceVerification[i - j] = _resourceVerification[i - j - 1];

		_resourceVerification[0] = rs->GetPipelineStateID();
	}

	// Set the resources! These can throw if resources are locked!
	if (_resourcesDT)
		pso.standardRootSignature->SetMaterialResources(rs, _resourcesDT);
	if (_samplersDT)
		pso.standardRootSignature->SetMaterialSamplers(rs, _samplersDT);
}
