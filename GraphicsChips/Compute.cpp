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
#include "Compute.h"
#include "Shader.h"
#include "StdChips/VectorChip.h"
#include "GraphicsResourceChip.h"
#include "RootSignature.h"
#include "GraphicsChips/Sampler.h"
#include "RenderSettings.h"
#include "StandardRootSignature.h"

using namespace m3d;

CHIPDESCV1_DEF(Compute, MTEXT("Compute"), COMPUTE_GUID, CHIP_GUID);


Compute::Compute()
{
	CREATE_CHILD(0, ROOTSIGNATURE_GUID, false, UP, MTEXT("Root Signature"));
	CREATE_CHILD(1, SHADER_GUID, false, UP, MTEXT("Compute Shader"));
	CREATE_CHILD(2, VECTORCHIP_GUID, false, UP, MTEXT("Target Size (XYZ)"));
	CREATE_CHILD(3, GRAPHICSRESOURCECHIP_GUID, true, UP, MTEXT("Constant Buffer Views"));
	CREATE_CHILD(4, GRAPHICSRESOURCECHIP_GUID, true, UP, MTEXT("Shader Resource Views"));
	CREATE_CHILD(5, GRAPHICSRESOURCECHIP_GUID, true, BOTH, MTEXT("Unordered Access Views"));
	CREATE_CHILD(6, SAMPLER_GUID, true, UP, MTEXT("Samplers"));
}

#define DEVCHECK(x) x

