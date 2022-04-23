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

#include "pch.h"
#include "Engine.h"
#include "DocumentManager.h"
#include "ChipManager.h"
#include "ClassManager.h"
#include "FunctionSignatures.h"
#include "GraphicsChips/Graphics.h"
#include "Application.h"
#include "M3DCore/PlatformDef.h"
#include "M3DCore/HighPrecisionTimer.h"
#include "Environment.h"
#include <fstream>
#include <mutex>

using namespace m3d;


m3d::Engine* m3d::engine = nullptr;


namespace m3d
{
struct EngineImpl
{
	// CS for adding messages. (keep it on top to make it be destroyed last!)
	std::mutex csMsg;
	// Class internal only
	std::wofstream msgfile;
	// Manager keeping track of d3d-stuff.
	Graphics* graphics = nullptr;
	// Application environment.
	Application* application = nullptr;
	// Mananger keeping track of documents.
	DocumentManager dm;
	// Manager keeping track of chips.
	ChipManager cm;
	// Manager keeping track of classes.
	ClassManager cgm;
	// Manager keeping track of function signatures.
	FunctionSignatureManager fsm;
	// The environment keeping track of library paths etc.
	Environment env;
	// Timer updated every frame
	HighPrecisionTimer timer;
	// Current frame rate.
	FPS fps;
	// Last frame time in seconds.
	int64 dt = 0;
	// Current application time. Accumulated _dt!
	int64 appTime = 0;
	// The time when the app timer was stopped.
	int64 appTimeStoppedTime = 0;
	// Time to substract from the timer.
	int64 appTimeSubTime = 0;
	// This is the lowest message severity reported.
	MessageSeverity vsDbgLevel = DINFO;
	// This is the current frame number. Incremented for every new frame.
	uint32 frameNr = 0;
	// This is a timestamp set at the beginning of a new frame using the GetClockTime() function.
	int32 frameTime = 0;
	// The time when the frame time was stopped.
	int32 frameTimeStoppedTime = -1;
	// Time to substract from clock() function.
	int32 frameTimeSub = 0;
	// The directory for 3rd-party dependencies
	Path thirdDir;
	// The command line arguments set when starting the viewer.
	List<String> cmdLineArguments;
	// The .exe file (self extracting archive) started when executing the project.
	Path exeFile;
	// Editor Mode
	EditMode editMode = EditMode::EM_EDIT_RUN;
	//
	bool isRunning = false;
};
}


bool Engine::Create()
{
	if (engine)
		return false;
	engine = mmnew Engine();
	return engine != nullptr;
}

void Engine::Destroy()
{
	msg(DINFO, MTEXT("Engine::~Destroy() called."));
	if (!engine)
		return;
	mmdelete(engine);
	engine = nullptr;
}

Engine::Engine()
{
	_impl = mmnew EngineImpl();
}

Engine::~Engine()
{
	msg(DINFO, MTEXT("Engine::~Engine() called."));
	assert(_impl->application == nullptr);
	assert(_impl->graphics == nullptr);
	mmdelete(_impl);
}


#ifdef WINDESKTOP
// Note: windows update KB2533623 needs to be installed!!!
typedef DLL_DIRECTORY_COOKIE (WINAPI *__AddDllDirectory)(_In_ PCSTR);
typedef BOOL (WINAPI *__SetDefaultDllDirectories)(_In_ DWORD);
#endif

