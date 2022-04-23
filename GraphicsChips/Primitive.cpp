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
#include "Primitive.h"
#include "M3DEngine/DocumentSaveLoadUtil.h"
#include "StdChips/Value.h"
#include "StdChips/VectorChip.h"
#include "StdChips/MatrixChip.h"
#include "Primitives/Primitives.h"
#include "M3DEngine/ProjectDependencies.h"
#include <d3d12.h>
#include <DirectXTK12/SimpleMath.h>


using namespace m3d;



CHIPDESCV1_DEF(Primitive, MTEXT("Primitive"), PRIMITIVE_GUID, STDGEOMETRY_GUID);



Primitive::Primitive()
{
	CREATE_CHILD(0, VECTORCHIP_GUID, false, UP, MTEXT("Subdivision"));
	CREATE_CHILD(1, VECTORCHIP_GUID, false, UP, MTEXT("Dimensions"));
	CREATE_CHILD(2, MATRIXCHIP_GUID, false, UP, MTEXT("Transform"));
}

bool Primitive::CopyChip(Chip* chip)
{
	Primitive* c = dynamic_cast<Primitive*>(chip);
	B_RETURN(Chip::CopyChip(c)); // Skip Geometry!
	SetPrimitiveType(c->_pt);
	_flags = c->_flags;
	return true;
}

bool Primitive::LoadChip(DocumentLoader& loader)
{
	B_RETURN(Chip::LoadChip(loader)); // Skip Geometry!
	PrimitiveType pt;
	LOAD("primitiveType|pt", pt);
	LOADDEF("flags", _flags, PrimitiveFlags::NONE);
	SetPrimitiveType(pt);
	return true;
}

bool Primitive::SaveChip(DocumentSaver& saver) const
{
	B_RETURN(Chip::SaveChip(saver)); // Skip Geometry!
	SAVE("primitiveType", _pt);
	SAVEDEF("flags", _flags, PrimitiveFlags::NONE);
	return true;
}

void Primitive::SetPrimitiveType(PrimitiveType pt)
{
	if (_pt == pt)
		return;
	_pt = pt;
	Clear();
}

void Primitive::SetFlags(PrimitiveFlags f)
{
	if (_flags == f)
		return;
	_flags = f;
	Clear();
}


void Primitive::Update()
{
	if (Refresh) {
		XMUINT4 subdiv = XMUINT4(10, 10, 10, 2);
		XMFLOAT4 dim = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		ChildPtr<VectorChip> ch0 = GetChild(0);
		if (ch0) {
			XMFLOAT4 v = ch0->GetVector();
			subdiv = XMUINT4(std::max(uint32(v.x), 1u), std::max(uint32(v.y), 1u), std::max(uint32(v.z), 1u), std::max(uint32(v.w), 1u));
		}
		ChildPtr<VectorChip> ch1 = GetChild(1);
		if (ch1) {
			XMFLOAT4 v = ch1->GetVector();
			dim = v;
		}

		bool t = false;
		ChildPtr<MatrixChip> ch2 = GetChild(2);
		XMFLOAT4X4 transform = ch2 ? ch2->GetMatrix() : MatrixChip::IDENTITY;
		for (uint32 i = 0; i < 16; i++)
			if (((float32*)& transform._11)[i] != ((float32*)& _transform._11)[i])
				t = true;

		if (t || std::memcmp(&_subdivision, &subdiv, sizeof(XMUINT4)) != 0 || std::memcmp(&_dimensions, &dim, sizeof(XMFLOAT4)) != 0) {
			_subdivision = subdiv;
			_dimensions = dim;
			_transform = transform;
			Clear();
		}
	}
}

