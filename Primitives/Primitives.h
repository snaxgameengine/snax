// Copyright (c) 2013 - 2022 Frank-Vegar Mortensen <franksvm(at)outlook(dot)com>.
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.

#pragma once

#include "Exports.h"
#include <vector>

namespace prim
{

typedef float FLOAT;

struct Vec2
{
	FLOAT x, y;
};

struct Vec3
{
	FLOAT x, y, z;
};

struct uVec3
{
	unsigned v0, v1, v2;
};

struct dVec2
{
	double x, y;
};

struct dVec3
{
	double x, y, z;
};

struct iVec2
{
	int x, y;
};

struct iVec3
{
	int x, y, z;
};

struct Mesh
{
	std::vector<Vec3> p;
	std::vector<Vec3> n;
	std::vector<Vec2> tc;
	std::vector<Vec3> t;
	std::vector<Vec3> bt;
	std::vector<uVec3> indices;
};

#define TORAD(x) ((x) / 180.0 * 3.1415926535898)

class PRIMITIVES_API Primitives
{
public:

	/// Rectangular box centered at origin aligned along the x, y and z axis.
	/// @param size Half of the side length in x (0), y (1) and z (2) direction.
	/// @param segments The number of segments in x (0), y (1) and z (2)
	/// directions. All should be >= 1. If any one is zero faces in that
	/// direction are not genereted. If more than one is zero the mesh is empty.
	static bool boxMesh(
		Mesh &mesh,
		const dVec3& size = { 1.0, 1.0, 1.0 },
		const iVec3& segments = { 8, 8, 8 }
	);

	/// Like CylinderMesh but with end caps.
	/// @param radius Radius of the cylinder along the xy-plane.
	/// @param size Half of the length cylinder along the z-axis.
	/// @param slices Number of subdivisions around the z-axis.
	/// @param segments Number of subdivisions along the z-axis.
	/// @param rings Number of subdivisions on the caps.
	/// @param start Counterclockwise angle around the z-axis relative to x-axis.
	/// @param sweep Counterclockwise angle around the z-axis.
	static bool cappedCylinderMesh(
		Mesh &mesh,
		double radius = 1.0,
		double size = 1.0,
		int slices = 32,
		int segments = 8,
		int rings = 4,
		double start = 0.0,
		double sweep = TORAD(360.0)
	);

	/// A cone with a cap centered at origin pointing towards positive z-axis.
	/// @param radius Radius of the flat (negative z) end along the xy-plane.
	/// @param size Half of the length of the cylinder along the z-axis.
	/// @param slices Number of subdivisions around the z-axis.
	/// @param segments Number of subdivisions along the z-axis.
	/// @param rings Number of subdivisions of the cap.
	/// @param start Counterclockwise angle around the z-axis relative to the
	/// positive x-axis.
	/// @param sweep Counterclockwise angle around the z-axis.
	static bool cappedConeMesh(
		Mesh &mesh,
		double radius = 1.0,
		double size = 1.0,
		int slices = 32,
		int segments = 8,
		int rings = 4,
		double start = 0.0,
		double sweep = TORAD(360.0)
	);

	/// Like TubeMesh but with end caps.
	/// @param radius The outer radius of the cylinder on the xy-plane.
	/// @param innerRadius The inner radius of the cylinder on the xy-plane.
	/// @param size Half of the length of the cylinder along the z-axis.
	/// @param slices Number nubdivisions around the z-axis.
	/// @param segments Number of subdivisions along the z-axis.
	/// @param rings Number radial subdivisions in the cap.
	/// @param start Counterclockwise angle around the z-axis relative to the x-axis.
	/// @param sweep Counterclockwise angle around the z-axis.
	static bool cappedTubeMesh(
		Mesh &mesh,
		double radius = 1.0,
		double innerRadius = 0.75,
		double size = 1.0,
		int slices = 32,
		int segments = 8,
		int rings = 1,
		double start = 0.0,
		double sweep = TORAD(360.0)
	);

	/// A cone centered at origin tip pointing towards z-axis.
	///@param radius Radius of the negative z end on the xy-plane.
	///@param size Half of the length of the cylinder along the z-axis.
	///@param slices Number of subdivisions around the z-axis.
	///@param segments Number subdivisions along the z-axis.
	///@param start Counterclockwise angle around the z-axis relative to the x-axis.
	///@param sweep Counterclockwise angle around the z-axis.
	static bool coneMesh(
		Mesh &mesh,
		double radius = 1.0,
		double size = 1.0,
		int slices = 32,
		int segments = 8,
		double start = 0.0,
		double sweep = TORAD(360.0)
	);