bool Engine::Init(Application *application, Path chipDir, Path thirdDir, const List<Path>& libDirs)
{
	assert(application != nullptr);

	_impl->application = application;
	if (!_impl->env.Init(libDirs)) {
		msg(WARN, MTEXT("Engine: Failed to initialize environment."));
		return false;
	}

	if (!(chipDir.IsAbsolute() && thirdDir.IsAbsolute()))
		return false;

	_impl->thirdDir = thirdDir.GetAbsolute();

	// If running in the desktop environment, we have the privilege of setting directories for dlls. Else, like wp8, we have to stuff everything in the main directory.
#ifdef WINDESKTOP
	SetDllDirectoryA(thirdDir.AsString().c_str());

	// NOTE: We need to use this stuff because some systems may not have KB2533623 installed.
	__SetDefaultDllDirectories fSetDefaultDllDirectories = (__SetDefaultDllDirectories)GetProcAddress(GetModuleHandleA(MTEXT("kernel32.dll")), "SetDefaultDllDirectories");
	__AddDllDirectory fAddDllDirectory = (__AddDllDirectory)GetProcAddress(GetModuleHandleA(MTEXT("kernel32.dll")), "AddDllDirectory");
	if (fSetDefaultDllDirectories && fAddDllDirectory) {
		BOOL b = (*fSetDefaultDllDirectories)(LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);
		DLL_DIRECTORY_COOKIE c = (*fAddDllDirectory)(chipDir.AsString().c_str());
	}
	else {
		msg(WARN, MTEXT("Engine: KB2533623 not found on this system. Using alternative functions."));
		Char buff[32767];
		DWORD size = GetEnvironmentVariableA(MTEXT("PATH"), buff, 32767);
		if (size == 0)
			return false;
		String str(buff);
		str += MTEXT(";") + chipDir.AsString();
		if (!SetEnvironmentVariableA(MTEXT("PATH"), str.c_str()))
			return false;
	}
#endif

	if (!_impl->cm.FindChips(chipDir)) {
		msg(FATAL, MTEXT("Engine: The chip manager failed to initiate."));
		return false;
	}

	static const Guid GRAPHICS_GUID = { 0x13278213, 0x8360, 0x4a1b, { 0xa8, 0x5d, 0xc7, 0x8c, 0x42, 0xb3, 0xed, 0x94 } };

	Guid graphics = GRAPHICS_GUID; // The only thing to switch between the engines.

	// Set up the graphics engine.
	_impl->graphics = (Graphics*)_impl->cm.CreateChip(graphics);

	if (!_impl->graphics) {
		msg(FATAL, MTEXT("Engine: Failed to set up graphics engine."));
		return false;
	}

	return true;
}

void Engine::Reset()
{
	_impl->dm.Clear();
	_impl->cgm.Clear2();
	_impl->cm.ClearGlobalChips();
	functionStack.ResetPerfFrame();
	_impl->appTime = 0;
}

void Engine::Clear()
{
	Reset();
	SAFE_RELEASE(_impl->graphics);
	_impl->application = nullptr;
}

Application* Engine::GetApplication() const { return _impl->application; }
Graphics* Engine::GetGraphics() { return _impl->graphics; }
DocumentManager* Engine::GetDocumentManager() { return &_impl->dm; }
ChipManager* Engine::GetChipManager() { return &_impl->cm; }
ClassManager* Engine::GetClassManager() { return &_impl->cgm; }
FunctionSignatureManager* Engine::GetFunctionSignatureManager() { return &_impl->fsm; }
Environment* Engine::GetEnvironment() { return &_impl->env; }
uint32 Engine::GetFrameNr() const { return _impl->frameNr; }
int32 Engine::GetFrameTime() const { return _impl->frameTime; }
const HighPrecisionTimer& Engine::GetTimer() const { return _impl->timer; }
int64 Engine::GetDt() const { return _impl->dt; }
int64 Engine::GetAppTime() const { return _impl->appTime; }
const FPS& Engine::GetFPS() const { return _impl->fps; }
bool Engine::IsBreakMode() const { return _impl->frameTimeStoppedTime != -1; }
bool Engine::IsRunning() const { return _impl->isRunning; }
Path Engine::GetThirdDepsDirectory() const { return _impl->thirdDir; }
const List<String>& Engine::GetCmdLineArguments() const { return _impl->cmdLineArguments; }
void Engine::SetCmdLineArguements(const List<String>& args) { _impl->cmdLineArguments = args; }
EditMode Engine::GetEditMode() const { return _impl->editMode; }
void Engine::SetEditMode(EditMode editMode) { _impl->editMode = editMode; }


void Engine::SetMessageFile(Path p)
{
	if (_impl->msgfile.is_open())
		_impl->msgfile.close();
	if (p.IsFile())
		_impl->msgfile.open(p.AsString().c_str(), std::ios::out);
}

void Engine::Message(MessageSeverity severity, String message, ClassID cgID, ChipID chipID)
{
	static const Char *MSG[6] = {MTEXT(" - DEBUG: "), MTEXT(" - INFO: "), MTEXT(" - NOTICE: "), MTEXT(" - WARNING: "), MTEXT(" - FATAL: "), MTEXT(" - ")};

	uint32 threadID = (uint32)GetCurrentThreadId(); // todo: use standard C?? std::this_thread...

	std::unique_lock<std::mutex> cb(_impl->csMsg);

	time_t t = time(0);

	if (severity >= _impl->vsDbgLevel) {
		Char buff[64];

		tm tt;
		localtime_s(&tt, &t);

		strftime(buff, 64, MTEXT("%d/%m/%y %H:%M:%S"), &tt);

		String s = String(buff) + strUtils::format(MTEXT(" [% 5i]"), threadID) + MSG[severity] + message + MTEXT("\n");

#if defined( DEBUG ) || defined( _DEBUG )
		OutputDebugStringA(s.c_str()); // Also try writing to output window in Visual Studio. NOTE: OutputDebugStringA is not allowed when submitting app to AppStore!!
#endif

		if (_impl->msgfile.is_open()) {
			_impl->msgfile << s.c_str();
			_impl->msgfile.flush();
		}
	}

	if (_impl->application) {
		ApplicationMessage msg = { severity, t, threadID, message, cgID, chipID };
		_impl->application->MessagedAdded(msg);
	}
}

