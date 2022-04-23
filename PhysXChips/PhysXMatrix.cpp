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
#include "PhysXMatrix.h"
#include "PhysXRigidActor.h"
#include "PhysX.h"

using namespace m3d;


CHIPDESCV1_DEF(PhysXMatrix, MTEXT("PhysX Matrix"), PHYSXMATRIX_GUID, MATRIXCHIP_GUID);


PhysXMatrix::PhysXMatrix()
{
	CREATE_CHILD(0, PHYSXRIGIDACTOR_GUID, false, BOTH, MTEXT("Actor"));
}

PhysXMatrix::~PhysXMatrix()
{
}

const XMFLOAT4X4 &PhysXMatrix::GetMatrix()
{
	ChildPtr<PhysXRigidActor> chActor = GetChild(0);
	if (!chActor)
		return _matrix = IDENTITY;

	PxRigidActor *actor = chActor->GetRigidActor();
	if (!actor)
		return _matrix = IDENTITY;

	PhysXActorData *data = (PhysXActorData*)actor->userData;

	PxMat44 gp(data->globalPose);
	_matrix = XMFLOAT4X4((float32*)&gp);

	return _matrix;
}

void PhysXMatrix::SetMatrix(const XMFLOAT4X4 &m)
{
	ChildPtr<PhysXRigidActor> chActor = GetChild(0);
	if (!chActor)
		return;

	PxRigidActor *actor = chActor->GetRigidActor();
	if (!actor)
		return;

	PxTransform t = PxTransform((PxMat44&)m);
	if (!t.isSane())
		return;

	_matrix = m;

	if (actor->is<PxRigidDynamic>()) {
		if (actor->is<PxRigidDynamic>()->getRigidBodyFlags() & PxRigidBodyFlag::eKINEMATIC) {
			PhysXActorData *p = (PhysXActorData*)actor->userData;
			assert(p);
			p->kinematicTarget = t;
			p->moveKinematic = true;
			return;
		}
	}
	actor->setGlobalPose(t);
}