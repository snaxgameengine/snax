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
#include "GraphicsDefines.h"
#include "Graphics.h"
#include "PipelineStatePool.h"
#include "Shaders.h"


namespace m3d
{

static const Guid GRAPHICSSTATE_GUID = { 0xc80642c6, 0x0537, 0x41fe, { 0xa2, 0x8b, 0x16, 0x59, 0x54, 0x8a, 0x98, 0x38 } };


class GRAPHICSCHIPS_API GraphicsState : public Chip, public GraphicsUsage
{
	CHIPDESC_DECL;
public:
	// The different elements of a complete state.
	enum class Element {
		SHADERS, RS_FM, RS_CM, RS_FCC, RS_DB, RS_DBC, RS_SSDB, RS_DCE, RS_MSE, RS_AALE, RS_FSC, RS_CR,
		DSS_DE, DSS_DWM, DSS_DCF, DSS_SE, DSS_SRM, DSS_SWM, DSS_FF_SFO, DSS_FF_SDFO, DSS_FF_SPO, DSS_FF_SF, DSS_BF_SFO, DSS_BF_SDFO, DSS_BF_SPO, DSS_BF_SF,
		BS_ATOCE, BS_IBE, BS_SM,
		BS_RT0_BE, BS_RT0_LOE, BS_RT0_SB, BS_RT0_DB, BS_RT0_BO, BS_RT0_SBA, BS_RT0_DBA, BS_RT0_BOA, BS_RT0_LO, BS_RT0_RTWM,
		BS_RT1_BE, BS_RT1_LOE, BS_RT1_SB, BS_RT1_DB, BS_RT1_BO, BS_RT1_SBA, BS_RT1_DBA, BS_RT1_BOA, BS_RT1_LO, BS_RT1_RTWM,
		BS_RT2_BE, BS_RT2_LOE, BS_RT2_SB, BS_RT2_DB, BS_RT2_BO, BS_RT2_SBA, BS_RT2_DBA, BS_RT2_BOA, BS_RT2_LO, BS_RT2_RTWM,
		BS_RT3_BE, BS_RT3_LOE, BS_RT3_SB, BS_RT3_DB, BS_RT3_BO, BS_RT3_SBA, BS_RT3_DBA, BS_RT3_BOA, BS_RT3_LO, BS_RT3_RTWM,
		BS_RT4_BE, BS_RT4_LOE, BS_RT4_SB, BS_RT4_DB, BS_RT4_BO, BS_RT4_SBA, BS_RT4_DBA, BS_RT4_BOA, BS_RT4_LO, BS_RT4_RTWM,
		BS_RT5_BE, BS_RT5_LOE, BS_RT5_SB, BS_RT5_DB, BS_RT5_BO, BS_RT5_SBA, BS_RT5_DBA, BS_RT5_BOA, BS_RT5_LO, BS_RT5_RTWM,
		BS_RT6_BE, BS_RT6_LOE, BS_RT6_SB, BS_RT6_DB, BS_RT6_BO, BS_RT6_SBA, BS_RT6_DBA, BS_RT6_BOA, BS_RT6_LO, BS_RT6_RTWM,
		BS_RT7_BE, BS_RT7_LOE, BS_RT7_SB, BS_RT7_DB, BS_RT7_BO, BS_RT7_SBA, BS_RT7_DBA, BS_RT7_BOA, BS_RT7_LO, BS_RT7_RTWM, BS_RT_END__
	};

	// Different state types.
	enum class ElementType { INVALID, UINT8, UINT, INT, FLOAT, M3D_FILL_MODE, M3D_CULL_MODE, M3D_CONSERVATIVE_RASTERIZATION_MODE, M3D_DEPTH_WRITE_MASK, M3D_COMPARISON_FUNC, M3D_STENCIL_OP, M3D_BLEND, M3D_BLEND_OP, M3D_LOGIC_OP };

	// The value an element can have as a union.
	struct ElementValue
	{
		ElementType type;
		union
		{
			BOOL etBOOL;
			UINT8 etUINT8;
			UINT etUINT;
			INT etINT;
			FLOAT etFLOAT;
			M3D_FILL_MODE etM3D_FILL_MODE;
			M3D_CULL_MODE etM3D_CULL_MODE;
			M3D_CONSERVATIVE_RASTERIZATION_MODE etM3D_CONSERVATIVE_RASTERIZATION_MODE;
			M3D_DEPTH_WRITE_MASK etM3D_DEPTH_WRITE_MASK;
			M3D_COMPARISON_FUNC etM3D_COMPARISON_FUNC;
			M3D_STENCIL_OP etM3D_STENCIL_OP;
			M3D_BLEND etM3D_BLEND;
			M3D_BLEND_OP etM3D_BLEND_OP;
			M3D_LOGIC_OP etM3D_LOGIC_OP;
		};

