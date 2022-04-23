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
#include "StandardRootSignature.h"
#include "RenderSettings.h"
#include "ResourceStateTracker.h"
#include "D3D12Formats.h"

using namespace m3d;

#define MATERIAL_CBV_SRV_UAV_INDEX 0u
#define MATERIAL_SAMPLER_INDEX 1u
#define STD2_INDEX 2u
#define STD1_INDEX 3u

const UINT StandardRootSignatureLayout::MaxStaticSamplers = 128u;
const RegisterAndSpace StandardRootSignatureLayout::DefaultSamplerRegister = RegisterAndSpace(1u, 0u);
const RegisterAndSpace StandardRootSignatureLayout::StaticSamplerRegister = RegisterAndSpace(10u, 0u);
const RegisterAndSpace StandardRootSignatureLayout::Std1CBVRegister = RegisterAndSpace(0u, 0u);
const RegisterAndSpace StandardRootSignatureLayout::Std2CBVRegister = RegisterAndSpace(1u, 0u);
const RegisterAndSpace StandardRootSignatureLayout::MaterialCBVRegister = RegisterAndSpace(2u, 0u);
const RegisterAndSpace StandardRootSignatureLayout::MaterialSRVRegister = RegisterAndSpace(0u, 0u);
const RegisterAndSpace StandardRootSignatureLayout::MaterialUAVRegister = RegisterAndSpace(0u, 0u);
const RegisterAndSpace StandardRootSignatureLayout::MaterialSamplerRegister = RegisterAndSpace(1u, 1u);


StandardRootSignatureLayout::StandardRootSignatureLayout(D3D12_RESOURCE_BINDING_TIER ResourceBindingTier, UINT MaterialCBVCount, UINT MaterialSRVCount, UINT MaterialUAVCount, UINT MaterialSamplerCount, UINT LocalResourcesIdx, UINT LocalSamplerIdx, UINT Std1Idx, UINT Std2Idx) :
	ResourceBindingTier(ResourceBindingTier), MaterialCBVCount(MaterialCBVCount), MaterialSRVCount(MaterialSRVCount), MaterialUAVCount(MaterialUAVCount), MaterialSamplerCount(MaterialSamplerCount),
	LocalResourcesIdx(LocalResourcesIdx), LocalSamplerIdx(LocalSamplerIdx), Std1Idx(Std1Idx), Std2Idx(Std2Idx)
{
}

StandardRootSignatureLayout::~StandardRootSignatureLayout()
{
}

