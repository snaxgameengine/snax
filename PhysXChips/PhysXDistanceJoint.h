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
#include "PhysXJoint.h"

namespace m3d
{


static const Guid PHYSXDISTANCEJOINT_GUID = { 0x28946abe, 0x6d76, 0x4282, { 0xb3, 0x8d, 0x99, 0xd7, 0x63, 0x52, 0x65, 0xbf } };



class PHYSXCHIPS_API PhysXDistanceJoint : public PhysXJoint
{
	CHIPDESC_DECL;
public:
	PhysXDistanceJoint();
	virtual ~PhysXDistanceJoint();

	virtual bool CopyChip(Chip *chip) override;
	virtual bool LoadChip(DocumentLoader &loader) override;
	virtual bool SaveChip(DocumentSaver &saver) const override;

	virtual PxJoint *CreateJoint(PxPhysics& physics, PxRigidActor* actor0, const PxTransform& localFrame0, PxRigidActor* actor1, const PxTransform& localFrame1);

	virtual bool IsMinDistanceEnabled() const;
	virtual bool IsMaxDistanceEnabled() const;
	virtual float32 GetMinDistance() const;
	virtual float32 GetMaxDistance() const;
	virtual float32 GetTolerance() const;
	virtual bool IsSpringEnabled() const;
	virtual float32 GetStiffness() const;
	virtual float32 GetDamping() const;

	virtual void SetMinDistanceEnabled(bool b, bool chipSettings = true);
	virtual void SetMaxDistanceEnabled(bool b, bool chipSettings = true);
	virtual void SetDistanceRange(float32 minD, float32 maxD, bool chipSettings = true);
	virtual void SetTolerance(float32 d, bool chipSettings = true);
	virtual void SetSpringEnabled(bool b, bool chipSettings = true);
	virtual void SetStiffness(float32 d, bool chipSettings = true);
	virtual void SetDamping(float32 d, bool chipSettings = true);

protected:
	bool _minDistanceEnable;
	bool _maxDistanceEnable;
	float32 _minDistance;
	float32 _maxDistance;
	float32 _tolerance;
	bool _springEnable;
	float32 _stiffness;
	float32 _damping;

};



}