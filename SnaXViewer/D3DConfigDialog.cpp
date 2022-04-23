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
#include "D3DConfigDialog.h"
/*
#include "resource.h"
#include "Engine.h"
#include "./../D3DChips/D3DGraphics.h"


using namespace m3d;

HRESULT DXGIEnumerations::Enumerate(IDXGIFactory1 *factory, DXGI_FORMAT format)
{
	HRESULT hr;

	_format = format;

	// Enumerate adapters.
	for(UINT i = 0;; i++) {
		SIDXGIAdapter1 adapter;

		hr = factory->EnumAdapters1(i, &adapter);
		if(hr == DXGI_ERROR_NOT_FOUND)
			break;

		V_RETURN(hr);

		if (adapter) {
			Adapter a;
			a.adapter = adapter;
			for(UINT j = 0;; j++) {
				SIDXGIOutput output;
				hr = adapter->EnumOutputs(j, &output);
				if(hr == DXGI_ERROR_NOT_FOUND)
					break;
			
				V_RETURN(hr);

				// only add outputs that are attached to the desktop
				// TODO: AttachedToDesktop seems to be always 0
//				DXGI_OUTPUT_DESC od;
//				output->GetDesc(&od);

				Output o;
				o.output = output;
				UINT count = 0;
				V_RETURN(output->GetDisplayModeList(_format, 0, &count, nullptr));
				if (count) {
					o.modes.resize(count);
					V_RETURN(output->GetDisplayModeList(_format, 0, &count, &o.modes.front()));
				}
				a.outputs.push_back(o);
			}
			_adapters.push_back(a);
		}
	}


	return S_OK;
}




BOOL CALLBACK D3DConfigDialog_Proc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	D3DConfigDialog *dlg = reinterpret_cast<D3DConfigDialog*>(message == WM_INITDIALOG ? lParam : GetWindowLong(hwnd, GWLP_USERDATA));

	if(!dlg)
		return FALSE;

	return dlg->OnWndMsg(hwnd, message, wParam, lParam);
}

D3DConfigDialog::D3DConfigDialog()
{
	_wnd = NULL;
	_sel = nullptr;
}

D3DConfigDialog::~D3DConfigDialog()
{
}


int D3DConfigDialog::Show(D3DConfigDialogSelection &selection)
{
	_sel = &selection;
	INT_PTR result = DialogBoxParam(GetModuleHandle(nullptr), MAKEINTRESOURCE(IDD_DIALOG1), nullptr, (DLGPROC) D3DConfigDialog_Proc, (LPARAM)this) ;
	if (result == -1)
		return -1; // error
	if (result == IDC_OK)
		return 1; //ok
	return 0; // cancel
}



BOOL D3DConfigDialog::Init()
{
	HRESULT hr;

	Button_SetCheck(GetDlgItem(_wnd, IDC_SHOWALLRES), FALSE);
	Button_SetCheck(GetDlgItem(_wnd, IDC_FULLSCREEN), _sel->fullscreen);

	HWND cb = GetDlgItem(_wnd, IDC_ADAPTER);
	ComboBox_ResetContent(cb);

	hr = _enums.Enumerate(((D3DGraphics*)engine->GetGraphics())->GetDXGIFactory(), _sel->mode.Format);

	for (unsigned i = 0; i < _enums.GetAdapters().size(); i++) {
		DXGI_ADAPTER_DESC1 desc;
		_enums.GetAdapters()[i].adapter->GetDesc1(&desc);

		ComboBox_SetItemData(cb, ComboBox_AddString(cb, desc.Description), i);
	}

	ComboBox_SetCurSel(cb, 0);
	_updateOutput();

	return TRUE;
}

void D3DConfigDialog::Destroy()
{
}

BOOL D3DConfigDialog::OnWndMsg(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) {   
        case WM_COMMAND:
 			return OnCmdMsg (hwnd, (int)LOWORD(wParam), (HWND)(lParam), (UINT)HIWORD(wParam));
        case WM_SYSCOMMAND:
			if(wParam == SC_CLOSE)
				EndDialog(_wnd, 0);
			return FALSE;
        case WM_INITDIALOG:
			{
				_wnd = hwnd;
				SetWindowLong(_wnd, GWLP_USERDATA, (DWORD)this);
				if (!Init())
					EndDialog(_wnd, -1);
				{
				   // center the dialog
				   RECT r2;
				   GetWindowRect(hwnd, &r2);
				   POINT pt;
				   int w = GetSystemMetrics(SM_CXSCREEN);
				   int h = GetSystemMetrics(SM_CYSCREEN);
				   pt.x = w/2 - (r2.right - r2.left)/2;
				   pt.y = h/2 - (r2.bottom - r2.top)/2;
				   SetWindowPos(hwnd, HWND_TOP, pt.x, pt.y, 0, 0, SWP_NOSIZE);
				}
				return TRUE;
			}
            return TRUE;
		case WM_DESTROY:
			Destroy();
			break;
	}

	return FALSE;
}

BOOL D3DConfigDialog::OnCmdMsg(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
	switch (id) {
	case IDC_OK:
		EndDialog(_wnd, IDC_OK);
		return TRUE;
	case IDC_ADAPTER:
		if (codeNotify == CBN_SELCHANGE) {/*
			HWND cbA = GetDlgItem(_wnd, IDC_ADAPTER);
			int i = ComboBox_GetCurSel(cbA);
			_sel->adapter;
			IDXGIAdapter1 *s = ((i == CB_ERR) ? SIDXGIAdapter1() : (_enums.GetAdapters()[(int)ComboBox_GetItemData(cbA, i)].adapter)); * /
			_updateOutput();
		}
		return TRUE;
	case IDC_OUTPUT:
		if (codeNotify == CBN_SELCHANGE) {/*
			HWND cbA = GetDlgItem(_wnd, IDC_ADAPTER);
			int i = ComboBox_GetCurSel(cbA);
			HWND cbO = GetDlgItem(_wnd, IDC_OUTPUT);
			int j = ComboBox_GetCurSel(cbO);
			_sel->output = (i == CB_ERR || j == CB_ERR) ? SIDXGIOutput() : _enums.GetAdapters()[(int)ComboBox_GetItemData(cbA, i)].outputs[(int)ComboBox_GetItemData(cbO, j)].output; * /
			_updateResolution();
		}
		return TRUE;
	case IDC_SHOWALLRES:
		if (codeNotify == BN_CLICKED)
			_updateResolution();
		return TRUE;
	}
	return FALSE;
}

void D3DConfigDialog::_updateOutput()
{
	HWND cbA = GetDlgItem(_wnd, IDC_ADAPTER);
	HWND cbO = GetDlgItem(_wnd, IDC_OUTPUT);
	int i = ComboBox_GetCurSel(cbA);
	if (i != CB_ERR) {
		int aIndex = (int)ComboBox_GetItemData(cbA, i);
		ComboBox_ResetContent(cbO);
		for (unsigned i = 0; i < _enums.GetAdapters()[aIndex].outputs.size(); i++) {
			DXGI_OUTPUT_DESC desc;
			_enums.GetAdapters()[aIndex].outputs[i].output->GetDesc(&desc);
			ComboBox_SetItemData(cbO, ComboBox_AddString(cbO, desc.DeviceName), i);
		}
		ComboBox_SetCurSel(cbO, 0);
	}
	_updateResolution();
}

void D3DConfigDialog::_updateResolution()
{
	HWND cbA = GetDlgItem(_wnd, IDC_ADAPTER);
	HWND cbO = GetDlgItem(_wnd, IDC_OUTPUT);
	HWND cbR = GetDlgItem(_wnd, IDC_RESOLUTION);

	ComboBox_ResetContent(cbR);
	int i = ComboBox_GetCurSel(cbA);
	if (i == CB_ERR)
		return;
	int aIndex = (int)ComboBox_GetItemData(cbA, i);
	i = ComboBox_GetCurSel(cbO);
	if (i == CB_ERR)
		return;
	int oIndex = (int)ComboBox_GetItemData(cbO, i);
	const DXGIEnumerations::Output &o = _enums.GetAdapters()[aIndex].outputs[oIndex];
	DXGI_OUTPUT_DESC desc;
	o.output->GetDesc(&desc);
	FLOAT ar = (FLOAT)(desc.DesktopCoordinates.right - desc.DesktopCoordinates.left) / (desc.DesktopCoordinates.bottom - desc.DesktopCoordinates.top);

	BOOL showAll = Button_GetCheck(GetDlgItem(_wnd, IDC_SHOWALLRES));
	 
	int best = 0;
	float bestDiff = 1.0e10f;

	//CheckMultisampleQualityLevels(
	
	for (unsigned i = 0; i < o.modes.size(); i++) {
		const DXGI_MODE_DESC &m = o.modes[i];
		if (showAll || ((FLOAT)m.Width / m.Height) == ar) {
			String s = String::fromNum(m.Width) + MTEXT(" x ") + String::fromNum(m.Height) + MTEXT(" @ ") + String::fromNum((FLOAT)m.RefreshRate.Numerator / m.RefreshRate.Denominator, MTEXT("%.2f")) + MTEXT(" Hz");
			int index =  ComboBox_AddString(cbR, s.c_str());
			ComboBox_SetItemData(cbR, index, i);
			float diff = 0.0f;
			diff += fabs((FLOAT)(_sel->mode.Width - o.modes[i].Width) / _sel->mode.Width);
			diff += fabs((FLOAT)(_sel->mode.Height - o.modes[i].Height) / _sel->mode.Height);
			if (_sel->mode.RefreshRate.Denominator != 0)
				diff += fabs((FLOAT)((FLOAT)_sel->mode.RefreshRate.Numerator/_sel->mode.RefreshRate.Denominator - (FLOAT)o.modes[i].RefreshRate.Numerator/o.modes[i].RefreshRate.Denominator) / ((FLOAT)_sel->mode.RefreshRate.Numerator/_sel->mode.RefreshRate.Denominator));
			if (diff < bestDiff) {
				best = index;
				bestDiff = diff;
			}
		}
	}
	ComboBox_SetCurSel(cbR, best);
}

*/