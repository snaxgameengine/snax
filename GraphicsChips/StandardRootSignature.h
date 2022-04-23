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
#include "D3D12Include.h"
#include "PipelineStatePool.h"
#include "DescriptorHeapManager.h"
#include <vector>


namespace m3d
{

class StandardRootSignature;
class RenderSettings;

struct RegisterAndSpace
{
	UINT Register;
	UINT Space;
	RegisterAndSpace(UINT Register = 0u, UINT Space = 0u) : Register(Register), Space(Space) {}
};

class StandardRootSignatureLayout : public std::enable_shared_from_this<StandardRootSignatureLayout>
{
private:
	StandardRootSignatureLayout(D3D12_RESOURCE_BINDING_TIER ResourceBindingTier, UINT MaterialCBVCount, UINT MaterialSRVCount, UINT MaterialUAVCount, UINT MaterialSamplerCount, UINT LocalResourcesIdx, UINT LocalSamplerIdx, UINT Std1Idx, UINT Std2Idx);
public:
	~StandardRootSignatureLayout();


	// Creates an extended D3D12_ROOT_SIGNATURE_DESC with a back-reference to us.
	static RootSignatureDesc CreateRootSignatureDesc(D3D12_RESOURCE_BINDING_TIER ResourceBindingTier, UINT MaterialCBVCount = 4u, UINT MaterialSRVCount = 8u, UINT MaterialUAVCount = 2u, UINT MaterialSamplerCount = 4u, const List<D3D12_STATIC_SAMPLER_DESC> &staticSamplers = List<D3D12_STATIC_SAMPLER_DESC>());
	// Creates a standard root signature which will act as a binding between the shader inputs and m3d's standard resource inputs.
	virtual StandardRootSignature *GetStandardGraphicsRootSignature(UINT staticSamplerCount, const ShaderInputBindDescMap &shaderInputMap) const;
	virtual StandardRootSignature *GetStandardComputeRootSignature(UINT staticSamplerCount, const ShaderInputBindDescMap &shaderInputMap) const;

	const D3D12_RESOURCE_BINDING_TIER ResourceBindingTier;
	const UINT MaterialCBVCount;
	const UINT MaterialSRVCount;
	const UINT MaterialUAVCount;
	const UINT MaterialSamplerCount;

	UINT LocalResourcesIdx;
	UINT LocalSamplerIdx;
	UINT Std1Idx;
	UINT Std2Idx;
	
	UINT GetLocalResourceCount() const { return MaterialCBVCount + MaterialSRVCount + MaterialUAVCount; }

	// SPACE REGISTER  TYPE
	// 0     b0        Std1 buffer
	// 0     b1        Std2 buffer
	// 0     b2        Material cbv
	// 0     t0-tN     Material srv
	// 0     u0-uN     Material uav
	// 0     s1-s9     Default samplers (Note: Only SM5.1 supports more than 16 samplers (s16->) and spaces.. (eg register(s5, space1))
	// 0     s10-sN    Static samplers (Note: Only SM5.1 supports more than 16 samplers (s16->) and spaces.. (eg register(s5, space1))
	// 1     s1-sN     Material samplers (register s0 does not work!??)
	// 1     b0-bN     Global cbv
	// 1     t0-tN     Global srv
	// 1     u0-uN     Global uav
	static const UINT MaxStaticSamplers;
	static const RegisterAndSpace DefaultSamplerRegister;
	static const RegisterAndSpace StaticSamplerRegister;
	static const RegisterAndSpace Std1CBVRegister;
	static const RegisterAndSpace Std2CBVRegister;
	static const RegisterAndSpace MaterialCBVRegister;
	static const RegisterAndSpace MaterialSRVRegister;
	static const RegisterAndSpace MaterialUAVRegister;
	static const RegisterAndSpace MaterialSamplerRegister;
	static const RegisterAndSpace GlobalCBVRegister;
	static const RegisterAndSpace GlobalSRVRegister;
	static const RegisterAndSpace GlobalUAVRegister;
};


class StandardRootSignature
{
public:
	struct CBV
	{
		SID3D12Resource resource;
		D3D12_CONSTANT_BUFFER_VIEW_DESC desc = { 0, 0 };
		BufferLayoutID bufferLayoutID = InvalidBufferLayoutID;

		bool operator==(const CBV& rhs) const { return resource == rhs.resource && desc == rhs.desc && bufferLayoutID == rhs.bufferLayoutID; }
		bool operator!=(const CBV& rhs) const { return !(*this == rhs); }
	};