void buildPrimitiveMesh(Primitive* p, const prim::Mesh& mesh, String name)
{
	XMMATRIX transform = XMLoadFloat4x4(&p->GetTransform());

	if (!XMMatrixIsIdentity(transform)) {
		XMMATRIX ntransform = XMMatrixTranspose(XMMatrixInverse(nullptr, transform));
		XMVector3TransformCoordStream((XMFLOAT3*)& mesh.p.front(), sizeof(XMFLOAT3), (XMFLOAT3*)& mesh.p.front(), sizeof(XMFLOAT3), mesh.p.size(), transform);
		XMVector3TransformNormalStream((XMFLOAT3*)& mesh.n.front(), sizeof(XMFLOAT3), (XMFLOAT3*)& mesh.n.front(), sizeof(XMFLOAT3), mesh.n.size(), ntransform);
	}

	bool eN = (p->GetFlags() & Primitive::PrimitiveFlags::NORMALS) != Primitive::PrimitiveFlags::NONE;
	bool eTC = (p->GetFlags() & Primitive::PrimitiveFlags::TEXCOORDS) != Primitive::PrimitiveFlags::NONE;
	bool eT = (p->GetFlags() & Primitive::PrimitiveFlags::TANGENTS) != Primitive::PrimitiveFlags::NONE;

	for (size_t i = 0; i < mesh.p.size(); i++) p->AddPosition((XMFLOAT3&)mesh.p[i]);
	if (eN)
		for (size_t i = 0; i < mesh.n.size(); i++) p->AddNormal((XMFLOAT3&)mesh.n[i]);
	if (eTC)
		for (size_t i = 0; i < mesh.tc.size(); i++) p->AddTexCoord((XMFLOAT2&)mesh.tc[i], 0);
	if (eT) {
		for (size_t i = 0; i < mesh.t.size(); i++) p->AddTangent((XMFLOAT3&)mesh.t[i]);
		for (size_t i = 0; i < mesh.bt.size(); i++) p->AddBitangent((XMFLOAT3&)mesh.bt[i]);
	}
	for (size_t i = 0; i < mesh.indices.size(); i++) {
		p->AddIndex(mesh.indices[i].v0);
		p->AddIndex(mesh.indices[i].v1);
		p->AddIndex(mesh.indices[i].v2);
	}
	p->CommitSubset(M3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST, name);
}

