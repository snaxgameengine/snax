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
#include "MainWindow.h"
#include "App.h"
#include <DirectXTK/SimpleMath.h>
#include <qapplication.h>
#include <qclipboard.h>
#include <qstandardpaths.h>
#include <qdir.h>
#include <qstylefactory.h>
#include "M3DEngine/DocumentLoader.h"

// UPDATE THIS FOR EACH NEW VERSION!!!
#define STUDIO_VERSION "1.7.0.0"


using namespace m3d;


Guid L2K() { return NullGUID; }



int main(int argc, char *argv[])
{
	int result = -1;

	SetErrorMode(SEM_FAILCRITICALERRORS); // Without this, calling LoadLibrary() that fail, will quit the application...

	// This will prevent (un)installation while the app is running!
	HANDLE hAppMutex = CreateMutexA(NULL, FALSE, MTEXT("SnaX_running"));

	HRESULT hr;

	hr = CoInitializeEx(0, COINIT_APARTMENTTHREADED);
	if (FAILED(hr)) {
		MessageBoxA(NULL, MTEXT("CoInitializeEx(...) Failed"), MTEXT("Error"), MB_OK|MB_ICONERROR);
		ReleaseMutex(hAppMutex);
		return result;
	}

	hr = CoInitializeSecurity(NULL, -1, NULL, NULL, RPC_C_AUTHN_LEVEL_DEFAULT, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE, NULL);
	if (FAILED(hr)) {
		MessageBoxA(NULL, MTEXT("CoInitializeSecurity(...) Failed"), MTEXT("Error"), MB_OK|MB_ICONERROR);
		CoUninitialize();
		ReleaseMutex(hAppMutex);
		return result;
	}

	DocumentLoader::SetKey(&L2K); // Set an invalid key L2K!

	if (!Engine::Create()) {
		MessageBoxA(NULL, MTEXT("Failed to start Engine."), MTEXT("Error"), MB_OK|MB_ICONERROR);
		CoUninitialize();
		ReleaseMutex(hAppMutex);
		return result;
	}



	{
		QApplication::setLibraryPaths(QStringList(QString(argv[0]).left(QString(argv[0]).lastIndexOf('\\')) + "\\qtplugins")); // plugins folder are in exe folder!
		QApplication::setStyle("Fusion");
		QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
		QGuiApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
		QGuiApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::RoundPreferFloor);
		int qargc = 1; // Ignore all other arguments for QT.
		App a(qargc, argv);
		QCoreApplication::setOrganizationName("SnaX Game Engine");
		QCoreApplication::setOrganizationDomain("snaxgameengine.com");
		QCoreApplication::setApplicationName("SnaX Developer");
		QCoreApplication::setApplicationVersion(STUDIO_VERSION);


		// Make sure AppData-folder exist.
		QString appDataFolder = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
		QDir(appDataFolder).mkpath(".");

		{
			QString fileToOpen;

			// Improve this stuff!
			if (argc > 1) {
				fileToOpen = QString(argv[1]);
				if (!QFile::exists(fileToOpen)) {
					MessageBoxA(NULL, MTEXT("Invalid file name."), MTEXT("Error"), MB_OK|MB_ICONERROR);
					fileToOpen.clear();
				}
			}

			MainWindow w;
			if (w.init(fileToOpen)) {
				a.SetEngineActive(true);
				w.show();
				result = a.exec();
			}
			w.clear();
		}
	}

	Engine::Destroy();

	CoUninitialize();
	ReleaseMutex(hAppMutex);

	return result;
}
