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
#include "GraphicsState.h"
#include "RootSignature.h"
#include "Shaders.h"
#include "M3DEngine/DocumentSaveLoadUtil.h"
#include "D3DBlobUtil.h"
#include "StandardRootSignature.h"
#include "RenderSettings.h"


using namespace m3d;

namespace m3d
{
	bool SerializeDocumentData(DocumentSaver& saver, const GraphicsState::ElementValue& data)
	{
		SAVE("type", data.type);
		switch (data.type)
		{
		case GraphicsState::ElementType::INVALID: return true;
		case GraphicsState::ElementType::UINT8: SAVE("value", data.etUINT8); break;
		case GraphicsState::ElementType::UINT: SAVE("value", data.etUINT); break;
		case GraphicsState::ElementType::INT: SAVE("value", data.etINT); break;
		case GraphicsState::ElementType::FLOAT: SAVE("value", data.etFLOAT); break;
		case GraphicsState::ElementType::M3D_FILL_MODE: SAVE("value", data.etM3D_FILL_MODE); break;
		case GraphicsState::ElementType::M3D_CULL_MODE: SAVE("value", data.etM3D_CULL_MODE); break;
		case GraphicsState::ElementType::M3D_CONSERVATIVE_RASTERIZATION_MODE: SAVE("value", data.etM3D_CONSERVATIVE_RASTERIZATION_MODE); break;
		case GraphicsState::ElementType::M3D_DEPTH_WRITE_MASK: SAVE("value", data.etM3D_DEPTH_WRITE_MASK); break;
		case GraphicsState::ElementType::M3D_COMPARISON_FUNC: SAVE("value", data.etM3D_COMPARISON_FUNC); break;
		case GraphicsState::ElementType::M3D_STENCIL_OP: SAVE("value", data.etM3D_STENCIL_OP); break;
		case GraphicsState::ElementType::M3D_BLEND: SAVE("value", data.etM3D_BLEND); break;
		case GraphicsState::ElementType::M3D_BLEND_OP: SAVE("value", data.etM3D_BLEND_OP); break;
		case GraphicsState::ElementType::M3D_LOGIC_OP: SAVE("value", data.etM3D_LOGIC_OP); break;
		default: return false;
		}
		return true;
	}

	bool DeserializeDocumentData(DocumentLoader& loader, GraphicsState::ElementValue& data)
	{
		LOAD("type", data.type);
		switch (data.type)
		{
		case GraphicsState::ElementType::INVALID: return true;
		case GraphicsState::ElementType::UINT8: LOAD("value", data.etUINT8); break;
		case GraphicsState::ElementType::UINT: LOAD("value", data.etUINT); break;
		case GraphicsState::ElementType::INT: LOAD("value", data.etINT); break;
		case GraphicsState::ElementType::FLOAT: LOAD("value", data.etFLOAT); break;
		case GraphicsState::ElementType::M3D_FILL_MODE: LOAD("value", data.etM3D_FILL_MODE); break;
		case GraphicsState::ElementType::M3D_CULL_MODE: LOAD("value", data.etM3D_CULL_MODE); break;
		case GraphicsState::ElementType::M3D_CONSERVATIVE_RASTERIZATION_MODE: LOAD("value", data.etM3D_CONSERVATIVE_RASTERIZATION_MODE); break;
		case GraphicsState::ElementType::M3D_DEPTH_WRITE_MASK:LOAD("value", data.etM3D_DEPTH_WRITE_MASK); break;
		case GraphicsState::ElementType::M3D_COMPARISON_FUNC: LOAD("value", data.etM3D_COMPARISON_FUNC); break;
		case GraphicsState::ElementType::M3D_STENCIL_OP: LOAD("value", data.etM3D_STENCIL_OP); break;
		case GraphicsState::ElementType::M3D_BLEND: LOAD("value", data.etM3D_BLEND); break;
		case GraphicsState::ElementType::M3D_BLEND_OP: LOAD("value", data.etM3D_BLEND_OP); break;
		case GraphicsState::ElementType::M3D_LOGIC_OP: LOAD("value", data.etM3D_LOGIC_OP); break;
		default: return false;
		}
		return true;
	}
}


CHIPDESCV1_DEF(GraphicsState, MTEXT("Graphics State"), GRAPHICSSTATE_GUID, CHIP_GUID);


GraphicsState::GraphicsState()
{
	CREATE_CHILD(0, ROOTSIGNATURE_GUID, false, UP, MTEXT("Root Signature"));
	CREATE_CHILD(1, SHADERS_GUID, false, UP, MTEXT("Shaders"));
	CREATE_CHILD(2, GRAPHICSSTATE_GUID, true, UP, MTEXT("Inherited States"));

	_hasShaders = false;
	_psoID = 0;
	_rootSignatureID = 0;
}

GraphicsState::~GraphicsState()
{
}

bool GraphicsState::CopyChip(Chip* chip)
{
	GraphicsState* c = dynamic_cast<GraphicsState*>(chip);
	B_RETURN(Chip::CopyChip(c));
	_elements = c->_elements;
	return true;
}

bool GraphicsState::LoadChip(DocumentLoader& loader)
{
	B_RETURN(Chip::LoadChip(loader));
	LOAD("elements", _elements);
	return true;
}

