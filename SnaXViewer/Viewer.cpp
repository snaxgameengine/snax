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
#include "Viewer.h"
#include "M3DEngine/Engine.h"
#include "M3DEngine/InputManager.h"
#include "M3DEngine/ClassManager.h"
#include "D3DConfigDialog.h"
#include "M3DEngine/RenderWindowManager.h"
#include "ViewerWindow.h"
#include "M3DEngine/DocumentLoader.h"
#include "M3DEngine/Application.h"
#include "M3DEngine/DocumentManager.h"
#include "M3DEngine/Document.h"
#include "GraphicsChips/Graphics.h"

using namespace m3d;

#define VIEWER_NAME MTEXT("SnaXViewer")


#define CHIPS_PATH MTEXT("Chips\\")
#define THIRD_PATH MTEXT("3rd\\")
#define LIB_PATH MTEXT("Libraries\\")


GUID L2K() 
{ 
	static const GUID UNLOCK_LEVEL2_KEY = { 0x066d4ff4, 0xdab0, 0x4c47, { 0xa9, 0x4a, 0xfa, 0x59, 0xee, 0x11, 0x5d, 0xec } };
	return UNLOCK_LEVEL2_KEY; 
}



Viewer::Viewer() : _hInstance(NULL), _window(nullptr)
{
	DocumentLoader::SetKey(&L2K);
}

Viewer::~Viewer()
{
}

Viewer *Viewer::GetViewer()
{
	static Viewer v;
	return &v;
}

bool Viewer::Run(HINSTANCE hInstance, Path project, List<String> arguments, Path exeFile, String libDirs)
{
	if (!Engine::Create())
		return false;

	bool r;

	if (r = _init(hInstance, project, arguments, exeFile, libDirs)) {
		_run();
		engine->Reset();
	}

	if (_window) {
		_window->Destroy();
		mmdelete(_window);
		_window = nullptr;
	}

	engine->Clear();

	Engine::Destroy();

	return r;
}

bool Viewer::_init(HINSTANCE hInstance, Path project, List<String> arguments, Path exeFile, String libDirs)
{
	engine->SetCmdLineArguements(arguments);

	_exeFile = exeFile;

	engine->SetMessageFile(Path(MTEXT("log.txt")));

	Path viewerPath;
	CHAR p[MAX_PATH] = {MCHAR('\0')};
	if (GetModuleFileNameA(NULL, p, MAX_PATH) > 0)
		viewerPath = Path::File(p).GetDirectory();

	List<Path> libPaths;
	if (libDirs.empty()) {
		libPaths.push_back(Path::Dir(Path(LIB_PATH), viewerPath));
	}
	else {

	}

	// Set directories and search for chips.
	if (!engine->Init(this, Path::Dir(Path(CHIPS_PATH), viewerPath), Path::Dir(Path(THIRD_PATH), viewerPath), libPaths)) {
		msg(FATAL, MTEXT("Failed to set up directories"));
		return false;
	}

	// Initialize graphics!
	if (!engine->GetGraphics()->Init()) {
		msg(FATAL, MTEXT("Failed to initialize graphics."));
		return false;
	}


#if defined( DEBUG ) || defined( _DEBUG )
	engine->GetGraphics()->SetRequestDebugDevice(true);
#endif

	// Create the window class.
	if (!ViewerWindow::CreateWindowClass(hInstance, __MsgProc)) {
		msg(FATAL, MTEXT("Failed to create window class."));
		return false;
	}

	_window = mmnew ViewerWindow();

	// Init the render window.
	if (!_window->Init()) {
		msg(FATAL, MTEXT("Failed to initialize application window."));
		return false;
	}

	// Load the project
	Document *doc = engine->GetDocumentManager()->GetDocument(project);
	if (!doc) {
		_msgbox(MTEXT("Failed to load project: ") + project.AsString() + MTEXT("."));
		return false;
	}

	Class *cg = doc->GetStartClass();
	if (!cg) {
		_msgbox(MTEXT("Document did not contain an entry point: ") + project.AsString() + MTEXT("."));
		return false;
	}

	engine->GetClassManager()->SetStartClass(cg);

	return true;
}

void Viewer::Destroy()
{
}

Path Viewer::GetExeFile() const
{
	return _exeFile;
}

Path Viewer::GetApplicationFile() const
{
	Path viewerPath;
	CHAR p[MAX_PATH] = { MCHAR('\0') };
	if (GetModuleFileNameA(NULL, p, MAX_PATH) > 0)
		viewerPath = Path::File(p);
	return viewerPath;
}

void Viewer::Quit()
{
	PostQuitMessage(0);
}

void Viewer::DestroyDeviceObjects()
{
	if (_window)
		_window->OnDestroyDevice();
}

void Viewer::_msgbox(String s)
{
	MessageBoxA(NULL, s.c_str(), VIEWER_NAME, MB_ICONERROR | MB_OK);
}

void Viewer::_run()
{
	MSG wmsg;
	memset(&wmsg, 0, sizeof(MSG));
	//PeekMessage( &wmsg, NULL, 0U, 0U, PM_NOREMOVE );

	while (true) {
		// Use PeekMessage() so we can use idle time to render the scene. 
		while (PeekMessage(&wmsg, NULL, 0U, 0U, PM_REMOVE) != 0 && wmsg.message != WM_QUIT) {
			// Translate and dispatch the message
			if (!_window || _window->GetWindowHandle() == NULL || TranslateAccelerator(_window->GetWindowHandle(), NULL, &wmsg) == NULL) {
				TranslateMessage(&wmsg);
				DispatchMessage(&wmsg);
			}
		}
		if (WM_QUIT == wmsg.message)
			break;

		// Render a frame during idle time (no messages are waiting)

		if (_window->GetPresentResult() == PRESENT_OCCLUDED) {
			_window->Present(true);
			Sleep(10);
			continue;
		}

		// Update input manager
		engine->GetApplication()->GetInputManager()->Update(); // TODO: should be moved inside engine->Run()

		// Run a frame in the engine!
		engine->Run();

		UINT r = _window->Present();
		if (r == PRESENT_DEVICE_RESET || r == PRESENT_DEVICE_REMOVED) {
			msg(WARN, MTEXT("The D3D12 graphics device was removed. We will try to recreate the device, but it could happen that not all your graphics resources preserves their current state..."));
			engine->GetGraphics()->DestroyDevice();
		}

		// Clear graphics state.
		engine->GetGraphics()->ClearState();
	}
}

LRESULT Viewer::__MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return Viewer::GetViewer()->_MsgProc(hWnd, uMsg, wParam, lParam);
}

LRESULT Viewer::_MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	MSG msg;
	msg.hwnd = hWnd;
	msg.lParam = lParam;
	msg.message = uMsg;
	msg.wParam = wParam;
	msg.pt = POINT();
	msg.time = 0;

	_im.MsgProc(&msg);

    switch( uMsg )
    {
        case WM_CLOSE:
            PostQuitMessage( 0 );
			break;
        case WM_SIZE:
			if (_window && hWnd == _window->GetWindowHandle())
				_window->OnResizeWindow();
			break;
		case WM_DISPLAYCHANGE:
			break;
    };

    return DefWindowProc( hWnd, uMsg, wParam, lParam );
}
