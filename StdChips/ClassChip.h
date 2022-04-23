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
#include "M3DCore/Path.h"

namespace m3d
{


static const Guid CLASSCHIP_GUID = { 0x021d51cc, 0x7cfb, 0x484c, { 0xb7, 0xa3, 0xf4, 0x9c, 0x05, 0x70, 0x3f, 0xe0 } };

class STDCHIPS_API ClassChip : public Chip
{
	CHIPDESC_DECL;
public:
	ClassChip();
	virtual ~ClassChip();

	virtual bool CopyChip(Chip *chip) override;
	virtual bool LoadChip(DocumentLoader &loader) override;
	virtual bool SaveChip(DocumentSaver &saver) const override;

	virtual Class *GetCG();
	virtual void SetCG(Class *clazz);

	virtual Class *GetChipCG() const { return _clazz; }

	virtual void RestoreChip() override { GetCG(); }

	virtual bool IsPreloadEnabled() const { return _preload; }
	virtual void EnablePreload(bool b) { _preload = b; }

protected:
	bool _preload;
	Path _filename;
	Guid _clazzGlobalID;

	Class *_clazz;

};



}