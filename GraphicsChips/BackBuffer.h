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
#include "GraphicsResourceChip.h"


namespace m3d
{

class D3D12RenderWindow;


static const Guid BACKBUFFER_GUID = { 0xd27ff489, 0x2396, 0x4efe, { 0x98, 0x71, 0x7, 0x5, 0x65, 0x5b, 0x86, 0x22 } };


class GRAPHICSCHIPS_API BackBuffer : public GraphicsResourceChip
{
	CHIPDESC_DECL;
public:
	BackBuffer();

	bool CopyChip(Chip* chip) override;
	bool LoadChip(DocumentLoader& loader) override;
	bool SaveChip(DocumentSaver& saver) const override;

	bool isSRGB() const { return _useSrgb; }
	void setSRGB(bool srgb) { _useSrgb = srgb; }

	void UpdateChip(BufferLayoutID layoutID = InvalidBufferLayoutID) override {}

	ID3D12Resource* GetResource() override;
	const Descriptor& GetRenderTargetViewDescriptor() override;

	virtual D3D12RenderWindow* GetRenderWindow();

private:
	bool _useSrgb = true;
};



}
