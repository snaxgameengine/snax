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
#include "PipelineStatePool.h"

namespace m3d
{

static const Guid ROOTSIGNATURE_GUID = { 0x0a02b379, 0x6b5c, 0x4c62, { 0xad, 0xec, 0x1f, 0x83, 0x56, 0xe4, 0x9a, 0xab } };



class GRAPHICSCHIPS_API RootSignature : public Chip, public GraphicsUsage
{
	CHIPDESC_DECL;
public:
	RootSignature();
	~RootSignature();

	bool CopyChip(Chip* chip) override;
	bool LoadChip(DocumentLoader& loader) override;
	bool SaveChip(DocumentSaver& saver) const override;

	UINT GetLocalCBVCount() const { return _localCBVCount; }
	UINT GetLocalSRVCount() const { return _localSRVCount; }
	UINT GetLocalUAVCount() const { return _localUAVCount; }
	UINT GetLocalSamplerCount() const { return _localSamplerCount; }

	void SetLocalCBVCount(UINT n) { _localCBVCount = n; }
	void SetLocalSRVCount(UINT n) { _localSRVCount = n; }
	void SetLocalUAVCount(UINT n) { _localUAVCount = n; }
	void SetLocalSamplerCount(UINT n) { _localSamplerCount = n; }

	virtual RootSignatureID GetRootSignatureID();

protected:
	RootSignatureID _rootSignatureID;

	UINT _localCBVCount;
	UINT _localSRVCount;
	UINT _localUAVCount;
	UINT _localSamplerCount;
};

}