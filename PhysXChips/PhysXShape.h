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
#include "M3DEngine/Chip.h"

namespace m3d
{

	
static const Guid PHYSXSHAPE_GUID = { 0x14842055, 0x49ee, 0x47e1, { 0x86, 0xe2, 0xf2, 0x32, 0xeb, 0xd9, 0x0b, 0x02 } };



class PHYSXCHIPS_API PhysXShape : public Chip
{
	CHIPDESC_DECL;
public:
	PhysXShape();
	virtual ~PhysXShape();

	virtual bool CopyChip(Chip *chip) override;
	virtual bool LoadChip(DocumentLoader &loader) override;
	virtual bool SaveChip(DocumentSaver &saver) const override;

	enum Type { T_NONE, T_BOX, T_CAPSULE, T_CONVEX_MESH, T_HEIGHT_FIELD, T_PLANE, T_SPHERE, T_TRIANGLE_MESH };

	virtual Type GetType() const { return _type; }
	virtual void SetType(Type t);

	virtual PxShape *GetShape();
	virtual PxReal GetMass() const { return _mass; }

protected:
	PxShape *_shape;
	PxReal _mass;

	Type _type;
	float32 _contactOffset;
	float32 _restOffset;

/*
  eSIMULATION_SHAPE = (1<<0), 
  eSCENE_QUERY_SHAPE = (1<<1), 
  eTRIGGER_SHAPE = (1<<2), 
  eVISUALIZATION = (1<<3), 
  ePARTICLE_DRAIN = (1<<4), 
  eUSE_SWEPT_BOUNDS = (1<<6) 
*/
};



}