RootSignatureDesc StandardRootSignatureLayout::CreateRootSignatureDesc(D3D12_RESOURCE_BINDING_TIER ResourceBindingTier, UINT MaterialCBVCount, UINT MaterialSRVCount, UINT MaterialUAVCount, UINT MaterialSamplerCount, const List<D3D12_STATIC_SAMPLER_DESC> &staticSamplers)
{
	RootSignatureDesc rsDesc;

	CD3DX12_ROOT_PARAMETER rootParameters[4];
	UINT rootIdx = 0;

	UINT LocalResourcesIdx = 0;
	UINT LocalSamplerIdx = 0;
	UINT Std1Idx = 0;
	UINT Std2Idx = 0;

	CD3DX12_DESCRIPTOR_RANGE ranges0[3];
	CD3DX12_DESCRIPTOR_RANGE ranges1[1];

	if (MaterialCBVCount + MaterialSRVCount + MaterialUAVCount > 0) {
		UINT ranges0Count = 0;
		if (MaterialCBVCount > 0)
			ranges0[ranges0Count++].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, MaterialCBVCount, MaterialCBVRegister.Register, MaterialCBVRegister.Space);
		if (MaterialSRVCount > 0)
			ranges0[ranges0Count++].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, MaterialSRVCount, MaterialSRVRegister.Register, MaterialSRVRegister.Space);
		if (MaterialUAVCount > 0)
			ranges0[ranges0Count++].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, MaterialUAVCount, MaterialUAVRegister.Register, MaterialUAVRegister.Space);

		LocalResourcesIdx = rootIdx++;
		rootParameters[LocalResourcesIdx].InitAsDescriptorTable(ranges0Count, ranges0, D3D12_SHADER_VISIBILITY_ALL);
	}

	if (MaterialSamplerCount > 0) {
		// Note: Samplers are not allowed in the same descriptor table as SRV/CBV/UAV..
		ranges1[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, MaterialSamplerCount, MaterialSamplerRegister.Register, MaterialSamplerRegister.Space);
		
		LocalSamplerIdx = rootIdx++;
		rootParameters[LocalSamplerIdx].InitAsDescriptorTable(_countof(ranges1), ranges1, D3D12_SHADER_VISIBILITY_ALL);
	}

	Std2Idx = rootIdx++;
	rootParameters[Std2Idx].InitAsConstantBufferView(Std2CBVRegister.Register, Std2CBVRegister.Space);
	Std1Idx = rootIdx++;
	rootParameters[Std1Idx].InitAsConstantBufferView(Std1CBVRegister.Register, Std1CBVRegister.Space);

	List<D3D12_STATIC_SAMPLER_DESC> staticSamplersCombined = staticSamplers;
	// Add default samplers
	{
		CD3DX12_STATIC_SAMPLER_DESC sam1(StandardRootSignatureLayout::DefaultSamplerRegister.Register + 0, D3D12_FILTER_MIN_MAG_MIP_POINT, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP);
		sam1.RegisterSpace = StandardRootSignatureLayout::DefaultSamplerRegister.Space;
		staticSamplersCombined.push_back(sam1);

		CD3DX12_STATIC_SAMPLER_DESC sam2(StandardRootSignatureLayout::DefaultSamplerRegister.Register + 1, D3D12_FILTER_MIN_MAG_MIP_LINEAR, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP);
		sam2.RegisterSpace = StandardRootSignatureLayout::DefaultSamplerRegister.Space;
		staticSamplersCombined.push_back(sam2);

		CD3DX12_STATIC_SAMPLER_DESC sam3(StandardRootSignatureLayout::DefaultSamplerRegister.Register + 2, D3D12_FILTER_ANISOTROPIC, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP);
		sam3.RegisterSpace = StandardRootSignatureLayout::DefaultSamplerRegister.Space;
		staticSamplersCombined.push_back(sam3);

		CD3DX12_STATIC_SAMPLER_DESC sam4(StandardRootSignatureLayout::DefaultSamplerRegister.Register + 3, D3D12_FILTER_MIN_MAG_MIP_POINT);
		sam4.RegisterSpace = StandardRootSignatureLayout::DefaultSamplerRegister.Space;
		staticSamplersCombined.push_back(sam4);

		CD3DX12_STATIC_SAMPLER_DESC sam5(StandardRootSignatureLayout::DefaultSamplerRegister.Register + 4, D3D12_FILTER_MIN_MAG_MIP_LINEAR);
		sam5.RegisterSpace = StandardRootSignatureLayout::DefaultSamplerRegister.Space;
		staticSamplersCombined.push_back(sam5);

		CD3DX12_STATIC_SAMPLER_DESC sam6(StandardRootSignatureLayout::DefaultSamplerRegister.Register + 5);
		sam6.RegisterSpace = StandardRootSignatureLayout::DefaultSamplerRegister.Space;
		staticSamplersCombined.push_back(sam6);
	}


	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Init(rootIdx, rootParameters, (UINT)staticSamplersCombined.size(), staticSamplersCombined.size() ? &staticSamplersCombined.front() : 0, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	rsDesc = rootSignatureDesc;
	rsDesc.StandardLayout.reset(new StandardRootSignatureLayout(ResourceBindingTier, MaterialCBVCount, MaterialSRVCount, MaterialUAVCount, MaterialSamplerCount, LocalResourcesIdx, LocalSamplerIdx, Std1Idx, Std2Idx));

	return rsDesc;
}

