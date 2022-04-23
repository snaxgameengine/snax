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

#include "M3DCore/Path.h"
#include "M3DEngine/Engine.h"
#include "M3DEngine/Application.h"
#include "ViewerInputManager.h"
#include "M3DCore/GuidUtil.h"

namespace m3d
{

class ViewerWindow;

class Viewer : public Application
{
public:
	static Viewer *GetViewer();

	bool Run(HINSTANCE hInstance, Path project, List<String> arguments, Path exeFile, String libDirs);
	void Destroy();

protected:
	// Application instance.
	HINSTANCE _hInstance;
	// Primary application window.
	ViewerWindow *_window;

	ViewerInputManager _im;

	Path _exeFile;

	bool _init(HINSTANCE hInstance, Path project, List<String> arguments, Path exeFile, String libDirs);
	void _run();
	void _msgbox(String s);

	Viewer();
	~Viewer();

	virtual ExeEnvironment GetExeEnvironment() const override { return ExeEnvironment::EXE_VIEWER; }
	virtual Path GetExeFile() const override;
	virtual Path GetApplicationFile() const override;
	virtual void Quit() override;
	virtual void MessagedAdded(const ApplicationMessage &msg) override {}
	virtual void DestroyDeviceObjects() override;
	virtual int GetDisplayOrientation() override { return 0; }
	virtual void ChipMessageAdded(Chip *chip, const ChipMessage &msg) override {}
	virtual void ChipMessageRemoved(Chip *chip, const ChipMessage &msg) override {}
	virtual InputManager *GetInputManager() override { return &_im; }
	virtual void Break(Chip *chip) {}
	virtual bool IsBreakPointsEnabled() const override { return false; }
	virtual unsigned GetFeatureMask() const override { return 0xFFFFFFFF; } // Enable all features!

	static LRESULT CALLBACK __MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT CALLBACK _MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

};


}