void Primitive::CreateDeviceObjects()
{
	_subsets.clear();

	switch (_pt)
	{
	case PrimitiveType::SQUARE:
	{
		uint32 indexBase = 0;
		XMMATRIX transform = XMLoadFloat4x4(&_transform);
		_generateSquare(_subdivision.x, _subdivision.y, indexBase, transform);
		CommitSubset(M3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP, MTEXT("Square"));
		break;
	}
	case PrimitiveType::BOX:
	{
		prim::Mesh mesh;
		if (prim::Primitives::boxMesh(mesh, { _dimensions.x, _dimensions.y, _dimensions.z }, { (int)_subdivision.x, (int)_subdivision.y, (int)_subdivision.z })) // 3xsize, 3xsegments
			buildPrimitiveMesh(this, mesh, MTEXT("Box"));
		break;
	}
	case PrimitiveType::SPHERE:
	{
		prim::Mesh mesh;
		if (prim::Primitives::sphereMesh(mesh, _dimensions.x, _subdivision.x, _subdivision.y)) // radius, slices, segments
			buildPrimitiveMesh(this, mesh, MTEXT("Sphere"));
		break;
	}
	case PrimitiveType::CYLINDER:
	{
		prim::Mesh mesh;
		if (prim::Primitives::cylinderMesh(mesh, _dimensions.x, _dimensions.y, _subdivision.x, _subdivision.y)) // radius, size, slices, segments
			buildPrimitiveMesh(this, mesh, MTEXT("Cylinder"));
		break;
	}
	case PrimitiveType::CONE:
	{
		prim::Mesh mesh;
		if (prim::Primitives::coneMesh(mesh, _dimensions.x, _dimensions.y, _subdivision.x, _subdivision.y)) // radius, size, slices, segments
			buildPrimitiveMesh(this, mesh, MTEXT("Cone"));
		break;
	}
	case PrimitiveType::TORUS:
	{
		prim::Mesh mesh;
		if (prim::Primitives::torusMesh(mesh, _dimensions.y, _dimensions.x, _subdivision.x, _subdivision.y)) // innerRadius, outerradius, slices, segments
			buildPrimitiveMesh(this, mesh, MTEXT("Torus"));
		break;
	}
	case PrimitiveType::CAPSULE:
	{
		prim::Mesh mesh;
		if (prim::Primitives::capsuleMesh(mesh, _dimensions.x, _dimensions.y, _subdivision.x, _subdivision.y, _subdivision.z)) // radius, size, slices, segments, rings
			buildPrimitiveMesh(this, mesh, MTEXT("Capsule"));
		break;
	}
	case PrimitiveType::CAPPED_CYLINDER:
	{
		prim::Mesh mesh;
		if (prim::Primitives::cappedCylinderMesh(mesh, _dimensions.x, _dimensions.y, _subdivision.x, _subdivision.y, _subdivision.z)) // radius, size, slices, segments, rings
			buildPrimitiveMesh(this, mesh, MTEXT("Capped cylinder"));
		break;
	}
	case PrimitiveType::CAPPED_CONE:
	{
		prim::Mesh mesh;
		if (prim::Primitives::cappedConeMesh(mesh, _dimensions.x, _dimensions.y, _subdivision.x, _subdivision.y, _subdivision.z)) // radius, size, slices, segments, rings
			buildPrimitiveMesh(this, mesh, MTEXT("Capped cone"));
		break;
	}
	case PrimitiveType::CAPPED_TUBE:
	{
		prim::Mesh mesh;
		if (prim::Primitives::cappedTubeMesh(mesh, _dimensions.x, _dimensions.y, _dimensions.z, _subdivision.x, _subdivision.y, _subdivision.z)) // radius, innerRadius, size, slices, segments, rings
			buildPrimitiveMesh(this, mesh, MTEXT("Capped tube"));
		break;
	}
	case PrimitiveType::DODECAHEDRON:
	{
		prim::Mesh mesh;
		if (prim::Primitives::dodecahedronMesh(mesh, _dimensions.x, _subdivision.x, _subdivision.y)) // radius, segments, rings
			buildPrimitiveMesh(this, mesh, MTEXT("Dodecahedron"));
		break;
	}
	case PrimitiveType::DISK:
	{
		prim::Mesh mesh;
		if (prim::Primitives::diskMesh(mesh, _dimensions.x, _dimensions.y, _subdivision.x, _subdivision.y)) // radius, innerRadius, slices, rings
			buildPrimitiveMesh(this, mesh, MTEXT("Disk"));
		break;
	}
	case PrimitiveType::ICOSAHEDRON:
	{
		prim::Mesh mesh;
		if (prim::Primitives::icosahedronMesh(mesh, _dimensions.x, _subdivision.x)) // radius, segments
			buildPrimitiveMesh(this, mesh, MTEXT("Icosahedron"));
		break;
	}
	case PrimitiveType::ICOSPHERE:
	{
		prim::Mesh mesh;
		if (prim::Primitives::icoSphereMesh(mesh, _dimensions.x, _subdivision.x)) // radius, segments
			buildPrimitiveMesh(this, mesh, MTEXT("Icosphere"));
		break;
	}
	case PrimitiveType::PLANE:
	{
		prim::Mesh mesh;
		if (prim::Primitives::planeMesh(mesh, { _dimensions.x, _dimensions.y }, { (int)_subdivision.x, (int)_subdivision.y })) // 2xsize, 2xsegments
			buildPrimitiveMesh(this, mesh, MTEXT("Plane"));
		break;
	}
	case PrimitiveType::ROUNDED_BOX:
	{
		prim::Mesh mesh;
		if (prim::Primitives::roundedBoxMesh(mesh, _dimensions.w, { _dimensions.x, _dimensions.y, _dimensions.z }, _subdivision.w, { (int)_subdivision.x, (int)_subdivision.y, (int)_subdivision.z })) // radius, 3xsize, slices, 3xsegments
			buildPrimitiveMesh(this, mesh, MTEXT("Rounded box"));
		break;
	}
	case PrimitiveType::SPHERICAL_CONE:
	{
		prim::Mesh mesh;
		if (prim::Primitives::sphericalConeMesh(mesh, _dimensions.x, _dimensions.y, _subdivision.x, _subdivision.y, _subdivision.z)) // radius, size, slices, segments, rings
			buildPrimitiveMesh(this, mesh, MTEXT("Spherical cone"));
		break;
	}
	case PrimitiveType::SPHEREICAL_TRIANGLE:
	{
		prim::Mesh mesh;
		if (prim::Primitives::sphericalTriangleMesh(mesh, _dimensions.x, _subdivision.x)) // radius, segments (3xpoints, segments)
			buildPrimitiveMesh(this, mesh, MTEXT("Spherical triangle"));
		break;
	}
	case PrimitiveType::SPRING:
	{
		prim::Mesh mesh;
		if (prim::Primitives::springMesh(mesh, _dimensions.y, _dimensions.x, _dimensions.z, _subdivision.x, _subdivision.y)) // minor, major, size, slice, segments
			buildPrimitiveMesh(this, mesh, MTEXT("Spring"));
		break;
	}
	case PrimitiveType::TEAPOT:
	{
		prim::Mesh mesh;
		if (prim::Primitives::teapotMesh(mesh, _dimensions.x, _subdivision.x)) // segments
			buildPrimitiveMesh(this, mesh, MTEXT("Teapot"));
		break;
	}
	case PrimitiveType::TORUS_KNOT:
	{
		prim::Mesh mesh;
		if (prim::Primitives::torusKnotMesh(mesh, _subdivision.w, _subdivision.z, _subdivision.x, _subdivision.y)) // p, q, slices, segments
			buildPrimitiveMesh(this, mesh, MTEXT("Torus knot"));
		break;
	}
	case PrimitiveType::TRIANGLE:
	{
		prim::Mesh mesh;
		if (prim::Primitives::triangleMesh(mesh, _dimensions.x, _subdivision.x)) // radius, segments (3xpoints, segments)
			buildPrimitiveMesh(this, mesh, MTEXT("Triangle"));
		break;
	}
	case PrimitiveType::TUBE:
	{
		prim::Mesh mesh;
		if (prim::Primitives::tubeMesh(mesh, _dimensions.x, _dimensions.y, _dimensions.z, _subdivision.x, _subdivision.y)) // radius, innerRadius, size, slices, segments
			buildPrimitiveMesh(this, mesh, MTEXT("Tube"));
		break;
	}
	case PrimitiveType::GEOMERTYLESS_POINTLIST:
	{
		GeometrySubset ss;
		ss.name = MTEXT("Point List");
		ss.pt = M3D_PRIMITIVE_TOPOLOGY_POINTLIST;
		ss.baseVertexLocation = 0;
		ss.startLocation = 0;
		ss.count = std::max(1u, _subdivision.x);
		AddSubset(ss);
		break;
	}
	break;
	case PrimitiveType::GEOMERTYLESS_LINELIST:
	{
		GeometrySubset ss;
		ss.name = MTEXT("Line List");
		ss.pt = M3D_PRIMITIVE_TOPOLOGY_LINELIST;
		ss.baseVertexLocation = 0;
		ss.startLocation = 0;
		ss.count = std::max(1u, _subdivision.x) * 2;
		AddSubset(ss);
		break;
	}
	break;
	case PrimitiveType::GEOMERTYLESS_LINESTRIP:
	{
		GeometrySubset ss;
		ss.name = MTEXT("Line Strip");
		ss.pt = M3D_PRIMITIVE_TOPOLOGY_LINESTRIP;
		ss.baseVertexLocation = 0;
		ss.startLocation = 0;
		ss.count = std::max(2u, _subdivision.x);
		AddSubset(ss);
		break;
	}
	break;
	case PrimitiveType::GEOMERTYLESS_TRIANGLELIST:
	{
		GeometrySubset ss;
		ss.name = MTEXT("Triangle List");
		ss.pt = M3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		ss.baseVertexLocation = 0;
		ss.startLocation = 0;
		ss.count = std::max(1u, _subdivision.x) * 3;
		AddSubset(ss);
		break;
		//return D3D12Geometry::CreateDeviceObjects(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, { (const D3D12_INPUT_ELEMENT_DESC*)nullptr, 0u }, List<DataBuffer>(), List<UINT>());
	}
	break;
	case PrimitiveType::GEOMETRYLESS_TRIANGLESTRIP:
	{
		GeometrySubset ss;
		ss.name = MTEXT("Triangle Strip");
		ss.pt = M3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
		ss.baseVertexLocation = 0;
		ss.startLocation = 0;
		ss.count = std::max(3u, _subdivision.x);
		AddSubset(ss);
		break;
	}
	break;
	case PrimitiveType::RANDOM_POINTS:
	{
		XMMATRIX transform = XMLoadFloat4x4(&_transform);
		for (uint32 i = 0, j = std::max(2u, _subdivision.x); i < j; i++) {
			XMFLOAT3 p;
			XMStoreFloat3(&p, XMVector3TransformCoord(XMVectorSet(float32(rand()) / RAND_MAX - 0.5f, float32(rand()) / RAND_MAX - 0.5f, float32(rand()) / RAND_MAX - 0.5f, 0.0f), transform));
			AddPosition(p);
		}
		CommitSubset(M3D_PRIMITIVE_TOPOLOGY_POINTLIST, MTEXT("Random Points"));
	}
	break;
	default:
		break;
	};

	/*
	case PrimitiveType::SPHERE:
	{
	uint32 subdiv = _subdiv == -1 ? 14 : _subdiv;
	uint32 indexBase = 0;
	_generateSphere(subdiv, indexBase, transform);
	CommitSubset(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP, MTEXT("Sphere"));
	}
	break;*/

	StdGeometry::CreateDeviceObjects();

//	return CreateDeviceObjects();
}