bool GraphicsState::SaveChip(DocumentSaver& saver) const
{
	B_RETURN(Chip::SaveChip(saver));
	SAVE("elements", _elements);
	return true;
}

void GraphicsState::SetStateElements(const StateElementMap& elements)
{
	if (_elements == elements)
		return;
	_elements = elements;
}

void GraphicsState::CallChip()
{
	try
	{
		UpdateChip(); // throws!

		RootSignatureID rsID = GetRootSignatureID(); // throws!
		PipelineStateDescID psoID = GetPipelineStateID(); // throws!

		rs()->SetPipelineStateDesc(psoID);
		rs()->SetGraphicsRootSignature(rsID);
	}
	catch (const ChipException& exc)
	{
		AddException(exc);
	}
}

void GraphicsState::UpdateChip()
{
	RefreshT refresh(Refresh);
	if (!refresh) {
		if (_rootSignatureID == 0) // Root signature is 0 if uninitialized!
			throw ReinitPreventedException(this);
		return;
	}

	ChipExceptionScope ces(this); // Functions that are not a chip may throw. Report from us!

	// Clear state!
	_psoID = 0;
	_rootSignatureID = 0;
	_inheritedElements.clear();
	_hasShaders = false;
	_shaders = ShaderPipelineDesc();

	RootSignatureID rootSignatureID = 0;

	// Update root signature!
	ChildPtr<RootSignature> ch0 = GetChild(0);
	if (ch0) {
		rootSignatureID = ch0->GetRootSignatureID(); // throws!
	}

	bool hasShaders = false;
	ShaderPipelineDesc shaders;
	{
		ChildPtr<Shaders> ch1 = GetChild(1);
		if (ch1) {
			shaders = ch1->GetProgram(); // throws!
			hasShaders = true;
		}
	}

	StateElementMap ie;
	ie.insert(_elements.begin(), _elements.end());
	// Inherit state from child states.
	for (uint32 i = 0, j = GetSubConnectionCount(2); i < j; i++) {
		ChildPtr<GraphicsState> ch2 = GetChild(2, i);
		if (ch2) {
			ch2->UpdateChip(); // throws!
			ie.insert(ch2->_inheritedElements.begin(), ch2->_inheritedElements.end()); // We inherits states that are not already set!
			if (!hasShaders && ch2->_hasShaders) {
				shaders = ch2->_shaders; // Inherit shaders if we don't have any!
				hasShaders = true;
			}
			if (rootSignatureID == 0)
				rootSignatureID = ch2->_rootSignatureID; // Inherit the root signature if we don't have any!
		}
	}

	if (rootSignatureID == 0) { // No root signature connected. Use a default signature!
		static List<D3D12_STATIC_SAMPLER_DESC> defaultSamplers;

		static RootSignatureID defaultRSID[3] =
		{
			GetPipelineStatePool()->RegisterRootSignatureDesc(StandardRootSignatureLayout::CreateRootSignatureDesc(D3D12_RESOURCE_BINDING_TIER_1)), // throws!
			GetPipelineStatePool()->RegisterRootSignatureDesc(StandardRootSignatureLayout::CreateRootSignatureDesc(D3D12_RESOURCE_BINDING_TIER_2)), // throws!
			GetPipelineStatePool()->RegisterRootSignatureDesc(StandardRootSignatureLayout::CreateRootSignatureDesc(D3D12_RESOURCE_BINDING_TIER_3)) // throws!
		};
		rootSignatureID = defaultRSID[graphics()->GetResourceBindingTier() - 1]; // Careful - enum must match array indices!
	}

	// All passed! Set state to chip!
	_inheritedElements = ie;
	_hasShaders = hasShaders;
	_shaders = shaders;
	_rootSignatureID = rootSignatureID;
}

RootSignatureID GraphicsState::GetRootSignatureID()
{
	if (_rootSignatureID == 0)
		throw GraphicsException(this, MTEXT("No root signature defined!"), FATAL);
	return _rootSignatureID;
}

PipelineStateDescID GraphicsState::GetPipelineStateID()
{
	if (_psoID != 0)
		return _psoID; // State already registered. Return ID!

	if (!_hasShaders)
		throw GraphicsException(this, MTEXT("No shaders are defined! Shaders are neccessary to create a complete pipeline state!"), FATAL);

	PipelineStateDesc desc;

	_fill(_inheritedElements, desc);

	desc.VS = _shaders.byteCodes[0];
	desc.HS = _shaders.byteCodes[1];
	desc.DS = _shaders.byteCodes[2];
	desc.GS = _shaders.byteCodes[3];
	desc.PS = _shaders.byteCodes[4];
	desc.Pipeline.uniforms = _shaders.uniforms;
	desc.Pipeline.inputParameters = _shaders.inputParameters;
	desc.Pipeline.outputParameters = _shaders.outputParameters;

	// TODO: Verify state completeness!

	if (graphics()->GetDebug()) // If we have a debug device, we set the debug flag for the PSO!
		desc.Flags |= D3D12_PIPELINE_STATE_FLAG_NONE;//D3D12_PIPELINE_STATE_FLAG_TOOL_DEBUG;

	_psoID = GetPipelineStatePool()->RegisterPipelineStateDesc(desc);

	return _psoID;
}

