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
#include "PhysXConvexMesh.h"
#include "GraphicsChips/StdGeometry.h"
#include "StdChips/MatrixChip.h"
#include <cooking/PxCooking.h> 
#include "PhysXSDK.h"


using namespace m3d;


CHIPDESCV1_DEF(PhysXConvexMesh, MTEXT("PhysX Convex Mesh"), PHYSXCONVEXMESH_GUID, CHIP_GUID);


PhysXConvexMesh::PhysXConvexMesh()
{
	CREATE_CHILD(0, STDGEOMETRY_GUID, true, UP, MTEXT("Geometries"));

	_mesh = nullptr;
}

PhysXConvexMesh::~PhysXConvexMesh()
{
	if (_mesh)
		_mesh->release();
}

bool PhysXConvexMesh::CopyChip(Chip *chip) 
{
	PhysXConvexMesh *c = dynamic_cast<PhysXConvexMesh*>(chip);
	B_RETURN(Chip::CopyChip(c));

	return true;
}

bool PhysXConvexMesh::LoadChip(DocumentLoader &loader) 
{
	B_RETURN(Chip::LoadChip(loader));

	return true;
}

bool PhysXConvexMesh::SaveChip(DocumentSaver &saver) const
{
	B_RETURN(Chip::SaveChip(saver));

	return true;
}

PxConvexMesh *PhysXConvexMesh::GetConvexMesh()
{
	if (_mesh)
		return _mesh;

	List<PxVec3> vertices;

	for (uint32 i = 0; i < GetSubConnectionCount(0); i++) {
		ChildPtr<StdGeometry> ch = GetChild(0, i);
		if (!ch)
			continue;
		const List<XMFLOAT3> &v = ch->GetPositions();
		for (size_t j = 0; j < v.size(); j++)
			vertices.push_back(*(PxVec3*)&v[j]);
	}

	if (vertices.empty())
		return nullptr;

	PxConvexMeshDesc convexDesc;
	convexDesc.points.count = (PxU32)vertices.size();
	convexDesc.points.stride = sizeof(PxVec3);
	convexDesc.points.data = &vertices.front();
	convexDesc.flags = PxConvexFlag::eCOMPUTE_CONVEX;

	PhysXSDK *sdk = (PhysXSDK*)engine->GetChipManager()->GetGlobalChip(PHYSXSDK_GUID);
	PxCooking *cooking = sdk->GetCooking();
	if (!cooking)
		return nullptr;
	/*
	PxDefaultMemoryOutputStream buf;
	if(!cooking->cookConvexMesh(convexDesc, buf))
		return nullptr;

	PxDefaultMemoryInputData input(buf.getData(), buf.getSize());
	_mesh = sdk->GetPhysics()->createConvexMesh(input);
	*/
	return _mesh;
}