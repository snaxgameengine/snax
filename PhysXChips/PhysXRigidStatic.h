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
#include "PhysXRigidActor.h"
#include "PhysX.h"


namespace m3d
{


static const Guid PHYSXRIGIDSTATIC_GUID = { 0x02a24733, 0x4f6f, 0x4c69, { 0xbb, 0x76, 0x58, 0xb2, 0x38, 0x10, 0xa6, 0xdb } };



class PHYSXCHIPS_API PhysXRigidStatic : public PhysXRigidActor
{
	CHIPDESC_DECL;
public:
	PhysXRigidStatic();
	virtual ~PhysXRigidStatic();

	virtual void CallChip();

	virtual PxRigidStatic *CreateActor(const PxTransform &pose);
	virtual void DestroyActor();

	virtual PxRigidActor *GetRigidActor() const { return _actor; }

protected:
	PxRigidStatic *_actor;

};



}