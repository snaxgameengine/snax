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
#include "PhysXTriangleMesh.h"
#include "GraphicsChips/StdGeometry.h"
#include "StdChips/MatrixChip.h"
#include "PhysXSDK.h"
//#include <PxTkStream.h>
#include <cooking/PxCooking.h> 

using namespace m3d;


CHIPDESCV1_DEF(PhysXTriangleMesh, MTEXT("PhysX Triangle Mesh"), PHYSXTRIANGLEMESH_GUID, CHIP_GUID);


PhysXTriangleMesh::PhysXTriangleMesh()
{
	CREATE_CHILD(0, STDGEOMETRY_GUID, true, UP, MTEXT("Geometries"));

	_mesh = nullptr;
}

PhysXTriangleMesh::~PhysXTriangleMesh()
{
	if (_mesh)
		_mesh->release();
}

bool PhysXTriangleMesh::CopyChip(Chip *chip) 
{
	PhysXTriangleMesh *c = dynamic_cast<PhysXTriangleMesh*>(chip);
	B_RETURN(Chip::CopyChip(c));

	return true;
}

bool PhysXTriangleMesh::LoadChip(DocumentLoader &loader) 
{
	B_RETURN(Chip::LoadChip(loader));

	return true;
}

bool PhysXTriangleMesh::SaveChip(DocumentSaver &saver) const
{
	B_RETURN(Chip::SaveChip(saver));

	return true;
}

PxTriangleMesh *PhysXTriangleMesh::GetTriangleMesh()
{
	if (_mesh)
		return _mesh;
/*
	List<PxVec3> vertices;

	for (uint32 i = 0; i < GetSubConnectionCount(0); i++) {
		ChildPtr<StdDrawable> ch = GetChild(0, i);
		if (!ch)
			continue;
		const List<XMFLOAT3> &v = ch->GetPositions();
		for (size_t j = 0; j < v.size(); j++)
			vertices.push_back(*(PxVec3*)&v[j]);
	}

	if (vertices.empty())
		return nullptr;

	PxConvexMeshDesc convexDesc;
	convexDesc.points.count = vertices.size();
	convexDesc.points.stride = sizeof(PxVec3);
	convexDesc.points.data = &vertices.front();
	convexDesc.flags = PxConvexFlag::eCOMPUTE_CONVEX;

	PhysXSDK *sdk = (PhysXSDK*)engine->GetChipManager()->GetGlobalChip(PHYSXSDK_GUID);
	PxCooking *cooking = sdk->GetCooking();
	if (!cooking)
		return nullptr;

	PxToolkit::MemoryOutputStream buf;
	if(!cooking->cookConvexMesh(convexDesc, buf))
		return nullptr;

	PxToolkit::MemoryInputData input(buf.getData(), buf.getSize());
	_mesh = sdk->GetPhysics()->createConvexMesh(input);
	*/
	return _mesh;
}