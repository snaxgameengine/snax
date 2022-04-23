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
#include "GlobalDef.h"


namespace m3d
{

struct ChipMessage;
class InputManager;

enum class ExeEnvironment : uint32 { EXE_EDITOR, EXE_VIEWER, EXE_UNDEFINED = 0xFFFFFFFF };

struct ApplicationMessage
{
	MessageSeverity severity;
	time_t timestamp;
	uint32 threadID;
	String message;
	uint32 clazzID;
	uint32 chipID;
};

class M3DENGINE_API Application
{
public:
	// Return the environment we are running in.
	virtual ExeEnvironment GetExeEnvironment() const = 0;
	// This is the exe (self extracting archive) started.
	virtual Path GetExeFile() const = 0;
	// This is the path of the actual application running. Developer or viewer process.
	virtual Path GetApplicationFile() const = 0;
	// Ask the application to quit.
	virtual void Quit() = 0;
	// Called by the engine every time a new message is added.
	virtual void MessagedAdded(const ApplicationMessage &msg) = 0;
	// Called by the engine when a message regarding the given chip is added. The message remain valid until removed.
	virtual void ChipMessageAdded(Chip *chip, const ChipMessage &msg) = 0;
	// Called by the engine when a message regarding the given chip is removed.
	virtual void ChipMessageRemoved(Chip *chip, const ChipMessage &msg) = 0;
	// All graphics-resources must be released!
	virtual void DestroyDeviceObjects() = 0;
	// Gets the display orientation. Useful for devices like tables/phones.
	virtual int32 GetDisplayOrientation() = 0;
	// Gets the input manager. The application (desktop/wp8 etc) is responsible for providing input through its native system.
	virtual InputManager *GetInputManager() = 0;
	// Called by engine when execution should be paused.
	virtual void Break(Chip *chip) = 0;
	// Enable/Disable break points globally.
	virtual bool IsBreakPointsEnabled() const = 0;
	// Returns a bitmask to enable/disable various features.
	virtual uint32 GetFeatureMask() const = 0;
};


}