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
#include "M3DCore/Containers.h"
#include "BufferLayout.h"
#include "GraphicsChips/GraphicsDefines.h"

namespace m3d
{

typedef uint32 RootSignatureID;
typedef uint32 PipelineStateDescID;

struct PipelineStateID
{
	union
	{
		uint32 id;
		struct 
		{
			RootSignatureID rsID : 8;
			PipelineStateDescID psID : 8;
			PipelineStateDescID pisID : 8;
			PipelineStateDescID posID : 8;
		};
	};
	PipelineStateID(uint32 id = 0) : id(id) {}
	PipelineStateID(PipelineStateDescID psID, PipelineStateDescID pisID, PipelineStateDescID posID, RootSignatureID rsID) : rsID(rsID), psID(psID), pisID(pisID), posID(posID) 
	{
		assert(psID < 256);
		assert(pisID < 256);
		assert(posID < 256);
		assert(rsID < 256);
	}
	uint32 operator()() const { return id; }
	bool operator==(PipelineStateID rhs) const { return id == rhs.id; }
	bool operator!=(PipelineStateID rhs) const { return id != rhs.id; }
	bool operator<(PipelineStateID rhs) const { return id < rhs.id; }
};


struct RootParameter : public D3D12_ROOT_PARAMETER
{
	RootParameter() { DescriptorTable.pDescriptorRanges = nullptr; }
	RootParameter(const RootParameter &desc) : RootParameter() { *this = desc; }
	RootParameter(const D3D12_ROOT_PARAMETER &desc) : RootParameter() { *this = desc; }
	~RootParameter()
	{
		if (ParameterType == D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE)
			delete[] DescriptorTable.pDescriptorRanges;
	}
	RootParameter &operator=(const RootParameter &desc) { return *this = (const D3D12_ROOT_PARAMETER&)desc; }
	RootParameter &operator=(const D3D12_ROOT_PARAMETER &desc)
	{
		this->~RootParameter();
		(D3D12_ROOT_PARAMETER&)*this = desc;
		if (ParameterType == D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE) {
			DescriptorTable.pDescriptorRanges = new D3D12_DESCRIPTOR_RANGE[DescriptorTable.NumDescriptorRanges];
			std::memcpy(const_cast<D3D12_DESCRIPTOR_RANGE*>(DescriptorTable.pDescriptorRanges), desc.DescriptorTable.pDescriptorRanges, sizeof(D3D12_DESCRIPTOR_RANGE) * DescriptorTable.NumDescriptorRanges);
		}
		return *this;
	}
};


class StandardRootSignatureLayout;

struct RootSignatureDesc : public D3D12_ROOT_SIGNATURE_DESC
{
	RootSignatureDesc() : StandardLayout(nullptr) { pParameters = nullptr; pStaticSamplers = nullptr; }
	RootSignatureDesc(const RootSignatureDesc &desc) : RootSignatureDesc() { *this = desc; }
	RootSignatureDesc(const D3D12_ROOT_SIGNATURE_DESC &desc) : RootSignatureDesc() { *this = desc; }
	~RootSignatureDesc()
	{
		delete[] (RootParameter*)pParameters;
		delete[] pStaticSamplers;
	}
	RootSignatureDesc &operator=(const RootSignatureDesc &desc) { StandardLayout = desc.StandardLayout; return *this = (const D3D12_ROOT_SIGNATURE_DESC&)desc; }
	RootSignatureDesc &operator=(const D3D12_ROOT_SIGNATURE_DESC &desc)
	{
		delete[](RootParameter*)pParameters;
		delete[] pStaticSamplers;
		(D3D12_ROOT_SIGNATURE_DESC&)*this = desc;
		pParameters = new RootParameter[NumParameters];
		for (UINT i = 0; i < NumParameters; i++)
			((RootParameter*)(pParameters))[i] = desc.pParameters[i];
		pStaticSamplers = new D3D12_STATIC_SAMPLER_DESC[NumStaticSamplers];
		std::memcpy(const_cast<D3D12_STATIC_SAMPLER_DESC*>(pStaticSamplers), desc.pStaticSamplers, sizeof(D3D12_STATIC_SAMPLER_DESC) * NumStaticSamplers);
		return *this;
	}

