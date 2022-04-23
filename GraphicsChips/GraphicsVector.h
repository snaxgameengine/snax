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
#include "StdChips/VectorChip.h"
#include "Graphics.h"



namespace m3d
{

static const Guid GRAPHICSVECTOR_GUID = { 0xd3e5f5d3, 0xd22b, 0x472f,{ 0x86, 0x10, 0x23, 0x8e, 0xed, 0xc0, 0x32, 0x3b } };


class GRAPHICSCHIPS_API GraphicsVector : public VectorChip, public GraphicsUsage
{
	CHIPDESC_DECL;
public:
	enum class OperatorType {
		NONE,
		DOWNLOADED_PIXEL
	};

	GraphicsVector();
	~GraphicsVector();

	bool CopyChip(Chip* chip) override;
	bool LoadChip(DocumentLoader& loader) override;
	bool SaveChip(DocumentSaver& saver) const override;

	OperatorType GetType() const { return _type; }
	void SetType(OperatorType type);

	const XMFLOAT4& GetVector() override;
	void SetVector(const XMFLOAT4& v) override {}

protected:
	OperatorType _type = OperatorType::NONE;
};



}

