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
#include "D3D12Formats.h"
#include "Graphics.h"
#include "ResourceStateTracker.h"
#include "DescriptorHeapManager.h"


namespace m3d
{

static const Guid READBACKBUFFER_GUID = { 0x19049d65, 0x4be1, 0x48e9,{ 0xba, 0xc7, 0x1f, 0x72, 0x90, 0x4e, 0x71, 0x8d } };



class GRAPHICSCHIPS_API ReadbackBuffer : public Chip, public GraphicsUsage
{
	CHIPDESC_DECL;
public:
	ReadbackBuffer();
	~ReadbackBuffer();

	void OnDestroyDevice() override;

	void SetResource(ID3D12Resource* resource, D3D12_PLACED_SUBRESOURCE_FOOTPRINT fp);

	bool IsDownloading() const;
	bool IsDownloadComplete() const;
	bool GetPixel(XMFLOAT4& p, UINT x, UINT y, UINT z);

protected:
	RID3D12Resource _res;
	D3D12_PLACED_SUBRESOURCE_FOOTPRINT _fp = { 0 };
	UINT64 _readbackStamp = 0ull;
	std::function<void(XMFLOAT4& v, const BYTE* pixel)> _convertFunc;
	BYTE* _data = nullptr;
	UINT _bpp = 0u;

};

}