		ElementValue() : type(ElementType::INVALID), etUINT(-1) {}
		ElementValue(UINT8 v) : type(ElementType::UINT8), etUINT8(v) {}
		ElementValue(UINT v) : type(ElementType::UINT), etUINT(v) {}
		ElementValue(INT v) : type(ElementType::INT), etINT(v) {} // also for BOOL
		ElementValue(FLOAT v) : type(ElementType::FLOAT), etFLOAT(v) {}
		ElementValue(M3D_FILL_MODE v) : type(ElementType::M3D_FILL_MODE), etM3D_FILL_MODE(v) {}
		ElementValue(M3D_CULL_MODE v) : type(ElementType::M3D_CULL_MODE), etM3D_CULL_MODE(v) {}
		ElementValue(M3D_CONSERVATIVE_RASTERIZATION_MODE v) : type(ElementType::M3D_CONSERVATIVE_RASTERIZATION_MODE), etM3D_CONSERVATIVE_RASTERIZATION_MODE(v) {}
		ElementValue(M3D_DEPTH_WRITE_MASK v) : type(ElementType::M3D_DEPTH_WRITE_MASK), etM3D_DEPTH_WRITE_MASK(v) {}
		ElementValue(M3D_COMPARISON_FUNC v) : type(ElementType::M3D_COMPARISON_FUNC), etM3D_COMPARISON_FUNC(v) {}
		ElementValue(M3D_STENCIL_OP v) : type(ElementType::M3D_STENCIL_OP), etM3D_STENCIL_OP(v) {}
		ElementValue(M3D_BLEND v) : type(ElementType::M3D_BLEND), etM3D_BLEND(v) {}
		ElementValue(M3D_BLEND_OP v) : type(ElementType::M3D_BLEND_OP), etM3D_BLEND_OP(v) {}

		ElementValue(M3D_LOGIC_OP v) : type(ElementType::M3D_LOGIC_OP), etM3D_LOGIC_OP(v) {}

		bool operator==(const ElementValue& rhs) const
		{
			if (type != rhs.type)
				return false;
			switch (type)
			{
			case ElementType::INVALID: return true;
			case ElementType::UINT8: return etUINT8 == rhs.etUINT8;
			case ElementType::UINT: return etUINT == rhs.etUINT;
			case ElementType::INT: return etINT == rhs.etINT;
			case ElementType::FLOAT: return etFLOAT == rhs.etFLOAT;
			case ElementType::M3D_FILL_MODE: return etM3D_FILL_MODE == rhs.etM3D_FILL_MODE;
			case ElementType::M3D_CULL_MODE: return etM3D_CULL_MODE == rhs.etM3D_CULL_MODE;
			case ElementType::M3D_CONSERVATIVE_RASTERIZATION_MODE: return etM3D_CONSERVATIVE_RASTERIZATION_MODE == rhs.etM3D_CONSERVATIVE_RASTERIZATION_MODE;
			case ElementType::M3D_DEPTH_WRITE_MASK: return etM3D_DEPTH_WRITE_MASK == rhs.etM3D_DEPTH_WRITE_MASK;
			case ElementType::M3D_COMPARISON_FUNC: return etM3D_COMPARISON_FUNC == rhs.etM3D_COMPARISON_FUNC;
			case ElementType::M3D_STENCIL_OP: return etM3D_STENCIL_OP == rhs.etM3D_STENCIL_OP;
			case ElementType::M3D_BLEND: return etM3D_BLEND == rhs.etM3D_BLEND;
			case ElementType::M3D_BLEND_OP: return etM3D_BLEND_OP == rhs.etM3D_BLEND_OP;
			case ElementType::M3D_LOGIC_OP: return etM3D_LOGIC_OP == rhs.etM3D_LOGIC_OP;
			}
			return false;
		}
	};

	// A Map from a state element to its value.
	typedef Map<Element, ElementValue> StateElementMap;


	GraphicsState();
	virtual ~GraphicsState();

	virtual bool CopyChip(Chip* chip) override;
	virtual bool LoadChip(DocumentLoader& loader) override;
	virtual bool SaveChip(DocumentSaver& saver) const override;

	// Gets the state elements set for this chip.
	virtual const StateElementMap& GetStateElements() const { return _elements; }
	// Set state elements.
	virtual void SetStateElements(const StateElementMap& elements);
	
	// TEMPORARY: For GraphicsState_Dlg
	// Returns the default state elements. All should be in this map! (For use by dialog mainly)
	const StateElementMap& GetDefaultStateElements() const;
	int32 GetCurrentStateID() const { return (int32)_psoID; }

	// Calling the chip will set the state!
	virtual void CallChip() override;

	// Updates id for root signature nad pipeline state.
	virtual void UpdateChip();

	// Gets the root signnature id. May have been inherited!
	virtual RootSignatureID GetRootSignatureID();
	// Gets the pipeline state desc id. This will be constructed from data collected in UpdateChip().
	virtual PipelineStateDescID GetPipelineStateID();
	// Checks if the pipeline state is is created!
	virtual bool HasPipelineStateID() const { return _psoID != 0; }


protected:
	StateElementMap _elements;

	bool _isInit;

	StateElementMap _inheritedElements;
	bool _hasShaders;
	ShaderPipelineDesc _shaders;

	// Current pipeline state id.
	PipelineStateDescID _psoID;
	// Current root signature id.
	RootSignatureID _rootSignatureID;

	void _fill(const StateElementMap& elements, PipelineStateDesc& desc);
	void _fill(Element e, const ElementValue& ev, PipelineStateDesc& desc); 
};



}