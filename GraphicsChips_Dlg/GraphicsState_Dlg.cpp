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

#include "stdafx.h"
#include "GraphicsState_Dlg.h"
#include <qplaintextedit.h>
#include <d3dcompiler.h>


using namespace m3d;


DIALOGDESC_DEF(GraphicsState_Dlg, GRAPHICSSTATE_GUID);




GraphicsState_Dlg::GraphicsState_Dlg()
{
	ui.setupUi(this);
	
	// NOTE: Trying to set a font on a combo box seems to give a crash on exit!
//	QFont font1;
//	font1.setFamily(QStringLiteral("Consolas"));
//	ui.comboBox_renderTargetWriteMask->setFont(font1);

	_blockSignals = true;

	_labelFont = _labelFontBold = ui.label_fillMode->font();
	_labelFontBold.setBold(true);

	ui.checkBox_showInherited->setChecked(true);

	ui.comboBox_fillMode->addItem("", -1);
	ui.comboBox_fillMode->addItem("Wireframe", M3D_FILL_MODE_WIREFRAME);
	ui.comboBox_fillMode->addItem("Solid", M3D_FILL_MODE_SOLID);

	ui.comboBox_cullMode->addItem("", -1);
	ui.comboBox_cullMode->addItem("None", M3D_CULL_MODE_NONE);
	ui.comboBox_cullMode->addItem("Front", M3D_CULL_MODE_FRONT);
	ui.comboBox_cullMode->addItem("Back", M3D_CULL_MODE_BACK);

	ui.comboBox_frontCounterClockwise->addItem("", -1);
	ui.comboBox_frontCounterClockwise->addItem("No", FALSE);
	ui.comboBox_frontCounterClockwise->addItem("Yes", TRUE);

	ui.comboBox_depthClipEnable->addItem("", -1);
	ui.comboBox_depthClipEnable->addItem("No", FALSE);
	ui.comboBox_depthClipEnable->addItem("Yes", TRUE);
	
	ui.comboBox_multisampleEnable->addItem("", -1);
	ui.comboBox_multisampleEnable->addItem("No", FALSE);
	ui.comboBox_multisampleEnable->addItem("Yes", TRUE);

	ui.comboBox_antialiasedLineEnable->addItem("", -1);
	ui.comboBox_antialiasedLineEnable->addItem("No", FALSE);
	ui.comboBox_antialiasedLineEnable->addItem("Yes", TRUE);

	ui.comboBox_forcedSampleCount->addItem("", -1);
	ui.comboBox_forcedSampleCount->addItem("0", 0);
	ui.comboBox_forcedSampleCount->addItem("1", 1);
	ui.comboBox_forcedSampleCount->addItem("2", 2);
	ui.comboBox_forcedSampleCount->addItem("4", 4);
	ui.comboBox_forcedSampleCount->addItem("8", 8);
	ui.comboBox_forcedSampleCount->addItem("16", 16);

	ui.comboBox_conservativeRaster->addItem("", -1);
	ui.comboBox_conservativeRaster->addItem("Off", M3D_CONSERVATIVE_RASTERIZATION_MODE_OFF);
	ui.comboBox_conservativeRaster->addItem("On", M3D_CONSERVATIVE_RASTERIZATION_MODE_ON);

	ui.comboBox_depthEnable->addItem("", -1);
	ui.comboBox_depthEnable->addItem("No", FALSE);
	ui.comboBox_depthEnable->addItem("Yes", TRUE);

	ui.comboBox_depthWriteMask->addItem("", -1);
	ui.comboBox_depthWriteMask->addItem("Zero", M3D_DEPTH_WRITE_MASK_ZERO);
	ui.comboBox_depthWriteMask->addItem("All", M3D_DEPTH_WRITE_MASK_ALL);

	ui.comboBox_depthComparisonFunction->addItem("", -1);
	ui.comboBox_depthComparisonFunction->addItem("Never", M3D_COMPARISON_FUNC_NEVER);
	ui.comboBox_depthComparisonFunction->addItem("Less", M3D_COMPARISON_FUNC_LESS);
	ui.comboBox_depthComparisonFunction->addItem("Equal", M3D_COMPARISON_FUNC_EQUAL);
	ui.comboBox_depthComparisonFunction->addItem("Less Equal", M3D_COMPARISON_FUNC_LESS_EQUAL);
	ui.comboBox_depthComparisonFunction->addItem("Greater", M3D_COMPARISON_FUNC_GREATER);
	ui.comboBox_depthComparisonFunction->addItem("Not Equal", M3D_COMPARISON_FUNC_NOT_EQUAL);
	ui.comboBox_depthComparisonFunction->addItem("Greater Equal", M3D_COMPARISON_FUNC_GREATER_EQUAL);
	ui.comboBox_depthComparisonFunction->addItem("Always", M3D_COMPARISON_FUNC_ALWAYS);

	ui.comboBox_stencilEnable->addItem("", -1);
	ui.comboBox_stencilEnable->addItem("No", FALSE);
	ui.comboBox_stencilEnable->addItem("Yes", TRUE);

	ui.comboBox_ff_stencilFailOp->addItem("", -1);
	ui.comboBox_ff_stencilFailOp->addItem("Keep", M3D_STENCIL_OP_KEEP);
	ui.comboBox_ff_stencilFailOp->addItem("Zero", M3D_STENCIL_OP_ZERO);
	ui.comboBox_ff_stencilFailOp->addItem("Replace", M3D_STENCIL_OP_REPLACE);
	ui.comboBox_ff_stencilFailOp->addItem("Increment Saturated", M3D_STENCIL_OP_INCR_SAT);
	ui.comboBox_ff_stencilFailOp->addItem("Decrement Saturated", M3D_STENCIL_OP_DECR_SAT);
	ui.comboBox_ff_stencilFailOp->addItem("Invert", M3D_STENCIL_OP_INVERT);
	ui.comboBox_ff_stencilFailOp->addItem("Increment", M3D_STENCIL_OP_INCR);
	ui.comboBox_ff_stencilFailOp->addItem("Decrement", M3D_STENCIL_OP_DECR);

	ui.comboBox_ff_stencilDepthFailOp->addItem("", -1);
	ui.comboBox_ff_stencilDepthFailOp->addItem("Keep", M3D_STENCIL_OP_KEEP);
	ui.comboBox_ff_stencilDepthFailOp->addItem("Zero", M3D_STENCIL_OP_ZERO);
	ui.comboBox_ff_stencilDepthFailOp->addItem("Replace", M3D_STENCIL_OP_REPLACE);
	ui.comboBox_ff_stencilDepthFailOp->addItem("Increment Saturated", M3D_STENCIL_OP_INCR_SAT);
	ui.comboBox_ff_stencilDepthFailOp->addItem("Decrement Saturated", M3D_STENCIL_OP_DECR_SAT);
	ui.comboBox_ff_stencilDepthFailOp->addItem("Invert", M3D_STENCIL_OP_INVERT);
	ui.comboBox_ff_stencilDepthFailOp->addItem("Increment", M3D_STENCIL_OP_INCR);
	ui.comboBox_ff_stencilDepthFailOp->addItem("Decrement", M3D_STENCIL_OP_DECR);

	ui.comboBox_ff_stencilPassOp->addItem("", -1);
	ui.comboBox_ff_stencilPassOp->addItem("Keep", M3D_STENCIL_OP_KEEP);
	ui.comboBox_ff_stencilPassOp->addItem("Zero", M3D_STENCIL_OP_ZERO);
	ui.comboBox_ff_stencilPassOp->addItem("Replace", M3D_STENCIL_OP_REPLACE);
	ui.comboBox_ff_stencilPassOp->addItem("Increment Saturated", M3D_STENCIL_OP_INCR_SAT);
	ui.comboBox_ff_stencilPassOp->addItem("Decrement Saturated", M3D_STENCIL_OP_DECR_SAT);
	ui.comboBox_ff_stencilPassOp->addItem("Invert", M3D_STENCIL_OP_INVERT);
	ui.comboBox_ff_stencilPassOp->addItem("Increment", M3D_STENCIL_OP_INCR);
	ui.comboBox_ff_stencilPassOp->addItem("Decrement", M3D_STENCIL_OP_DECR);

	ui.comboBox_ff_stencilFunction->addItem("", -1);
	ui.comboBox_ff_stencilFunction->addItem("Never", M3D_COMPARISON_FUNC_NEVER);
	ui.comboBox_ff_stencilFunction->addItem("Less", M3D_COMPARISON_FUNC_LESS);
	ui.comboBox_ff_stencilFunction->addItem("Equal", M3D_COMPARISON_FUNC_EQUAL);
	ui.comboBox_ff_stencilFunction->addItem("Less Equal", M3D_COMPARISON_FUNC_LESS_EQUAL);
	ui.comboBox_ff_stencilFunction->addItem("Greater", M3D_COMPARISON_FUNC_GREATER);
	ui.comboBox_ff_stencilFunction->addItem("Not Equal", M3D_COMPARISON_FUNC_NOT_EQUAL);
	ui.comboBox_ff_stencilFunction->addItem("Greater Equal", M3D_COMPARISON_FUNC_GREATER_EQUAL);
	ui.comboBox_ff_stencilFunction->addItem("Always", M3D_COMPARISON_FUNC_ALWAYS);

	ui.comboBox_bf_stencilFailOp->addItem("", -1);
	ui.comboBox_bf_stencilFailOp->addItem("Keep", M3D_STENCIL_OP_KEEP);
	ui.comboBox_bf_stencilFailOp->addItem("Zero", M3D_STENCIL_OP_ZERO);
	ui.comboBox_bf_stencilFailOp->addItem("Replace", M3D_STENCIL_OP_REPLACE);
	ui.comboBox_bf_stencilFailOp->addItem("Increment Saturated", M3D_STENCIL_OP_INCR_SAT);
	ui.comboBox_bf_stencilFailOp->addItem("Decrement Saturated", M3D_STENCIL_OP_DECR_SAT);
	ui.comboBox_bf_stencilFailOp->addItem("Invert", M3D_STENCIL_OP_INVERT);
	ui.comboBox_bf_stencilFailOp->addItem("Increment", M3D_STENCIL_OP_INCR);
	ui.comboBox_bf_stencilFailOp->addItem("Decrement", M3D_STENCIL_OP_DECR);

	ui.comboBox_bf_stencilDepthFailOp->addItem("", -1);
	ui.comboBox_bf_stencilDepthFailOp->addItem("Keep", M3D_STENCIL_OP_KEEP);
	ui.comboBox_bf_stencilDepthFailOp->addItem("Zero", M3D_STENCIL_OP_ZERO);
	ui.comboBox_bf_stencilDepthFailOp->addItem("Replace", M3D_STENCIL_OP_REPLACE);
	ui.comboBox_bf_stencilDepthFailOp->addItem("Increment Saturated", M3D_STENCIL_OP_INCR_SAT);
	ui.comboBox_bf_stencilDepthFailOp->addItem("Decrement Saturated", M3D_STENCIL_OP_DECR_SAT);
	ui.comboBox_bf_stencilDepthFailOp->addItem("Invert", M3D_STENCIL_OP_INVERT);
	ui.comboBox_bf_stencilDepthFailOp->addItem("Increment", M3D_STENCIL_OP_INCR);
	ui.comboBox_bf_stencilDepthFailOp->addItem("Decrement", M3D_STENCIL_OP_DECR);

	ui.comboBox_bf_stencilPassOp->addItem("", -1);
	ui.comboBox_bf_stencilPassOp->addItem("Keep", M3D_STENCIL_OP_KEEP);
	ui.comboBox_bf_stencilPassOp->addItem("Zero", M3D_STENCIL_OP_ZERO);
	ui.comboBox_bf_stencilPassOp->addItem("Replace", M3D_STENCIL_OP_REPLACE);
	ui.comboBox_bf_stencilPassOp->addItem("Increment Saturated", M3D_STENCIL_OP_INCR_SAT);
	ui.comboBox_bf_stencilPassOp->addItem("Decrement Saturated", M3D_STENCIL_OP_DECR_SAT);
	ui.comboBox_bf_stencilPassOp->addItem("Invert", M3D_STENCIL_OP_INVERT);
	ui.comboBox_bf_stencilPassOp->addItem("Increment", M3D_STENCIL_OP_INCR);
	ui.comboBox_bf_stencilPassOp->addItem("Decrement", M3D_STENCIL_OP_DECR);

	ui.comboBox_bf_stencilFunction->addItem("", -1);
	ui.comboBox_bf_stencilFunction->addItem("Never", M3D_COMPARISON_FUNC_NEVER);
	ui.comboBox_bf_stencilFunction->addItem("Less", M3D_COMPARISON_FUNC_LESS);
	ui.comboBox_bf_stencilFunction->addItem("Equal", M3D_COMPARISON_FUNC_EQUAL);
	ui.comboBox_bf_stencilFunction->addItem("Less Equal", M3D_COMPARISON_FUNC_LESS_EQUAL);
	ui.comboBox_bf_stencilFunction->addItem("Greater", M3D_COMPARISON_FUNC_GREATER);
	ui.comboBox_bf_stencilFunction->addItem("Not Equal", M3D_COMPARISON_FUNC_NOT_EQUAL);
	ui.comboBox_bf_stencilFunction->addItem("Greater Equal", M3D_COMPARISON_FUNC_GREATER_EQUAL);
	ui.comboBox_bf_stencilFunction->addItem("Always", M3D_COMPARISON_FUNC_ALWAYS);

	ui.comboBox_atocEnable->addItem("", -1);
	ui.comboBox_atocEnable->addItem("No", FALSE);
	ui.comboBox_atocEnable->addItem("Yes", TRUE);

	ui.comboBox_independentBlendEnable->addItem("", -1);
	ui.comboBox_independentBlendEnable->addItem("No", FALSE);
	ui.comboBox_independentBlendEnable->addItem("Yes", TRUE);

	ui.comboBox_blendEnable->addItem("", -1);
	ui.comboBox_blendEnable->addItem("No", FALSE);
	ui.comboBox_blendEnable->addItem("Yes", TRUE);

	ui.comboBox_logicOpEnable->addItem("", -1);
	ui.comboBox_logicOpEnable->addItem("No", FALSE);
	ui.comboBox_logicOpEnable->addItem("Yes", TRUE);

	ui.comboBox_sourceBlend->addItem("", -1);
	ui.comboBox_sourceBlend->addItem("Zero", M3D_BLEND_ZERO);
	ui.comboBox_sourceBlend->addItem("One", M3D_BLEND_ONE);
	ui.comboBox_sourceBlend->addItem("Src Color", M3D_BLEND_SRC_COLOR);
	ui.comboBox_sourceBlend->addItem("Inv Src Color", M3D_BLEND_INV_SRC_COLOR);
	ui.comboBox_sourceBlend->addItem("Src Alpha", M3D_BLEND_SRC_ALPHA);
	ui.comboBox_sourceBlend->addItem("Inv Src Alpha", M3D_BLEND_INV_SRC_ALPHA);
	ui.comboBox_sourceBlend->addItem("Dest Alpha", M3D_BLEND_DEST_ALPHA);
	ui.comboBox_sourceBlend->addItem("Inv Dest Alpha", M3D_BLEND_INV_DEST_ALPHA);
	ui.comboBox_sourceBlend->addItem("Dest Color", M3D_BLEND_DEST_COLOR);
	ui.comboBox_sourceBlend->addItem("Inv Dest Color", M3D_BLEND_INV_DEST_COLOR);
	ui.comboBox_sourceBlend->addItem("Src Alpha Sat", M3D_BLEND_SRC_ALPHA_SAT);
	ui.comboBox_sourceBlend->addItem("Blend Factor", M3D_BLEND_BLEND_FACTOR);
	ui.comboBox_sourceBlend->addItem("Inv Blend Factor", M3D_BLEND_INV_BLEND_FACTOR);
	ui.comboBox_sourceBlend->addItem("Src1 Color", M3D_BLEND_SRC1_COLOR);
	ui.comboBox_sourceBlend->addItem("Inv Src1 Color", M3D_BLEND_INV_SRC1_COLOR);
	ui.comboBox_sourceBlend->addItem("Src1 Alpha", M3D_BLEND_SRC1_ALPHA);
	ui.comboBox_sourceBlend->addItem("Inv Src1 alpha", M3D_BLEND_INV_SRC1_ALPHA);

	ui.comboBox_destBlend->addItem("", -1);
	ui.comboBox_destBlend->addItem("Zero", M3D_BLEND_ZERO);
	ui.comboBox_destBlend->addItem("One", M3D_BLEND_ONE);
	ui.comboBox_destBlend->addItem("Src Color", M3D_BLEND_SRC_COLOR);
	ui.comboBox_destBlend->addItem("Inv Src Color", M3D_BLEND_INV_SRC_COLOR);
	ui.comboBox_destBlend->addItem("Src Alpha", M3D_BLEND_SRC_ALPHA);
	ui.comboBox_destBlend->addItem("Inv Src Alpha", M3D_BLEND_INV_SRC_ALPHA);
	ui.comboBox_destBlend->addItem("Dest Alpha", M3D_BLEND_DEST_ALPHA);
	ui.comboBox_destBlend->addItem("Inv Dest Alpha", M3D_BLEND_INV_DEST_ALPHA);
	ui.comboBox_destBlend->addItem("Dest Color", M3D_BLEND_DEST_COLOR);
	ui.comboBox_destBlend->addItem("Inv Dest Color", M3D_BLEND_INV_DEST_COLOR);
	ui.comboBox_destBlend->addItem("Src Alpha Sat", M3D_BLEND_SRC_ALPHA_SAT);
	ui.comboBox_destBlend->addItem("Blend Factor", M3D_BLEND_BLEND_FACTOR);
	ui.comboBox_destBlend->addItem("Inv Blend Factor", M3D_BLEND_INV_BLEND_FACTOR);
	ui.comboBox_destBlend->addItem("Src1 Color", M3D_BLEND_SRC1_COLOR);
	ui.comboBox_destBlend->addItem("Inv Src1 Color", M3D_BLEND_INV_SRC1_COLOR);
	ui.comboBox_destBlend->addItem("Src1 Alpha", M3D_BLEND_SRC1_ALPHA);
	ui.comboBox_destBlend->addItem("Inv Src1 alpha", M3D_BLEND_INV_SRC1_ALPHA);

	ui.comboBox_blendOp->addItem("", -1);
	ui.comboBox_blendOp->addItem("Add", M3D_BLEND_OP_ADD);
	ui.comboBox_blendOp->addItem("Subtract", M3D_BLEND_OP_SUBTRACT);
	ui.comboBox_blendOp->addItem("Rev Subtract", M3D_BLEND_OP_REV_SUBTRACT);
	ui.comboBox_blendOp->addItem("Min", M3D_BLEND_OP_MIN);
	ui.comboBox_blendOp->addItem("Max", M3D_BLEND_OP_MAX);

	ui.comboBox_srcBlendAlpha->addItem("", -1);
	ui.comboBox_srcBlendAlpha->addItem("Zero", M3D_BLEND_ZERO);
	ui.comboBox_srcBlendAlpha->addItem("One", M3D_BLEND_ONE);
	ui.comboBox_srcBlendAlpha->addItem("Src Alpha", M3D_BLEND_SRC_ALPHA);
	ui.comboBox_srcBlendAlpha->addItem("Inv Src Alpha", M3D_BLEND_INV_SRC_ALPHA);
	ui.comboBox_srcBlendAlpha->addItem("Dest Alpha", M3D_BLEND_DEST_ALPHA);
	ui.comboBox_srcBlendAlpha->addItem("Inv Dest Alpha", M3D_BLEND_INV_DEST_ALPHA);
	ui.comboBox_srcBlendAlpha->addItem("Src Alpha Sat", M3D_BLEND_SRC_ALPHA_SAT);
	ui.comboBox_srcBlendAlpha->addItem("Blend Factor", M3D_BLEND_BLEND_FACTOR);
	ui.comboBox_srcBlendAlpha->addItem("Inv Blend Factor", M3D_BLEND_INV_BLEND_FACTOR);
	ui.comboBox_srcBlendAlpha->addItem("Src1 Alpha", M3D_BLEND_SRC1_ALPHA);
	ui.comboBox_srcBlendAlpha->addItem("Inv Src1 alpha", M3D_BLEND_INV_SRC1_ALPHA);

	ui.comboBox_destBlendAlpha->addItem("", -1);
	ui.comboBox_destBlendAlpha->addItem("Zero", M3D_BLEND_ZERO);
	ui.comboBox_destBlendAlpha->addItem("One", M3D_BLEND_ONE);
	ui.comboBox_destBlendAlpha->addItem("Src Alpha", M3D_BLEND_SRC_ALPHA);
	ui.comboBox_destBlendAlpha->addItem("Inv Src Alpha", M3D_BLEND_INV_SRC_ALPHA);
	ui.comboBox_destBlendAlpha->addItem("Dest Alpha", M3D_BLEND_DEST_ALPHA);
	ui.comboBox_destBlendAlpha->addItem("Inv Dest Alpha", M3D_BLEND_INV_DEST_ALPHA);
	ui.comboBox_destBlendAlpha->addItem("Src Alpha Sat", M3D_BLEND_SRC_ALPHA_SAT);
	ui.comboBox_destBlendAlpha->addItem("Blend Factor", M3D_BLEND_BLEND_FACTOR);
	ui.comboBox_destBlendAlpha->addItem("Inv Blend Factor", M3D_BLEND_INV_BLEND_FACTOR);
	ui.comboBox_destBlendAlpha->addItem("Src1 Alpha", M3D_BLEND_SRC1_ALPHA);
	ui.comboBox_destBlendAlpha->addItem("Inv Src1 alpha", M3D_BLEND_INV_SRC1_ALPHA);

	ui.comboBox_blendOpAlpha->addItem("", -1);
	ui.comboBox_blendOpAlpha->addItem("Add", M3D_BLEND_OP_ADD);
	ui.comboBox_blendOpAlpha->addItem("Subtract", M3D_BLEND_OP_SUBTRACT);
	ui.comboBox_blendOpAlpha->addItem("Rev Subtract", M3D_BLEND_OP_REV_SUBTRACT);
	ui.comboBox_blendOpAlpha->addItem("Min", M3D_BLEND_OP_MIN);
	ui.comboBox_blendOpAlpha->addItem("Max", M3D_BLEND_OP_MAX);

	ui.comboBox_logicOp->addItem("", -1);
	ui.comboBox_logicOp->addItem("Clear", M3D_LOGIC_OP_CLEAR);
	ui.comboBox_logicOp->addItem("Set", M3D_LOGIC_OP_SET);
	ui.comboBox_logicOp->addItem("Copy", M3D_LOGIC_OP_COPY);
	ui.comboBox_logicOp->addItem("Copy Inverted", M3D_LOGIC_OP_COPY_INVERTED);
	ui.comboBox_logicOp->addItem("Noop", M3D_LOGIC_OP_NOOP);
	ui.comboBox_logicOp->addItem("Invert", M3D_LOGIC_OP_INVERT);
	ui.comboBox_logicOp->addItem("And", M3D_LOGIC_OP_AND);
	ui.comboBox_logicOp->addItem("Nand", M3D_LOGIC_OP_NAND);
	ui.comboBox_logicOp->addItem("Or", M3D_LOGIC_OP_OR);
	ui.comboBox_logicOp->addItem("Nor", M3D_LOGIC_OP_NOR);
	ui.comboBox_logicOp->addItem("Xor", M3D_LOGIC_OP_XOR);
	ui.comboBox_logicOp->addItem("Equiv", M3D_LOGIC_OP_EQUIV);
	ui.comboBox_logicOp->addItem("Reverse", M3D_LOGIC_OP_AND_REVERSE);
	ui.comboBox_logicOp->addItem("Inverted", M3D_LOGIC_OP_AND_INVERTED);
	ui.comboBox_logicOp->addItem("Reverse", M3D_LOGIC_OP_OR_REVERSE);
	ui.comboBox_logicOp->addItem("Or Inverted", M3D_LOGIC_OP_OR_INVERTED);

	ui.comboBox_renderTargetWriteMask->addItem("", -1);
	ui.comboBox_renderTargetWriteMask->addItem("----", 0);
	ui.comboBox_renderTargetWriteMask->addItem("R---", M3D_COLOR_WRITE_ENABLE_RED);
	ui.comboBox_renderTargetWriteMask->addItem("-G--", M3D_COLOR_WRITE_ENABLE_GREEN);
	ui.comboBox_renderTargetWriteMask->addItem("--B-", M3D_COLOR_WRITE_ENABLE_BLUE);
	ui.comboBox_renderTargetWriteMask->addItem("---A", M3D_COLOR_WRITE_ENABLE_ALPHA);
	ui.comboBox_renderTargetWriteMask->addItem("RG--", M3D_COLOR_WRITE_ENABLE_RED|M3D_COLOR_WRITE_ENABLE_GREEN);
	ui.comboBox_renderTargetWriteMask->addItem("R-B-", M3D_COLOR_WRITE_ENABLE_RED|M3D_COLOR_WRITE_ENABLE_BLUE);
	ui.comboBox_renderTargetWriteMask->addItem("R--A", M3D_COLOR_WRITE_ENABLE_RED|M3D_COLOR_WRITE_ENABLE_ALPHA);
	ui.comboBox_renderTargetWriteMask->addItem("-GB-", M3D_COLOR_WRITE_ENABLE_GREEN|M3D_COLOR_WRITE_ENABLE_BLUE);
	ui.comboBox_renderTargetWriteMask->addItem("-G-A", M3D_COLOR_WRITE_ENABLE_GREEN|M3D_COLOR_WRITE_ENABLE_ALPHA);
	ui.comboBox_renderTargetWriteMask->addItem("--BA", M3D_COLOR_WRITE_ENABLE_BLUE|M3D_COLOR_WRITE_ENABLE_ALPHA);
	ui.comboBox_renderTargetWriteMask->addItem("RGB-", M3D_COLOR_WRITE_ENABLE_RED|M3D_COLOR_WRITE_ENABLE_GREEN|M3D_COLOR_WRITE_ENABLE_BLUE);
	ui.comboBox_renderTargetWriteMask->addItem("RG-A", M3D_COLOR_WRITE_ENABLE_RED|M3D_COLOR_WRITE_ENABLE_GREEN|M3D_COLOR_WRITE_ENABLE_ALPHA);
	ui.comboBox_renderTargetWriteMask->addItem("R-BA", M3D_COLOR_WRITE_ENABLE_RED|M3D_COLOR_WRITE_ENABLE_BLUE|M3D_COLOR_WRITE_ENABLE_ALPHA);
	ui.comboBox_renderTargetWriteMask->addItem("-GBA", M3D_COLOR_WRITE_ENABLE_GREEN|M3D_COLOR_WRITE_ENABLE_BLUE|M3D_COLOR_WRITE_ENABLE_ALPHA);
	ui.comboBox_renderTargetWriteMask->addItem("RGBA", M3D_COLOR_WRITE_ENABLE_ALL);

	connect(ui.buttonGroup_rt, &QButtonGroup::idClicked, this, &GraphicsState_Dlg::onRTChanged);
	
	_psoID = 0;
	_blockSignals = false;
}