StandardRootSignature *StandardRootSignatureLayout::GetStandardGraphicsRootSignature(UINT staticSamplerCount, const ShaderInputBindDescMap &shaderInputMap) const
{
	StandardRootSignature *s = new StandardGraphicsRootSignature(staticSamplerCount, shared_from_this());
	try {
		s->Init(shaderInputMap);
	}
	catch (const GraphicsException &e) {
		delete s;
		throw e;
	}
	return s;
}

StandardRootSignature *StandardRootSignatureLayout::GetStandardComputeRootSignature(UINT staticSamplerCount, const ShaderInputBindDescMap &shaderInputMap) const
{
	StandardRootSignature *s = new StandardComputeRootSignature(staticSamplerCount, shared_from_this());
	try {
		s->Init(shaderInputMap);
	}
	catch (const GraphicsException &e) {
		delete s;
		throw e;
	}
	return s;
}


StandardRootSignature::StandardRootSignature(UINT staticSamplerCount, std::shared_ptr<const StandardRootSignatureLayout> layout) : _staticSamplerCount(staticSamplerCount), _layout(layout)
{
	_stdUsageMask = 0;
	_materialCBVUsed = false;
	_materialSRVUsed = false;
	_materialUAVUsed = false;
	_materialSamplersUsed = false;
}

StandardRootSignature::~StandardRootSignature()
{
}

bool StandardRootSignature::NeedsMaterialResources() const 
{ 
	return _materialCBVUsed || _materialSRVUsed || _materialUAVUsed || _layout->ResourceBindingTier < D3D12_RESOURCE_BINDING_TIER_3; 
}

bool StandardRootSignature::NeedsMaterialSamplers() const 
{ 
	return _materialSamplersUsed || _layout->ResourceBindingTier < D3D12_RESOURCE_BINDING_TIER_2;
}

void StandardRootSignature::Init(const ShaderInputBindDescMap &shaderInputMap)
{
	_shaderInputMap = shaderInputMap;
	_stdUsageMask = 0;
	_materialCBVUsed = false;
	_materialSRVUsed = false;
	_materialUAVUsed = false;
	_materialSamplersUsed = false;

	// std1
	{
		auto n = shaderInputMap.find(ShaderInputBindDescKey(D3D_SIT_CBUFFER, StandardRootSignatureLayout::Std1CBVRegister.Register, StandardRootSignatureLayout::Std1CBVRegister.Space));
		if (n != shaderInputMap.end()) {
			const ShaderInputBindDesc &d = n->second;
			if (d.Size != sizeof(StdCB1))
				throw GraphicsException(MTEXT("Invalid Std1 buffer."));
			for (UINT i = 0; i < STD1_ELEMENT_COUNT; i++)
				if (d.usageMask & (1 << i))
					_stdUsageMask |= STD1_VIEW << i;
		}
	}

	// std2
	{
		auto n = shaderInputMap.find(ShaderInputBindDescKey(D3D_SIT_CBUFFER, StandardRootSignatureLayout::Std2CBVRegister.Register, StandardRootSignatureLayout::Std2CBVRegister.Space));
		if (n != shaderInputMap.end()) {
			const ShaderInputBindDesc &d = n->second;
			if (d.Size != sizeof(StdCB2))
				throw GraphicsException(MTEXT("Invalid Std2 buffer."));
			for (UINT i = 0; i < STD2_ELEMENT_COUNT; i++)
				if (d.usageMask & (1 << i))
					_stdUsageMask |= STD2_WORLD << i;
		}
	}

	for (UINT i = 0; i < _layout->MaterialCBVCount; i++) {
		auto m = _shaderInputMap.find(ShaderInputBindDescKey(ShaderInputBindDescKey::CBV, StandardRootSignatureLayout::MaterialCBVRegister.Register + i, StandardRootSignatureLayout::MaterialCBVRegister.Space));
		if (m != _shaderInputMap.end()) {
			_materialCBVUsed = true;
			break;
		}
	}

	for (UINT i = 0; i < _layout->MaterialSRVCount; i++) {
		auto m = _shaderInputMap.find(ShaderInputBindDescKey(ShaderInputBindDescKey::SRV, StandardRootSignatureLayout::MaterialSRVRegister.Register + i, StandardRootSignatureLayout::MaterialSRVRegister.Space));
		if (m != _shaderInputMap.end()) {
			_materialSRVUsed = true;
			break;
		}
	}

	for (UINT i = 0; i < _layout->MaterialUAVCount; i++) {
		auto m = _shaderInputMap.find(ShaderInputBindDescKey(ShaderInputBindDescKey::UAV, StandardRootSignatureLayout::MaterialUAVRegister.Register + i, StandardRootSignatureLayout::MaterialUAVRegister.Space));
		if (m != _shaderInputMap.end()) {
			_materialUAVUsed = true;
			break;
		}
	}

	for (UINT i = 0; i < _layout->MaterialSamplerCount; i++) {
		auto m = _shaderInputMap.find(ShaderInputBindDescKey(ShaderInputBindDescKey::SAMPLER, StandardRootSignatureLayout::MaterialSamplerRegister.Register + i, StandardRootSignatureLayout::MaterialSamplerRegister.Space));
		if (m != _shaderInputMap.end()) {
			_materialSamplersUsed = true;
			break;
		}
	}
}


