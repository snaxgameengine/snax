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
#include "StdGeometry.h"
#include "M3DCore/MagicEnum.h"

namespace m3d
{


static const Guid PRIMITIVE_GUID = { 0xb3fa0bc3, 0xc4aa, 0x4be6, { 0xb8, 0x2c, 0xb7, 0xc, 0x22, 0x89, 0x73, 0x26 } };


class GRAPHICSCHIPS_API Primitive : public StdGeometry
{
	CHIPDESC_DECL;
public:
	enum class PrimitiveType {
		NONE,
		SQUARE,
		BOX,
		SPHERE,
		CYLINDER, //
		CONE,
		TORUS, //
		CAPSULE, //
		CAPPED_CYLINDER,
		CAPPED_CONE,
		CAPPED_TUBE,
		DODECAHEDRON,
		DISK,
		ICOSAHEDRON,
		ICOSPHERE,
		PLANE,
		ROUNDED_BOX,
		SPHERICAL_CONE,
		SPHEREICAL_TRIANGLE,
		SPRING,
		TEAPOT,
		TORUS_KNOT,
		TRIANGLE,
		TUBE,
		GEOMERTYLESS_POINTLIST = 101,
		GEOMERTYLESS_LINELIST,
		GEOMERTYLESS_LINESTRIP,
		GEOMERTYLESS_TRIANGLELIST,
		GEOMETRYLESS_TRIANGLESTRIP,
		GEOMERTYLESS_LINELIST_ADJ, //
		GEOMERTYLESS_LINESTRIP_ADJ, //
		GEOMERTYLESS_TRIANGLELIST_ADJ, // 
		GEOMETRYLESS_TRIANGLESTRIP_ADJ, //
		RANDOM_POINTS = 200
	};

	Primitive();

	bool CopyChip(Chip* chip) override;
	bool LoadChip(DocumentLoader& loader) override;
	bool SaveChip(DocumentSaver& saver) const override;

	void Update() override;
	void CreateDeviceObjects() override;

	virtual PrimitiveType GetPrimitiveType() const { return _pt; }
	virtual void SetPrimitiveType(PrimitiveType pt);

	enum class PrimitiveFlags { NONE = 0, NORMALS = 0x01, TEXCOORDS = 0x02, TANGENTS = 0x05 };

	virtual PrimitiveFlags GetFlags() const { return _flags; }
	virtual void SetFlags(PrimitiveFlags f);

	virtual const XMFLOAT4X4& GetTransform() const { return _transform; }

protected:
	PrimitiveType _pt = PrimitiveType::NONE;
	XMUINT4 _subdivision = XMUINT4(10, 10, 10, 2); // 3xsegments/(slices, segments, rings)/( segments, rings)/(slices, rings)/2xsegments/(slices, 3xsegments)/(slice, segments)/( p, q, slices, segments)
	XMFLOAT4 _dimensions = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f); // (radius, size)/(minor, major)/(radius, innerRadius, size)/(radius, innerRadius)/2xsize/(radius, 3xsize)/(minor, major, size)
	XMFLOAT4X4 _transform = XMFLOAT4X4(1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);
	PrimitiveFlags _flags = PrimitiveFlags::NORMALS | PrimitiveFlags::TEXCOORDS | PrimitiveFlags::TANGENTS;

	void _generateSphere(uint32 subdivU, uint32& indexBase, CXMMATRIX m);

	void _generateSquare(uint32 subdivU, uint32 subdivV, uint32& indexBase, CXMMATRIX m);
	void _generateSphereOct(uint32 subdiv, uint32& indexBase, CXMMATRIX m, CXMMATRIX lm, CXMMATRIX tm);
	void _generateCircularArea(uint32 subdiv, uint32 subdivBreak, uint32& indexBase, CXMMATRIX m);
	void _generateCone(uint32 subdiv, uint32& indexBase, CXMMATRIX m);
};

}