GraphicsState_Dlg::~GraphicsState_Dlg()
{
}

void GraphicsState_Dlg::Init()
{
	_initElements = _elements = GetChip()->GetStateElements();
	_defaultElements = GetChip()->GetDefaultStateElements();
	_psoID = GetChip()->GetCurrentStateID();// GetChip()->HasPipelineStateID() ? GetChip()->GetPipelineStateID() : 0;

	_update_ui();
}

void GraphicsState_Dlg::OnCancel()
{
	GetChip()->SetStateElements(_initElements);
}

void GraphicsState_Dlg::AfterApply()
{
	_initElements = _elements = GetChip()->GetStateElements();
}

void GraphicsState_Dlg::Update()
{
	int32 psoID = GetChip()->GetCurrentStateID();// GetChip()->HasPipelineStateID() ? GetChip()->GetPipelineStateID() : 0;
	if (_psoID != psoID) {
		_psoID = psoID;
		if (ui.checkBox_showInherited->isChecked())
			_update_ui();
	}
}

void GraphicsState_Dlg::_setIndex(QLabel *label, QComboBox *cb, const GraphicsState::StateElementMap &m, GraphicsState::Element e)
{
	auto getIdx = [cb](GraphicsState::ElementValue ev) ->int32 {
		int32 idx = -1;
		switch (ev.type)
		{
//		case GraphicsState::ElementType::BOOL: idx = cb->findData(ev.etBOOL ? 1 : 0); break;
		case GraphicsState::ElementType::UINT8: idx = cb->findData(ev.etUINT8); break;
		case GraphicsState::ElementType::UINT: idx = cb->findData(ev.etUINT); break;
		case GraphicsState::ElementType::INT: idx = cb->findData(ev.etINT); break;
		case GraphicsState::ElementType::FLOAT: idx = cb->findData(ev.etFLOAT); break;
		case GraphicsState::ElementType::M3D_FILL_MODE: idx = cb->findData(ev.etM3D_FILL_MODE); break;
		case GraphicsState::ElementType::M3D_CULL_MODE: idx = cb->findData(ev.etM3D_CULL_MODE); break;
		case GraphicsState::ElementType::M3D_CONSERVATIVE_RASTERIZATION_MODE: idx = cb->findData(ev.etM3D_CONSERVATIVE_RASTERIZATION_MODE); break;
		case GraphicsState::ElementType::M3D_DEPTH_WRITE_MASK: idx = cb->findData(ev.etM3D_DEPTH_WRITE_MASK); break;
		case GraphicsState::ElementType::M3D_COMPARISON_FUNC: idx = cb->findData(ev.etM3D_COMPARISON_FUNC); break;
		case GraphicsState::ElementType::M3D_STENCIL_OP: idx = cb->findData(ev.etM3D_STENCIL_OP); break;
		case GraphicsState::ElementType::M3D_BLEND: idx = cb->findData(ev.etM3D_BLEND); break;
		case GraphicsState::ElementType::M3D_BLEND_OP: idx = cb->findData(ev.etM3D_BLEND_OP); break;
		case GraphicsState::ElementType::M3D_LOGIC_OP: idx = cb->findData(ev.etM3D_LOGIC_OP); break;
		}
		return idx;
	};

	int32 idx = -1;
	auto n = m.find(e);
	if (n != m.end()) {
		label->setFont(_labelFontBold); 
		idx = getIdx(n->second);
	}
	else {
		label->setFont(_labelFont);
		if (_psoID != 0 && ui.checkBox_showInherited->isChecked()) {
			n = _defaultElements.find(e);
			idx = getIdx(n->second);
		}
		else
			idx = 0;
	}
	cb->setCurrentIndex(idx);
}