void StandardRootSignature::CreateMaterialResources(Chip *msgChip, const CBVVector &cbv, const SRVVector &srv, const UAVVector &uav, DescriptorHeapManager *heapManager, DescriptorTable **dt) const
{
	*dt = nullptr;

	const uint32 N = _layout->MaterialCBVCount + _layout->MaterialSRVCount + _layout->MaterialUAVCount;
	if (N == 0)
		return;

	heapManager->GetDescriptorTable(N, dt);

	if (*dt == nullptr)
		return; // Could not create descriptor table!

	if (cbv.size() > _layout->MaterialCBVCount)
		msgChip->AddMessage(ChipMessage(MTEXT(""), MTEXT("Too many CBV provided. Increase your root signature limit."), WARN));
	if (srv.size() > _layout->MaterialSRVCount)
		msgChip->AddMessage(ChipMessage(MTEXT(""), MTEXT("Too many SRV provided. Increase your root signature limit."), WARN));
	if (uav.size() > _layout->MaterialUAVCount)
		msgChip->AddMessage(ChipMessage(MTEXT(""), MTEXT("Too many UAV provided. Increase your root signature limit."), WARN));

	{
		ShaderInputBindDescMap::const_iterator n = _shaderInputMap.end();

		for (UINT i = 0; i < _layout->MaterialCBVCount; i++) {
			UINT descIdx = i;

			if (n == _shaderInputMap.end() || (StandardRootSignatureLayout::MaterialCBVRegister.Register + i) >= (n->second.BindPoint + n->second.BindCount))
				n = _shaderInputMap.find(ShaderInputBindDescKey(ShaderInputBindDescKey::CBV, StandardRootSignatureLayout::MaterialCBVRegister.Register + i, StandardRootSignatureLayout::MaterialCBVRegister.Space));

			if (n != _shaderInputMap.end())
			{
				bool ok = false;

				if (i < cbv.size()) {
					const CBV& v = cbv[i];

					if (v.resource) {
						Descriptor& d = (*dt)->InitDescriptor(descIdx, DescriptorType::CBV, TRUE);
						d.resource = v.resource;
						*d.cbv = v.desc;
						d.bufferLayoutID = v.bufferLayoutID;
						ok = true;
					}
				}

				if (!ok) {
					Descriptor& d = (*dt)->InitDescriptor(descIdx, DescriptorType::CBV, TRUE);
					*d.cbv = CD3DX12_CONSTANT_BUFFER_VIEW_DESC(0ull, 0u);
				}
			}
			else
			{
				if (_layout->ResourceBindingTier < D3D12_RESOURCE_BINDING_TIER_3)
				{
					Descriptor& d = (*dt)->InitDescriptor(descIdx, DescriptorType::CBV, TRUE);
					*d.cbv = CD3DX12_CONSTANT_BUFFER_VIEW_DESC(0ull, 0u);
				}
			}
		}
	}

	{
		ShaderInputBindDescMap::const_iterator n = _shaderInputMap.end();

		for (UINT i = 0; i < _layout->MaterialSRVCount; i++) {
			UINT descIdx = _layout->MaterialCBVCount + i;

			if (n == _shaderInputMap.end() || (StandardRootSignatureLayout::MaterialSRVRegister.Register + i) >= (n->second.BindPoint + n->second.BindCount))
				n = _shaderInputMap.find(ShaderInputBindDescKey(ShaderInputBindDescKey::SRV, StandardRootSignatureLayout::MaterialSRVRegister.Register + i, StandardRootSignatureLayout::MaterialSRVRegister.Space));

			if (n != _shaderInputMap.end())
			{
				bool ok = false;

				if (i < srv.size()) {
					const SRV& v = srv[i];

					if (v.resource) {
						Descriptor& d = (*dt)->InitDescriptor(descIdx, DescriptorType::SRV, TRUE);
						d.resource = v.resource;
						*d.srv = v.desc;
						d.bufferLayoutID = v.bufferLayoutID;
						ok = true;
					}
				}

				if (!ok) {
					Descriptor& d = (*dt)->InitDescriptor(descIdx, DescriptorType::SRV, TRUE);
					*d.srv = CD3DX12_SHADER_RESOURCE_VIEW_DESC((D3D12_SRV_DIMENSION)n->second.Dimension, DXGI_FORMAT_R8G8B8A8_UNORM);
				}
			}
			else {
				if (_layout->ResourceBindingTier < D3D12_RESOURCE_BINDING_TIER_3) {
					Descriptor& d = (*dt)->InitDescriptor(descIdx, DescriptorType::SRV, TRUE);
					*d.srv = CD3DX12_SHADER_RESOURCE_VIEW_DESC(D3D12_SRV_DIMENSION_TEXTURE2D, DXGI_FORMAT_R8G8B8A8_UNORM);
				}
			}
		}
	}

	for (UINT i = 0; i < _layout->MaterialUAVCount; i++) {
		UINT descIdx = _layout->MaterialCBVCount + _layout->MaterialSRVCount + i;

		bool ok = false;

		if (i < uav.size()) {
			const UAV &v = uav[i];

			if (v.resource) {
				Descriptor &d = (*dt)->InitDescriptor(descIdx, DescriptorType::UAV, TRUE);
				d.resource = v.resource;
				*d.uav = v.desc;
				d.bufferLayoutID = v.bufferLayoutID;
				ok = true;
			}
		}

		if (!ok && _layout->ResourceBindingTier < D3D12_RESOURCE_BINDING_TIER_3) {
			Descriptor &d = (*dt)->InitDescriptor(descIdx, DescriptorType::UAV, TRUE);
			*d.uav = CD3DX12_UNORDERED_ACCESS_VIEW_DESC(D3D12_UAV_DIMENSION_TEXTURE2D, DXGI_FORMAT_R8G8B8A8_UNORM);
		}
	}

	(*dt)->CreateDescriptors();
}

