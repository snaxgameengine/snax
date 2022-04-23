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
#include "Graphics.h"

namespace m3d
{

static const Guid GRAPHICSCOMMAND_GUID = { 0xa2c861a1, 0x4990, 0x4cb3, { 0xac, 0xbd, 0x33, 0x46, 0xd3, 0xe3, 0xf6, 0xa2 } };


class GRAPHICSCHIPS_API GraphicsCommand : public Chip, public GraphicsUsage
{
	CHIPDESC_DECL;
public:
	enum class OperatorType {
		NONE,
		CLEAR_RTV,
		SET_RTV,
		SET_VIEWPORTS,
		SET_VIEWPORTS_ABS,
		CLEAR_DSV,
		unused0,
		CREATE_DEVICE,
		GET_ADAPTER_INFO,
		GET_OUTPUT_INFO,
		FULLSCREEN,
		WINDOWED,
		UNBIND_RTV,
		UNBIND_SRV,

		UPLOAD_SUBRESOURCE = 100,
		RESOLVE_SUBRESOURCE,
		COPY_RESOURCE,
		COPY_SUBRESOURCE_REGION,
		DOWNLOAD_SUBRESOURCE_REGION,
		DESTROY_RESOURCE,
		SET_TEXTURE_PROPS,

		/*		CMD_MAP_RESOURCE_READ,
				CMD_MAP_RESOURCE_WRITE,
				CMD_MAP_RESOURCE_READ_WRITE,
				CMD_MAP_RESOURCE_WRITE_DISCARD,
				___CMD_MAP_RESOURCE_WRITE_NO_OVERWRITE, // not used
				CMD_UNMAP_RESOURCE,

				CMD_LOAD_TEXTURE_FROM_FILE = 150,

				//D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT

				CMD_SET_GLOBAL_VALUES = 200,
				CMD_SET_GLOBAL_VECTORS,
				CMD_SET_GLOBAL_MATRICES,
				CMD_SET_GLOBAL_SHADER_RESOURCES,
		*/
		SET_CAMERA_MATRIES = 300,

		DEBUG_DRAW = 10000,
		DEBUG_ADD_POINT,
		DEBUG_ADD_LINE,
		DEBUG_ADD_TRIANGLE

	};

	GraphicsCommand();
	~GraphicsCommand();

	bool CopyChip(Chip* chip) override;
	bool LoadChip(DocumentLoader& loader) override;
	bool SaveChip(DocumentSaver& saver) const override;

	OperatorType GetCommand() const { return _cmd; }
	void SetCommand(OperatorType cmd);

	void CallChip() override;

protected:
	OperatorType _cmd = OperatorType::NONE;
};



}
