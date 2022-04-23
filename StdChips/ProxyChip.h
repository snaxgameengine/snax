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

static const Guid PROXYCHIP_GUID = { 0x7908d608, 0xa149, 0x4286, { 0x82, 0x6d, 0x27, 0x0e, 0xde, 0xde, 0x12, 0xfe } };

class STDCHIPS_API ProxyChip : public Chip
{
	CHIPDESC_DECL;
	CHIPMSG(ChipTypeNotSetException, WARN, MTEXT("Chip type not set!"))
	CHIPMSG(CastFailedException, WARN, MTEXT("A connected child is of an incompatible type!"))
public:
	ProxyChip();
	virtual ~ProxyChip();

	virtual bool CopyChip(Chip *chip) override;
	virtual bool LoadChip(DocumentLoader &loader) override;
	virtual bool SaveChip(DocumentSaver &saver) const override;

	// Fast casting!
	virtual ProxyChip *AsProxyChip() override { return this; }
	// Overrided. Returns the chip type set for this proxy. Special chips like the TemplateChip can override this for advanced use!
	virtual const Guid &GetChipType() const override { return _type; }
	// Returns the chip type set for this proxy chip DO NOT override this method!
	virtual const Guid &GetChipTypeAtProxy() const { return _type; }

	virtual ChipChildPtr GetChip() override;

	virtual bool SetChipType(const Guid &type);

	virtual bool IsChipTypeSet() const { return _isTypeSet; }

private:
	Guid _type;
	bool _isTypeSet;

};



}