	std::shared_ptr<const StandardRootSignatureLayout> StandardLayout;
};


struct GRAPHICSCHIPS_API InputElementDesc : public D3D12_INPUT_ELEMENT_DESC
{
	static Set<String> SemanticNamePool;
	InputElementDesc(const InputElementDesc &desc) : InputElementDesc((const D3D12_INPUT_ELEMENT_DESC&)desc) {}
	InputElementDesc(const D3D12_INPUT_ELEMENT_DESC &desc) : D3D12_INPUT_ELEMENT_DESC(desc)
	{
		SemanticName = SemanticNamePool.insert(SemanticName).first->c_str();
	}
	InputElementDesc &operator=(const InputElementDesc &rhs) { return *this = (const D3D12_INPUT_ELEMENT_DESC&)rhs; }
	InputElementDesc &operator=(const D3D12_INPUT_ELEMENT_DESC &rhs) 
	{ 
		(D3D12_INPUT_ELEMENT_DESC&)*this = rhs; 
		SemanticName = SemanticNamePool.insert(SemanticName).first->c_str(); 
		return *this;
	}
};


struct InputLayoutDesc : public D3D12_INPUT_LAYOUT_DESC
{
	InputLayoutDesc() { pInputElementDescs = nullptr; NumElements = 0; }
	InputLayoutDesc(const InputLayoutDesc &desc) : InputLayoutDesc() { *this = desc; }
	InputLayoutDesc(const D3D12_INPUT_LAYOUT_DESC &desc) : InputLayoutDesc() { *this = desc; }
	~InputLayoutDesc()
	{
		delete[] pInputElementDescs;
	}
	InputLayoutDesc &operator=(const InputLayoutDesc &desc) { return *this = (const D3D12_INPUT_LAYOUT_DESC&)desc; }
	InputLayoutDesc &operator=(const D3D12_INPUT_LAYOUT_DESC &desc) 
	{
		this->~InputLayoutDesc();
		(D3D12_INPUT_LAYOUT_DESC&)*this = desc;
		pInputElementDescs = new D3D12_INPUT_ELEMENT_DESC[NumElements];
		for (UINT i = 0; i < NumElements; i++)
			const_cast<D3D12_INPUT_ELEMENT_DESC*>(pInputElementDescs)[i] = InputElementDesc(desc.pInputElementDescs[i]);
		return *this;
	}
	bool operator<(const InputLayoutDesc &rhs) const
	{
		return NumElements < rhs.NumElements || NumElements == rhs.NumElements && std::memcmp(pInputElementDescs, rhs.pInputElementDescs, sizeof(D3D12_INPUT_ELEMENT_DESC) * NumElements) < 0;
	}

	bool HasElement(const Char *name, UINT index) const
	{
		for (UINT i = 0; i < NumElements; i++)
			if (pInputElementDescs[i].SemanticIndex == index && strcmp(pInputElementDescs[i].SemanticName, name) == 0)
				return true;
		return false;
	}
};

struct InputLayoutDesc2 : public InputLayoutDesc
{
	InputLayoutDesc2() : Capacity(0) {}
	void AddElement(const D3D12_INPUT_ELEMENT_DESC &element)
	{
		if (Capacity <= NumElements) {
			const D3D12_INPUT_ELEMENT_DESC *pOld = pInputElementDescs;
			Capacity = NumElements == 0 ? 4 : NumElements * 2;
			pInputElementDescs = new D3D12_INPUT_ELEMENT_DESC[Capacity];
			if (pOld)
				std::memcpy(const_cast<D3D12_INPUT_ELEMENT_DESC*>(pInputElementDescs), pOld, sizeof(D3D12_INPUT_ELEMENT_DESC) * NumElements);
		}

		const_cast<D3D12_INPUT_ELEMENT_DESC*>(pInputElementDescs)[NumElements++] = element;
	}

