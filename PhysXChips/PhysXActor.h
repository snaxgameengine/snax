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
#include "PhysX.h"


namespace m3d
{


static const Guid PHYSXACTOR_GUID = { 0xd2cbb0af, 0x9319, 0x4a04, { 0x9d, 0x8f, 0xb5, 0xe1, 0x60, 0x39, 0x6d, 0x93 } };



class PHYSXCHIPS_API PhysXActor : public Chip, public PhysXSceneObject
{
	CHIPDESC_DECL;
public:
	PhysXActor();
	virtual ~PhysXActor();

	virtual bool CopyChip(Chip *chip) override;
	virtual bool LoadChip(DocumentLoader &loader) override;
	virtual bool SaveChip(DocumentSaver &saver) const override;

	virtual void OnSceneDestroyed() override { DestroyActor(); }

	virtual bool Exist() const;
	virtual PxActor *GetActor() const { return nullptr; }
	virtual void DestroyActor() {}

	virtual bool IsDisableGravity() const;
	virtual bool IsSendSleepNotifies() const;
	virtual bool IsVisualization() const;
	virtual PxDominanceGroup GetDominanceGroup() const; 

	virtual void SetDisableGravity(bool b, bool chipSettings = true);
	virtual void SetSendSleepNotifies(bool b, bool chipSettings = true);
	virtual void SetVisualization(bool b, bool chipSettings = true);
	virtual void SetDominanceGroup(PxDominanceGroup b, bool chipSettings = true);

protected:
	bool _disableGravity;
	bool _sendSleepNotifies;
	bool _visualization;
	PxDominanceGroup _dominanceGroup;

	virtual void OnRelease() { DestroyActor(); }

};



}