void GraphicsState_Dlg::_setText(QLabel *label, QLineEdit *le, const GraphicsState::StateElementMap &m, GraphicsState::Element e)
{
	auto getStr = [](GraphicsState::ElementValue ev) -> QString {
		QString s;
		switch (ev.type)
		{
		case GraphicsState::ElementType::UINT8: s = "0x" + QString::number((UINT)ev.etUINT8, 16).toUpper(); break;
		case GraphicsState::ElementType::UINT: s = QString::number(ev.etUINT); break;
		case GraphicsState::ElementType::INT: s = QString::number(ev.etINT); break;
		case GraphicsState::ElementType::FLOAT: s = QString::number(ev.etFLOAT); break;
		default: s = "?"; break;
		}
		return s;
	};
	QString s;
	auto n = m.find(e);
	if (n != m.end()) {
		label->setFont(_labelFontBold);
		s = getStr(n->second);
	}
	else {
		label->setFont(_labelFont);
		if (_psoID != 0 && ui.checkBox_showInherited->isChecked()) {
			n = _defaultElements.find(e);
			s = getStr(n->second);
		}
	}
	le->setText(s);

	_texts[le] = s;
}

void GraphicsState_Dlg::_update_ui()
{
	_update_rs_ui();
	_update_dss_ui();
	_update_bs_ui();
}