	UINT Capacity;
};


struct SoDeclarationEntry : public D3D12_SO_DECLARATION_ENTRY
{
	SoDeclarationEntry(const SoDeclarationEntry &desc) : SoDeclarationEntry((const D3D12_SO_DECLARATION_ENTRY&)desc) {}
	SoDeclarationEntry(const D3D12_SO_DECLARATION_ENTRY &desc) : D3D12_SO_DECLARATION_ENTRY(desc)
	{
		SemanticName = InputElementDesc::SemanticNamePool.insert(SemanticName).first->c_str();
	}
	SoDeclarationEntry &operator=(const SoDeclarationEntry &desc) { return *this = (const D3D12_SO_DECLARATION_ENTRY&)desc; }
	SoDeclarationEntry &operator=(const D3D12_SO_DECLARATION_ENTRY &desc)
	{
		(D3D12_SO_DECLARATION_ENTRY&)*this = desc;
		SemanticName = InputElementDesc::SemanticNamePool.insert(SemanticName).first->c_str();
	}
};

struct StreamOutputDesc : public D3D12_STREAM_OUTPUT_DESC
{
	StreamOutputDesc() { pSODeclaration = 0; pBufferStrides = 0; NumEntries = 0; NumStrides = 0; RasterizedStream = 0; }
	StreamOutputDesc(const StreamOutputDesc &desc) : StreamOutputDesc() { *this = desc; }
	StreamOutputDesc(const D3D12_STREAM_OUTPUT_DESC &desc) : StreamOutputDesc() { *this = desc; }
	~StreamOutputDesc() 
	{
		delete[] pSODeclaration;
		delete[] pBufferStrides;
	}
	StreamOutputDesc &operator=(const StreamOutputDesc &desc) { return *this = (const D3D12_STREAM_OUTPUT_DESC&)desc; }
	StreamOutputDesc &operator=(const D3D12_STREAM_OUTPUT_DESC &desc)
	{
		this->~StreamOutputDesc();
		(D3D12_STREAM_OUTPUT_DESC&)*this = desc;
		if (NumEntries > 0) {
			pSODeclaration = new D3D12_SO_DECLARATION_ENTRY[NumEntries];
			for (UINT i = 0; i < NumEntries; i++)
				const_cast<D3D12_SO_DECLARATION_ENTRY*>(pSODeclaration)[i] = SoDeclarationEntry(desc.pSODeclaration[i]);
		}
		if (NumStrides > 0) {
			pBufferStrides = new UINT[NumStrides];
			std::memcpy(const_cast<UINT*>(pBufferStrides), desc.pBufferStrides, sizeof(UINT) * NumStrides);
		}
		return *this;
	}

	bool operator==(const StreamOutputDesc &rhs) const
	{
		return NumEntries == rhs.NumEntries && NumStrides == rhs.NumStrides && RasterizedStream == rhs.RasterizedStream &&
			std::memcmp(pBufferStrides, rhs.pBufferStrides, sizeof(UINT) * NumStrides) == 0 &&
			std::memcmp(pSODeclaration, rhs.pSODeclaration, sizeof(D3D12_SO_DECLARATION_ENTRY) * NumEntries) == 0;
	}

	size_t hash() const
	{
		return 0; // TODO
	}
};


struct PipelineInputStateDesc
{
	InputLayoutDesc InputLayout;
	D3D12_INDEX_BUFFER_STRIP_CUT_VALUE IBStripCutValue;
	D3D12_PRIMITIVE_TOPOLOGY_TYPE PrimitiveTopologyType;

	PipelineInputStateDesc() : IBStripCutValue(D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED), PrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_UNDEFINED)
	{
	}

	bool operator<(const PipelineInputStateDesc &rhs) const 
	{
		return IBStripCutValue < rhs.IBStripCutValue || IBStripCutValue == rhs.IBStripCutValue && (PrimitiveTopologyType < rhs.PrimitiveTopologyType || PrimitiveTopologyType == rhs.PrimitiveTopologyType && InputLayout < rhs.InputLayout);
	}
};

struct PipelineOutputStateDesc
{
	UINT NumRenderTargets;
	DXGI_FORMAT RTVFormats[ 8 ];
	DXGI_FORMAT DSVFormat;
	DXGI_SAMPLE_DESC SampleDesc;

	PipelineOutputStateDesc() : NumRenderTargets(0), DSVFormat(DXGI_FORMAT_UNKNOWN)
	{
		memset(RTVFormats, 0, sizeof(RTVFormats));
		SampleDesc = { 1, 0 };
	}

	bool operator<(const PipelineOutputStateDesc &rhs) const
	{
		return std::memcmp(this, &rhs, sizeof(PipelineOutputStateDesc)) < 0;
	}
};

enum ShaderStage
{
	ShaderStage_VERTEX	= 0x01,
	ShaderStage_HULL	= 0x02,
	ShaderStage_DOMAIN	= 0x04,
	ShaderStage_GEOMETRY= 0x08,
	ShaderStage_PIXEL	= 0x10,
	ShaderStage_COMPUTE	= 0x20,
	ShaderStage_ALL		= 0x3F
};