void Primitive::_generateSquare(uint32 subdivU, uint32 subdivV, uint32& indexBase, CXMMATRIX m)
{
	subdivU += 2;
	subdivV += 2;
	SimpleMath::Vector3 n, t, b;

	SimpleMath::Vector3::TransformNormal(SimpleMath::Vector3(0.0f, 0.0f, -1.0f), m).Normalize(n);
	SimpleMath::Vector3::TransformNormal(SimpleMath::Vector3(1.0f, 0.0f, 0.0f), m).Normalize(t);
	SimpleMath::Vector3::TransformNormal(SimpleMath::Vector3(0.0f, 1.0f, 0.0f), m).Normalize(b);

	bool eN = (_flags & PrimitiveFlags::NORMALS) != PrimitiveFlags::NONE;
	bool eT = (_flags & PrimitiveFlags::TEXCOORDS) != PrimitiveFlags::NONE;
	bool eTB = (_flags & PrimitiveFlags::TANGENTS) != PrimitiveFlags::NONE;

	for (uint32 j = 0; j < subdivV; j++) {
		for (uint32 i = 0; i < subdivU; i++) {
			AddPosition(SimpleMath::Vector3::Transform(SimpleMath::Vector3(-0.5f + (FLOAT)i / (subdivU - 1), -0.5f + (FLOAT)j / (subdivV - 1), 0.0f), m));
			if (eN)
				AddNormal(n);
			if (eT)
				AddTexCoord(SimpleMath::Vector2((FLOAT)i / (subdivU - 1), (FLOAT)(subdivV - 1 - j) / (subdivV - 1)));
			if (eTB) {
				AddTangent(t);
				AddBitangent(b);
			}
			if (j < subdivV - 1) {
				AddIndex(indexBase + j * subdivU + i);
				AddIndex(indexBase + j * subdivU + subdivU + i);
			}
		}
		if (j < subdivV - 2) {
//			if (graphics()->GetFeatureLevel() >= D3D_FEATURE_LEVEL_10_0) // Restarting strip does not work for <FL10!
				AddIndex(-1);
//			else {
//				for (uint32 i = 0; i < subdivU; i++) {
//					AddIndex(indexBase + j * subdivU + 2 * subdivU - i - 1);
//					AddIndex(indexBase + j * subdivU + 2 * subdivU - i - 1);
//				}
//			}
		}
	}

	indexBase += subdivU * subdivV;
}