void GraphicsState_Dlg::_update_rs_ui()
{
	_blockSignals = true;

	_setIndex(ui.label_fillMode, ui.comboBox_fillMode, _elements, GraphicsState::Element::RS_FM);
	_setIndex(ui.label_cullMode, ui.comboBox_cullMode, _elements, GraphicsState::Element::RS_CM);
	_setIndex(ui.label_frontCounterClockwise, ui.comboBox_frontCounterClockwise, _elements, GraphicsState::Element::RS_FCC);
	_setText(ui.label_depthBias, ui.lineEdit_depthBias, _elements, GraphicsState::Element::RS_DB);
	_setText(ui.label_depthBiasClamp, ui.lineEdit_depthBiasClamp, _elements, GraphicsState::Element::RS_DBC);
	_setText(ui.label_slopeScaledDepthBias, ui.lineEdit_slopeScaledDepthBias, _elements, GraphicsState::Element::RS_SSDB);
	_setIndex(ui.label_depthClipEnable, ui.comboBox_depthClipEnable, _elements, GraphicsState::Element::RS_DCE);
	_setIndex(ui.label_multisampleEnable, ui.comboBox_multisampleEnable, _elements, GraphicsState::Element::RS_MSE);
	_setIndex(ui.label_antialiasedLineEnable, ui.comboBox_antialiasedLineEnable, _elements, GraphicsState::Element::RS_AALE);
	_setIndex(ui.label_forcedSampleCount, ui.comboBox_forcedSampleCount, _elements, GraphicsState::Element::RS_FSC);
	_setIndex(ui.label_conservativeRaster, ui.comboBox_conservativeRaster, _elements, GraphicsState::Element::RS_CR);

	_blockSignals = false;
}

