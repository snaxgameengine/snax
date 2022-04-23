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

static const Guid PHYSXCOMMAND_GUID = { 0x8348dec5, 0x8ea4, 0x481a, { 0x9e, 0x41, 0x32, 0xf8, 0x14, 0xdf, 0xee, 0x29 } };



class PHYSXCHIPS_API PhysXCommand : public Chip
{
	CHIPDESC_DECL;
public:
	PhysXCommand();
	virtual ~PhysXCommand();

	virtual bool CopyChip(Chip *chip) override;
	virtual bool LoadChip(DocumentLoader &loader) override;
	virtual bool SaveChip(DocumentSaver &saver) const override;

	virtual void CallChip();

	enum Command { CMD_NONE, 
		CMD_SCENE_SIMULATE, 
		CMD_SCENE_FETCH_RESULTS, 
		CMD_SCENE_DEBUG_RENDER,
		CMD_SCENE_START,
		CMD_SCENE_STOP,
		CMD_SCENE_DESTROY,
		CMD_ACTOR_DESTROY = 400,
		CMD_RIGIDDYNAMIC_ADD_GLOBAL_FORCE_AT_GLOBAL_POINT = 500,
		CMD_RIGIDDYNAMIC_ADD_GLOBAL_FORCE_AT_LOCAL_POINT,
		CMD_RIGIDDYNAMIC_ADD_LOCAL_FORCE_AT_GLOBAL_POINT,
		CMD_RIGIDDYNAMIC_ADD_LOCAL_FORCE_AT_LOCAL_POINT,
		CMD_RIGIDDYNAMIC_ADD_GLOBAL_TORQUE,
		CMD_RIGIDDYNAMIC_ADD_LOCAL_TORQUE,
		CMD_RIGIDDYNAMIC_CLEAR_FORCES_AND_TORQUES,
		CMD_JOINT_DESTROY = 600
	};

	virtual Command GetCommand() const { return _cmd; }
	virtual void SetCommand(Command cmd);

protected:
	Command _cmd;
};



}