void Primitive::_generateSphere(uint32 subdiv, uint32& indexBase, CXMMATRIX m)
{
	subdiv += 2;

	const uint32 t = 4; // Number of sections to divide the xz circle into.

	bool eN = (_flags & PrimitiveFlags::NORMALS) != PrimitiveFlags::NONE;
	bool eT = (_flags & PrimitiveFlags::TEXCOORDS) != PrimitiveFlags::NONE;
	bool eTB = (_flags & PrimitiveFlags::TANGENTS) != PrimitiveFlags::NONE;

	uint32 k = 0, n = t + 1;
	for (uint32 j = 0; j < subdiv; j++, k = n, n = j * t + 1) { // lower part
																  //n = uint32(4.0f * (subdiv - 1) * sin(XM_PIDIV2 * std::max(j, 1) / (subdiv - 1)) + 1.5f); // note: the formula for n makes sure we have an even triangulation over the sphere.
		float32 h = (float32)j / (subdiv - 1);
		float32 y = -cos(XM_PIDIV2 * h);
		float32 r = sqrt(1.0f - y * y);
		for (uint32 i = 0; i < n; i++) {
			float32 w = (float32)i / (n - 1);
			float32 x = cos(XM_2PI * w) * r;
			float32 z = sin(XM_2PI * w) * r;
			AddPosition(SimpleMath::Vector3::Transform(SimpleMath::Vector3(x, y, z), m));
			if (eN)
				AddNormal(SimpleMath::Vector3::TransformNormal(SimpleMath::Vector3(x, y, z), m));
			if (eT)
				AddTexCoord(SimpleMath::Vector2(w, 1.0f - 0.5f * h));
			if (eTB) {
				float32 tx = cos(XM_2PI * w + XM_PIDIV2);
				float32 tz = sin(XM_2PI * w + XM_PIDIV2);
				AddTangent(SimpleMath::Vector3::TransformNormal(SimpleMath::Vector3(tx, 0.0f, tz), m));
				AddBitangent(SimpleMath::Vector3::TransformNormal(SimpleMath::Vector3(tx, 0.0f, tz).Cross(SimpleMath::Vector3(x, y, z)), m));
			}
			if (j > 0) {
				AddIndex(indexBase + i * (k - 1) / (n - 1));
				AddIndex(indexBase + k + i);
			}
		}
		if (j > 0) {
			//AddIndex(-1); // Not neccessary, but it prevent some artifacts in wireframe mode. (Not available for FL<10)
			indexBase += k;
		}
	}

	for (uint32 j = 0; j < subdiv - 1; j++, k = n) { // upper part
													   //n = uint32(4.0f * (subdiv - 1) * sin(XM_PIDIV2 * std::max(subdiv - j - 2, 1) / (subdiv - 1)) + 1.5f);
		n = std::max(k - t, t + 1);
		float32 h = (float32)(subdiv - j - 2) / (subdiv - 1);
		float32 y = cos(XM_PIDIV2 * h);
		float32 r = sqrt(1.0f - y * y);
		for (uint32 i = 0; i < n; i++) {
			float32 w = (float32)i / (n - 1);
			float32 x = cos(XM_2PI * w) * r;
			float32 z = sin(XM_2PI * w) * r;
			AddPosition(SimpleMath::Vector3::Transform(SimpleMath::Vector3(x, y, z), m));
			if (eN)
				AddNormal(SimpleMath::Vector3::TransformNormal(SimpleMath::Vector3(x, y, z), m));
			if (eT)
				AddTexCoord(SimpleMath::Vector2(w, 0.5f * h));
			if (eTB) {
				float32 tx = cos(XM_2PI * w + XM_PIDIV2);
				float32 tz = sin(XM_2PI * w + XM_PIDIV2);
				AddTangent(SimpleMath::Vector3::TransformNormal(SimpleMath::Vector3(tx, 0.0f, tz), m));
				AddBitangent(SimpleMath::Vector3::TransformNormal(SimpleMath::Vector3(tx, 0.0f, tz).Cross(SimpleMath::Vector3(x, y, z)), m));
			}
		}
		AddIndex(indexBase);
		AddIndex(indexBase + k);
		for (uint32 i = 1; i < k - 1; i++) {
			AddIndex(indexBase + i);
			AddIndex(indexBase + k + (i + 1) * (n - 1) / (k - 1));
		}
		AddIndex(indexBase + k - 1);
		AddIndex(indexBase + k + n - 1);
		//AddIndex(-1); // Not neccessary, but it prevent some artifacts in wireframe mode. (Not available for FL<10)
		indexBase += k;
	}
	indexBase += k;
}