void StandardRootSignature::VerifyMaterialResources(const DescriptorTable *dt) const
{
	if (dt == nullptr && _layout->GetLocalResourceCount() > 0)
		throw GraphicsException(MTEXT("No material descriptor table."), FATAL);

	if (dt == nullptr)
		return; // OK!

	if (dt->count < (_layout->MaterialCBVCount + _layout->MaterialSRVCount + _layout->MaterialUAVCount))
		throw GraphicsException(MTEXT("Descriptor table for material resources is too small."), FATAL);

	for (UINT i = 0; i < _layout->MaterialCBVCount; i++) {
		UINT descIdx = i;

		auto m = _shaderInputMap.find(ShaderInputBindDescKey(ShaderInputBindDescKey::CBV, StandardRootSignatureLayout::MaterialCBVRegister.Register + i, StandardRootSignatureLayout::MaterialCBVRegister.Space)); // Find shaders descriptor of the buffer.
		if (m == _shaderInputMap.end())
			continue; // Resource not used by shaders! OK!

		const Descriptor &d = dt->GetDescriptor(descIdx);

		if (d.type == DescriptorType::NONE)
			throw GraphicsException(strUtils::ConstructString(MTEXT("No resource is provided for material CBV index %1.")).arg(i), FATAL);

		if (d.type != DescriptorType::CBV)
			throw GraphicsException(strUtils::ConstructString(MTEXT("Invalid descriptor type for material CBV index %1.")).arg(i), FATAL);

		String msg;
		if (!BufferLayoutManager::GetInstance().IsSubsetOf(m->second.bufferLayoutID, d.bufferLayoutID, msg))
			throw GraphicsException(strUtils::ConstructString(MTEXT("Buffer layout does not match shader for material CBV %1: %2")).arg(i).arg(msg), FATAL);

		if (d.cbv->SizeInBytes < m->second.Size)
			throw GraphicsException(strUtils::ConstructString(MTEXT("The supplied buffer at material SRV %1 is too small.")).arg(i), FATAL);
	}

	for (UINT i = 0; i < _layout->MaterialSRVCount; i++) {
		UINT descIdx = _layout->MaterialCBVCount + i;

		auto m = _shaderInputMap.find(ShaderInputBindDescKey(ShaderInputBindDescKey::SRV, StandardRootSignatureLayout::MaterialSRVRegister.Register + i, StandardRootSignatureLayout::MaterialSRVRegister.Space)); // Find shaders descriptor of the buffer.
		if (m == _shaderInputMap.end())
			continue; // Resource not used by shaders! OK!

		const Descriptor &d = dt->GetDescriptor(descIdx);

		if (d.type == DescriptorType::NONE)
			throw GraphicsException(strUtils::ConstructString(MTEXT("No resource is provided for material SRV index %1.")).arg(i), FATAL);

		if (d.type != DescriptorType::SRV)
			throw GraphicsException(strUtils::ConstructString(MTEXT("Invalid descriptor type for material SRV index %1.")).arg(i), FATAL);

		if (m->second.Type == D3D_SIT_TBUFFER) {
			if (d.srv->ViewDimension != D3D12_SRV_DIMENSION_BUFFER)
				throw GraphicsException(strUtils::ConstructString(MTEXT("A buffer is expected for material SRV index %1.")).arg(i), FATAL);

			String msg;
			if (!BufferLayoutManager::GetInstance().IsSubsetOf(m->second.bufferLayoutID, d.bufferLayoutID, msg))
				throw GraphicsException(strUtils::ConstructString(MTEXT("Buffer layout does not match shader for material SRV %1: %1")).arg(i).arg(msg), FATAL);

			if (dxgiformat::BitsPerPixel(d.srv->Format) * d.srv->Buffer.NumElements < m->second.Size * 8)
				throw GraphicsException(strUtils::ConstructString(MTEXT("The supplied buffer at material SRV %1 is too small.")).arg(i), FATAL);
		}
		else
			if ((D3D12_SRV_DIMENSION)m->second.Dimension != d.srv->ViewDimension)
				throw GraphicsException(strUtils::ConstructString(MTEXT("View dimension (Texture1D, Texture2D etc) for material SRV %1 does not match shader.")).arg(i), FATAL);

		if (d.srv->ViewDimension == D3D12_SRV_DIMENSION_TEXTURE2DMS || d.srv->ViewDimension == D3D12_SRV_DIMENSION_TEXTURE2DMSARRAY)
			if(m->second.NumSamples != d.resource->GetDesc().SampleDesc.Count)
				throw GraphicsException(strUtils::ConstructString(MTEXT("Sample count for multisampled texture at material SRV %1 does not match shader.")).arg(i), FATAL);
	}

	for (UINT i = 0; i < _layout->MaterialUAVCount; i++) {
		UINT descIdx = _layout->MaterialCBVCount + _layout->MaterialSRVCount + i;

		auto m = _shaderInputMap.find(ShaderInputBindDescKey(ShaderInputBindDescKey::UAV, StandardRootSignatureLayout::MaterialUAVRegister.Register + i, StandardRootSignatureLayout::MaterialUAVRegister.Space)); // Find shaders descriptor of the buffer.
		if (m == _shaderInputMap.end())
			continue; // Resource not used by shaders! OK!

		const Descriptor &d = dt->GetDescriptor(descIdx);

		if (d.type == DescriptorType::NONE)
			throw GraphicsException(strUtils::ConstructString(MTEXT("No resource is provided for material UAV index %1.")).arg(i), FATAL);

		if (d.type != DescriptorType::UAV)
			throw GraphicsException(strUtils::ConstructString(MTEXT("Invalid descriptor type for material UAV index %1.")).arg(i), FATAL);

		if (m->second.Type != D3D_SIT_UAV_RWTYPED)
			throw GraphicsException(MTEXT("Only D3D_SIT_UAV_RWTYPED supported for UAV."), FATAL);

/*			if (d.srv->ViewDimension != D3D12_SRV_DIMENSION_BUFFER)
				throw GraphicsException(MTEXT("Invalid view dimension."), FATAL);

			String msg;
			if (!BufferLayoutManager::GetInstance().IsSubsetOf(m->second.bufferLayoutID, d.bufferLayoutID, msg))
				throw GraphicsException(String(MTEXT("Buffer layout does not match: %1")).arg(msg), FATAL);

			if (dxgiformat::BitsPerPixel(d.srv->Format) * d.srv->Buffer.NumElements < m->second.Size * 8)
				throw GraphicsException(MTEXT("The supplied buffer is too small."), FATAL);*/

		if ((D3D12_SRV_DIMENSION)m->second.Dimension != d.srv->ViewDimension)
			throw GraphicsException(MTEXT("Invalid view dimension."), FATAL);
	}
}

