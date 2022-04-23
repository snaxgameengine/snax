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


static const Guid CLASSDIAGRAMCHIP_GUID = { 0x996ca8cf, 0x8b7f, 0x4cac, { 0x83, 0x84, 0x10, 0x2f, 0x14, 0x64, 0xd7, 0xfe } };


class STDCHIPS_API ClassDiagramChip : public Chip
{
	CHIPDESC_DECL;
public:
	ClassDiagramChip();
	virtual ~ClassDiagramChip();

	bool CanCreateFunction() const { return false; }

	virtual Class *GetCG() const { return _clazz; }
	virtual void SetCG(Class *clazz) { _clazz = clazz; }

	virtual String GetName() const override;
	virtual void SetName(String name) override {}

	virtual bool SetChild(Chip *child, uint32 index, uint32 subIndex) override;

	virtual void UnlinkChildren();

	virtual void OnBaseClassAdded(Chip *c, uint32 conn = -1);
	virtual void OnBaseClassRemoved(ClassDiagramChip *c);

protected:
	Class *_clazz;


};



}