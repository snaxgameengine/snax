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

#include "GraphicsResourceChip.h"

namespace m3d
{

static const Guid GRAPHICSBUFFER_GUID = { 0x7cdcb1ee, 0x8bc7, 0x486e, { 0x85, 0x14, 0xd0, 0xb1, 0xec, 0x8e, 0x6d, 0xc4 } };


class GRAPHICSCHIPS_API GraphicsBuffer : public GraphicsResourceChip
{
	CHIPDESC_DECL;
public:
	GraphicsBuffer();
	~GraphicsBuffer();

	bool CopyChip(Chip* chip) override;
	bool LoadChip(DocumentLoader& loader) override;
	bool SaveChip(DocumentSaver& saver) const override;

	UINT64 GetSize() const { return _size; }
	M3D_RESOURCE_FLAGS GetFlags() const { return _flags; }
	bool IsGPUOnly() const { return _gpuOnly; }

	void SetSize(UINT64 size);
	void SetFlags(M3D_RESOURCE_FLAGS flags);
	void SetGPUOnly(bool gpuOnly);

	void CallChip() override;

	void ClearResource() override;

	void UpdateChip(BufferLayoutID layoutID = InvalidBufferLayoutID) override;

protected:
	UINT64 _currentSize = 0;

	UINT64 _size = 0;
	M3D_RESOURCE_FLAGS _flags = M3D_RESOURCE_FLAG_NONE;
	bool _gpuOnly = false;

	void _update(BufferLayoutID layoutID);
	void _createBuffer(UINT64 size, D3D12_RESOURCE_STATES state);
};


}