/*
struct ShaderBufferVariableDesc
{
	String Name;
	D3D_SHADER_VARIABLE_CLASS Class;
	D3D_SHADER_VARIABLE_TYPE Type;
	UINT Rows;
	UINT Columns;
	UINT Elements;
	UINT Offset;
	UINT Size;
	UINT Stages;

	ShaderBufferVariableDesc() {}
	ShaderBufferVariableDesc(const D3D12_SHADER_TYPE_DESC &b) : Name(b.Name), Class(b.Class), Type(b.Type), Rows(b.Rows), Columns(b.Columns), Elements(b.Elements), Offset(b.Offset), Size(0), Stages(0) {}
	ShaderBufferVariableDesc(const D3D12_SHADER_VARIABLE_DESC &a, const D3D12_SHADER_TYPE_DESC &b) : Name(a.Name), Class(b.Class), Type(b.Type), Rows(b.Rows), Columns(b.Columns), Elements(b.Elements), Offset(b.Offset), Size(0), Stages(0) {}
};*/

struct ShaderInputBindDesc
{
	// Same as D3D12_SHADER_INPUT_BIND_DESC
	String Name;
	D3D_SHADER_INPUT_TYPE Type;
	UINT BindPoint;
	UINT BindCount;
	UINT uFlags;
	D3D_RESOURCE_RETURN_TYPE ReturnType;
	D3D_SRV_DIMENSION Dimension;
	UINT NumSamples;
	UINT Space;

	UINT Stages; // Which of the shader stages is using this resource.
	UINT Size; // In bytes for buffers.
	BufferLayout bufferLayout; // Layout of members in a buffer.
	UINT usageMask; // Usage mask for up to 32 members in a buffer. Used by std1&2.
	
	BufferLayoutID bufferLayoutID;

	ShaderInputBindDesc() : Type(D3D_SIT_CBUFFER), BindPoint(0), BindCount(0), uFlags(0), ReturnType(D3D_RETURN_TYPE_UNORM), Dimension(D3D_SRV_DIMENSION_UNKNOWN), NumSamples(0), Space(0), Stages(0), Size(0), usageMask(0), bufferLayoutID(InvalidBufferLayoutID) {}
	ShaderInputBindDesc(const D3D12_SHADER_INPUT_BIND_DESC &rhs) : 
		Name(rhs.Name), Type(rhs.Type), BindPoint(rhs.BindPoint), BindCount(rhs.BindCount), 
		uFlags(rhs.uFlags), ReturnType(rhs.ReturnType), Dimension(rhs.Dimension), 
		NumSamples(rhs.NumSamples), Space(rhs.Space), Stages(0), Size(0), usageMask(0), bufferLayoutID(InvalidBufferLayoutID) {}

	bool operator==(const ShaderInputBindDesc &rhs) const
	{
		bool b = Name == rhs.Name && Type == rhs.Type && BindPoint == rhs.BindPoint && BindCount == rhs.BindCount && uFlags == rhs.uFlags && ReturnType == rhs.ReturnType && Dimension == rhs.Dimension && NumSamples == rhs.NumSamples && Space == rhs.Space
			&& bufferLayoutID == rhs.bufferLayoutID; // Added feb 20: If not testing for bufferLayout, changes to buffer in shader does not update layout id in material...
		return b;
	}

	bool isCompatible(const ShaderInputBindDesc &rhs) const
	{
		return Type == rhs.Type && ReturnType == rhs.ReturnType && Dimension == rhs.Dimension && NumSamples == rhs.NumSamples && bufferLayout.IsCompatible(rhs.bufferLayout);
	}
};

struct ShaderInputBindDescKey
{
	enum RegisterType { CBV, SRV, SAMPLER, UAV };

	RegisterType Register;
	UINT Space;
	UINT BindPoint;

	ShaderInputBindDescKey(RegisterType Register = CBV, UINT BindPoint = 0, UINT Space = 0) : Register(Register), Space(Space), BindPoint(BindPoint)
	{
	}
		
	ShaderInputBindDescKey(D3D_SHADER_INPUT_TYPE Type, UINT BindPoint = 0, UINT Space = 0) : Space(Space), BindPoint(BindPoint)
	{
		static const RegisterType RegisterTypeByInputType[] = { CBV, SRV, SRV, SAMPLER, UAV, UAV, UAV, UAV, UAV, UAV, UAV, UAV }; // TODO: not confirmed for all types...
		Register = RegisterTypeByInputType[Type];
	}

