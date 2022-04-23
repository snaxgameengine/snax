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
#include "PhysXShape.h"
#include "StdChips/MatrixChip.h"
#include "PhysXMaterial.h"
#include "StdChips/Value.h"
#include "PhysXSDK.h"
#include "PhysXConvexMesh.h"
#include "PhysXTriangleMesh.h"



using namespace m3d;


CHIPDESCV1_DEF(PhysXShape, MTEXT("PhysX Shape"), PHYSXSHAPE_GUID, CHIP_GUID);


PhysXShape::PhysXShape()
{
	CREATE_CHILD(0, MATRIXCHIP_GUID, false, UP, MTEXT("Transformation"));
	CREATE_CHILD(1, PHYSXMATERIAL_GUID, false, UP, MTEXT("Material"));
	CREATE_CHILD(2, VALUE_GUID, false, UP, MTEXT("Mass"));

	_shape = nullptr;
	_mass = 1.0f;
	_type = T_NONE;
}

PhysXShape::~PhysXShape()
{
	PX_RELEASE(_shape)
}

bool PhysXShape::CopyChip(Chip *chip)
{
	PhysXShape *c = dynamic_cast<PhysXShape*>(chip);
	B_RETURN(Chip::CopyChip(c));
	SetType(c->_type);
	return true;
}

bool PhysXShape::LoadChip(DocumentLoader &loader)
{
	B_RETURN(Chip::LoadChip(loader));
	Type type;
	LOAD("type", (uint32&)type);
	SetType(type);
	return true;
}

bool PhysXShape::SaveChip(DocumentSaver &saver) const
{
	B_RETURN(Chip::SaveChip(saver));
	SAVE("type", (uint32)_type);
	return true;
}

void PhysXShape::SetType(Type t)
{
	if (_type == t)
		return;

	_type = t;

	switch (_type)
	{
	case T_BOX:
	case T_CAPSULE:
	case T_PLANE:
	case T_SPHERE:
		ClearConnections(3);
		break;
	case T_CONVEX_MESH:
		CREATE_CHILD(3, PHYSXCONVEXMESH_GUID, false, UP, MTEXT("Convex Mesh"));
		ClearConnections(4);
		break;
	case T_HEIGHT_FIELD:
		ClearConnections();
		break;
	case T_TRIANGLE_MESH:
		CREATE_CHILD(3, PHYSXTRIANGLEMESH_GUID, false, UP, MTEXT("Triangle Mesh"));
		ClearConnections(4);
		break;
	default:
		ClearConnections();
		break;
	}

	PX_RELEASE(_shape)
}

PxShape *PhysXShape::GetShape()
{
	if (_shape)
		return _shape;

	PhysXSDK *sdk = (PhysXSDK*)engine->GetChipManager()->GetGlobalChip(PHYSXSDK_GUID);

	XMFLOAT3 scaling(1.0f, 1.0f, 1.0f);
	XMFLOAT4 rotation(0.0f, 0.0f, 0.0f, 1.0f);
	XMFLOAT3 translation(0.0f, 0.0f, 0.0f);

	ChildPtr<MatrixChip> ch0 = GetChild(0);
	if (ch0) {
		XMMATRIX m = XMLoadFloat4x4(&ch0->GetMatrix());
		XMVECTOR s, r, t;
		if (!XMMatrixDecompose(&s, &r, &t, m))
			return nullptr;
		XMStoreFloat3(&scaling, s);
		XMStoreFloat4(&rotation, r);
		XMStoreFloat3(&translation, t);
	}
	PxTransform transform(PxVec3(translation.x, translation.y, translation.z), PxQuat(rotation.x, rotation.y, rotation.z, rotation.w));

	PxMaterial *material = nullptr;
	ChildPtr<PhysXMaterial> ch1 = GetChild(1);
	if (ch1)
		material = ch1->GetMaterial();
	
	if (!material) {
		PhysXSDK *sdk = (PhysXSDK*)engine->GetChipManager()->GetGlobalChip(PHYSXSDK_GUID);
		material = sdk->GetDefaultMaterial();
		if (!material)
			return nullptr;
	}

	_mass = 1.0f;
	ChildPtr<Value> chMass = GetChild(2);
	if (chMass)
		_mass = (PxReal)chMass->GetValue();
	_mass = std::max(0.0f, _mass);
	
	switch (_type)
	{
	case T_BOX:
		_shape = sdk->GetPhysics()->createShape(PxBoxGeometry(scaling.x, scaling.y, scaling.z), *material); break;
	case T_CAPSULE:
		_shape = sdk->GetPhysics()->createShape(PxCapsuleGeometry(scaling.y, scaling.x * 0.5f), *material); break;
	case T_CONVEX_MESH:
		{
			ChildPtr<PhysXConvexMesh> ch = GetChild(3);
			PxConvexMesh *mesh = ch ? ch->GetConvexMesh() : nullptr;
			if (!mesh)
				return nullptr;
			_shape = sdk->GetPhysics()->createShape(PxConvexMeshGeometry(mesh, PxMeshScale(PxVec3(scaling.x, scaling.y, scaling.z), PxQuat(0.0f, 0.0f, 0.0f, 1.0f))), *material);
			break;
		}
	case T_HEIGHT_FIELD:
		return nullptr;
	case T_PLANE:
		_shape = sdk->GetPhysics()->createShape(PxPlaneGeometry(), *material); break;
	case T_SPHERE:
		_shape = sdk->GetPhysics()->createShape(PxSphereGeometry(scaling.x), *material); break;
	case T_TRIANGLE_MESH:
		{
			ChildPtr<PhysXTriangleMesh> ch = GetChild(3);
			PxTriangleMesh *mesh = ch ? ch->GetTriangleMesh() : nullptr;
			if (!mesh)
				return nullptr;
			_shape = sdk->GetPhysics()->createShape(PxTriangleMeshGeometry(mesh, PxMeshScale(PxVec3(scaling.x, scaling.y, scaling.z), PxQuat(0.0f, 0.0f, 0.0f, 1.0f))), *material);
			break;
		}
	default:
		break;
	}

	if (_shape) {
		_shape->setLocalPose(transform);
	}

	return _shape;
}


