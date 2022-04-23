// Copyright (c) 2013 - 2022 Frank-Vegar Mortensen <franksvm(at)outlook(dot)com>.
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.

#include "stdafx.h"
#include "Primitives.h"
//#include <generator\BezierMesh.hpp>
#include <generator\BoxMesh.hpp>
#include <generator\CappedCylinderMesh.hpp>
#include <generator\CappedConeMesh.hpp>
#include <generator\CappedTubeMesh.hpp>
#include <generator\ConeMesh.hpp>
#include <generator\CapsuleMesh.hpp>
//#include <generator\ConvexPolygonMesh.hpp>
#include <generator\CylinderMesh.hpp>
#include <generator\DodecahedronMesh.hpp>
#include <generator\DiskMesh.hpp>
#include <generator\IcosahedronMesh.hpp>
#include <generator\IcoSphereMesh.hpp>
#include <generator\PlaneMesh.hpp>
#include <generator\RoundedBoxMesh.hpp>
#include <generator\SphereMesh.hpp>
#include <generator\SphericalConeMesh.hpp>
#include <generator\SphericalTriangleMesh.hpp>
#include <generator\SpringMesh.hpp>
#include <generator\TeapotMesh.hpp>
#include <generator\TorusKnotMesh.hpp>
#include <generator\TorusMesh.hpp>
#include <generator\TubeMesh.hpp>
#include <generator\utils.hpp>
#include <generator\ScaleMesh.hpp>
#include <generator\RotateMesh.hpp>
#include "tgen.h"


template<typename T>
bool buildMesh(prim::Mesh &mesh, const T &n)
{
//	auto m = generator::rotateMesh(n, glm::dquat(sqrt(0.5), -sqrt(0.5), 0.0, 0.0));

	int vertexCount = generator::count(n.vertices());
	int indexCount = generator::count(n.triangles());

	if (vertexCount < 1 || indexCount < 3)
		return false;

	mesh.p.clear();
	mesh.n.clear();
	mesh.tc.clear();
	mesh.t.clear();
	mesh.bt.clear();
	mesh.tc.clear();
	mesh.indices.clear();

	mesh.p.reserve(vertexCount);
	mesh.n.reserve(vertexCount);
	mesh.tc.reserve(vertexCount);
	mesh.bt.reserve(vertexCount);
	mesh.tc.reserve(vertexCount);
	mesh.indices.reserve(indexCount);

	for (auto vertices = n.vertices(); !vertices.done(); vertices.next()) {
		auto vertex = vertices.generate();
		// Transform from RH til LH. y-axis is up in both cases.
		mesh.p.push_back({ (FLOAT)vertex.position.y, (FLOAT)vertex.position.z, (FLOAT)-vertex.position.x }); 
		mesh.n.push_back({ (FLOAT)vertex.normal.y, (FLOAT)vertex.normal.z, (FLOAT)-vertex.normal.x });
		// Swap s and t for texture coordinates.
		mesh.tc.push_back({(FLOAT)vertex.texCoord.y, (FLOAT)vertex.texCoord.x });
	}

	for (auto triangles = n.triangles(); !triangles.done(); triangles.next()) {
		auto triangle = triangles.generate();
		mesh.indices.push_back({ (unsigned)triangle.vertices.x, (unsigned)triangle.vertices.z, (unsigned)triangle.vertices.y }); // Reverse winding!
	}

	std::vector<prim::Vec3> t, bt;
	tgen::computeCornerTSpace((const std::vector<tgen::VIndexT>&)mesh.indices, (const std::vector<tgen::VIndexT>&)mesh.indices, (const std::vector<tgen::Vec3>&)mesh.p, (const std::vector<tgen::Vec2>&)mesh.tc, (std::vector<tgen::Vec3>&)t, (std::vector<tgen::Vec3>&)bt);
	tgen::computeVertexTSpace((const std::vector<tgen::VIndexT>&)mesh.indices, (const std::vector<tgen::Vec3>&)t, (const std::vector<tgen::Vec3>&)bt, mesh.p.size(), (std::vector<tgen::Vec3>&)mesh.t, (std::vector<tgen::Vec3>&)mesh.bt);
	tgen::orthogonalizeTSpace((const std::vector<tgen::Vec3>&)mesh.n, (std::vector<tgen::Vec3>&)mesh.t, (std::vector<tgen::Vec3>&)mesh.bt);

	return true;
}

bool prim::Primitives::boxMesh(Mesh &mesh, const dVec3& size, const iVec3& segments)
{
	return buildMesh(mesh, generator::BoxMesh((const glm::dvec3&)size, (const glm::ivec3&)segments));
}

bool prim::Primitives::cappedCylinderMesh(Mesh &mesh, double radius, double size, int slices, int segments, int rings, double start, double sweep)
{
	return buildMesh(mesh, generator::CappedCylinderMesh(radius, size, slices, segments, rings, start, sweep));
}

bool prim::Primitives::cappedConeMesh(Mesh &mesh, double radius, double size, int slices, int segments, int rings, double start, double sweep)
{
	return buildMesh(mesh, generator::CappedConeMesh(radius, size, slices, segments, rings, start, sweep));
}