	bool operator==(const ShaderInputBindDescKey &rhs) const { 
		bool b= Register == rhs.Register && Space == rhs.Space && BindPoint == rhs.BindPoint;
		return b;
	}
	bool operator<(const ShaderInputBindDescKey &rhs) const { return Register < rhs.Register || (Register == rhs.Register && (Space < rhs.Space || Space == rhs.Space && BindPoint < rhs.BindPoint)); }
	bool IsSameRegisterAndSpace(const ShaderInputBindDescKey &rhs) const { return Register == rhs.Register && Space == rhs.Space; }
};

typedef Map<ShaderInputBindDescKey, ShaderInputBindDesc> ShaderInputBindDescMap;

struct SignatureParameter
{
	String semanticName;
	UINT semanticIndex;
	D3D_REGISTER_COMPONENT_TYPE componentType;  // Scalar type (e.g. uint, float, etc.)
	BYTE mask; // x, xy, xyz, xyzw, etc
	BYTE readWriteMask; // Which of the components are written/read.
	UINT stream; // Stream index (geometry shader output?)
	D3D_MIN_PRECISION minPrecision; // Minimum desired interpolation precision	

	SignatureParameter()
	{
	}

	bool operator==(const SignatureParameter &rhs) const
	{
		return semanticName == rhs.semanticName && semanticIndex == rhs.semanticIndex && componentType == rhs.componentType && mask == rhs.mask && readWriteMask == rhs.readWriteMask && stream == rhs.stream && minPrecision == rhs.minPrecision;
	}

	bool isCompatibleWithOutputParameter(const SignatureParameter &outputParameter) const
	{
		return semanticName == outputParameter.semanticName && semanticIndex == outputParameter.semanticIndex && componentType == outputParameter.componentType;
	}
};


struct ShaderDesc
{
	SID3DBlob byteCode;
	ShaderModel sm;
	ShaderType st;
	List<SignatureParameter> inputParameters;
	List<SignatureParameter> outputParameters;
	ShaderInputBindDescMap uniforms;
	UINT ThreadGroupSize[3] = {0, 0, 0};

	ShaderDesc() : sm(ShaderModel::UNDEFINED), st(ShaderType::UNDEFINED) {}
};

struct ShaderPipelineDesc
{
	SID3DBlob byteCodes[5];
	UINT shaderStages;
	List<SignatureParameter> inputParameters;
	List<SignatureParameter> outputParameters;
	ShaderInputBindDescMap uniforms;

	ShaderPipelineDesc() : shaderStages(0) {}

	bool isEqual(ShaderPipelineDesc &rhs) const
	{
		if (!(inputParameters == rhs.inputParameters))
			return false;
		if (!(outputParameters == rhs.outputParameters))
			return false;
		if (!(uniforms == rhs.uniforms))
			return false;
		for (uint32 i = 0; i < 5; i++)
			if (byteCodes[i] != rhs.byteCodes[i]) {
				if (byteCodes[i]->GetBufferSize() != rhs.byteCodes[i]->GetBufferSize())
					return false;
				if (std::memcmp(byteCodes[i]->GetBufferPointer(), rhs.byteCodes[i]->GetBufferPointer(), byteCodes[i]->GetBufferSize()) != 0)
					return false;
				//return false;
			}
		return true;
	}

};


struct PipelineStateDesc
{
	SID3DBlob VS;
	SID3DBlob PS;
	SID3DBlob DS;
	SID3DBlob HS;
	SID3DBlob GS;
	StreamOutputDesc StreamOutput; // Maybe to be moved to DescOut...
	D3D12_BLEND_DESC BlendState;
	UINT SampleMask;
	D3D12_RASTERIZER_DESC RasterizerState;
	D3D12_DEPTH_STENCIL_DESC DepthStencilState;
	UINT NodeMask;
	D3D12_CACHED_PIPELINE_STATE CachedPSO;
	D3D12_PIPELINE_STATE_FLAGS Flags;

	struct {
		List<SignatureParameter> inputParameters;
		List<SignatureParameter> outputParameters;
		ShaderInputBindDescMap uniforms;
	} Pipeline;