void GraphicsState_Dlg::_update_dss_ui()
{
	_blockSignals = true;

	_setIndex(ui.label_depthEnable, ui.comboBox_depthEnable, _elements, GraphicsState::Element::DSS_DE);
	_setIndex(ui.label_depthWriteMask, ui.comboBox_depthWriteMask, _elements, GraphicsState::Element::DSS_DWM);
	_setIndex(ui.label_depthComparisonFunc, ui.comboBox_depthComparisonFunction, _elements, GraphicsState::Element::DSS_DCF);
	_setIndex(ui.label_stencilEnable, ui.comboBox_stencilEnable, _elements, GraphicsState::Element::DSS_SE);
	_setText(ui.label_stencilReadMask, ui.lineEdit_stencilReadMask, _elements, GraphicsState::Element::DSS_SRM);
	_setText(ui.label_stencilWriteMask, ui.lineEdit_stencilWriteMask, _elements, GraphicsState::Element::DSS_SWM);
	_setIndex(ui.label_ff_stencilFailOp, ui.comboBox_ff_stencilFailOp, _elements, GraphicsState::Element::DSS_FF_SFO);
	_setIndex(ui.label_ff_depthFailOp, ui.comboBox_ff_stencilDepthFailOp, _elements, GraphicsState::Element::DSS_FF_SDFO);
	_setIndex(ui.label_ff_stencilPassOp, ui.comboBox_ff_stencilPassOp, _elements, GraphicsState::Element::DSS_FF_SPO);
	_setIndex(ui.label_ff_stencilFunc, ui.comboBox_ff_stencilFunction, _elements, GraphicsState::Element::DSS_FF_SF);
	_setIndex(ui.label_bf_stencilFailOp, ui.comboBox_bf_stencilFailOp, _elements, GraphicsState::Element::DSS_BF_SFO);
	_setIndex(ui.label_bf_stencilDepthFailOp, ui.comboBox_bf_stencilDepthFailOp, _elements, GraphicsState::Element::DSS_BF_SDFO);
	_setIndex(ui.label_bf_stencilPassOp, ui.comboBox_bf_stencilPassOp, _elements, GraphicsState::Element::DSS_BF_SPO);
	_setIndex(ui.label_bf_stencilFunc, ui.comboBox_bf_stencilFunction, _elements, GraphicsState::Element::DSS_BF_SF);

	_blockSignals = false;
}

