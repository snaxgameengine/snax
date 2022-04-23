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

namespace m3d
{


static const Guid _3DOBJECT_GUID = { 0xe899c774, 0x3e4d, 0x404e, { 0xb0, 0xc3, 0xce, 0xae, 0xa3, 0x7c, 0xec, 0xec } };


class GRAPHICSCHIPS_API _3DObject : public Chip
{
	CHIPDESC_DECL;
public:
	_3DObject();
	~_3DObject();

	bool CopyChip(Chip *chip) override;
	bool LoadChip(DocumentLoader &loader) override;
	bool SaveChip(DocumentSaver &saver) const override;

	// Do rendering using immidiate context.
	void CallChip() override;
	// Check if this object passes view frustum culling.
	bool CheckFrustumCulling();

	virtual void Render(const XMFLOAT4X4 &parentMatrix, uint32 instanceCount = 1, uint32 startInstanceLocation = 0, bool enableFrustumCulling = true);

	bool GetEnableFrustumCulling() const { return _enableFrustumCulling; }
	void SetEnableFrustumCulling(bool b) { _enableFrustumCulling = b; }

private:
	bool _enableFrustumCulling = true;
};




}