void Compute::UpdateChip()
{
	if (!Refresh)
		return;

	device(); // Do this to force creation of device!

	PipelineStateDescID psID = _psID;
	RootSignatureID rsID = _rsID;

	ChildPtr<RootSignature> ch0 = GetChild(0);
	DEVCHECK(if (!ch0) throw MissingChildException(0));
	rsID = ch0->GetRootSignatureID();
	ID3D12RootSignature *s = graphics()->GetPipelineStatePool()->GetRootSignature(rsID);

	ChildPtr<Shader> ch1 = GetChild(1);
	DEVCHECK(if (!ch1) throw MissingChildException(1));
	if (ch1->GetUpdateStamp() != _csUpdateStamp) {
		_csUpdateStamp = ch1->GetUpdateStamp();
		const ShaderDesc &sd = ch1->GetShader(ShaderType::CS);
		ComputePipelineStateDesc psDesc;
		psDesc.CS = sd.byteCode;
		psDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
		psDesc.uniforms = sd.uniforms;
		psID = graphics()->GetPipelineStatePool()->RegisterComputePipelineStateDesc(psDesc);
		if (_psID != 0 && _psID != psID)
			graphics()->GetPipelineStatePool()->UnregisterComputePipelineStateDesc(psID);
		std::memcpy(_threadGroupSize, sd.ThreadGroupSize, sizeof(_threadGroupSize));
	}

	StandardRootSignature::CBVVector cbvList;
	StandardRootSignature::SRVVector srvList;
	StandardRootSignature::UAVVector uavList;

	const ComputePipelineStateDesc* psDesc = rs()->GetPipelineStatePool()->GetComputePipelineStateDesc(psID);
	assert(psDesc != nullptr);

	for (uint32 i = 0, j = GetSubConnectionCount(3); i < j; i++) {
		try
		{
			BufferLayoutID layoutID = InvalidBufferLayoutID;
			if (psDesc) {
				auto a = psDesc->uniforms.find(ShaderInputBindDescKey(ShaderInputBindDescKey::CBV, StandardRootSignatureLayout::MaterialCBVRegister.Register + i, StandardRootSignatureLayout::MaterialCBVRegister.Space));
				layoutID = a != psDesc->uniforms.end() ? a->second.bufferLayoutID : InvalidBufferLayoutID;
			}
			StandardRootSignature::CBV cbv;
			ChildPtr<GraphicsResourceChip> ch3 = GetChild(3, i);
			if (ch3) {
				ch3->UpdateChip(layoutID);
				cbv.resource = ch3->GetResource();
				ch3->GetConstantBufferViewDesc(cbv.desc);
				cbv.bufferLayoutID = ch3->GetBufferLayoutID();
			}
			cbvList.push_back(cbv);
		}
		catch (const ChipException &exp)
		{
			cbvList.push_back(StandardRootSignature::CBV()); // Add an empty CBV in this case!
			AddException(exp); // Report and continue!
		}
	}

	for (uint32 i = 0, j = GetSubConnectionCount(4); i < j; i++) {
		try
		{
			BufferLayoutID layoutID = InvalidBufferLayoutID;
			if (psDesc) {
				auto a = psDesc->uniforms.find(ShaderInputBindDescKey(ShaderInputBindDescKey::SRV, StandardRootSignatureLayout::MaterialSRVRegister.Register + i, StandardRootSignatureLayout::MaterialSRVRegister.Space));
				layoutID = a != psDesc->uniforms.end() ? a->second.bufferLayoutID : InvalidBufferLayoutID;
			}
			StandardRootSignature::SRV srv;
			ChildPtr<GraphicsResourceChip> ch4 = GetChild(4, i);
			if (ch4) {
				ch4->UpdateChip(layoutID);
				srv.resource = ch4->GetResource();
				ch4->GetShaderResourceViewDesc(srv.desc);
				srv.bufferLayoutID = ch4->GetBufferLayoutID();
			}
			srvList.push_back(srv);
		}
		catch (const ChipException &exp)
		{
			srvList.push_back(StandardRootSignature::SRV()); // Add an empty SRV in this case!
			AddException(exp); // Report and continue!
		}
	}

	for (uint32 i = 0, j = GetSubConnectionCount(5); i < j; i++) {
		try
		{
			BufferLayoutID layoutID = InvalidBufferLayoutID;
			if (psDesc) {
				auto a = psDesc->uniforms.find(ShaderInputBindDescKey(ShaderInputBindDescKey::UAV, StandardRootSignatureLayout::MaterialUAVRegister.Register + i, StandardRootSignatureLayout::MaterialUAVRegister.Space));
				layoutID = a != psDesc->uniforms.end() ? a->second.bufferLayoutID : InvalidBufferLayoutID;
			}
			StandardRootSignature::UAV uav;
			ChildPtr<GraphicsResourceChip> ch5 = GetChild(5, i);
			if (ch5) {
				ch5->UpdateChip(layoutID);
				uav.resource = ch5->GetResource();
				ch5->GetUnorderedAccessViewDesc(uav.desc);
				uav.bufferLayoutID = ch5->GetBufferLayoutID();
			}
			uavList.push_back(uav);
		}
		catch (const ChipException &exp)
		{
			uavList.push_back(StandardRootSignature::UAV()); // Add an empty UAV in this case!
			AddException(exp); // Report and continue!
		}
	}

	if (_cbvList != cbvList || _srvList != srvList || _uavList != uavList || _rsID != rsID) {
		_cbvList = cbvList;
		_srvList = srvList;
		_uavList = uavList;
		_resourcesDT = nullptr;
	}

	List<D3D12_SAMPLER_DESC> samplers;
	for (uint32 i = 0, j = GetSubConnectionCount(6); i < j; i++) {
		ChildPtr<Sampler> ch6 = GetChild(6, i);
		if (ch6)
			samplers.push_back((const D3D12_SAMPLER_DESC&)ch6->GetSamplerDesc());
		else
			samplers.push_back(CD3DX12_SAMPLER_DESC(CD3DX12_DEFAULT()));
	}

	// Samplers changed, recreate DT. Same if root signature changed.
	if (samplers != _samplers || _rsID != rsID) {
		_samplers = samplers;
		_samplersDT = nullptr; // Clear the descriptor table. We create it when rendering!
		memset(_resourceVerification, 0, sizeof(_resourceVerification)); // Clear the verification cache!
	}

	_rsID = rsID;
	_psID = psID;

}

