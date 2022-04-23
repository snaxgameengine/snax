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
#include "PhysXRigidBody.h"
#include "PhysX.h"


namespace m3d
{

	
static const Guid PHYSXRIGIDDYNAMIC_GUID = { 0x456d654d, 0xa3bf, 0x40da, { 0xae, 0x84, 0xcf, 0x20, 0xbf, 0x2a, 0xce, 0x40 } };



class PHYSXCHIPS_API PhysXRigidDynamic : public PhysXRigidBody
{
	CHIPDESC_DECL;
public:
	PhysXRigidDynamic();
	virtual ~PhysXRigidDynamic();

	virtual bool CopyChip(Chip *chip) override;
	virtual bool LoadChip(DocumentLoader &loader) override;
	virtual bool SaveChip(DocumentSaver &saver) const override;

	virtual void CallChip();

	virtual PxRigidDynamic *CreateActor(const PxTransform &pose);
	virtual void DestroyActor();

	virtual PxRigidDynamic *GetRigidDynamic() const { return _actor; }
	virtual PxRigidBody *GetRigidBody() const override { return GetRigidDynamic(); }

	virtual void AddForce(const PhysXForce &force);
	virtual void ClearForces();

	virtual bool IsKinematic();
	virtual float32 GetLinearDamping();
	virtual float32 GetAngularDamping();
	virtual float32 GetMaxAngularVelocity();
	virtual float32 GetSleepThreshold();
	virtual void GetMinSolverIterations(uint32 &pos, uint32 &vel);
	virtual float32 GetContactReportThreshold();

	virtual void SetKinematic(bool k, bool chipSettings = true);
	virtual void SetLinearDamping(float32 f, bool chipSettings = true);
	virtual void SetAngularDamping(float32 f, bool chipSettings = true);
	virtual void SetMaxAngularVelocity(float32 f, bool chipSettings = true);
	virtual void SetSleepThreshold(float32 f, bool chipSettings = true);
	virtual void SetMinSolverIterations(uint32 pos, uint32 vel, bool chipSettings = true);
	virtual void SetContactReportThreshold(float32 f, bool chipSettings = true);

protected:
	PxRigidDynamic *_actor;
	bool _kinematic;
	float32 _linearDamping;
	float32 _angularDamping;
	float32 _maxAngularVelocity;
	float32 _sleepThreshold;
	uint32 _minPositionIters;
	uint32 _minVelocityIters;
	float32 _contactReportThreshold;

};



}