void StandardRootSignature::CreateMaterialSamplers(Chip *msgChip, const List<D3D12_SAMPLER_DESC> &samplers, DescriptorHeapManager *samplerHeapManager, DescriptorTable **samplerDt) const
{
	samplerHeapManager->GetDescriptorTable(_layout->MaterialSamplerCount, samplerDt);

	if (!samplerDt)
		return;

	if (samplers.size() > _layout->MaterialSamplerCount)
		msgChip->AddMessage(ChipMessage(MTEXT(""), MTEXT("Too many samplers provided. Increase your root signature limit."), WARN));

	for (UINT i = 0; i < _layout->MaterialSamplerCount; i++) {
		if (samplers.size() > i) {
			Descriptor &d = (*samplerDt)->InitDescriptor(i, DescriptorType::SAMPLER, TRUE);
			*d.sampler = samplers[i];
		}
		else if (_layout->ResourceBindingTier < D3D12_RESOURCE_BINDING_TIER_2) {
			Descriptor &d = (*samplerDt)->InitDescriptor(i, DescriptorType::SAMPLER, TRUE);
			*d.sampler = CD3DX12_SAMPLER_DESC(CD3DX12_DEFAULT());
		}
	}

	(*samplerDt)->CreateDescriptors();
}

void StandardRootSignature::VerifyMaterialSamplers(const DescriptorTable *dt) const
{
	if (dt == nullptr && _layout->MaterialSamplerCount > 0)
		throw GraphicsException(MTEXT("No sampler descriptor table."), FATAL);

	if (dt == nullptr)
		return; // OK!

	if (dt->count < _layout->MaterialSamplerCount)
		throw GraphicsException(MTEXT("Descriptor table for material samplers is too small."), FATAL);

	for (UINT i = 0; i < _layout->MaterialSamplerCount; i++) {
		UINT descIdx = i;

		auto m = _shaderInputMap.find(ShaderInputBindDescKey(ShaderInputBindDescKey::SAMPLER, StandardRootSignatureLayout::MaterialSamplerRegister.Register + i, StandardRootSignatureLayout::MaterialSamplerRegister.Space)); // Find shaders descriptor.
		if (m == _shaderInputMap.end())
			continue; // Resource not used by shaders! OK!

		const Descriptor &d = dt->GetDescriptor(descIdx);

		if (d.type == DescriptorType::NONE)
			throw GraphicsException(strUtils::ConstructString(MTEXT("No sampler is provided for material Sampler index %1.")).arg(i), FATAL);
	}
}