void GraphicsState_Dlg::_update_bs_ui()
{
	_blockSignals = true;

	_setIndex(ui.label_atocEnable, ui.comboBox_atocEnable, _elements, GraphicsState::Element::BS_ATOCE);
	_setIndex(ui.label_independentBlendEnable, ui.comboBox_independentBlendEnable, _elements, GraphicsState::Element::BS_IBE);
	_setText(ui.label_sampleMask, ui.lineEdit_sampleMask, _elements, GraphicsState::Element::BS_SM);

	if (ui.buttonGroup_rt->checkedButton() == 0)
		ui.radioButton_rt1->setChecked(true);

	int32 rt = -2 - ui.buttonGroup_rt->checkedId();
	int32 c = ((int32)GraphicsState::Element::BS_RT1_BE - (int32)GraphicsState::Element::BS_RT0_BE) * rt;

	_setIndex(ui.label_blendEnable, ui.comboBox_blendEnable, _elements, (GraphicsState::Element)((int32)GraphicsState::Element::BS_RT0_BE + c));
	_setIndex(ui.label_logicOpEnable, ui.comboBox_logicOpEnable, _elements, (GraphicsState::Element)((int32)GraphicsState::Element::BS_RT0_LOE + c));
	_setIndex(ui.label_srcBlend, ui.comboBox_sourceBlend, _elements, (GraphicsState::Element)((int32)GraphicsState::Element::BS_RT0_SB + c));
	_setIndex(ui.label_destBlend, ui.comboBox_destBlend, _elements, (GraphicsState::Element)((int32)GraphicsState::Element::BS_RT0_DB + c));
	_setIndex(ui.label_blendOp, ui.comboBox_blendOp, _elements, (GraphicsState::Element)((int32)GraphicsState::Element::BS_RT0_BO + c));
	_setIndex(ui.label_srcBlendAlpha, ui.comboBox_srcBlendAlpha, _elements, (GraphicsState::Element)((int32)GraphicsState::Element::BS_RT0_SBA + c));
	_setIndex(ui.label_destBlendAlpha, ui.comboBox_destBlendAlpha, _elements, (GraphicsState::Element)((int32)GraphicsState::Element::BS_RT0_DBA + c));
	_setIndex(ui.label_blendOpAlpha, ui.comboBox_blendOpAlpha, _elements, (GraphicsState::Element)((int32)GraphicsState::Element::BS_RT0_BOA + c));
	_setIndex(ui.label_logicOp, ui.comboBox_logicOp, _elements, (GraphicsState::Element)((int32)GraphicsState::Element::BS_RT0_LO + c));
	_setIndex(ui.label_renderTargetWriteMask, ui.comboBox_renderTargetWriteMask, _elements, (GraphicsState::Element)((int32)GraphicsState::Element::BS_RT0_RTWM + c));

	auto isInRange = [](const GraphicsState::StateElementMap& map, GraphicsState::Element a, GraphicsState::Element b)
	{
		auto m = map.lower_bound(a);
		if (m == map.end())
			return false;
		return m->first < b;
	};

	ui.radioButton_rt1->setFont(isInRange(_elements, GraphicsState::Element::BS_RT0_BE, GraphicsState::Element::BS_RT1_BE) ? _labelFontBold : _labelFont);
	ui.radioButton_rt2->setFont(isInRange(_elements, GraphicsState::Element::BS_RT1_BE, GraphicsState::Element::BS_RT2_BE) ? _labelFontBold : _labelFont);
	ui.radioButton_rt3->setFont(isInRange(_elements, GraphicsState::Element::BS_RT2_BE, GraphicsState::Element::BS_RT3_BE) ? _labelFontBold : _labelFont);
	ui.radioButton_rt4->setFont(isInRange(_elements, GraphicsState::Element::BS_RT3_BE, GraphicsState::Element::BS_RT4_BE) ? _labelFontBold : _labelFont);
	ui.radioButton_rt5->setFont(isInRange(_elements, GraphicsState::Element::BS_RT4_BE, GraphicsState::Element::BS_RT5_BE) ? _labelFontBold : _labelFont);
	ui.radioButton_rt6->setFont(isInRange(_elements, GraphicsState::Element::BS_RT5_BE, GraphicsState::Element::BS_RT6_BE) ? _labelFontBold : _labelFont);
	ui.radioButton_rt7->setFont(isInRange(_elements, GraphicsState::Element::BS_RT6_BE, GraphicsState::Element::BS_RT7_BE) ? _labelFontBold : _labelFont);
	ui.radioButton_rt8->setFont(isInRange(_elements, GraphicsState::Element::BS_RT7_BE, GraphicsState::Element::BS_RT_END__) ?  _labelFontBold : _labelFont);

	_blockSignals = false;
}

