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
#include "TemplateChip.h"


namespace m3d
{


static const Guid INSTANCEDATA_GUID = { 0x5a1470ef, 0x802f, 0x46fd, { 0xa8, 0xf7, 0x6e, 0x79, 0xb6, 0xf5, 0xd3, 0xb7 } };

class STDCHIPS_API InstanceData : public TemplateChip
{
	CHIPDESC_DECL;
public:
	InstanceData();
	virtual ~InstanceData();

	virtual ChipChildPtr GetChip() override;

	virtual InstanceData *AsInstanceData() override { return this; }

	virtual bool SetChipType(const Guid &type) override;
	// This method will set the template before notifying the class about type set.
	virtual bool SetChipTypeAndCreateTemplate(const Guid &type, Chip *copyTemplateFrom);

};



}