	struct SRV
	{
		SID3D12Resource resource;
		D3D12_SHADER_RESOURCE_VIEW_DESC desc = { DXGI_FORMAT_UNKNOWN, D3D12_SRV_DIMENSION_UNKNOWN, 0 };
		BufferLayoutID bufferLayoutID = InvalidBufferLayoutID;

		bool operator==(const SRV& rhs) const { return resource == rhs.resource && desc == rhs.desc && bufferLayoutID == rhs.bufferLayoutID; }
		bool operator!=(const SRV& rhs) const { return !(*this == rhs); }
	};

	struct UAV
	{
		SID3D12Resource resource;
		D3D12_UNORDERED_ACCESS_VIEW_DESC desc = { DXGI_FORMAT_UNKNOWN, D3D12_UAV_DIMENSION_UNKNOWN };
		BufferLayoutID bufferLayoutID = InvalidBufferLayoutID;

		bool operator==(const UAV& rhs) const { return resource == rhs.resource && desc == rhs.desc && bufferLayoutID == rhs.bufferLayoutID; }
		bool operator!=(const UAV& rhs) const { return !(*this == rhs); }
	};

	typedef List<CBV> CBVVector;
	typedef List<SRV> SRVVector;
	typedef List<UAV> UAVVector;

	StandardRootSignature(UINT staticSamplerCount, std::shared_ptr<const StandardRootSignatureLayout> layout);
	virtual ~StandardRootSignature();

	void Init(const ShaderInputBindDescMap &shaderInputMap);

	UINT GetStdUsageMask() const { return _stdUsageMask; }
	bool NeedsMaterialResources() const;
	bool NeedsMaterialSamplers() const;

	virtual void SetStd1Buffer(RenderSettings *rs, D3D12_GPU_VIRTUAL_ADDRESS std1) = 0;
	virtual void SetStd2Buffer(RenderSettings *rs, D3D12_GPU_VIRTUAL_ADDRESS std2) = 0;
	virtual void SetMaterialResources(RenderSettings *rs, const DescriptorTable *descTable) = 0;
	virtual void SetMaterialSamplers(RenderSettings *rs, const DescriptorTable *descTable) = 0;

	
	// Creates the descriptor table based on the given resources.
	void CreateMaterialResources(Chip *msgChip, const CBVVector &cbv, const SRVVector &srv, const UAVVector &uav, DescriptorHeapManager *heapManager, DescriptorTable **dt) const;
	// Verifies if the given descriptor table is compatible with the signature!
	void VerifyMaterialResources(const DescriptorTable *dt) const;
	// Creates the descriptor table for the samplers.
	void CreateMaterialSamplers(Chip *msgChip, const List<D3D12_SAMPLER_DESC> &samplers, DescriptorHeapManager *samplerHeapManager, DescriptorTable **samplerDt) const;
	// Verifies the numbers of descriptors.
	void VerifyMaterialSamplers(const DescriptorTable *dt) const;

	std::shared_ptr<const StandardRootSignatureLayout> GetLayout() const { return _layout; }

protected:
	UINT _stdUsageMask;
	bool _materialCBVUsed;
	bool _materialSRVUsed;
	bool _materialUAVUsed;
	bool _materialSamplersUsed;

	ShaderInputBindDescMap _shaderInputMap;

	const UINT _staticSamplerCount;
	std::shared_ptr<const StandardRootSignatureLayout> _layout;

};

class StandardGraphicsRootSignature : public StandardRootSignature
{
public:
	StandardGraphicsRootSignature(UINT staticSamplerCount, std::shared_ptr<const StandardRootSignatureLayout> layout);
	~StandardGraphicsRootSignature();

	void SetStd1Buffer(RenderSettings *rs, D3D12_GPU_VIRTUAL_ADDRESS std1) override;
	void SetStd2Buffer(RenderSettings *rs, D3D12_GPU_VIRTUAL_ADDRESS std2) override;
	void SetMaterialResources(RenderSettings *rs, const DescriptorTable *descTable) override;
	void SetMaterialSamplers(RenderSettings *rs, const DescriptorTable *descTable) override;
};

class StandardComputeRootSignature : public StandardRootSignature
{
public:
	StandardComputeRootSignature(UINT staticSamplerCount, std::shared_ptr<const StandardRootSignatureLayout> layout);
	~StandardComputeRootSignature();

	void SetStd1Buffer(RenderSettings *rs, D3D12_GPU_VIRTUAL_ADDRESS std1) override;
	void SetStd2Buffer(RenderSettings *rs, D3D12_GPU_VIRTUAL_ADDRESS std2) override;
	void SetMaterialResources(RenderSettings *rs, const DescriptorTable *descTable) override;
	void SetMaterialSamplers(RenderSettings *rs, const DescriptorTable *descTable) override;
};




}