	PipelineStateDesc() : BlendState(CD3DX12_BLEND_DESC(D3D12_DEFAULT)), SampleMask(0xFFFFFFFF), RasterizerState(CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT)), DepthStencilState(CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT)), NodeMask(0), CachedPSO({ nullptr, 0 }), Flags(D3D12_PIPELINE_STATE_FLAG_NONE)
	{
	}

	bool operator==(const PipelineStateDesc &rhs) const
	{
		// Note: Do not include Pipeline as it is a derivative of the shaders!
		return VS == rhs.VS && PS == rhs.PS && DS == rhs.DS && HS == rhs.HS && GS == rhs.GS  && StreamOutput == rhs.StreamOutput &&
			BlendState == rhs.BlendState && SampleMask == rhs.SampleMask && RasterizerState == rhs.RasterizerState && 
			DepthStencilState == rhs.DepthStencilState && NodeMask == rhs.NodeMask && Flags == rhs.Flags;
	}

	size_t hash() const
	{
		return m3d::hash_combine_simple<m3d::Hash>(VS.get(), PS.get(), DS.get(), HS.get(), GS.get(), StreamOutput, BlendState, SampleMask, RasterizerState, DepthStencilState, NodeMask, Flags);
	}
};

struct ComputePipelineStateDesc
{
	SID3DBlob CS;
	UINT NodeMask;
	D3D12_CACHED_PIPELINE_STATE CachedPSO;
	D3D12_PIPELINE_STATE_FLAGS Flags;

	ShaderInputBindDescMap uniforms;

	ComputePipelineStateDesc() : NodeMask(0), CachedPSO({ nullptr, 0 }), Flags(D3D12_PIPELINE_STATE_FLAG_NONE)
	{
	}

	bool operator==(const ComputePipelineStateDesc &rhs) const
	{
		// Note: Do not include ShaderInputsMap as it is a derivative of the shaders!
		return CS == rhs.CS && NodeMask == rhs.NodeMask && Flags == rhs.Flags;
	}

	std::size_t hash() const
	{
		return m3d::hash_combine_simple<m3d::Hash>(CS.get(), NodeMask, Flags);
	}
};

class StandardRootSignature;

struct PipelineState
{
	SID3D12PipelineState pso;
	SID3D12RootSignature rootSignature;
	std::shared_ptr<StandardRootSignature> standardRootSignature;

	PipelineState() {}
	~PipelineState() {}

	bool IsValid() const { return pso != nullptr && rootSignature != nullptr; }
};

class GRAPHICSCHIPS_API PipelineStatePool 
{
public:
	PipelineStatePool();
	~PipelineStatePool();

	void SetDevice(ID3D12Device *device);
	void OnDestroyDevice();

	RootSignatureID RegisterRootSignatureDesc(const RootSignatureDesc &desc, String *msg = nullptr);
	ID3D12RootSignature *GetRootSignature(RootSignatureID id);
	const RootSignatureDesc *GetRootSignatureDesc(RootSignatureID id);

	// Register a PipelineStateDesc. Remember to call UnregisterPipelineStateDesc() when done with it.
	// It is neccessary to unregister because the state holds resources (shaders) we want to get rid of when no longer used!
	// NOTE: If a similar PipelineStateDesc already exist, the id to the existing one is returned, and a reference count is incremented!
	PipelineStateDescID RegisterPipelineStateDesc(const PipelineStateDesc &desc);
	// Unregster a PipelineStateDesc. Returns true if found.
	bool UnregisterPipelineStateDesc(PipelineStateDescID id);
	// Register a PipelineInputStateDesc.
	PipelineStateDescID RegisterPipelineInputStateDesc(const PipelineInputStateDesc &desc);
	// Register a PipelineOutputStateDesc.
	PipelineStateDescID RegisterPipelineOutputStateDesc(const PipelineOutputStateDesc &desc);

	PipelineStateDescID RegisterComputePipelineStateDesc(const ComputePipelineStateDesc &desc);
	bool UnregisterComputePipelineStateDesc(PipelineStateDescID id);

	const PipelineInputStateDesc *GetPipelineInputStateDesc(PipelineStateDescID id) const;
	const PipelineStateDesc* GetPipelineStateDesc(PipelineStateDescID id) const;
	const ComputePipelineStateDesc* GetComputePipelineStateDesc(PipelineStateDescID id) const;

