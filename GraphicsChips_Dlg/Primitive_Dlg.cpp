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

#include "StdAfx.h"
#include "Primitive_dlg.h"

using namespace m3d;


DIALOGDESC_DEF(Primitive_Dlg, PRIMITIVE_GUID);



void Primitive_Dlg::Init()
{
	ComboBoxInitList type;
	type.push_back(std::make_pair(MTEXT("Square"), RVariant((uint32)Primitive::PrimitiveType::SQUARE)));
	type.push_back(std::make_pair(MTEXT("Box"), RVariant((uint32)Primitive::PrimitiveType::BOX)));
	type.push_back(std::make_pair(MTEXT("Sphere"), RVariant((uint32)Primitive::PrimitiveType::SPHERE)));
	type.push_back(std::make_pair(MTEXT("Cylinder"), RVariant((uint32)Primitive::PrimitiveType::CYLINDER)));
	type.push_back(std::make_pair(MTEXT("Cone"), RVariant((uint32)Primitive::PrimitiveType::CONE)));
	type.push_back(std::make_pair(MTEXT("Torus"), RVariant((uint32)Primitive::PrimitiveType::TORUS)));
	type.push_back(std::make_pair(MTEXT("Capsule"), RVariant((uint32)Primitive::PrimitiveType::CAPSULE)));
	type.push_back(std::make_pair(MTEXT("Capped Cylinder"), RVariant((uint32)Primitive::PrimitiveType::CAPPED_CYLINDER)));
	type.push_back(std::make_pair(MTEXT("Capped Cone"), RVariant((uint32)Primitive::PrimitiveType::CAPPED_CONE)));
	type.push_back(std::make_pair(MTEXT("Capped Tube"), RVariant((uint32)Primitive::PrimitiveType::CAPPED_TUBE)));
	type.push_back(std::make_pair(MTEXT("Dodecahedron"), RVariant((uint32)Primitive::PrimitiveType::DODECAHEDRON)));
	type.push_back(std::make_pair(MTEXT("Disk"), RVariant((uint32)Primitive::PrimitiveType::DISK)));
	type.push_back(std::make_pair(MTEXT("Icosahedron"), RVariant((uint32)Primitive::PrimitiveType::ICOSAHEDRON)));
	type.push_back(std::make_pair(MTEXT("Icosphere"), RVariant((uint32)Primitive::PrimitiveType::ICOSPHERE)));
	type.push_back(std::make_pair(MTEXT("Plane"), RVariant((uint32)Primitive::PrimitiveType::PLANE)));
	type.push_back(std::make_pair(MTEXT("Rounded Box"), RVariant((uint32)Primitive::PrimitiveType::ROUNDED_BOX)));
	type.push_back(std::make_pair(MTEXT("Spherical Cone"), RVariant((uint32)Primitive::PrimitiveType::SPHERICAL_CONE)));
	type.push_back(std::make_pair(MTEXT("Spherical Triangle"), RVariant((uint32)Primitive::PrimitiveType::SPHEREICAL_TRIANGLE)));
	type.push_back(std::make_pair(MTEXT("Spring"), RVariant((uint32)Primitive::PrimitiveType::SPRING)));
	type.push_back(std::make_pair(MTEXT("Utah Teapot"), RVariant((uint32)Primitive::PrimitiveType::TEAPOT)));
	type.push_back(std::make_pair(MTEXT("Torus Knot"), RVariant((uint32)Primitive::PrimitiveType::TORUS_KNOT)));
	type.push_back(std::make_pair(MTEXT("Triangle"), RVariant((uint32)Primitive::PrimitiveType::TRIANGLE)));
	type.push_back(std::make_pair(MTEXT("Tube"), RVariant((uint32)Primitive::PrimitiveType::TUBE)));
	type.push_back(std::make_pair(MTEXT("Point List (No Geometry)"), RVariant((uint32)Primitive::PrimitiveType::GEOMERTYLESS_POINTLIST)));
	type.push_back(std::make_pair(MTEXT("Line List (No Geometry)"), RVariant((uint32)Primitive::PrimitiveType::GEOMERTYLESS_LINELIST)));
	type.push_back(std::make_pair(MTEXT("Line Strip (No Geometry)"), RVariant((uint32)Primitive::PrimitiveType::GEOMERTYLESS_LINESTRIP)));
	type.push_back(std::make_pair(MTEXT("Triangle List (No Geometry)"), RVariant((uint32)Primitive::PrimitiveType::GEOMERTYLESS_TRIANGLELIST)));
	type.push_back(std::make_pair(MTEXT("Triangle Strip (No Geometry)"), RVariant((uint32)Primitive::PrimitiveType::GEOMETRYLESS_TRIANGLESTRIP)));
	type.push_back(std::make_pair(MTEXT("Random Points"), RVariant((uint32)Primitive::PrimitiveType::RANDOM_POINTS)));

	AddComboBox(MTEXT("Type"), type, (uint32)GetChip()->GetPrimitiveType(), [this, type](Id id, RVariant v)
		{ 
			SetDirty(); 
			GetChip()->SetPrimitiveType((Primitive::PrimitiveType)v.ToUInt());
			String n(MTEXT("Primitive"));
			for (size_t i = 0; i < type.size(); i++)
				if (type[i].second == v.ToUInt())
					n = type[i].first;
			GetChip()->SetName(n);
		});
	AddCheckBox(MTEXT("Normals"), (GetChip()->GetFlags() & Primitive::PrimitiveFlags::NORMALS) != Primitive::PrimitiveFlags::NONE ? RCheckState::Checked : RCheckState::Unchecked, [this](Id id, RVariant v) 
		{ 
			SetDirty(); 
			GetChip()->SetFlags(v.ToUInt() == RCheckState::Checked ? (GetChip()->GetFlags() | Primitive::PrimitiveFlags::NORMALS) : (GetChip()->GetFlags() & ~Primitive::PrimitiveFlags::NORMALS));
		});
	AddCheckBox(MTEXT("Tangents & Bitangents"), (GetChip()->GetFlags() & Primitive::PrimitiveFlags::TANGENTS) != Primitive::PrimitiveFlags::NONE ? RCheckState::Checked : RCheckState::Unchecked, [this](Id id, RVariant v)
		{
			SetDirty();
			GetChip()->SetFlags(v.ToUInt() == RCheckState::Checked ? (GetChip()->GetFlags() | Primitive::PrimitiveFlags::TANGENTS) : (GetChip()->GetFlags() & ~Primitive::PrimitiveFlags::TANGENTS));
		});
	AddCheckBox(MTEXT("Texture Coordinates"), (GetChip()->GetFlags() & Primitive::PrimitiveFlags::TEXCOORDS) != Primitive::PrimitiveFlags::NONE ? RCheckState::Checked : RCheckState::Unchecked, [this](Id id, RVariant v)
		{
			SetDirty();
			GetChip()->SetFlags(v.ToUInt() == RCheckState::Checked ? (GetChip()->GetFlags() | Primitive::PrimitiveFlags::TEXCOORDS) : (GetChip()->GetFlags() & ~Primitive::PrimitiveFlags::TEXCOORDS));
		});
}
