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
#include "PhysXArticulationJointBase.h"


namespace m3d
{


static const Guid PHYSXARTICULATIONJOINTRC_GUID = { 0xdb90b177, 0x3336, 0x48b5,{ 0x8c, 0xb, 0xec, 0xc8, 0x18, 0xee, 0xa2, 0x94 } };



class PHYSXCHIPS_API PhysXArticulationJointRC : public PhysXArticulationJointBase
{
	CHIPDESC_DECL;
public:
	PhysXArticulationJointRC();
	virtual ~PhysXArticulationJointRC();

	bool CopyChip(Chip *chip) override;
	bool LoadChip(DocumentLoader &loader) override;
	bool SaveChip(DocumentSaver &saver) const override;

	virtual PxArticulationLink *CreateLink(PxArticulationReducedCoordinate *a, PxArticulationLink *p);

	void ReleaseJoint() override;

	struct AxisParams
	{
		PxArticulationMotion::Enum motion;
		PxReal lowLimit;
		PxReal highLimit;

		PxReal stiffness;
		PxReal damping;
		PxReal maxForce;
		bool isAccelerationDrive;

		PxReal driveTarget;
		PxReal driveTargetVel;

		AxisParams() : motion(PxArticulationMotion::eLOCKED), lowLimit(0.0f), highLimit(0.0f), stiffness(0.0f), damping(0.0f), maxForce(std::numeric_limits<float32>::max()), isAccelerationDrive(false), driveTarget(0.0f), driveTargetVel(0.0f) {}

		bool operator!=(const AxisParams &rhs) const
		{
			return motion != rhs.motion || lowLimit != rhs.lowLimit || highLimit != rhs.highLimit || stiffness != rhs.stiffness || damping != rhs.damping || maxForce != rhs.maxForce || isAccelerationDrive != rhs.isAccelerationDrive || driveTarget != rhs.driveTarget || driveTargetVel != rhs.driveTargetVel; 
		}
	};

	virtual PxArticulationJointType::Enum GetType() const { return _jointType; }
	virtual PxReal GetFrictionCoefficient() const { return _frictionCoefficient; }
	virtual PxReal GetMaxJointVelocity() const { return _maxJointVelocity; }
	virtual const AxisParams &GetAxisParams(PxArticulationAxis::Enum axis) const { return _axisParams[axis]; }

	virtual void SetType(PxArticulationJointType::Enum t);
	virtual void SetFrictionCoefficient(PxReal f);
	virtual void SetMaxJointVelocity(PxReal f);
	virtual void SetMotion(PxArticulationAxis::Enum axis, PxArticulationMotion::Enum motion);
	virtual void SetLowLimit(PxArticulationAxis::Enum axis, PxReal lowLimit);
	virtual void SetHighLimit(PxArticulationAxis::Enum axis, PxReal highLimit);
	virtual void SetStiffness(PxArticulationAxis::Enum axis, PxReal stiffness);
	virtual void SetDamping(PxArticulationAxis::Enum axis, PxReal damping);
	virtual void SetMaxForce(PxArticulationAxis::Enum axis, PxReal maxForce);
	virtual void SetIsAccelerationDrive(PxArticulationAxis::Enum axis, bool isAccelerationDrive);
	virtual void SetDriveTarget(PxArticulationAxis::Enum axis, PxReal driveTarget);
	virtual void SetDriveTargetVel(PxArticulationAxis::Enum axis, PxReal driveTargetVel);


protected:
	PxArticulationJointReducedCoordinate *_joint;
	
	PxArticulationJointType::Enum _jointType;
	PxReal _frictionCoefficient;
	PxReal _maxJointVelocity;
	AxisParams _axisParams[PxArticulationAxis::eCOUNT];

};



}
