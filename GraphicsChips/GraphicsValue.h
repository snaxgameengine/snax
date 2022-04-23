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
#include "StdChips/Value.h"
#include "Graphics.h"


namespace m3d
{


static const Guid GRAPHICSVALUE_GUID = { 0xcb1384d9, 0x4781, 0x4fad, { 0x8c, 0x71, 0xf5, 0xa1, 0x52, 0x92, 0x74, 0x5 } };


class GRAPHICSCHIPS_API GraphicsValue : public Value, public GraphicsUsage
{
	CHIPDESC_DECL; 
public:
	enum class OperatorType {
		NONE,
		RESOURCE_WIDTH,
		RESOURCE_HEIGHT,
		RESOURCE_DEPTH,
		RESOURCE_MIP_LEVELS,
		RESOURCE_ARRAY_SIZE,
		RESOURCE_MS_COUNT,
		RESOURCE_MS_QUALITY,
		RESOURCE_CALC_SUBRESOURCE,
		RESOURCE_EXIST,
		IS_DOWNLOAD_COMPLETE,
		RTV_WIDTH = 100,
		RTV_HEIGHT,
		DSV_WIDTH,
		DSV_HEIGHT,
		UAV_WIDTH,
		UAV_HEIGHT,
		SRV_WIDTH,
		SRV_HEIGHT,
		CURRENT_RTV_WIDTH,
		CURRENT_RTV_HEIGHT,
		DEVICE_EXIST = 200,
		NUM_ADAPTERS,
		DISPLAY_ORIENTATION = 250,
		CHECK_FRUSTUM_CULLING = 300
	};

	GraphicsValue();
	~GraphicsValue();

	bool CopyChip(Chip* chip) override;
	bool LoadChip(DocumentLoader& loader) override;
	bool SaveChip(DocumentSaver& saver) const override;

	OperatorType GetType() const { return _type; }
	void SetType(OperatorType type);

	value GetValue() override;
	void SetValue(value v) override {}

protected:
	OperatorType _type = OperatorType::NONE;
};



}