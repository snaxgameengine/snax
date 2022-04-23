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
#include "PhysXArticulationBase.h"


namespace m3d
{


static const Guid PHYSXARTICULATIONRC_GUID = { 0x3e778cb7, 0x3613, 0x41bb,{ 0xbf, 0x5f, 0x7e, 0x8c, 0x18, 0xb6, 0x50, 0x40 } };



class PHYSXCHIPS_API PhysXArticulationRC : public PhysXArticulationBase
{
	CHIPDESC_DECL;
public:
	PhysXArticulationRC();
	~PhysXArticulationRC();

	bool CopyChip(Chip *chip) override;
	bool LoadChip(DocumentLoader &loader) override;
	bool SaveChip(DocumentSaver &saver) const override;

	void CallChip() override;
		
	bool Exist() const override { return _articulation != nullptr; }
	PxArticulationBase *GetArticulation() const override { return _articulation; }
	void DestroyArticulation() override;

	virtual PxArticulationBase *CreateArticulation();

	virtual PxArticulationFlags GetFlags() const;
	virtual void SetFlags(PxArticulationFlags f);

protected:
	PxArticulationReducedCoordinate *_articulation;
	PxArticulationFlags _flags;

};



}