	/// Capsule (cylinder with spherical caps) centered at origin aligned along z-axis.
	/// @param radius Radius of the capsule on the xy-plane.
	/// @param size Half of the length between centers of the caps along the z-axis.
	/// @param slices Number of subdivisions around the z-axis.
	/// @param rings Number of radial subdivisions in the caps.
	/// @param start Counterclockwise angle relative to the x-axis.
	/// @param sweep Counterclockwise angle.
	static bool capsuleMesh(
		Mesh &mesh,
		double radius = 1.0,
		double size = 0.5,
		int slices = 32,
		int segments = 4,
		int rings = 8,
		double start = 0.0,
		double sweep = TORAD(360.0)
	);

	/// Cylinder centered at origin aligned along the z-axis.
	/// @param radius Radius of the cylinder along the xy-plane.
	/// @param size Half of the length of the cylinder along the z-axis.
	/// @param slices Subdivisions around the z-axis.
	/// @param segments Subdivisions along the z-axis.
	/// @param start Counterclockwise angle around the z-axis relative to the x-axis.
	/// @param sweep Counterclockwise angle around the z-axis.
	static bool cylinderMesh(
		Mesh &mesh,
		double radius = 1.0,
		double size = 1.0,
		int slices = 32,
		int segments = 8,
		double start = 0.0,
		double sweep = TORAD(360.0)
	);

	/// A regular dodecahedron centered at origin with given radius.
	/// Each face optionally subdivided along edges and/or radius.
	/// @param radius The radius of the enclosing sphere.
	/// @param segments The number segments along each edge. Should be >= 1.
	/// If <1 empty mesh is generated.
	/// @param rings The number of radial segments on each face. Should be >= 1.
	/// If <1 an empty mesh is generated.
	static bool dodecahedronMesh(
		Mesh &mesh,
		double radius = 1.0,
		int segments = 1, 
		int rings = 1
	);

	/// A circular disk centered at origin on the xy-plane.
	/// @param radius Outer radius of the disk on the xy-plane.
	/// @param innerRadius radius of the inner circle on the xy-plane.
	/// @param slices Number of subdivisions around the z-axis.
	/// @param rings Number of subdivisions along the radius.
	/// @param start Counterclockwise angle relative to the x-axis
	/// @param sweep Counterclockwise angle.
	static bool diskMesh(
		Mesh &mesh,
		double radius = 1.0,
		double innerRadius = 0.0,
		int slices = 32,
		int rings = 4,
		double start = 0.0,
		double sweep = TORAD(360.0)
	);

	/// Regular icosahedron centered at origin with given radius.
	/// @param radius The radius of the enclosing sphere.
	/// @param segments The number segments along each edge. Must be >= 1.
	static bool icosahedronMesh(
		Mesh &mesh, 
		double radius = 1.0, 
		int segments = 1
	);

	/// Icosphere aka spherical subdivided icosahedron
	/// @param radius The radius of the containing sphere.
	/// @param segments The number of segments per icosahedron edge. Must be >= 1.
	static bool icoSphereMesh(
		Mesh &mesh, 
		double radius = 1.0, 
		int segments = 4
	);

	/// A plane (rectangular grid) on the xy -plane normal pointing towards z-axis.
	/// @param size Half of the side length in x (0) and y (1) direction.
	/// @param segments Number of subdivisions in the x (0) and y (1) direction.
	static bool planeMesh(
		Mesh &mesh,
		const dVec2& size = { 1.0, 1.0 },
		const iVec2& segments = { 8, 8 }
	);

	/// Rectangular box with rounded edges centered at origin aligned along the x, y
	/// @param radius Radius of the rounded edges.
	/// @param size Half of the side length in x (0), y (1) and z (2) direction.
	/// @param slices Number subdivions around in the rounded edges.
	/// @param segments Number of subdivisons in x (0), y (1) and z (2)
	/// direction for the flat faces.
	static bool roundedBoxMesh(
		Mesh &mesh,
		double radius = 0.25,
		const dVec3& size = { 0.75, 0.75, 0.75 },
		int slices = 4,
		const iVec3& segments = { 8, 8, 8 });

	/// A sphere of the given radius centered around the origin.
	/// Subdivided around the z-axis in slices and along the z-axis in segments.
	/// @param radius The radius of the sphere
	/// @param slices Subdivisions around the z-azis (longitudes).
	/// @param segments Subdivisions along the z-azis (latitudes).
	/// @param sliceStart Counterclockwise angle around the z-axis relative to x-axis.
	/// @param sliceSweep Counterclockwise angle.
	/// @param segmentStart Counterclockwise angle relative to the z-axis.
	/// @param segmentSweep Counterclockwise angle.
	static bool sphereMesh(
		Mesh &mesh, 
		double radius = 1.0,
		int slices = 32,
		int segments = 16,
		double sliceStart = 0.0,
		double sliceSweep = TORAD(360.0),
		double segmentStart = 0.0, 
		double segmentSweep = TORAD(180.0));