void GraphicsState_Dlg::_update_chip()
{
	if (GetChip()->GetStateElements() != _elements) {
		GetChip()->SetStateElements(_elements);
		SetDirty();
	}
}

void GraphicsState_Dlg::onRTChanged(int32 id)
{
	_update_bs_ui();
}

template<typename T>
void __set(QComboBox *cb, GraphicsState::Element ET, GraphicsState::StateElementMap &m)
{
	if (cb->currentIndex() > 0)
		m[ET] = (T)cb->currentData().toUInt();
	else
		m.erase(ET);
}


template<typename T>
void __set(QLineEdit *le, GraphicsState::Element ET, GraphicsState::StateElementMap &m) {}

template<>
void __set<UINT8>(QLineEdit *le, GraphicsState::Element ET, GraphicsState::StateElementMap &m) 
{
	int32 base = 10;
	QString s = le->text().trimmed();
	if (s.startsWith("0x"))
		base = 16;
	if (s.isEmpty())
		m.erase(ET);
	else {
		bool ok = false;
		UINT i = (UINT)s.toInt(&ok, base);
		if (ok)
			m[ET] = (UINT8)(i > 255 ? 255 : i);
	}
}

template<>
void __set<INT>(QLineEdit *le, GraphicsState::Element ET, GraphicsState::StateElementMap &m) 
{
	QString s = le->text().trimmed();
	if (s.isEmpty())
		m.erase(ET);
	else {
		bool ok = false;
		INT i = (INT)s.toInt(&ok);
		if (ok)
			m[ET] = i;
	}
}

template<>
void __set<UINT>(QLineEdit *le, GraphicsState::Element ET, GraphicsState::StateElementMap &m) 
{
	int32 base = 10;
	QString s = le->text().trimmed();
	if (s.startsWith("0x"))
		base = 16;
	if (s.isEmpty())
		m.erase(ET);
	else {
		bool ok = false;
		UINT i = (UINT)s.toInt(&ok, base);
		if (ok)
			m[ET] = i;
	}
}

template<>
void __set<FLOAT>(QLineEdit *le, GraphicsState::Element ET, GraphicsState::StateElementMap &m) 
{
	QString s = le->text().trimmed();
	if (s.isEmpty())
		m.erase(ET);
	else {
		bool ok = false;
		FLOAT i = (FLOAT)s.toFloat(&ok);
		if (ok)
			m[ET] = i;
	}
}

void GraphicsState_Dlg::onSetRS()
{
	if (_blockSignals)
		return;

	QObject *s = sender();
	if (s == ui.comboBox_fillMode)
		__set<M3D_FILL_MODE>(ui.comboBox_fillMode, GraphicsState::Element::RS_FM, _elements);
	else if (s == ui.comboBox_cullMode)
		__set<M3D_CULL_MODE>(ui.comboBox_cullMode, GraphicsState::Element::RS_CM, _elements);
	else if (s == ui.comboBox_frontCounterClockwise)
		__set<BOOL>(ui.comboBox_frontCounterClockwise, GraphicsState::Element::RS_FCC, _elements);
	else if (s == ui.lineEdit_depthBias)
		__set<INT>(ui.lineEdit_depthBias, GraphicsState::Element::RS_DB, _elements);
	else if (s == ui.lineEdit_depthBiasClamp)
		__set<FLOAT>(ui.lineEdit_depthBiasClamp, GraphicsState::Element::RS_DBC, _elements);
	else if (s == ui.lineEdit_slopeScaledDepthBias)
		__set<FLOAT>(ui.lineEdit_slopeScaledDepthBias, GraphicsState::Element::RS_SSDB, _elements);
	else if (s == ui.comboBox_depthClipEnable)
		__set<BOOL>(ui.comboBox_depthClipEnable, GraphicsState::Element::RS_DCE, _elements);
	else if (s == ui.comboBox_multisampleEnable)
		__set<BOOL>(ui.comboBox_multisampleEnable, GraphicsState::Element::RS_MSE, _elements);
	else if (s == ui.comboBox_antialiasedLineEnable)
		__set<BOOL>(ui.comboBox_antialiasedLineEnable, GraphicsState::Element::RS_AALE, _elements);
	else if (s == ui.comboBox_forcedSampleCount)
		__set<UINT>(ui.comboBox_forcedSampleCount, GraphicsState::Element::RS_FSC, _elements);
	else if (s == ui.comboBox_conservativeRaster)
		__set<M3D_CONSERVATIVE_RASTERIZATION_MODE>(ui.comboBox_conservativeRaster, GraphicsState::Element::RS_CR, _elements);
	_update_rs_ui();
	_update_chip();
}

void GraphicsState_Dlg::onSetRS2()
{
	QLineEdit *s = (QLineEdit*)sender();
	if (_texts[s] != s->text())
		onSetRS();
}