void Compute::CallChip()
{
	try {
		D3D_DEBUG_REPORTER_BLOCK

		ChipExceptionScope ces(this);

		UpdateChip();

		ChildPtr<VectorChip> ch2 = GetChild(2);
		DEVCHECK(if (!ch2) throw MissingChildException(2));
			
		XMFLOAT4 v = ch2->GetVector();
		UINT threadGroupCount[3] = { (UINT)ceil(v.x / _threadGroupSize[0]), (UINT)ceil(v.y / _threadGroupSize[1]), (UINT)ceil(v.z / _threadGroupSize[2]) };
		DEVCHECK(if (threadGroupCount[0] > D3D12_CS_DISPATCH_MAX_THREAD_GROUPS_PER_DIMENSION) throw GraphicsException(MTEXT("Thread group count limit exceeded for X-dimension."), FATAL));
		DEVCHECK(if (threadGroupCount[1] > D3D12_CS_DISPATCH_MAX_THREAD_GROUPS_PER_DIMENSION) throw GraphicsException(MTEXT("Thread group count limit exceeded for Y-dimension."), FATAL));
		DEVCHECK(if (threadGroupCount[2] > UINT(graphics()->GetFeatureLevel() == D3D_FEATURE_LEVEL_10_0 ? 1 : D3D12_CS_DISPATCH_MAX_THREAD_GROUPS_PER_DIMENSION)) throw GraphicsException(MTEXT("Thread group count limit exceeded for Z-dimension."), FATAL));
		DEVCHECK(if (threadGroupCount[0] == 0 || threadGroupCount[1] == 0 || threadGroupCount[2] == 0) throw GraphicsException(MTEXT("Thread group count can not be 0 for any dimension."), FATAL));

		RenderSettings *rs = this->rs();

		rs->SetPipelineStateDesc(_psID);
		rs->SetGraphicsRootSignature(_rsID);

		rs->SetPipelineInputStateDesc(0);
		rs->SetPipelineOutputStateDesc(0);

		const PipelineState &pso = rs->GetComputePipelineState(); // Throws! Always returns a valid pso!

		if (!pso.IsValid() || pso.standardRootSignature == nullptr)
			throw GraphicsException(this, MTEXT("No valid pipeline state object is set."));

		rs->CommitComputeRootSignatureAndPipelineState();

		if (pso.standardRootSignature->NeedsMaterialResources()) {
			if (_resourcesDT == nullptr)
				pso.standardRootSignature->CreateMaterialResources(this, _cbvList, _srvList, _uavList, graphics()->GetHeapManager(), &_resourcesDT);

			if (_resourcesDT == nullptr)
				throw GraphicsException(this, MTEXT("Failed to create material resource descriptor.")); // Could not create the descriptor for some reason...
		}

		if (pso.standardRootSignature->NeedsMaterialSamplers()) {
			if (_samplersDT == nullptr)
				pso.standardRootSignature->CreateMaterialSamplers(this, _samplers, graphics()->GetSamplerHeapManager(), &_samplersDT);

			if (_samplersDT == nullptr)
				throw GraphicsException(this, MTEXT("Failed to create material sampler descriptor.")); // Could not create the descriptor for some reason...
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

		if (_resourcesDT)
			pso.standardRootSignature->SetMaterialResources(rs, _resourcesDT);
		if (_samplersDT)
			pso.standardRootSignature->SetMaterialSamplers(rs, _samplersDT);

		rs->CommitResourceBarriers();
		rs->Dispatch(threadGroupCount[0], threadGroupCount[1], threadGroupCount[2]);
	}
	catch (const ChipException &exp) {
		AddException(exp); // Report and continue!
	}

	// Clear the lock for any bound resources!
	rs()->ClearGraphicsRootDescriptorTables();
}