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

#include "pch.h"
#include "PhysXArticulationJointRC.h"
#include "PhysXArticulationLink.h"
#include "StdChips/MatrixChip.h"

using namespace m3d;


CHIPDESCV1_DEF(PhysXArticulationJointRC, MTEXT("PhysX Articulation Joint RC"), PHYSXARTICULATIONJOINTRC_GUID, PHYSXARTICULATIONJOINTBASE_GUID);


namespace m3d
{
	bool SerializeDocumentData(DocumentSaver &saver, const PhysXArticulationJointRC::AxisParams &data)
	{
		SAVEDEF("motion", data.motion, PxArticulationMotion::eLOCKED);
		SAVEDEF("lowLimit", data.lowLimit, 0.0f);
		SAVEDEF("highLimit", data.highLimit, 0.0f);
		SAVEDEF("stiffness", data.stiffness, 0.0f);
		SAVEDEF("damping", data.damping, 0.0f);
		SAVEDEF("maxForce", data.maxForce, std::numeric_limits<float32>::max());
		SAVEDEF("isAccelerationDrive", data.isAccelerationDrive, false);
		SAVEDEF("driveTarget", data.driveTarget, 0.0f);
		SAVEDEF("driveTargetVel", data.driveTargetVel, 0.0f);
		return true;
	}

	bool DeserializeDocumentData(DocumentLoader &loader, PhysXArticulationJointRC::AxisParams &data)
	{
		LOADDEF("motion", data.motion, PxArticulationMotion::eLOCKED);
		LOADDEF("lowLimit", data.lowLimit, 0.0f);
		LOADDEF("highLimit", data.highLimit, 0.0f);
		LOADDEF("stiffness", data.stiffness, 0.0f);
		LOADDEF("damping", data.damping, 0.0f);
		LOADDEF("maxForce", data.maxForce, std::numeric_limits<float32>::max());
		LOADDEF("isAccelerationDrive", data.isAccelerationDrive, false);
		LOADDEF("driveTarget", data.driveTarget, 0.0f);
		LOADDEF("driveTargetVel", data.driveTargetVel, 0.0f);
		return true;
	}
}


PhysXArticulationJointRC::PhysXArticulationJointRC()
{
	_joint = nullptr;
	_jointType = PxArticulationJointType::eFIX;
	_frictionCoefficient = 0.0f;
	_maxJointVelocity = std::numeric_limits<float32>::max();
}

PhysXArticulationJointRC::~PhysXArticulationJointRC()
{
	if (_joint) {
		PhysXActorData *d = (PhysXActorData*)_joint->getChildArticulationLink().userData;
		if (d && d->linkChip == this)
			d->linkChip = nullptr;
	}
}

bool PhysXArticulationJointRC::CopyChip(Chip *chip)
{
	PhysXArticulationJointRC *c = dynamic_cast<PhysXArticulationJointRC*>(chip);
	B_RETURN(PhysXArticulationJointBase::CopyChip(c));
	_jointType = c->_jointType;
	_frictionCoefficient = c->_frictionCoefficient;
	_maxJointVelocity = c->_maxJointVelocity;
	std::memcpy(_axisParams, c->_axisParams, sizeof(_axisParams));
	return true;
}

bool PhysXArticulationJointRC::LoadChip(DocumentLoader &loader)
{
	B_RETURN(PhysXArticulationJointBase::LoadChip(loader));
	LOADDEF("jointType", (uint32&)_jointType, PxArticulationJointType::eFIX);
	LOADDEF("FrictionCoefficient", _frictionCoefficient, 0.0f);
	LOADDEF("MaxJointVelocity", _maxJointVelocity, 0.0f);
	LOADDEF("axisTwist", _axisParams[PxArticulationAxis::eTWIST], AxisParams());
	LOADDEF("axisSwing1", _axisParams[PxArticulationAxis::eSWING1], AxisParams());
	LOADDEF("axisSwing2", _axisParams[PxArticulationAxis::eSWING2], AxisParams());
	LOADDEF("axisX", _axisParams[PxArticulationAxis::eX], AxisParams());
	LOADDEF("axisY", _axisParams[PxArticulationAxis::eY], AxisParams());
	LOADDEF("axisZ", _axisParams[PxArticulationAxis::eZ], AxisParams());
	return true;
}

