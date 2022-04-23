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
#include "GraphicsCommand_Dlg.h"

using namespace m3d;

DIALOGDESC_DEF(GraphicsCommand_Dlg, GRAPHICSCOMMAND_GUID);


void GraphicsCommand_Dlg::Init()
{
	AddItem(MTEXT("Clear Render Target Views"), (uint32)GraphicsCommand::OperatorType::CLEAR_RTV);
	AddItem(MTEXT("Set Render Target Views"), (uint32)GraphicsCommand::OperatorType::SET_RTV);
	AddItem(MTEXT("Set Viewports"), (uint32)GraphicsCommand::OperatorType::SET_VIEWPORTS_ABS);
	AddItem(MTEXT("Set Viewports (Relative)"), (uint32)GraphicsCommand::OperatorType::SET_VIEWPORTS);
	AddItem(MTEXT("Clear Depth Stencil Views"), (uint32)GraphicsCommand::OperatorType::CLEAR_DSV);
	AddItem(MTEXT("Create Device"), (uint32)GraphicsCommand::OperatorType::CREATE_DEVICE);
	AddItem(MTEXT("Get Adapter Info"), (uint32)GraphicsCommand::OperatorType::GET_ADAPTER_INFO);
	AddItem(MTEXT("Get Output Info"), (uint32)GraphicsCommand::OperatorType::GET_OUTPUT_INFO);
	AddItem(MTEXT("Go Fullscreen"), (uint32)GraphicsCommand::OperatorType::FULLSCREEN);
	AddItem(MTEXT("Go Windowed"), (uint32)GraphicsCommand::OperatorType::WINDOWED);
	AddItem(MTEXT("Unbind Render Target Views"), (uint32)GraphicsCommand::OperatorType::UNBIND_RTV);
	AddItem(MTEXT("Unbind Shader Resource Views"), (uint32)GraphicsCommand::OperatorType::UNBIND_SRV);


	AddItem(MTEXT("Upload Subresource"), (uint32)GraphicsCommand::OperatorType::UPLOAD_SUBRESOURCE);
	AddItem(MTEXT("Resolve Subresource"), (uint32)GraphicsCommand::OperatorType::RESOLVE_SUBRESOURCE);
	AddItem(MTEXT("Copy Resource"), (uint32)GraphicsCommand::OperatorType::COPY_RESOURCE);
	AddItem(MTEXT("Copy Subresource Region"), (uint32)GraphicsCommand::OperatorType::COPY_SUBRESOURCE_REGION);
	AddItem(MTEXT("Download Subresource Region"), (uint32)GraphicsCommand::OperatorType::DOWNLOAD_SUBRESOURCE_REGION);
	AddItem(MTEXT("Destroy Resource"), (uint32)GraphicsCommand::OperatorType::DESTROY_RESOURCE);
	AddItem(MTEXT("Set Texture Properties"), (uint32)GraphicsCommand::OperatorType::SET_TEXTURE_PROPS);


	/*	AddItem(MTEXT("Map Resource for Reading"), GraphicsCmd::OperatorType::MAP_RESOURCE_READ);
		AddItem(MTEXT("Map Resource for Writing"), GraphicsCmd::OperatorType::MAP_RESOURCE_WRITE);
		AddItem(MTEXT("Map Resource for Reading and Writing"), GraphicsCmd::OperatorType::MAP_RESOURCE_READ_WRITE);
		AddItem(MTEXT("Map Resource for Writing/Discard"), GraphicsCmd::OperatorType::MAP_RESOURCE_WRITE_DISCARD);
		AddItem(MTEXT("Unmap Resource"), GraphicsCmd::OperatorType::UNMAP_RESOURCE);

		AddItem(MTEXT("Load Texture From File"), GraphicsCmd::OperatorType::LOAD_TEXTURE_FROM_FILE);
		*/
	AddItem(MTEXT("Set Camera Matrices"), (uint32)GraphicsCommand::OperatorType::SET_CAMERA_MATRIES);


	AddItem(MTEXT("Draw Debug Geometry"), (uint32)GraphicsCommand::OperatorType::DEBUG_DRAW);
	AddItem(MTEXT("Add Debug Point"), (uint32)GraphicsCommand::OperatorType::DEBUG_ADD_POINT);
	AddItem(MTEXT("Add Debug Line"), (uint32)GraphicsCommand::OperatorType::DEBUG_ADD_LINE);
	AddItem(MTEXT("Add Debug Triangle"), (uint32)GraphicsCommand::OperatorType::DEBUG_ADD_TRIANGLE);

	sort();

	SetSelectionChangedCallback([this](RData data) -> bool {
		GraphicsCommand::OperatorType cmd = (GraphicsCommand::OperatorType)data;
		if (cmd == GetChip()->GetCommand())
			return false;
		GetChip()->SetCommand(cmd);
		return true;
		});

	SetInit((uint32)GetChip()->GetCommand(), (uint32)GraphicsCommand::OperatorType::NONE);
}
