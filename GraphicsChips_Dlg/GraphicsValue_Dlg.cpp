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

#include "StdAfx.h"
#include "GraphicsValue_dlg.h"


using namespace m3d;


DIALOGDESC_DEF(GraphicsValue_Dlg, GRAPHICSVALUE_GUID);


void GraphicsValue_Dlg::Init()
{
	AddItem(MTEXT("Get Resource Width"), (uint32)GraphicsValue::OperatorType::RESOURCE_WIDTH);
	AddItem(MTEXT("Get Resource Height"), (uint32)GraphicsValue::OperatorType::RESOURCE_HEIGHT);
	AddItem(MTEXT("Get Resource Depth"), (uint32)GraphicsValue::OperatorType::RESOURCE_DEPTH);
	AddItem(MTEXT("Get Resource Mip Levels"), (uint32)GraphicsValue::OperatorType::RESOURCE_MIP_LEVELS);
	AddItem(MTEXT("Get Resource Array size"), (uint32)GraphicsValue::OperatorType::RESOURCE_ARRAY_SIZE);
	AddItem(MTEXT("Get Resource MS Count"), (uint32)GraphicsValue::OperatorType::RESOURCE_MS_COUNT);
	AddItem(MTEXT("Get Resource MS Quality"), (uint32)GraphicsValue::OperatorType::RESOURCE_MS_QUALITY);
	AddItem(MTEXT("Calculate Subresource Index"), (uint32)GraphicsValue::OperatorType::RESOURCE_CALC_SUBRESOURCE);
	AddItem(MTEXT("Resource Exist?"), (uint32)GraphicsValue::OperatorType::RESOURCE_EXIST);
	AddItem(MTEXT("Is Download Complete?"), (uint32)GraphicsValue::OperatorType::IS_DOWNLOAD_COMPLETE);
	
	AddItem(MTEXT("Get Render Target Width"), (uint32)GraphicsValue::OperatorType::RTV_WIDTH);
	AddItem(MTEXT("Get Render Target Height"), (uint32)GraphicsValue::OperatorType::RTV_HEIGHT);
	AddItem(MTEXT("Get Depth Stencil Width"), (uint32)GraphicsValue::OperatorType::DSV_WIDTH);
	AddItem(MTEXT("Get Depth Stencil Height"), (uint32)GraphicsValue::OperatorType::DSV_HEIGHT);
	AddItem(MTEXT("Get Unordered Access Width"), (uint32)GraphicsValue::OperatorType::UAV_WIDTH);
	AddItem(MTEXT("Get Unordered Access Height"), (uint32)GraphicsValue::OperatorType::UAV_HEIGHT);
	AddItem(MTEXT("Get Shader Resource Width"), (uint32)GraphicsValue::OperatorType::SRV_WIDTH);
	AddItem(MTEXT("Get Shader Resource Height"), (uint32)GraphicsValue::OperatorType::SRV_HEIGHT);
	AddItem(MTEXT("Get Current Render Target Width"), (uint32)GraphicsValue::OperatorType::CURRENT_RTV_WIDTH);
	AddItem(MTEXT("Get Current Render Target Height"), (uint32)GraphicsValue::OperatorType::CURRENT_RTV_HEIGHT);

	

	AddItem(MTEXT("Device Exist?"), (uint32)GraphicsValue::OperatorType::DEVICE_EXIST);
	AddItem(MTEXT("Get Number of Graphics Adapters"), (uint32)GraphicsValue::OperatorType::NUM_ADAPTERS);
	AddItem(MTEXT("Get Display Orientation"), (uint32)GraphicsValue::OperatorType::DISPLAY_ORIENTATION);
	
	AddItem(MTEXT("Do View Frustum Culling Test"), (uint32)GraphicsValue::OperatorType::CHECK_FRUSTUM_CULLING);

	SetSelectionChangedCallback([this](RData data) -> bool {
		GraphicsValue::OperatorType cmd = (GraphicsValue::OperatorType)data;
		if (cmd == GetChip()->GetType())
			return false;
		GetChip()->SetType(cmd);
		return true;
		});

	SetInit((uint32)GetChip()->GetType(), (uint32)GraphicsValue::OperatorType::NONE);
}