void Engine::ChipMessageAdded(Chip *chip, const ChipMessage &msg)
{
	::msg(msg.severity, msg.msg, chip);
	if (_impl->application)
		_impl->application->ChipMessageAdded(chip, msg);
}

void Engine::ChipMessageRemoved(Chip *chip, const ChipMessage &msg)
{
	if (_impl->application)
		_impl->application->ChipMessageRemoved(chip, msg);
}

void Engine::Run()
{
	++_impl->frameNr;

//	if (!_timer.IsStarted())
//		_timer.Start();

	_impl->isRunning = true;

	_impl->timer.Tick();

	_impl->dt = std::min(std::max(_impl->timer.GetDt_us(), 100ll), 500000ll); // clamp dt to 2 hz - 10000 hz
	_impl->appTime += _impl->dt;

	_impl->fps.NewFrame(_impl->timer.GetTime_us());

	_impl->frameTime = GetClockTime();

	functionStack.StartOfFrame();

	_impl->graphics->ClearState();

	_impl->cm.OnNewFrame();

	_impl->graphics->OnNewFrame();

	GetClassManager()->Run();

	_impl->graphics->PostFrame();

	_impl->isRunning = false;

	bool endOfFrameOK = functionStack.EndOfFrame();

	assert(endOfFrameOK);
}

int32 Engine::GetClockTime() const
{
	if (_impl->frameTimeStoppedTime != -1)
		return _impl->frameTimeStoppedTime - _impl->frameTimeSub + (_impl->frameTimeStoppedTime < _impl->frameTimeSub ? (std::numeric_limits<int32>::max() + 1) : 0);
	int32 t = (int32)clock();
	return t - _impl->frameTimeSub + (t < _impl->frameTimeSub ? (std::numeric_limits<int32>::max() + 1) : 0); // / CLOCKS_PER_SEC
}

void Engine::StopClockTime()
{
	if (_impl->frameTimeStoppedTime == -1) // Timer not stopped?
		_impl->frameTimeStoppedTime = clock(); // Set stopped time!
}

void Engine::StartClockTime()
{
	if (_impl->frameTimeStoppedTime != -1) { // Stopped?
		int32 t = (int32)clock(); // Get current time
		_impl->frameTimeSub += t - _impl->frameTimeStoppedTime + (t < _impl->frameTimeStoppedTime ? (std::numeric_limits<int32>::max() + 1) : 0); // Get time since timer was stopped (taking possible loop into account)!
		if (_impl->frameTimeSub < 0)
			_impl->frameTimeSub += (std::numeric_limits<int32>::max() + 1);
		_impl->frameTimeStoppedTime = -1; // Timer not stopped!
	}
}

void Engine::StopAppTime()
{
}

void Engine::StartAppTime()
{
}

void Engine::Break(Chip *chip)
{
	StopClockTime(); // Stop the timer used to calculate how long it was since a chip7link was active!
	StopAppTime(); // Stop the application time.

	if (_impl->application)
		_impl->application->Break(chip);

	StartAppTime();
	StartClockTime();
}

void Engine::DestroyDeviceObjects()
{
	if (_impl->application)
		_impl->application->DestroyDeviceObjects();
	_impl->cgm.DestroyDeviceObjects();
	_impl->cm.DestroyDeviceObjects();
}









void m3d::msg(MessageSeverity severity, String message)
{
	engine->Message(severity, message);
}

void m3d::msg(MessageSeverity severity, String message, const Class *cg)
{
	if (!cg)
		return msg(severity, message);
	engine->Message(severity, message, cg->GetID());
}

void m3d::msg(MessageSeverity severity, String message, const Chip *chip)
{
	if (!chip)
		return msg(severity, message);
	engine->Message(severity, message, chip->GetClass() ? chip->GetClass()->GetID() : InvalidClassID, chip->GetID());
}

