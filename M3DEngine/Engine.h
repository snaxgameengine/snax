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


class HighPrecisionTimer;
class FPS;
struct ChipMessage;
struct EngineImpl;
class Environment;

enum class EditMode { EM_RUN, EM_EDIT_RUN, EM_EDIT };

// Accesses the engine after it is created using Engine::Create()
extern Engine M3DENGINE_API *engine;

class M3DENGINE_API Engine
{
	template<typename T> friend void mmdelete(const T*);
	friend struct EngineAccessor;
public:
	// Creates the engine. One of the first things an application do.
	static bool Create();
	// Destroys the engine just before quitting. There is no need for the Application to exist no more! Do NOT call engine after this!
	static void Destroy();

	// Initiates the system. Searches for chips, sets up graphics engine etc.
	bool Init(Application *application, Path chipDir, Path thirdDir, const List<Path> &libDirs);
	// Resets the engine. Makes it ready to load a new project.
	void Reset();
	// Clears the engine. Calls Reset() and destroys graphics. Must be called before Application is destroyed!
	void Clear();

	// Get the application running this thing. Qt editor, viewer, WP-viewer etc.
	Application* GetApplication() const;
	// Returns the graphics engine. Loaded at startup.
	Graphics* GetGraphics();
	// Returns the manager keeping track of documents.
	DocumentManager* GetDocumentManager();
	// Returns the manager keeping track of chips.
	ChipManager* GetChipManager();
	// Returns the manager keeping track of classes.
	ClassManager* GetClassManager();
	// Returns the manager keeping track of function signatures.
	FunctionSignatureManager* GetFunctionSignatureManager();
	// Returns the environment.
	Environment* GetEnvironment();

	// Sets the file to write debug messages to.
	void SetMessageFile(Path p);
	// Adds a new debug message. THREAD SAFE!
	void Message(MessageSeverity severity, String message, ClassID clazzID = InvalidClassID, ChipID chipID = InvalidChipID);
	// Called by a chip when reporting an issue. 
	void ChipMessageAdded(Chip *chip, const ChipMessage &msg);
	// Called to remove a chip message.
	void ChipMessageRemoved(Chip *chip, const ChipMessage &msg);
	
	// Called by application framework once every frame.
	void Run();
	// Current frame number.
	uint32 GetFrameNr() const;
	// Coarse frame timer updated every time Run() is called.
	int32 GetFrameTime() const;
	// Used for GetFrameTime.
	int32 GetClockTime() const;
	// Stops the clock timer. (To be used for break points)
	void StopClockTime();
	// Starts the clock timer. (To be used for break points)
	void StartClockTime();
	// Timer ticked every frame.
	const HighPrecisionTimer& GetTimer() const;
	// Gets frame time limited to 2 hz - 10000 hz.
	int64 GetDt() const;
	// Gets current application time.
	int64 GetAppTime() const;
	// Called when normal execution is halted, eg when application loses focus, at breakpoints etc.
	void StopAppTime();
	// Starts the app timer again. Called automatically at Run().
	void StartAppTime();
	// Gets framerate statistics.
	const FPS& GetFPS() const;
	// Called when program execution is to be paused (eg for break points)
	void Break(Chip *chip);
	bool IsBreakMode() const;
	// Returns true if Run() is called, but not yet returned.
	bool IsRunning() const;

	// The directory where 3rd-party dependencies are found
	Path GetThirdDepsDirectory() const;
	// Gets the command line arguments set when starting the viewer.
	const List<String>& GetCmdLineArguments() const;
	// Framework sets the command line arguments.
	void SetCmdLineArguements(const List<String>& args);
	// Returns the platform compiled for.
	//AppPlatform GetPlatform() const;

	EditMode GetEditMode() const;
	void SetEditMode(EditMode editMode);

	// This call will be distributed to the system to destroy all device objects. Mostly called by device itself.
	void DestroyDeviceObjects();

private:
	Engine();
	~Engine();

	EngineImpl* _impl;

};




}