StandardGraphicsRootSignature::StandardGraphicsRootSignature(UINT staticSamplerCount, std::shared_ptr<const StandardRootSignatureLayout> layout) : StandardRootSignature(staticSamplerCount, layout)
{
}

StandardGraphicsRootSignature::~StandardGraphicsRootSignature()
{
}

void StandardGraphicsRootSignature::SetStd1Buffer(RenderSettings *rs, D3D12_GPU_VIRTUAL_ADDRESS std1)
{
	rs->SetGraphicsRootConstantBufferView(_layout->Std1Idx, std1);
}

void StandardGraphicsRootSignature::SetStd2Buffer(RenderSettings *rs, D3D12_GPU_VIRTUAL_ADDRESS std2)
{
	rs->SetGraphicsRootConstantBufferView(_layout->Std2Idx, std2);
}

void StandardGraphicsRootSignature::SetMaterialResources(RenderSettings *rs, const DescriptorTable *descTable)
{
	rs->SetGraphicsRootDescriptorTable(_layout->LocalResourcesIdx, descTable);
}

void StandardGraphicsRootSignature::SetMaterialSamplers(RenderSettings *rs, const DescriptorTable *descTable)
{
	rs->SetGraphicsRootDescriptorTable(_layout->LocalSamplerIdx, descTable);
}



StandardComputeRootSignature::StandardComputeRootSignature(UINT staticSamplerCount, std::shared_ptr<const StandardRootSignatureLayout> layout) : StandardRootSignature(staticSamplerCount, layout)
{
}

StandardComputeRootSignature::~StandardComputeRootSignature()
{
}

void StandardComputeRootSignature::SetStd1Buffer(RenderSettings *rs, D3D12_GPU_VIRTUAL_ADDRESS std1)
{
	rs->SetComputeRootConstantBufferView(_layout->Std1Idx, std1);
}

void StandardComputeRootSignature::SetStd2Buffer(RenderSettings *rs, D3D12_GPU_VIRTUAL_ADDRESS std2)
{
	rs->SetComputeRootConstantBufferView(_layout->Std2Idx, std2);
}

void StandardComputeRootSignature::SetMaterialResources(RenderSettings *rs, const DescriptorTable *descTable)
{
	rs->SetComputeRootDescriptorTable(_layout->LocalResourcesIdx, descTable);
}

void StandardComputeRootSignature::SetMaterialSamplers(RenderSettings *rs, const DescriptorTable *descTable)
{
	rs->SetComputeRootDescriptorTable(_layout->LocalSamplerIdx, descTable);
}
