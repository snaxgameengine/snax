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
#include "StdChips/VectorChip.h"

namespace m3d
{


static const Guid PHYSXVECTOR_GUID = { 0xdb013520, 0x3b43, 0x4520, { 0xa6, 0xf8, 0x2d, 0x16, 0xaa, 0x20, 0xd1, 0x72 } };



class PHYSXCHIPS_API PhysXVector : public VectorChip
{
	CHIPDESC_DECL;
public:
	PhysXVector();
	virtual ~PhysXVector();

	virtual bool CopyChip(Chip *chip) override;
	virtual bool LoadChip(DocumentLoader &loader) override;
	virtual bool SaveChip(DocumentSaver &saver) const override;

	virtual const XMFLOAT4 &GetVector() override;
	virtual void SetVector(const XMFLOAT4 &v) override;

	enum Command { CMD_NONE, 
		CMD_SCENE_XET_GRAVITY,
		CMD_RIGID_BODY_XET_LINEAR_VELOCITY = 100,
		CMD_RIGID_BODY_XET_ANGULAR_VELOCITY,
		CMD_RIGID_BODY_XET_MASS_SPACE_INERTIA_TENSOR,
		CMD_CONSTRAINT_GET_LINEAR_FORCE = 200,
		CMD_CONSTRAINT_GET_ANGULAR_FORCE
	};

	virtual Command GetCommand() const { return _cmd; }
	virtual void SetCommand(Command cmd);

protected:
	Command _cmd;

};

}