	// Get the PipelineState containing the d3d12 device objects for pipeline state and root signature,
	// in addition to a StandardRootSignature, if any. Never nullptr! Throws on fail!
	const PipelineState &GetState(PipelineStateID id);
	const PipelineState &GetComputeState(PipelineStateID id);

private:
	RootSignatureID _registerRootSignature(ID3DBlob *signature, const RootSignatureDesc &desc);

	SID3D12Device _device;

	struct _RootSignature
	{
		SID3DBlob serialized;
		RootSignatureID id;
		RootSignatureDesc desc;
		SID3D12RootSignature signature;
		bool failed;

		_RootSignature(SID3DBlob serialized, RootSignatureID id, RootSignatureDesc desc, SID3D12RootSignature signature, bool failed) : serialized(serialized), id(id), desc(desc), signature(signature), failed(failed) {}

		bool operator==(const _RootSignature& rhs) const
		{
			if (serialized == rhs.serialized)
				return true;
			if (serialized == nullptr || serialized->GetBufferSize() != rhs.serialized->GetBufferSize())
				return false;
			return std::memcmp(serialized->GetBufferPointer(), rhs.serialized->GetBufferPointer(), serialized->GetBufferSize()) == 0;
		}

		std::size_t hash() const
		{
			std::size_t h = 0;
			for (SIZE_T i = 0; i < serialized.get()->GetBufferSize(); ++i)
				hash_combine<m3d::Hash>(h, ((uint8*)serialized.get()->GetBufferPointer())[i]);
			return h;
		}
	};

	template<typename T, typename H = Hash<T>>
	struct HashPtr
	{
		std::size_t operator()(std::shared_ptr<T> const& t) const noexcept
		{
			return H()(*t.get());
		}
	};

	template<typename T>
	struct EqualToPtr
	{
		bool operator()(std::shared_ptr<T> const& l, std::shared_ptr<T> const& r) const noexcept
		{
			if (l && r)
				return std::equal_to()(*l, *r);
			return (bool)l == (bool)r;
		}
	};

	struct _PipelineStateDesc
	{
		PipelineStateDesc desc;
		PipelineStateDescID id;
		mutable uint32 refCount;

		_PipelineStateDesc(PipelineStateDesc desc, PipelineStateDescID id) : desc(desc), id(id), refCount(1) {}

		bool operator==(const _PipelineStateDesc& rhs) const
		{
			return desc == rhs.desc;
		}

		std::size_t hash() const
		{
			return desc.hash();
		}
	};


	struct _ComputePipelineStateDesc
	{
		ComputePipelineStateDesc desc;
		PipelineStateDescID id;
		mutable uint32 refCount;

		_ComputePipelineStateDesc(ComputePipelineStateDesc desc, PipelineStateDescID id) : desc(desc), id(id), refCount(1) {}

		bool operator==(const _ComputePipelineStateDesc& rhs) const
		{
			return desc == rhs.desc;
		}

		std::size_t hash() const
		{
			return desc.hash();
		}
	};

	Map<PipelineStateID, PipelineState> _states;

	UnorderedSet<std::shared_ptr<_PipelineStateDesc>, HashPtr<_PipelineStateDesc>, EqualToPtr<_PipelineStateDesc>> _descs;
	Map<PipelineStateDescID, std::shared_ptr<_PipelineStateDesc>> _descsByID;

	UnorderedSet<std::shared_ptr<_ComputePipelineStateDesc>, HashPtr<_ComputePipelineStateDesc>, EqualToPtr<_ComputePipelineStateDesc>> _computeDescs;
	Map<PipelineStateDescID, std::shared_ptr<_ComputePipelineStateDesc>> _computeDescsByID;

	uint32 _descCounter;

	Map<PipelineInputStateDesc, PipelineStateDescID> _mapInDescToID;
	List<Map<PipelineInputStateDesc, PipelineStateDescID>::iterator> _inIDToMapNode;

	Map<PipelineOutputStateDesc, PipelineStateDescID> _mapOutDescToID;
	List<Map<PipelineOutputStateDesc, PipelineStateDescID>::iterator> _outIDToMapNode;

	UnorderedSet<std::shared_ptr<_RootSignature>, HashPtr<_RootSignature>, EqualToPtr<_RootSignature>> _rootSignatures;
	List<std::shared_ptr<_RootSignature>> _RootSignaturesByID;
};


}