void GraphicsState::_fill(const StateElementMap& elements, PipelineStateDesc& desc)
{
	for (const auto& n : elements)
		_fill(n.first, n.second, desc);
}

void GraphicsState::_fill(Element e, const ElementValue& ev, PipelineStateDesc& desc)
{
	switch (e)
	{
	case Element::RS_FM: desc.RasterizerState.FillMode = (D3D12_FILL_MODE)ev.etM3D_FILL_MODE; break;
	case Element::RS_CM: desc.RasterizerState.CullMode = (D3D12_CULL_MODE)ev.etM3D_CULL_MODE; break;
	case Element::RS_FCC: desc.RasterizerState.FrontCounterClockwise = ev.etBOOL; break;
	case Element::RS_DB: desc.RasterizerState.DepthBias = ev.etINT; break;
	case Element::RS_DBC: desc.RasterizerState.DepthBiasClamp = ev.etFLOAT; break;
	case Element::RS_SSDB: desc.RasterizerState.SlopeScaledDepthBias = ev.etFLOAT; break;
	case Element::RS_DCE: desc.RasterizerState.DepthClipEnable = ev.etBOOL; break;
	case Element::RS_MSE: desc.RasterizerState.MultisampleEnable = ev.etBOOL; break;
	case Element::RS_AALE: desc.RasterizerState.AntialiasedLineEnable = ev.etBOOL; break;
	case Element::RS_FSC: desc.RasterizerState.ForcedSampleCount = ev.etUINT; break;
	case Element::RS_CR: desc.RasterizerState.ConservativeRaster = (D3D12_CONSERVATIVE_RASTERIZATION_MODE)ev.etM3D_CONSERVATIVE_RASTERIZATION_MODE; break;
	case Element::DSS_DE: desc.DepthStencilState.DepthEnable = ev.etBOOL; break;
	case Element::DSS_DWM: desc.DepthStencilState.DepthWriteMask = (D3D12_DEPTH_WRITE_MASK)ev.etM3D_DEPTH_WRITE_MASK; break;
	case Element::DSS_DCF: desc.DepthStencilState.DepthFunc = (D3D12_COMPARISON_FUNC)ev.etM3D_COMPARISON_FUNC; break;
	case Element::DSS_SE: desc.DepthStencilState.StencilEnable = ev.etBOOL; break;
	case Element::DSS_SRM: desc.DepthStencilState.StencilReadMask = ev.etUINT8; break;
	case Element::DSS_SWM: desc.DepthStencilState.StencilWriteMask = ev.etUINT8; break;
	case Element::DSS_FF_SFO: desc.DepthStencilState.FrontFace.StencilFailOp = (D3D12_STENCIL_OP)ev.etM3D_STENCIL_OP; break;
	case Element::DSS_FF_SDFO: desc.DepthStencilState.FrontFace.StencilDepthFailOp = (D3D12_STENCIL_OP)ev.etM3D_STENCIL_OP; break;
	case Element::DSS_FF_SPO: desc.DepthStencilState.FrontFace.StencilPassOp = (D3D12_STENCIL_OP)ev.etM3D_STENCIL_OP; break;
	case Element::DSS_FF_SF:  desc.DepthStencilState.FrontFace.StencilFunc = (D3D12_COMPARISON_FUNC)ev.etM3D_COMPARISON_FUNC; break;
	case Element::DSS_BF_SFO: desc.DepthStencilState.BackFace.StencilFailOp = (D3D12_STENCIL_OP)ev.etM3D_STENCIL_OP; break;
	case Element::DSS_BF_SDFO: desc.DepthStencilState.BackFace.StencilDepthFailOp = (D3D12_STENCIL_OP)ev.etM3D_STENCIL_OP; break;
	case Element::DSS_BF_SPO: desc.DepthStencilState.BackFace.StencilPassOp = (D3D12_STENCIL_OP)ev.etM3D_STENCIL_OP; break;
	case Element::DSS_BF_SF:  desc.DepthStencilState.BackFace.StencilFunc = (D3D12_COMPARISON_FUNC)ev.etM3D_COMPARISON_FUNC; break;
	case Element::BS_ATOCE: desc.BlendState.AlphaToCoverageEnable = ev.etBOOL; break;
	case Element::BS_IBE: desc.BlendState.IndependentBlendEnable = ev.etBOOL; break;
	case Element::BS_SM: desc.SampleMask = ev.etUINT; break;
	case Element::BS_RT0_BE: desc.BlendState.RenderTarget[0].BlendEnable = ev.etBOOL; break;
	case Element::BS_RT0_LOE: desc.BlendState.RenderTarget[0].LogicOpEnable = ev.etBOOL; break;
	case Element::BS_RT0_SB: desc.BlendState.RenderTarget[0].SrcBlend = (D3D12_BLEND)ev.etM3D_BLEND; break;
	case Element::BS_RT0_DB: desc.BlendState.RenderTarget[0].DestBlend = (D3D12_BLEND)ev.etM3D_BLEND; break;
	case Element::BS_RT0_BO: desc.BlendState.RenderTarget[0].BlendOp = (D3D12_BLEND_OP)ev.etM3D_BLEND_OP; break;
	case Element::BS_RT0_SBA: desc.BlendState.RenderTarget[0].SrcBlendAlpha = (D3D12_BLEND)ev.etM3D_BLEND; break;
	case Element::BS_RT0_DBA: desc.BlendState.RenderTarget[0].DestBlendAlpha = (D3D12_BLEND)ev.etM3D_BLEND; break;
	case Element::BS_RT0_BOA: desc.BlendState.RenderTarget[0].BlendOpAlpha = (D3D12_BLEND_OP)ev.etM3D_BLEND_OP; break;
	case Element::BS_RT0_LO: desc.BlendState.RenderTarget[0].LogicOp = (D3D12_LOGIC_OP)ev.etM3D_LOGIC_OP; break;
	case Element::BS_RT0_RTWM: desc.BlendState.RenderTarget[0].RenderTargetWriteMask = ev.etUINT8; break;
	case Element::BS_RT1_BE: desc.BlendState.RenderTarget[1].BlendEnable = ev.etBOOL; break;
	case Element::BS_RT1_LOE: desc.BlendState.RenderTarget[1].LogicOpEnable = ev.etBOOL; break;
	case Element::BS_RT1_SB: desc.BlendState.RenderTarget[1].SrcBlend = (D3D12_BLEND)ev.etM3D_BLEND; break;
	case Element::BS_RT1_DB: desc.BlendState.RenderTarget[1].DestBlend = (D3D12_BLEND)ev.etM3D_BLEND; break;
	case Element::BS_RT1_BO: desc.BlendState.RenderTarget[1].BlendOp = (D3D12_BLEND_OP)ev.etM3D_BLEND_OP; break;
	case Element::BS_RT1_SBA: desc.BlendState.RenderTarget[1].SrcBlendAlpha = (D3D12_BLEND)ev.etM3D_BLEND; break;
	case Element::BS_RT1_DBA: desc.BlendState.RenderTarget[1].DestBlendAlpha = (D3D12_BLEND)ev.etM3D_BLEND; break;
	case Element::BS_RT1_BOA: desc.BlendState.RenderTarget[1].BlendOpAlpha = (D3D12_BLEND_OP)ev.etM3D_BLEND_OP; break;
	case Element::BS_RT1_LO: desc.BlendState.RenderTarget[1].LogicOp = (D3D12_LOGIC_OP)ev.etM3D_LOGIC_OP; break;
	case Element::BS_RT1_RTWM: desc.BlendState.RenderTarget[1].RenderTargetWriteMask = ev.etUINT8; break;
	case Element::BS_RT2_BE: desc.BlendState.RenderTarget[2].BlendEnable = ev.etBOOL; break;
	case Element::BS_RT2_LOE: desc.BlendState.RenderTarget[2].LogicOpEnable = ev.etBOOL; break;
	case Element::BS_RT2_SB: desc.BlendState.RenderTarget[2].SrcBlend = (D3D12_BLEND)ev.etM3D_BLEND; break;
	case Element::BS_RT2_DB: desc.BlendState.RenderTarget[2].DestBlend = (D3D12_BLEND)ev.etM3D_BLEND; break;
	case Element::BS_RT2_BO: desc.BlendState.RenderTarget[2].BlendOp = (D3D12_BLEND_OP)ev.etM3D_BLEND_OP; break;
	case Element::BS_RT2_SBA: desc.BlendState.RenderTarget[2].SrcBlendAlpha = (D3D12_BLEND)ev.etM3D_BLEND; break;
	case Element::BS_RT2_DBA: desc.BlendState.RenderTarget[2].DestBlendAlpha = (D3D12_BLEND)ev.etM3D_BLEND; break;
	case Element::BS_RT2_BOA: desc.BlendState.RenderTarget[2].BlendOpAlpha = (D3D12_BLEND_OP)ev.etM3D_BLEND_OP; break;
	case Element::BS_RT2_LO: desc.BlendState.RenderTarget[2].LogicOp = (D3D12_LOGIC_OP)ev.etM3D_LOGIC_OP; break;
	case Element::BS_RT2_RTWM: desc.BlendState.RenderTarget[2].RenderTargetWriteMask = ev.etUINT8; break;
	case Element::BS_RT3_BE: desc.BlendState.RenderTarget[3].BlendEnable = ev.etBOOL; break;
	case Element::BS_RT3_LOE: desc.BlendState.RenderTarget[3].LogicOpEnable = ev.etBOOL; break;
	case Element::BS_RT3_SB: desc.BlendState.RenderTarget[3].SrcBlend = (D3D12_BLEND)ev.etM3D_BLEND; break;
	case Element::BS_RT3_DB: desc.BlendState.RenderTarget[3].DestBlend = (D3D12_BLEND)ev.etM3D_BLEND; break;
	case Element::BS_RT3_BO: desc.BlendState.RenderTarget[3].BlendOp = (D3D12_BLEND_OP)ev.etM3D_BLEND_OP; break;
	case Element::BS_RT3_SBA: desc.BlendState.RenderTarget[3].SrcBlendAlpha = (D3D12_BLEND)ev.etM3D_BLEND; break;
	case Element::BS_RT3_DBA: desc.BlendState.RenderTarget[3].DestBlendAlpha = (D3D12_BLEND)ev.etM3D_BLEND; break;
	case Element::BS_RT3_BOA: desc.BlendState.RenderTarget[3].BlendOpAlpha = (D3D12_BLEND_OP)ev.etM3D_BLEND_OP; break;
	case Element::BS_RT3_LO: desc.BlendState.RenderTarget[3].LogicOp = (D3D12_LOGIC_OP)ev.etM3D_LOGIC_OP; break;
	case Element::BS_RT3_RTWM: desc.BlendState.RenderTarget[3].RenderTargetWriteMask = ev.etUINT8; break;
	case Element::BS_RT4_BE: desc.BlendState.RenderTarget[4].BlendEnable = ev.etBOOL; break;
	case Element::BS_RT4_LOE: desc.BlendState.RenderTarget[4].LogicOpEnable = ev.etBOOL; break;
	case Element::BS_RT4_SB: desc.BlendState.RenderTarget[4].SrcBlend = (D3D12_BLEND)ev.etM3D_BLEND; break;
	case Element::BS_RT4_DB: desc.BlendState.RenderTarget[4].DestBlend = (D3D12_BLEND)ev.etM3D_BLEND; break;
	case Element::BS_RT4_BO: desc.BlendState.RenderTarget[4].BlendOp = (D3D12_BLEND_OP)ev.etM3D_BLEND_OP; break;
	case Element::BS_RT4_SBA: desc.BlendState.RenderTarget[4].SrcBlendAlpha = (D3D12_BLEND)ev.etM3D_BLEND; break;
	case Element::BS_RT4_DBA: desc.BlendState.RenderTarget[4].DestBlendAlpha = (D3D12_BLEND)ev.etM3D_BLEND; break;
	case Element::BS_RT4_BOA: desc.BlendState.RenderTarget[4].BlendOpAlpha = (D3D12_BLEND_OP)ev.etM3D_BLEND_OP; break;
	case Element::BS_RT4_LO: desc.BlendState.RenderTarget[4].LogicOp = (D3D12_LOGIC_OP)ev.etM3D_LOGIC_OP; break;
	case Element::BS_RT4_RTWM: desc.BlendState.RenderTarget[4].RenderTargetWriteMask = ev.etUINT8; break;
	case Element::BS_RT5_BE: desc.BlendState.RenderTarget[5].BlendEnable = ev.etBOOL; break;
	case Element::BS_RT5_LOE: desc.BlendState.RenderTarget[5].LogicOpEnable = ev.etBOOL; break;
	case Element::BS_RT5_SB: desc.BlendState.RenderTarget[5].SrcBlend = (D3D12_BLEND)ev.etM3D_BLEND; break;
	case Element::BS_RT5_DB: desc.BlendState.RenderTarget[5].DestBlend = (D3D12_BLEND)ev.etM3D_BLEND; break;
	case Element::BS_RT5_BO: desc.BlendState.RenderTarget[5].BlendOp = (D3D12_BLEND_OP)ev.etM3D_BLEND_OP; break;
	case Element::BS_RT5_SBA: desc.BlendState.RenderTarget[5].SrcBlendAlpha = (D3D12_BLEND)ev.etM3D_BLEND; break;
	case Element::BS_RT5_DBA: desc.BlendState.RenderTarget[5].DestBlendAlpha = (D3D12_BLEND)ev.etM3D_BLEND; break;
	case Element::BS_RT5_BOA: desc.BlendState.RenderTarget[5].BlendOpAlpha = (D3D12_BLEND_OP)ev.etM3D_BLEND_OP; break;
	case Element::BS_RT5_LO: desc.BlendState.RenderTarget[5].LogicOp = (D3D12_LOGIC_OP)ev.etM3D_LOGIC_OP; break;
	case Element::BS_RT5_RTWM: desc.BlendState.RenderTarget[5].RenderTargetWriteMask = ev.etUINT8; break;
	case Element::BS_RT6_BE: desc.BlendState.RenderTarget[6].BlendEnable = ev.etBOOL; break;
	case Element::BS_RT6_LOE: desc.BlendState.RenderTarget[6].LogicOpEnable = ev.etBOOL; break;
	case Element::BS_RT6_SB: desc.BlendState.RenderTarget[6].SrcBlend = (D3D12_BLEND)ev.etM3D_BLEND; break;
	case Element::BS_RT6_DB: desc.BlendState.RenderTarget[6].DestBlend = (D3D12_BLEND)ev.etM3D_BLEND; break;
	case Element::BS_RT6_BO: desc.BlendState.RenderTarget[6].BlendOp = (D3D12_BLEND_OP)ev.etM3D_BLEND_OP; break;
	case Element::BS_RT6_SBA: desc.BlendState.RenderTarget[6].SrcBlendAlpha = (D3D12_BLEND)ev.etM3D_BLEND; break;
	case Element::BS_RT6_DBA: desc.BlendState.RenderTarget[6].DestBlendAlpha = (D3D12_BLEND)ev.etM3D_BLEND; break;
	case Element::BS_RT6_BOA: desc.BlendState.RenderTarget[6].BlendOpAlpha = (D3D12_BLEND_OP)ev.etM3D_BLEND_OP; break;
	case Element::BS_RT6_LO: desc.BlendState.RenderTarget[6].LogicOp = (D3D12_LOGIC_OP)ev.etM3D_LOGIC_OP; break;
	case Element::BS_RT6_RTWM: desc.BlendState.RenderTarget[6].RenderTargetWriteMask = ev.etUINT8; break;
	case Element::BS_RT7_BE: desc.BlendState.RenderTarget[7].BlendEnable = ev.etBOOL; break;
	case Element::BS_RT7_LOE: desc.BlendState.RenderTarget[7].LogicOpEnable = ev.etBOOL; break;
	case Element::BS_RT7_SB: desc.BlendState.RenderTarget[7].SrcBlend = (D3D12_BLEND)ev.etM3D_BLEND; break;
	case Element::BS_RT7_DB: desc.BlendState.RenderTarget[7].DestBlend = (D3D12_BLEND)ev.etM3D_BLEND; break;
	case Element::BS_RT7_BO: desc.BlendState.RenderTarget[7].BlendOp = (D3D12_BLEND_OP)ev.etM3D_BLEND_OP; break;
	case Element::BS_RT7_SBA: desc.BlendState.RenderTarget[7].SrcBlendAlpha = (D3D12_BLEND)ev.etM3D_BLEND; break;
	case Element::BS_RT7_DBA: desc.BlendState.RenderTarget[7].DestBlendAlpha = (D3D12_BLEND)ev.etM3D_BLEND; break;
	case Element::BS_RT7_BOA: desc.BlendState.RenderTarget[7].BlendOpAlpha = (D3D12_BLEND_OP)ev.etM3D_BLEND_OP; break;
	case Element::BS_RT7_LO: desc.BlendState.RenderTarget[7].LogicOp = (D3D12_LOGIC_OP)ev.etM3D_LOGIC_OP; break;
	case Element::BS_RT7_RTWM: desc.BlendState.RenderTarget[7].RenderTargetWriteMask = ev.etUINT8; break;
	}
}

