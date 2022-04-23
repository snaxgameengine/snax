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
/*
#include ".\..\D3DChips\D3DInclude.h"
#include "List.h"


namespace m3d
{


class DXGIEnumerations
{
public:
	struct Output
	{
		SIDXGIOutput output;
		List<DXGI_MODE_DESC> modes;
	};

	struct Adapter
	{
		SIDXGIAdapter1 adapter;
		List<Output> outputs;
	};

	HRESULT Enumerate(IDXGIFactory1 *factory, DXGI_FORMAT format);

	const List<Adapter> &GetAdapters() const { return _adapters; }

private:
	DXGI_FORMAT _format;

	List<Adapter> _adapters;
};

struct D3DConfigDialogSelection
{
	SIDXGIAdapter1 adapter;
	SIDXGIOutput output;
	bool fullscreen;
	DXGI_MODE_DESC mode;
	DXGI_SAMPLE_DESC ms;
	bool vSync;
	bool debug;
	D3D_DRIVER_TYPE type;
	D3D_FEATURE_LEVEL featureLevel;
};

class D3DConfigDialog
{
public:
	D3DConfigDialog();
	~D3DConfigDialog();

	int Show(D3DConfigDialogSelection &selection);

	BOOL Init();
	void Destroy();
	BOOL OnWndMsg(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
	BOOL OnCmdMsg(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);

protected:
	HWND _wnd;
	DXGIEnumerations _enums;
	D3DConfigDialogSelection *_sel;

	void _updateOutput();
	void _updateResolution();
};





}
*/