bool PhysXArticulationJointRC::SaveChip(DocumentSaver &saver) const
{
	B_RETURN(PhysXArticulationJointBase::SaveChip(saver));
	SAVEDEF("jointType", (const uint32&)_jointType, PxArticulationJointType::eFIX);
	SAVEDEF("frictionCoefficient", _frictionCoefficient, 0.0f);
	SAVEDEF("maxJointVelocity", _maxJointVelocity, 0.0f);
	SAVEDEF("axisTwist", _axisParams[PxArticulationAxis::eTWIST], AxisParams());
	SAVEDEF("axisSwing1", _axisParams[PxArticulationAxis::eSWING1], AxisParams());
	SAVEDEF("axisSwing2", _axisParams[PxArticulationAxis::eSWING2], AxisParams());
	SAVEDEF("axisX", _axisParams[PxArticulationAxis::eX], AxisParams());
	SAVEDEF("axisY", _axisParams[PxArticulationAxis::eY], AxisParams());
	SAVEDEF("axisZ", _axisParams[PxArticulationAxis::eZ], AxisParams());
	return true;

}

PxArticulationLink *PhysXArticulationJointRC::CreateLink(PxArticulationReducedCoordinate *a, PxArticulationLink *p)
{
	if (_joint)
		return nullptr;

	if (!a || !p)
		return nullptr;
	ChildPtr<PhysXArticulationLink> ch0 = GetChild(0);
	if (!ch0)
		return nullptr;
	PxArticulationLink *l = ch0->CreateLink(a, p);
	if (!l)
		return nullptr;

	PhysXActorData *d = (PhysXActorData*)l->userData;
	if (d)
		d->linkChip = this;

	_joint = static_cast<PxArticulationJointReducedCoordinate*>(l->getInboundJoint());

	PxTransform f1 = PxTransform(PxIdentity);
	PxTransform f2 = PxTransform(PxIdentity);

	if (_globalFrame) {
		ChildPtr<MatrixChip> chGlobalFrame = GetChild(1);
		XMFLOAT3 t;
		XMFLOAT4 r;
		XMFLOAT3 s;
		if (chGlobalFrame && chGlobalFrame->GetDecomposedMatrix(t, r, s)) {
			f1 = p->getGlobalPose().transformInv(PxTransform((PxVec3&)t, (PxQuat&)r));
			f2 = l->getGlobalPose().transformInv(PxTransform((PxVec3&)t, (PxQuat&)r));
		}
	}
	else {
		ChildPtr<MatrixChip> chFrame1 = GetChild(1);
		ChildPtr<MatrixChip> chFrame2 = GetChild(2);
		XMFLOAT3 t;
		XMFLOAT4 r;
		XMFLOAT3 s;
		if (chFrame1 && chFrame1->GetDecomposedMatrix(t, r, s))
			f1 = PxTransform((PxVec3&)t, (PxQuat&)r);
		if (chFrame2 && chFrame2->GetDecomposedMatrix(t, r, s))
			f2 = PxTransform((PxVec3&)t, (PxQuat&)r);
	}
	
	_joint->setParentPose(f1);
	_joint->setChildPose(f2);
	_joint->setJointType(_jointType);
	_joint->setFrictionCoefficient(_frictionCoefficient);
	_joint->setMaxJointVelocity(_maxJointVelocity);
	for (uint32 i = 0; i < PxArticulationAxis::eCOUNT; i++) {
		if (_axisParams[PxArticulationAxis::eTWIST + i].motion != PxArticulationMotion::eLOCKED) {
			auto a = (PxArticulationAxis::Enum)(PxArticulationAxis::eTWIST + i);
			_joint->setMotion(a, _axisParams[a].motion);
			_joint->setLimit(a, _axisParams[a].lowLimit, _axisParams[a].highLimit);
			_joint->setDrive(a, _axisParams[a].stiffness, _axisParams[a].damping, _axisParams[a].maxForce, _axisParams[a].isAccelerationDrive ? PxArticulationDriveType::eACCELERATION : PxArticulationDriveType::eFORCE);
			_joint->setDriveTarget(a, _axisParams[a].driveTarget);
			_joint->setDriveVelocity(a, _axisParams[a].driveTargetVel);
		}
	}

	return l;
}

void PhysXArticulationJointRC::ReleaseJoint()
{
	_joint = nullptr;
}

void PhysXArticulationJointRC::SetType(PxArticulationJointType::Enum t)
{
	_jointType = t;
	if (_joint) {
		_joint->getChildArticulationLink().getArticulation().wakeUp();
		_joint->setJointType(t);
	}
}

