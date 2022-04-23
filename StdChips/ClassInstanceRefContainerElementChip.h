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


#include "ClassInstanceRefChip.h"


namespace m3d
{

static const Guid CLASSINSTANCEREFCONTAINERELEMENTCHIP_GUID = { 0xe0a5a4a5, 0xbdea, 0x4263, { 0xbd, 0xc5, 0x64, 0x0a, 0x51, 0xdf, 0x2c, 0x66 } };


class STDCHIPS_API ClassInstanceRefContainerElementChip : public ClassInstanceRefChip
{
	CHIPDESC_DECL;
public:
	ClassInstanceRefContainerElementChip();
	virtual ~ClassInstanceRefContainerElementChip();

	virtual bool CopyChip(Chip *chip) override;
	virtual bool LoadChip(DocumentLoader &loader) override;
	virtual bool SaveChip(DocumentSaver &saver) const override;

	virtual ClassInstanceRef GetInstance() override;
	virtual void CreateInstance(Class *instanceOf) override;
	virtual void SetInstance(ClassInstanceRef ref, AssignType at) override;
	virtual void ClearInstance() override;
	virtual void MakeOwner() override;

	enum class OperatorType { NONE, ARRAY, VALUEMAP_BY_KEY, VALUEMAP_BY_INDEX, TEXTMAP_BY_KEY, TEXTMAP_BY_INDEX };

	virtual OperatorType GetOperatorType() const { return _ot; }
	virtual void SetOperatorType(OperatorType ot);

protected:
	OperatorType _ot;

};

}