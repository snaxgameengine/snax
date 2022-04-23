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
#include "StdChips/Value.h"

namespace m3d
{


static const Guid PHYSXVALUE_GUID = { 0x388c1557, 0x310f, 0x4284, { 0xb2, 0x9b, 0x8f, 0x8e, 0xd1, 0xea, 0x95, 0xd3 } };



class PHYSXCHIPS_API PhysXValue : public Value
{
	CHIPDESC_DECL;
public:
	PhysXValue();
	virtual ~PhysXValue();

	virtual bool CopyChip(Chip *chip) override;
	virtual bool LoadChip(DocumentLoader &loader) override;
	virtual bool SaveChip(DocumentSaver &saver) const override;

	virtual value GetValue() override;
	virtual void SetValue(value v) override;

	enum Command { CMD_NONE, 
		CMD_SCENE_EXIST,
		CMD_SCENE_IS_RUNNING,
		CMD_SCENE_IS_SIMULATING,
		CMD_SCENE_REAL_TIME_INDEX,
		CMD_SCENE_SIM_INDEX,
		CMD_SCENE_SIM_WAIT,
		CMD_ACTOR_EXIST = 100,
		CMD_RIGID_BODY_XET_MASS = 150,
		CMD_RIGID_DYNAMIC_XET_LINEAR_DAMPING = 160,
		CMD_RIGID_DYNAMIC_XET_ANGULAR_DAMPING,
		CMD_RIGID_DYNAMIC_XET_MAX_ANGULAR_VELOCITY,
		CMD_RIGID_DYNAMIC_XET_SLEEP_THRESHOLD,
		CMD_RIGID_DYNAMIC_IS_SLEEPING,
		CMD_JOINT_EXIST = 200
	};

	virtual Command GetCommand() const { return _cmd; }
	virtual void SetCommand(Command cmd);

protected:
	Command _cmd;

};

}