GraphicsState::StateElementMap _getDefaultStates()
{
	PipelineStateDesc desc;

	GraphicsState::StateElementMap m;

	m[GraphicsState::Element::RS_FM] = desc.RasterizerState.FillMode;
	m[GraphicsState::Element::RS_CM] = desc.RasterizerState.CullMode;
	m[GraphicsState::Element::RS_FCC] = desc.RasterizerState.FrontCounterClockwise;
	m[GraphicsState::Element::RS_DB] = desc.RasterizerState.DepthBias;
	m[GraphicsState::Element::RS_DBC] = desc.RasterizerState.DepthBiasClamp;
	m[GraphicsState::Element::RS_SSDB] = desc.RasterizerState.SlopeScaledDepthBias;
	m[GraphicsState::Element::RS_DCE] = desc.RasterizerState.DepthClipEnable;
	m[GraphicsState::Element::RS_MSE] = desc.RasterizerState.MultisampleEnable;
	m[GraphicsState::Element::RS_AALE] = desc.RasterizerState.AntialiasedLineEnable;
	m[GraphicsState::Element::RS_FSC] = desc.RasterizerState.ForcedSampleCount;
	m[GraphicsState::Element::RS_CR] = desc.RasterizerState.ConservativeRaster;

	m[GraphicsState::Element::DSS_DE] = desc.DepthStencilState.DepthEnable;
	m[GraphicsState::Element::DSS_DWM] = desc.DepthStencilState.DepthWriteMask;
	m[GraphicsState::Element::DSS_DCF] = desc.DepthStencilState.DepthFunc;
	m[GraphicsState::Element::DSS_SE] = desc.DepthStencilState.StencilEnable;
	m[GraphicsState::Element::DSS_SRM] = desc.DepthStencilState.StencilReadMask;
	m[GraphicsState::Element::DSS_SWM] = desc.DepthStencilState.StencilWriteMask;
	m[GraphicsState::Element::DSS_FF_SFO] = desc.DepthStencilState.FrontFace.StencilFailOp;
	m[GraphicsState::Element::DSS_FF_SDFO] = desc.DepthStencilState.FrontFace.StencilDepthFailOp;
	m[GraphicsState::Element::DSS_FF_SPO] = desc.DepthStencilState.FrontFace.StencilPassOp;
	m[GraphicsState::Element::DSS_FF_SF] = desc.DepthStencilState.FrontFace.StencilFunc;
	m[GraphicsState::Element::DSS_BF_SFO] = desc.DepthStencilState.BackFace.StencilFailOp;
	m[GraphicsState::Element::DSS_BF_SDFO] = desc.DepthStencilState.BackFace.StencilDepthFailOp;
	m[GraphicsState::Element::DSS_BF_SPO] = desc.DepthStencilState.BackFace.StencilPassOp;
	m[GraphicsState::Element::DSS_BF_SF] = desc.DepthStencilState.BackFace.StencilFunc;

	m[GraphicsState::Element::BS_ATOCE] = desc.BlendState.AlphaToCoverageEnable;
	m[GraphicsState::Element::BS_IBE] = desc.BlendState.IndependentBlendEnable;
	m[GraphicsState::Element::BS_SM] = desc.SampleMask;

	m[GraphicsState::Element::BS_RT0_BE] = desc.BlendState.RenderTarget[0].BlendEnable;
	m[GraphicsState::Element::BS_RT0_LOE] = desc.BlendState.RenderTarget[0].LogicOpEnable;
	m[GraphicsState::Element::BS_RT0_SB] = desc.BlendState.RenderTarget[0].SrcBlend;
	m[GraphicsState::Element::BS_RT0_DB] = desc.BlendState.RenderTarget[0].DestBlend;
	m[GraphicsState::Element::BS_RT0_BO] = desc.BlendState.RenderTarget[0].BlendOp;
	m[GraphicsState::Element::BS_RT0_SBA] = desc.BlendState.RenderTarget[0].SrcBlendAlpha;
	m[GraphicsState::Element::BS_RT0_DBA] = desc.BlendState.RenderTarget[0].DestBlendAlpha;
	m[GraphicsState::Element::BS_RT0_BOA] = desc.BlendState.RenderTarget[0].BlendOpAlpha;
	m[GraphicsState::Element::BS_RT0_LO] = desc.BlendState.RenderTarget[0].LogicOp;
	m[GraphicsState::Element::BS_RT0_RTWM] = desc.BlendState.RenderTarget[0].RenderTargetWriteMask;

	m[GraphicsState::Element::BS_RT1_BE] = desc.BlendState.RenderTarget[1].BlendEnable;
	m[GraphicsState::Element::BS_RT1_LOE] = desc.BlendState.RenderTarget[1].LogicOpEnable;
	m[GraphicsState::Element::BS_RT1_SB] = desc.BlendState.RenderTarget[1].SrcBlend;
	m[GraphicsState::Element::BS_RT1_DB] = desc.BlendState.RenderTarget[1].DestBlend;
	m[GraphicsState::Element::BS_RT1_BO] = desc.BlendState.RenderTarget[1].BlendOp;
	m[GraphicsState::Element::BS_RT1_SBA] = desc.BlendState.RenderTarget[1].SrcBlendAlpha;
	m[GraphicsState::Element::BS_RT1_DBA] = desc.BlendState.RenderTarget[1].DestBlendAlpha;
	m[GraphicsState::Element::BS_RT1_BOA] = desc.BlendState.RenderTarget[1].BlendOpAlpha;
	m[GraphicsState::Element::BS_RT1_LO] = desc.BlendState.RenderTarget[1].LogicOp;
	m[GraphicsState::Element::BS_RT1_RTWM] = desc.BlendState.RenderTarget[1].RenderTargetWriteMask;

	m[GraphicsState::Element::BS_RT2_BE] = desc.BlendState.RenderTarget[2].BlendEnable;
	m[GraphicsState::Element::BS_RT2_LOE] = desc.BlendState.RenderTarget[2].LogicOpEnable;
	m[GraphicsState::Element::BS_RT2_SB] = desc.BlendState.RenderTarget[2].SrcBlend;
	m[GraphicsState::Element::BS_RT2_DB] = desc.BlendState.RenderTarget[2].DestBlend;
	m[GraphicsState::Element::BS_RT2_BO] = desc.BlendState.RenderTarget[2].BlendOp;
	m[GraphicsState::Element::BS_RT2_SBA] = desc.BlendState.RenderTarget[2].SrcBlendAlpha;
	m[GraphicsState::Element::BS_RT2_DBA] = desc.BlendState.RenderTarget[2].DestBlendAlpha;
	m[GraphicsState::Element::BS_RT2_BOA] = desc.BlendState.RenderTarget[2].BlendOpAlpha;
	m[GraphicsState::Element::BS_RT2_LO] = desc.BlendState.RenderTarget[2].LogicOp;
	m[GraphicsState::Element::BS_RT2_RTWM] = desc.BlendState.RenderTarget[2].RenderTargetWriteMask;

	m[GraphicsState::Element::BS_RT3_BE] = desc.BlendState.RenderTarget[3].BlendEnable;
	m[GraphicsState::Element::BS_RT3_LOE] = desc.BlendState.RenderTarget[3].LogicOpEnable;
	m[GraphicsState::Element::BS_RT3_SB] = desc.BlendState.RenderTarget[3].SrcBlend;
	m[GraphicsState::Element::BS_RT3_DB] = desc.BlendState.RenderTarget[3].DestBlend;
	m[GraphicsState::Element::BS_RT3_BO] = desc.BlendState.RenderTarget[3].BlendOp;
	m[GraphicsState::Element::BS_RT3_SBA] = desc.BlendState.RenderTarget[3].SrcBlendAlpha;
	m[GraphicsState::Element::BS_RT3_DBA] = desc.BlendState.RenderTarget[3].DestBlendAlpha;
	m[GraphicsState::Element::BS_RT3_BOA] = desc.BlendState.RenderTarget[3].BlendOpAlpha;
	m[GraphicsState::Element::BS_RT3_LO] = desc.BlendState.RenderTarget[3].LogicOp;
	m[GraphicsState::Element::BS_RT3_RTWM] = desc.BlendState.RenderTarget[3].RenderTargetWriteMask;

	m[GraphicsState::Element::BS_RT4_BE] = desc.BlendState.RenderTarget[4].BlendEnable;
	m[GraphicsState::Element::BS_RT4_LOE] = desc.BlendState.RenderTarget[4].LogicOpEnable;
	m[GraphicsState::Element::BS_RT4_SB] = desc.BlendState.RenderTarget[4].SrcBlend;
	m[GraphicsState::Element::BS_RT4_DB] = desc.BlendState.RenderTarget[4].DestBlend;
	m[GraphicsState::Element::BS_RT4_BO] = desc.BlendState.RenderTarget[4].BlendOp;
	m[GraphicsState::Element::BS_RT4_SBA] = desc.BlendState.RenderTarget[4].SrcBlendAlpha;
	m[GraphicsState::Element::BS_RT4_DBA] = desc.BlendState.RenderTarget[4].DestBlendAlpha;
	m[GraphicsState::Element::BS_RT4_BOA] = desc.BlendState.RenderTarget[4].BlendOpAlpha;
	m[GraphicsState::Element::BS_RT4_LO] = desc.BlendState.RenderTarget[4].LogicOp;
	m[GraphicsState::Element::BS_RT4_RTWM] = desc.BlendState.RenderTarget[4].RenderTargetWriteMask;

	m[GraphicsState::Element::BS_RT5_BE] = desc.BlendState.RenderTarget[5].BlendEnable;
	m[GraphicsState::Element::BS_RT5_LOE] = desc.BlendState.RenderTarget[5].LogicOpEnable;
	m[GraphicsState::Element::BS_RT5_SB] = desc.BlendState.RenderTarget[5].SrcBlend;
	m[GraphicsState::Element::BS_RT5_DB] = desc.BlendState.RenderTarget[5].DestBlend;
	m[GraphicsState::Element::BS_RT5_BO] = desc.BlendState.RenderTarget[5].BlendOp;
	m[GraphicsState::Element::BS_RT5_SBA] = desc.BlendState.RenderTarget[5].SrcBlendAlpha;
	m[GraphicsState::Element::BS_RT5_DBA] = desc.BlendState.RenderTarget[5].DestBlendAlpha;
	m[GraphicsState::Element::BS_RT5_BOA] = desc.BlendState.RenderTarget[5].BlendOpAlpha;
	m[GraphicsState::Element::BS_RT5_LO] = desc.BlendState.RenderTarget[5].LogicOp;
	m[GraphicsState::Element::BS_RT5_RTWM] = desc.BlendState.RenderTarget[5].RenderTargetWriteMask;

	m[GraphicsState::Element::BS_RT6_BE] = desc.BlendState.RenderTarget[6].BlendEnable;
	m[GraphicsState::Element::BS_RT6_LOE] = desc.BlendState.RenderTarget[6].LogicOpEnable;
	m[GraphicsState::Element::BS_RT6_SB] = desc.BlendState.RenderTarget[6].SrcBlend;
	m[GraphicsState::Element::BS_RT6_DB] = desc.BlendState.RenderTarget[6].DestBlend;
	m[GraphicsState::Element::BS_RT6_BO] = desc.BlendState.RenderTarget[6].BlendOp;
	m[GraphicsState::Element::BS_RT6_SBA] = desc.BlendState.RenderTarget[6].SrcBlendAlpha;
	m[GraphicsState::Element::BS_RT6_DBA] = desc.BlendState.RenderTarget[6].DestBlendAlpha;
	m[GraphicsState::Element::BS_RT6_BOA] = desc.BlendState.RenderTarget[6].BlendOpAlpha;
	m[GraphicsState::Element::BS_RT6_LO] = desc.BlendState.RenderTarget[6].LogicOp;
	m[GraphicsState::Element::BS_RT6_RTWM] = desc.BlendState.RenderTarget[6].RenderTargetWriteMask;

	m[GraphicsState::Element::BS_RT7_BE] = desc.BlendState.RenderTarget[7].BlendEnable;
	m[GraphicsState::Element::BS_RT7_LOE] = desc.BlendState.RenderTarget[7].LogicOpEnable;
	m[GraphicsState::Element::BS_RT7_SB] = desc.BlendState.RenderTarget[7].SrcBlend;
	m[GraphicsState::Element::BS_RT7_DB] = desc.BlendState.RenderTarget[7].DestBlend;
	m[GraphicsState::Element::BS_RT7_BO] = desc.BlendState.RenderTarget[7].BlendOp;
	m[GraphicsState::Element::BS_RT7_SBA] = desc.BlendState.RenderTarget[7].SrcBlendAlpha;
	m[GraphicsState::Element::BS_RT7_DBA] = desc.BlendState.RenderTarget[7].DestBlendAlpha;
	m[GraphicsState::Element::BS_RT7_BOA] = desc.BlendState.RenderTarget[7].BlendOpAlpha;
	m[GraphicsState::Element::BS_RT7_LO] = desc.BlendState.RenderTarget[7].LogicOp;
	m[GraphicsState::Element::BS_RT7_RTWM] = desc.BlendState.RenderTarget[7].RenderTargetWriteMask;

	return m;
}

const GraphicsState::StateElementMap& GraphicsState::GetDefaultStateElements() const
{
	static const StateElementMap DefaultStates = _getDefaultStates();
	return DefaultStates;
}