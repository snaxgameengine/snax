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
#include "ProxyChip.h"


namespace m3d
{

static const Guid MULTIPROXY_GUID = { 0x9d9bf71a, 0x5aa3, 0x4488, { 0xa6, 0x27, 0x22, 0xfb, 0x34, 0x1c, 0xd0, 0x6b } };


class STDCHIPS_API MultiProxy : public ProxyChip
{
	CHIPDESC_DECL;
public:
	MultiProxy();
	virtual ~MultiProxy();

	bool CopyChip(Chip* chip) override;
	bool LoadChip(DocumentLoader& loader) override;
	bool SaveChip(DocumentSaver& saver) const override;

	bool CanCreateFunction() const { return false; }

	virtual ChipChildPtr GetChip() override;

	virtual uint32 GetMultiConnectionChildCount() const;
	virtual ChipChildPtr GetMultiConnectionChip(uint32 subIndex);

//	virtual bool SetChipType(const Guid& type) override;

//	virtual Parameter* AsParameter() override { return this; }

	enum class Mode { GROWING, INDEXED, COMBO };

	void SetMode(Mode mode);
	Mode GetMode() const { return _mode; }

private:
	Mode _mode;
};



}