	/// A cone with a spherical cap centered at origin tip pointing towards z-axis.
	/// Each point on the cap has equal distance from the tip.
	/// @param radius Radius of the negative z end on the xy-plane.
	/// @param size Half of the distance between cap and tip along the z-axis.
	/// @param slices Number of subdivisions around the z-axis.
	/// @param segments Number subdivisions along the z-axis.
	/// @param rings Number subdivisions in the cap.
	/// @param start Counterclockwise angle around the z-axis relative to the positive x-axis.
	/// @param sweep Counterclockwise angle around the z-axis.
	static bool sphericalConeMesh(
		Mesh &mesh,
		double radius = 1.0,
		double size = 1.0,
		int slices = 32,
		int segments = 8,
		int rings = 4,
		double start = 0.0,
		double sweep = TORAD(360.0));

	/// A triangular region on a surface of a sphere.
	/// @param radius Radius of the containing sphere.
	/// @param segments Number of subdivisions along each edge.
	static bool sphericalTriangleMesh(
		Mesh &mesh,
		double radius = 1.0,
		int segments = 4
	);

	/// A triangular region on a surface of a sphere.
	/// @param segments Number of subdivisions along each edge.
	static bool sphericalTriangleMesh(
		Mesh &mesh,
		const dVec3& v0,
		const dVec3& v1, 
		const dVec3& v2,
		int segments = 4
	);

	/// A spring aligned along the z-axis winding counterclockwise
	/// @param minor Radius of the spring it self.
	/// @param major Radius from the z-axis
	/// @param size Half of the length along the z-axis.
	/// @param slices Subdivisions around the spring.
	/// @param segments Subdivisions along the path.
	/// @param majorStart Counterclockwise angle around the z-axis relative to the x-axis.
	/// @param majorSweep Counterclockwise angle arounf the z-axis.
	static bool springMesh(
		Mesh &mesh,
		double minor = 0.25,
		double major = 1.0,
		double size = 1.0,
		int slices = 8,
		int segments = 32,
		double minorStart = 0.0,
		double minorSweep = TORAD(360.0),
		double majorStart = 0.0,
		double majorSweep = TORAD(720.0));

	/// Generates the Utah teapot using the original data.
	/// The lid is pointing towards the z axis and the spout towards the x axis.
	/// @param segments The number segments along each patch. Should be >= 1.
	/// If zero empty mesh is generated.
	static bool teapotMesh(
		Mesh &mesh,
		double size = 1.0,
		int segments = 8);

	/// A Circle extruded along a knot path.
	/// @param slices Number subdivisions around the circle.
	/// @param segments Number of subdivisions around the path.
	static bool torusKnotMesh(
		Mesh &mesh,
		int p = 2,
		int q = 3,
		int slices = 8,
		int segments = 96);

	/// Torus centered at origin on the xy-plane.
	/// @param minor Radius of the minor (inner) ring
	/// @param major Radius of the major (outer) ring
	/// @param slices Subdivisions around the minor ring
	/// @param segments Subdivisions around the major ring
	/// @param minorStart Counterclockwise angle relative to the xy-plane.
	/// @param minorSweep Counterclockwise angle around the circle.
	/// @param majorStart Counterclockwise angle around the z-axis relative to the x-axis.
	/// @param majorSweep Counterclockwise angle around the z-axis.
	static bool torusMesh(
		Mesh &mesh,
		double minor = 0.25,
		double major = 1.0,
		int slices = 16,
		int segments = 32,
		double minorStart = 0.0,
		double minorSweep = TORAD(360.0),
		double majorStart = 0.0,
		double majorSweep = TORAD(360.0));

	/// A triangular mesh on the xy -plane.
	/// Makes a regular triangle centered at origin.
	/// @param radius The radius of the containing circle.
	/// @param segments The number of segments along each edge. Must be >= 1.
	static bool triangleMesh(
		Mesh &mesh, 
		double radius = 1.0, 
		int segments = 4
	);

	/// A triangular mesh on the xy -plane.
	/// @param v0,v1,v2 The vertex positions of the triangle.
	/// @param segments The number of segments along each edge. Must be >= 1.
	static bool triangleMesh(Mesh &mesh,
		const dVec3& v0, 
		const dVec3& v1, 
		const dVec3& v2,
		int segments = 4
	);


	/// Tube (thick cylinder) centered at origin aligned along the z-axis.
	/// @param radius The outer radius of the cylinder on the xy-plane.
	/// @param innerRadius The inner radius of the cylinder on the xy-plane.
	/// @param size Half of the length of the cylinder along the z-axis.
	/// @param slices Subdivisions around the z-axis.
	/// @param segments Subdivisions along the z-axis.
	/// @param start Counterclockwise angle around the z-axis relative to the x-axis.
	/// @param sweep Counterclockwise angle around the z-axis.
	static bool tubeMesh(
		Mesh &mesh,
		double radius = 1.0,
		double innerRadius = 0.75,
		double size = 1.0,
		int slices = 32,
		int segments = 8,
		double start = 0.0,
		double sweep = TORAD(360.0) 
	);

};


}