void GraphicsState_Dlg::onSetDSS()
{
	if (_blockSignals)
		return;

	QObject *s = sender();
	if (s == ui.comboBox_depthEnable)
		__set<BOOL>(ui.comboBox_depthEnable, GraphicsState::Element::DSS_DE, _elements);
	else if (s == ui.comboBox_depthWriteMask)
		__set<M3D_DEPTH_WRITE_MASK>(ui.comboBox_depthWriteMask, GraphicsState::Element::DSS_DWM, _elements);
	else if (s == ui.comboBox_depthComparisonFunction)
		__set<M3D_COMPARISON_FUNC>(ui.comboBox_depthComparisonFunction, GraphicsState::Element::DSS_DCF, _elements);
	else if (s == ui.comboBox_stencilEnable)
		__set<BOOL>(ui.comboBox_stencilEnable, GraphicsState::Element::DSS_SE, _elements);
	else if (s == ui.lineEdit_stencilReadMask)
		__set<UINT8>(ui.lineEdit_stencilReadMask, GraphicsState::Element::DSS_SRM, _elements);
	else if (s == ui.lineEdit_stencilWriteMask)
		__set<UINT8>(ui.lineEdit_stencilWriteMask, GraphicsState::Element::DSS_SWM, _elements);
	else if (s == ui.comboBox_ff_stencilFailOp)
		__set<M3D_STENCIL_OP>(ui.comboBox_ff_stencilFailOp, GraphicsState::Element::DSS_FF_SFO, _elements);
	else if (s == ui.comboBox_ff_stencilDepthFailOp)
		__set<M3D_STENCIL_OP>(ui.comboBox_ff_stencilDepthFailOp, GraphicsState::Element::DSS_FF_SDFO, _elements);
	else if (s == ui.comboBox_ff_stencilPassOp)
		__set<M3D_STENCIL_OP>(ui.comboBox_ff_stencilPassOp, GraphicsState::Element::DSS_FF_SPO, _elements);
	else if (s == ui.comboBox_ff_stencilFunction)
		__set<M3D_COMPARISON_FUNC>(ui.comboBox_ff_stencilFunction, GraphicsState::Element::DSS_FF_SF, _elements);
	else if (s == ui.comboBox_bf_stencilFailOp)
		__set<M3D_STENCIL_OP>(ui.comboBox_bf_stencilFailOp, GraphicsState::Element::DSS_BF_SFO, _elements);
	else if (s == ui.comboBox_bf_stencilDepthFailOp)
		__set<M3D_STENCIL_OP>(ui.comboBox_bf_stencilDepthFailOp, GraphicsState::Element::DSS_BF_SDFO, _elements);
	else if (s == ui.comboBox_bf_stencilPassOp)
		__set<M3D_STENCIL_OP>(ui.comboBox_bf_stencilPassOp, GraphicsState::Element::DSS_BF_SPO, _elements);
	else if (s == ui.comboBox_bf_stencilFunction)
		__set<M3D_COMPARISON_FUNC>(ui.comboBox_bf_stencilFunction, GraphicsState::Element::DSS_BF_SF, _elements);
	_update_dss_ui();
	_update_chip();
}

void GraphicsState_Dlg::onSetDSS2()
{
	QLineEdit *s = (QLineEdit*)sender();
	if (_texts[s] != s->text())
		onSetDSS();
}

void GraphicsState_Dlg::onSetBS()
{
	if (_blockSignals)
		return;

	int32 rt = -2 - ui.buttonGroup_rt->checkedId();
	int32 c = ((int32)GraphicsState::Element::BS_RT1_BE - (int32)GraphicsState::Element::BS_RT0_BE) * rt;

	QObject *s = sender();
	if (s == ui.comboBox_atocEnable)
		__set<BOOL>(ui.comboBox_atocEnable, GraphicsState::Element::BS_ATOCE, _elements);
	else if (s == ui.comboBox_independentBlendEnable)
		__set<BOOL>(ui.comboBox_independentBlendEnable, GraphicsState::Element::BS_IBE, _elements);
	else if (s == ui.lineEdit_sampleMask)
		__set<UINT>(ui.lineEdit_sampleMask, GraphicsState::Element::BS_SM, _elements);
	else {
		if (rt == -1)
			return;
		if (s == ui.comboBox_blendEnable)
			__set<BOOL>(ui.comboBox_blendEnable, (GraphicsState::Element)((int32)GraphicsState::Element::BS_RT0_BE + c), _elements);
		else if (s == ui.comboBox_logicOpEnable)
			__set<BOOL>(ui.comboBox_logicOpEnable, (GraphicsState::Element)((int32)GraphicsState::Element::BS_RT0_LOE + c), _elements);
		else if (s == ui.comboBox_sourceBlend)
			__set<BOOL>(ui.comboBox_sourceBlend, (GraphicsState::Element)((int32)GraphicsState::Element::BS_RT0_SB + c), _elements);
		else if (s == ui.comboBox_destBlend)
			__set<BOOL>(ui.comboBox_destBlend, (GraphicsState::Element)((int32)GraphicsState::Element::BS_RT0_DB + c), _elements);
		else if (s == ui.comboBox_blendOp)
			__set<BOOL>(ui.comboBox_blendOp, (GraphicsState::Element)((int32)GraphicsState::Element::BS_RT0_BO + c), _elements);
		else if (s == ui.comboBox_srcBlendAlpha)
			__set<BOOL>(ui.comboBox_srcBlendAlpha, (GraphicsState::Element)((int32)GraphicsState::Element::BS_RT0_SBA + c), _elements);
		else if (s == ui.comboBox_destBlendAlpha)
			__set<BOOL>(ui.comboBox_destBlendAlpha, (GraphicsState::Element)((int32)GraphicsState::Element::BS_RT0_DBA + c), _elements);
		else if (s == ui.comboBox_blendOpAlpha)
			__set<BOOL>(ui.comboBox_blendOpAlpha, (GraphicsState::Element)((int32)GraphicsState::Element::BS_RT0_BOA + c), _elements);
		else if (s == ui.comboBox_logicOp)
			__set<BOOL>(ui.comboBox_logicOp, (GraphicsState::Element)((int32)GraphicsState::Element::BS_RT0_LO + c), _elements);
		else if (s == ui.comboBox_renderTargetWriteMask)
			__set<BOOL>(ui.comboBox_renderTargetWriteMask, (GraphicsState::Element)((int32)GraphicsState::Element::BS_RT0_RTWM + c), _elements);

	}

	_update_bs_ui();
	_update_chip();
}

void GraphicsState_Dlg::onSetBS2()
{
	QLineEdit *s = (QLineEdit*)sender();
	if (_texts[s] != s->text())
		onSetBS();
}

void GraphicsState_Dlg::onShowInheritedChanged()
{
	_update_ui();
}

void GraphicsState_Dlg::keyPressEvent(QKeyEvent *evt)
{
	if(evt->key() == Qt::Key_Enter || evt->key() == Qt::Key_Return)
		return;
	StandardDialogPage::keyPressEvent(evt);
}