void PhysXArticulationJointRC::SetFrictionCoefficient(PxReal f)
{
	_frictionCoefficient = f;
	if (_joint) {
		_joint->getChildArticulationLink().getArticulation().wakeUp();
		_joint->setFrictionCoefficient(f);
	}
}

void PhysXArticulationJointRC::SetMaxJointVelocity(PxReal f)
{
	_maxJointVelocity = f;
	if (_joint) {
		_joint->getChildArticulationLink().getArticulation().wakeUp();
		_joint->setMaxJointVelocity(f);
	}
}

void PhysXArticulationJointRC::SetMotion(PxArticulationAxis::Enum axis, PxArticulationMotion::Enum motion)
{
	_axisParams[axis].motion = motion;
	if (_joint) {
		_joint->getChildArticulationLink().getArticulation().wakeUp();
		_joint->setMotion(axis, motion);
	}
}

void PhysXArticulationJointRC::SetLowLimit(PxArticulationAxis::Enum axis, PxReal lowLimit)
{
	_axisParams[axis].lowLimit = lowLimit;
	if (_joint) {
		_joint->getChildArticulationLink().getArticulation().wakeUp();
		_joint->setLimit(axis, lowLimit, _axisParams[axis].highLimit);
	}
}

void PhysXArticulationJointRC::SetHighLimit(PxArticulationAxis::Enum axis, PxReal highLimit)
{
	_axisParams[axis].highLimit = highLimit;
	if (_joint) {
		_joint->getChildArticulationLink().getArticulation().wakeUp();
		_joint->setLimit(axis, _axisParams[axis].lowLimit, highLimit);
	}
}

void PhysXArticulationJointRC::SetStiffness(PxArticulationAxis::Enum axis, PxReal stiffness)
{
	_axisParams[axis].stiffness = stiffness;
	if (_joint) {
		_joint->getChildArticulationLink().getArticulation().wakeUp();
		_joint->setDrive(axis, stiffness, _axisParams[axis].damping, _axisParams[axis].maxForce, _axisParams[axis].isAccelerationDrive ? PxArticulationDriveType::eACCELERATION : PxArticulationDriveType::eFORCE);
	}
}

void PhysXArticulationJointRC::SetDamping(PxArticulationAxis::Enum axis, PxReal damping)
{
	_axisParams[axis].damping = damping;
	if (_joint) {
		_joint->getChildArticulationLink().getArticulation().wakeUp();
		_joint->setDrive(axis, _axisParams[axis].stiffness, damping, _axisParams[axis].maxForce, _axisParams[axis].isAccelerationDrive ? PxArticulationDriveType::eACCELERATION : PxArticulationDriveType::eFORCE);
	}
}

void PhysXArticulationJointRC::SetMaxForce(PxArticulationAxis::Enum axis, PxReal maxForce)
{
	_axisParams[axis].maxForce = maxForce;
	if (_joint) {
		_joint->getChildArticulationLink().getArticulation().wakeUp();
		_joint->setDrive(axis, _axisParams[axis].stiffness, _axisParams[axis].damping, maxForce, _axisParams[axis].isAccelerationDrive ? PxArticulationDriveType::eACCELERATION : PxArticulationDriveType::eFORCE);
	}
}

void PhysXArticulationJointRC::SetIsAccelerationDrive(PxArticulationAxis::Enum axis, bool isAccelerationDrive)
{
	_axisParams[axis].isAccelerationDrive = isAccelerationDrive;
	if (_joint) {
		_joint->getChildArticulationLink().getArticulation().wakeUp();
		_joint->setDrive(axis, _axisParams[axis].stiffness, _axisParams[axis].damping, _axisParams[axis].maxForce, isAccelerationDrive ? PxArticulationDriveType::eACCELERATION : PxArticulationDriveType::eFORCE);
	}
}

void PhysXArticulationJointRC::SetDriveTarget(PxArticulationAxis::Enum axis, PxReal driveTarget)
{
	_axisParams[axis].driveTarget = driveTarget;
	if (_joint) {
		_joint->getChildArticulationLink().getArticulation().wakeUp();
		_joint->setDriveTarget(axis, driveTarget);
	}
}

void PhysXArticulationJointRC::SetDriveTargetVel(PxArticulationAxis::Enum axis, PxReal driveTargetVel)
{
	_axisParams[axis].driveTargetVel = driveTargetVel;
	if (_joint) {
		_joint->getChildArticulationLink().getArticulation().wakeUp();
		_joint->setDriveVelocity(axis, driveTargetVel);
	}
}