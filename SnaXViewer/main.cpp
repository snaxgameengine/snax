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
#include "resource.h"
#include "Viewer.h"
#include "M3DEngine/Engine.h"
#include "M3DEngine/DocumentFileTypes.h"
#include <memory>
#include <iostream>
#include <fstream>

#define LIB_PATH MTEXT("Libraries\\")

/*
#if defined(WIN32) && defined(_DEBUG)
	#define _CRTDBG_MAP_ALLOC
	#include <stdlib.h>
	#include <crtdbg.h>
	#define DEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
	#define new DEBUG_NEW
#endif*/

/*
OBSOBS:

The CRT adds a header to the beginning of every memory block that is allocated. If you have defined the symbol _CRTDBG_MAP_ALLOC before including crtdbg.h, this memory block header contains a pointer to the name of the file where the CRT function that allocated the memory block was called from. #File Error# is emitted instead of the file name when the executing process does not have read access to the memory reference by this pointer.

There are two probable causes for not having read access to the memory reference by this pointer:

Heap corruption. Overwriting the block of memory preceding the block giving the #File Error# may have caused corruption in the block header. Usually, there are other errors as well, such as "Damage after Normal Block."
The memory block was allocated in a DLL that was unloaded prior to the _CrtMemDumpAllObjectsSince() call.

*/



using namespace m3d;


String GetFileNameFromDialog()
{
	Char filename[MAX_PATH] = MTEXT("");
	OPENFILENAMEA ofn;
	ofn.lStructSize = sizeof(OPENFILENAMEA);
	ofn.hwndOwner = NULL;
	ofn.hInstance = NULL;
	ofn.lpstrFilter = MTEXT("All Files (*.*)\0*.*\0Projects (*.m3j;*.m3b;*.m3x)\0*.m3j;*.m3b;*.m3x\0\0");
	ofn.lpstrCustomFilter = NULL;
	ofn.nMaxCustFilter = 0;
	ofn.nFilterIndex = 2;
	ofn.lpstrFile = filename;
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = MTEXT("c:\\");
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_NOCHANGEDIR;
	ofn.nFileOffset = 0;
	ofn.nFileExtension = 0;
	ofn.lpstrDefExt = NULL;
	ofn.lCustData = 0;
	ofn.lpfnHook = 0;
	ofn.lpTemplateName = NULL;
	ofn.pvReserved = NULL;
	ofn.dwReserved = 0;
	ofn.FlagsEx = 0;
	ofn.lpstrTitle = MTEXT("SnaXViewer - Open Project");
	BOOL b = GetOpenFileNameA(&ofn);
	if (b) {
		return filename;
	}
	return MTEXT("");
}


int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
/*#if defined(DEBUG) | defined(_DEBUG)
	// Enable run-time memory check for debug builds.
	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
	_CrtSetReportMode( _CRT_ERROR, _CRTDBG_MODE_DEBUG );
#endif*/

	SetErrorMode(SEM_FAILCRITICALERRORS); // Without this, calling LoadLibrary() that fail, will quit the application...

	Path start; // Project to start

	// Check if we have a startup file.
	std::ifstream startup("startup");
	if (startup.is_open()) {
		Char line[256];
		startup.getline(line,256);
		String s = line;
		if (s.length() > 0)
			start = Path::File(s);
		startup.close();
	}
	
	// Parse command line and find startgroup of project. Use dialog box if neccessary!
	// Argument 0 should ALWAYS be the start group to open if directory of the viewer does not contain a startup file.
	int argc = __argc;
	LPSTR* argv = __argv;// lpCmdLine[0] != MCHAR('\0') ? CommandLineToArgvW(lpCmdLine, &argc) : nullptr;
	Path sfxFile;
	List<String> arguments;
	if (argc > 1) {
		int i = 1; // arg0 is name of exe.
		if (!start.IsFile())
			start = Path::File(argv[i++]);
		for (; i < argc; i++) {
			String s(argv[i]);
			if (s.substr(0, 10) == MTEXT("__sfxFile="))
				sfxFile = Path::File(s.substr(10));
			else
				arguments.push_back(s);
		}
	}

	if (!start.IsFile()) {
		String s = GetFileNameFromDialog();
		start = Path::File(s);
	}
	if (!start.IsFile())
		return -1;

	Viewer::GetViewer()->Run(hInstance, start, arguments, sfxFile, MTEXT(""));

	Viewer::GetViewer()->Destroy();

	return 0;
}