bool prim::Primitives::cappedTubeMesh(Mesh &mesh, double radius, double innerRadius, double size, int slices, int segments, int rings, double start, double sweep)
{
	return buildMesh(mesh, generator::CappedTubeMesh(radius, innerRadius, size, slices, segments, rings, start, sweep));
}

bool prim::Primitives::coneMesh(Mesh &mesh, double radius, double size, int slices, int segments, double start, double sweep)
{
	return buildMesh(mesh, generator::ConeMesh(radius, size, slices, segments, start, sweep));
}

bool prim::Primitives::capsuleMesh(Mesh &mesh, double radius, double size, int slices, int segments, int rings, double start, double sweep)
{
	return buildMesh(mesh, generator::CapsuleMesh(radius, size, slices, segments, rings, start, sweep));
}

bool prim::Primitives::cylinderMesh(Mesh &mesh, double radius, double size, int slices, int segments, double start, double sweep)
{
	return buildMesh(mesh, generator::CylinderMesh(radius, size, slices, segments, start, sweep));
}

bool prim::Primitives::dodecahedronMesh(Mesh &mesh, double radius, int segments, int rings)
{
	return buildMesh(mesh, generator::DodecahedronMesh(radius, segments, rings));
}

bool prim::Primitives::diskMesh(Mesh &mesh, double radius, double innerRadius, int slices, int rings, double start, double sweep)
{
	return buildMesh(mesh, generator::DiskMesh(radius, innerRadius, slices, rings, start, sweep));
}

bool prim::Primitives::icosahedronMesh(Mesh &mesh, double radius, int segments)
{
	return buildMesh(mesh, generator::IcosahedronMesh(radius, segments));
}

bool prim::Primitives::icoSphereMesh(Mesh &mesh, double radius, int segments)
{
	return buildMesh(mesh, generator::IcoSphereMesh(radius, segments));
}

bool prim::Primitives::planeMesh(Mesh &mesh, const dVec2& size, const iVec2& segments)
{
	return buildMesh(mesh, generator::PlaneMesh((const glm::dvec2&)size, (const glm::ivec2&)segments));
}

bool prim::Primitives::roundedBoxMesh(Mesh &mesh, double radius, const dVec3& size, int slices, const iVec3& segments)
{
	return buildMesh(mesh, generator::RoundedBoxMesh(radius, (const glm::dvec3&)size, slices, (const glm::ivec3&)segments));
}

bool prim::Primitives::sphereMesh(Mesh &mesh, double radius, int slices, int segments, double sliceStart, double sliceSweep, double segmentStart, double segmentSweep)
{
	return buildMesh(mesh, generator::SphereMesh(radius, slices, segments, sliceStart, sliceSweep, segmentStart, segmentSweep));
}

bool prim::Primitives::sphericalConeMesh(Mesh &mesh, double radius, double size, int slices, int segments, int rings, double start, double sweep)
{
	return buildMesh(mesh, generator::SphericalConeMesh(radius, size, slices, segments, rings, start, sweep));
}

bool prim::Primitives::sphericalTriangleMesh(Mesh &mesh, double radius, int segments)
{
	return buildMesh(mesh, generator::SphericalTriangleMesh(radius, segments));
}

bool prim::Primitives::sphericalTriangleMesh(Mesh &mesh, const dVec3& v0, const dVec3& v1, const dVec3& v2, int segments)
{
	return buildMesh(mesh, generator::SphericalTriangleMesh((const glm::dvec3&)v0, (const glm::dvec3&)v1, (const glm::dvec3&)v2, segments));
}

bool prim::Primitives::springMesh(Mesh &mesh, double minor, double major, double size, int slices, int segments, double minorStart, double minorSweep, double majorStart, double majorSweep)
{
	return buildMesh(mesh, generator::SpringMesh(minor, major, size, slices, segments, minorStart, minorSweep, majorStart, majorSweep));
}

bool prim::Primitives::teapotMesh(Mesh &mesh, double size, int segments)
{
	return buildMesh(mesh, generator::scaleMesh(generator::TeapotMesh(segments), { size * 0.5, size * 0.5, size * 0.5 }));
}

bool prim::Primitives::torusKnotMesh(Mesh &mesh, int p, int q, int slices, int segments)
{
	return buildMesh(mesh, generator::TorusKnotMesh(p, q, slices, segments));
}

bool prim::Primitives::torusMesh(Mesh &mesh, double minor, double major, int slices, int segments, double minorStart, double minorSweep, double majorStart, double majorSweep)
{
	return buildMesh(mesh, generator::TorusMesh(minor, major, slices, segments, minorStart, minorSweep, majorStart, majorSweep));
}

bool prim::Primitives::triangleMesh(Mesh &mesh, double radius, int segments)
{
	return buildMesh(mesh, generator::TriangleMesh(radius, segments));
}

bool prim::Primitives::triangleMesh(Mesh &mesh, const dVec3& v0, const dVec3& v1, const dVec3& v2, int segments)
{
	return buildMesh(mesh, generator::TriangleMesh((const glm::dvec3&)v0, (const glm::dvec3&)v1, (const glm::dvec3&)v2, segments));
}

bool prim::Primitives::tubeMesh(Mesh &mesh, double radius, double innerRadius, double size, int slices, int segments, double start, double sweep)
{
	return buildMesh(mesh, generator::TubeMesh(radius, innerRadius, size, slices, segments, start, sweep));
}