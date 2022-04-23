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
#include "DXEditorWidget.h"
#include "M3DEngine/Engine.h"
#include "EditorInputManager.h"
#include "M3DEngine/Renderwindow.h"
#include "M3DEngine/ChipManager.h"
#include "M3DEngine/Function.h"
#include "M3DEngine/DocumentXMLLoader.h"
#include "TextItemDialog.h"
#include "EditorWidget.h"
#include "M3DEngineEXt/ClassDiagram.h"
#include "M3DCore/Util.h"
#include "M3DEngineExt/ClassRenderer2.h"
#include "StdChips/ClassChip.h"
#include "StdChips/FunctionCall.h"
#include <DirectXTK/WICTextureLoader.h>
#include <qcolordialog.h>
#include <QContextMenuEvent>
#include <QMessageBox>
#include <qfiledialog.h>
#include <DirectXTK/ScreenGrab.h>
#include <wincodec.h>


using namespace m3d;


DXEditorWidget::DXEditorWidget(QWidget * parent, Qt::WindowFlags f) : QWidget(parent, f)
{
	setAttribute(Qt::WA_PaintOnScreen);
	setAttribute(Qt::WA_NativeWindow);

	_clazzr = nullptr;
	_classView = nullptr;
	_templateExtent = Vector4(0.0f, 0.0f, 0.0f, 0.0f);
	_isDraggingTemplate = false;

	_im = new EditorInputManager();

	_isD3DInit = false;
	_testNext = false;
	_forceVSync = false;

	_clazzr = new ClassRenderer2();

	_generalMenu = new QMenu(this);
	QMenu *addMenu = _generalMenu->addMenu("Add");
	
	_addChipAction = addMenu->addAction(QIcon(":/EditorApp/Resources/chip.png"), "Chip", this, &DXEditorWidget::addChipFromMenu);
	_addRectangleAction = addMenu->addAction(QIcon(":/EditorApp/Resources/add_rectangle.png"), "Rectangle", this, &DXEditorWidget::addRectangle);
	_addImageAction = addMenu->addAction(QIcon(":/EditorApp/Resources/add_image.png"), "Image", this, &DXEditorWidget::addImage);
	_addTextAction = addMenu->addAction(QIcon(":/EditorApp/Resources/add_text.png"), "Text", this, &DXEditorWidget::addText);
	_addFolderAction = addMenu->addAction(QIcon(":/EditorApp/Resources/open.png"), "Folder", this, &DXEditorWidget::addFolder);
	_generalMenu->addSeparator();
	_generalMenu->addAction(ActionManager::instance().getAction(ActionID::CUT));
	_generalMenu->addAction(ActionManager::instance().getAction(ActionID::COPY));
	_generalMenu->addAction(ActionManager::instance().getAction(ActionID::PASTE));
	_generalMenu->addAction(ActionManager::instance().getAction(ActionID::DELITE));
	_generalMenu->addSeparator();
	_classDescAction = _generalMenu->addAction("Class Description...");

	_chipMenu = new QMenu(this);
	_chipPropAction = _chipMenu->addAction(QIcon(":/EditorApp/Resources/prop-dialog.png"), "Chip Properties...");
	_commentsAction = _chipMenu->addAction(QIcon(":/EditorApp/Resources/note.png"), "Comments...");
	_setStartAction = _chipMenu->addAction(QIcon(":/EditorApp/Resources/start.png"), "Set as Start Chip");
	_chipMenu->addSeparator();
	QMenu *cmm = _chipMenu->addMenu("Convert to");
	_convertInstanceDataAction = cmm->addAction(QIcon(":/EditorApp/Resources/instance-data.png"), "Instance Data");
	_convertFunctionDataAction = cmm->addAction(QIcon(":/EditorApp/Resources/function-data.png"), "Function Data");
	_convertFunctionCallAction = cmm->addAction(QIcon(":/EditorApp/Resources/func-call.png"), "Function Call");
	_convertParameterAction = cmm->addAction(QIcon(":/EditorApp/Resources/parameter.png"), "Parameter");
	_convertProxyAction = cmm->addAction(QIcon(":/EditorApp/Resources/proxy.png"), "Proxy");
	_convertStdChipAction = cmm->addAction(QIcon(":/EditorApp/Resources/chip.png"), "Standard Chip");
	_chipMenu->addSeparator();
	_chipMenu->addAction(ActionManager::instance().getAction(ActionID::CUT));
	_chipMenu->addAction(ActionManager::instance().getAction(ActionID::COPY));
	_chipMenu->addAction(ActionManager::instance().getAction(ActionID::DELITE));
	_chipMenu->addSeparator();
	_arrangeChildrenAction = _chipMenu->addAction("Arrange Children");

	_childConnMenu = new QMenu(this);
	_convertToGrowingConnectionAction = _childConnMenu->addAction("Convert to growing connection");
	_convertToMultiConnectionAction = _childConnMenu->addAction("Convert to multi-connection");

	_folderMenu = new QMenu(this);
	_enterFolderAction = _folderMenu->addAction(QIcon(":/EditorApp/Resources/folder_open.png"), "Enter Folder");
	_renameFolderAction = _folderMenu->addAction("Rename Folder...");
	_unpackFolderAction = _folderMenu->addAction("Unpack Folder");
	_folderMenu->addSeparator();
	_folderMenu->addAction(ActionManager::instance().getAction(ActionID::CUT));
	_folderMenu->addAction(ActionManager::instance().getAction(ActionID::COPY));
	_folderMenu->addAction(ActionManager::instance().getAction(ActionID::DELITE));

	_linkMenu = new QMenu(this);
	_linkMenu->addAction(ActionManager::instance().getAction(ActionID::DELITE));

	_backgroundItemMenu = new QMenu(this);
	_editItemAction = _backgroundItemMenu->addAction("Edit Item...");
	_backgroundItemMenu->addSeparator();
	_backgroundItemMenu->addAction(ActionManager::instance().getAction(ActionID::CUT));
	_backgroundItemMenu->addAction(ActionManager::instance().getAction(ActionID::COPY));
	_backgroundItemMenu->addAction(ActionManager::instance().getAction(ActionID::DELITE));

	_addChildMenu = new AddChipMenu(this);

	EditorD3DDevice::GetInstance()->AddListener(this);
}

DXEditorWidget::~DXEditorWidget()
{
	EditorD3DDevice::GetInstance()->RemoveListener(this);
	_classView = nullptr; // We're not the owner!
	SAFE_DELETE(_clazzr);
	SAFE_DELETE(_im);
}

ActionMask DXEditorWidget::getActions()
{
	return ActionMask(ActionID::CUT) | ActionMask(ActionID::COPY) | ActionMask(ActionID::PASTE) | ActionMask(ActionID::DELITE);
}

bool DXEditorWidget::isActionEnabled(ActionID id)
{
	ClassView *doc = _classView;
	if (!doc)
		return false;

	bool canCopy = doc->CanCopy();
	bool canPaste = doc->CanPaste();
	bool canDelete = doc->CanDelete();

	switch (id)
	{
	case ActionID::CUT: return canCopy && canDelete;
	case ActionID::COPY: return canCopy;
	case ActionID::PASTE: return canPaste && (QApplication::clipboard()->mimeData() && QApplication::clipboard()->mimeData()->text().left(20).contains("\"snax\""));
	case ActionID::DELITE: return canDelete;
	}

	return false;
}

void DXEditorWidget::triggered(ActionID id)
{
	switch (id) 
	{
	case ActionID::CUT: return cut();
	case ActionID::COPY: return copy();
	case ActionID::PASTE: return paste();
	case ActionID::DELITE: return delite();
	}
}


bool DXEditorWidget::nativeEvent(const QByteArray &eventType, void *msg, qintptr *result)
{
	MSG *message = (MSG*)msg;
	if (message->message == WM_SIZE) {
		if (_isD3DInit)
			resizeDX();
		_skippedFrames = -1;
		InvalidateRect(message->hwnd, 0, true);
	}
	else if (message->message == WM_PAINT) {
		static bool skipPaintMessage = false; // assert() does not work without this. New paint messages are being sent while the assert dialog is shown causing trouble!

		if (skipPaintMessage)
			return false;

		skipPaintMessage = true;

		if (!_isD3DInit) {
			if (SUCCEEDED(initD3D()))
				_isD3DInit = true;
			else
				return false;
		}


		if (_testNext) {
			HRESULT hr = _swapChain->Present(0, DXGI_PRESENT_TEST);
			_testNext = hr == DXGI_PRESENT_TEST;
		}
		else {
		
			newFrame();
		
		}

		skipPaintMessage = false;
	}

	return false;
}

void DXEditorWidget::invalidateGraphics()
{
	InvalidateRect((HWND)winId(), 0, false); // Triggers a WM_PAINT message
}

void DXEditorWidget::OnDestroyDevice()
{
	_clazzr->OnDestroyDevice();
	_rtv = nullptr;
	_swapChain = nullptr;
	_textWriter.OnDestroyDevice();
	_fonts.clear();
	_isD3DInit = false;
}

void DXEditorWidget::SetFPSProps(bool showFPS, bool limitFPS)
{
	_showFPS = showFPS;
	_limitFPS = limitFPS;
}


void DXEditorWidget::setView(ClassView *classView)
{
	_classView = classView;

	// Make sure copy/paste actions are updated!
	ActionManager::instance().updateActions();
}

HRESULT FindCompatibleSampleDesc(ID3D11Device *device, DXGI_FORMAT fmt, DXGI_SAMPLE_DESC &sDesc)
{
	HRESULT hr;
	for (UINT i = sDesc.Count; i > 0; i--) {
		UINT q = 0;
		V_RETURN(device->CheckMultisampleQualityLevels(fmt, i, &q));
		if (q > 0) {
			if (i == sDesc.Count)
				q = std::min(q, sDesc.Quality + 1);
			sDesc.Count = i;
			sDesc.Quality = q - 1;
			return S_OK;
		}
	}

	return E_FAIL;
}

HRESULT DXEditorWidget::initD3D()
{
	HRESULT hr;

	SIDXGIDevice DXGIDevice;
	SIDXGIAdapter DXGIAdapter;
	SIDXGIFactory DXGIFactory;

	ID3D11Device *device = EditorD3DDevice::GetInstance()->GetDevice();
	if (!device)
		return E_FAIL;

	V_RETURN(_textWriter.Init(device, nullptr));


	V_RETURN(device->QueryInterface(IID_IDXGIDevice, (void**)&DXGIDevice));
	V_RETURN(DXGIDevice->GetAdapter(&DXGIAdapter));
	V_RETURN(DXGIAdapter->GetParent(IID_IDXGIFactory, (void**)&DXGIFactory));

	RECT clientRect;
	GetWindowRect((HWND)winId(), &clientRect);

	// Update current windowed size!
	UINT windowedWidth = clientRect.right - clientRect.left;
	UINT windowedHeight = clientRect.bottom - clientRect.top;

	DXGI_SWAP_CHAIN_DESC scDesc;
	scDesc.BufferDesc.Width = windowedWidth;
	scDesc.BufferDesc.Height = windowedHeight;
	scDesc.BufferDesc.RefreshRate.Numerator = 0;
	scDesc.BufferDesc.RefreshRate.Denominator = 0;
	scDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	scDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	scDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	scDesc.SampleDesc.Count = 4;
	scDesc.SampleDesc.Quality = 4;
	scDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	scDesc.BufferCount = 3;
	scDesc.OutputWindow = (HWND)winId();
	scDesc.Windowed = TRUE;
	scDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	scDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	// Check MSAA settings!
	V_RETURN(FindCompatibleSampleDesc(device, scDesc.BufferDesc.Format, scDesc.SampleDesc));

	V_RETURN(DXGIFactory->CreateSwapChain(DXGIDevice, &scDesc, &_swapChain));

	V_RETURN(_getRenderTargetView());

	V_RETURN(_clazzr->Init(device));

	return S_OK;
}

HRESULT DXEditorWidget::_getRenderTargetView()
{
	HRESULT hr;

	SID3D11Texture2D backBuffer;
	V_RETURN(_swapChain->GetBuffer(0, IID_ID3D11Texture2D, (void**)&backBuffer));

	backBuffer->GetDesc(&_bbDesc);

	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
	rtvDesc.Format = _bbDesc.Format;
	if (_bbDesc.SampleDesc.Count > 1) 
		rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMS;
	else {
		rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		rtvDesc.Texture2D.MipSlice = 0;
	}

	SID3D11Device device;
	backBuffer->GetDevice(&device);

	V_RETURN(device->CreateRenderTargetView(backBuffer, &rtvDesc, &_rtv));

	return S_OK;
}

HRESULT DXEditorWidget::resizeDX()
{
	HRESULT hr;

	_rtv = nullptr;

	DXGI_SWAP_CHAIN_DESC desc;
	V_RETURN(_swapChain->GetDesc(&desc));

	V_RETURN(_swapChain->ResizeBuffers(desc.BufferCount, 0, 0, desc.BufferDesc.Format, desc.Flags));

	return _getRenderTargetView();
}


HRESULT DXEditorWidget::newFrame()
{
	HRESULT hr;
//	QPoint p = mapFromGlobal(QCursor::pos());

	// Limit framerate of editor view to 60FPS.
	if (_limitFPS && _skippedFrames != -1) // Skip test on first frame, or window resize.
	{
		static const int64 fpsTarget = 1000 * 1000 / 60;

		_hpt.Tick();
		int64 t = _hpt.GetTime_us();
		if (((double)(t - _lastRenderTime) * (1.0 + 0.5 / (_skippedFrames + 1))) < fpsTarget) {
			_skippedFrames++;
			return S_OK;
		}
	}

	_im->Update();//XMFLOAT2(p.x(), p.y()));
	_im->SetDevicePixelRatio(devicePixelRatioF());

	ID3D11Device *device = EditorD3DDevice::GetInstance()->GetDevice();

	SID3D11DeviceContext context;
	device->GetImmediateContext(&context);

	ID3D11RenderTargetView *rtv = _rtv;
	context->OMSetRenderTargets(1, &rtv, 0);

	D3D11_VIEWPORT vp;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0.0f;
	vp.TopLeftY = 0.0f;
	vp.Width = (FLOAT)_bbDesc.Width;
	vp.Height = (FLOAT)_bbDesc.Height;

	context->RSSetViewports(1, &vp);

	QPoint p = mapToGlobal(QPoint(0, 0));
	p.setX(devicePixelRatioF() * p.x());
	p.setY(devicePixelRatioF() * p.y());

	if (_classView) {
		_classView->PreRenderProcssing(_im, vp.Width, vp.Height, p.x(), p.y());
		V_RETURN(_clazzr->Render(context, *_classView, devicePixelRatio(), _isDraggingTemplate ? &_templateExtent : nullptr, hasFocus()));
		_classView->PostRenderProcssing(_im, this);
	}
	else {
		const Vector4 v(0.5, 0.5, 0.5, 1.0);
		context->ClearRenderTargetView(_rtv, (float*)&v);
	}


	// Statistics.
	_hpt.Tick();
	_lastRenderTime = _hpt.GetTime_us();
	_skippedFrames = 0;
	_fps.NewFrame(_lastRenderTime);

	if (_showFPS) {
		String txt = strUtils::format(MTEXT("FPS: %i"), unsigned(_fps.GetFPS() + 0.5));

		{
			int scale = devicePixelRatio();
			auto a = _fonts.find(scale);
			if (a == _fonts.end()) {
				_fonts[scale].Init(MTEXT("Arial"), 96 * scale, 10);
				a = _fonts.find(scale);
			}

			_textWriter.SetFont(&a->second);
		}

		_textWriter.Write(context, XMFLOAT2(5.0f, 5.0f), TextWriter::LEFT_TOP, false, XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f), txt);
	}

//	engine->GetGraphics()->DumpMessages();

	HRESULT presentResult = _swapChain->Present(/*_forceVSync ? 1 :*/ 0, (_testNext ? DXGI_PRESENT_TEST : 0));
	if (presentResult == DXGI_ERROR_DEVICE_REMOVED) {
		HRESULT deviceRemovedReason = device->GetDeviceRemovedReason();
		String txt;
		switch (deviceRemovedReason)
		{
		case DXGI_ERROR_DEVICE_HUNG: txt = MTEXT("Editor D3D11-device removed. Device hung. Trying to recreate device..."); break;
		case DXGI_ERROR_DEVICE_REMOVED: txt = MTEXT("Editor D3D11-device removed. Device was physically removed or driver upgrade occured. Trying to recreate device..."); break;
		case DXGI_ERROR_DEVICE_RESET: txt = MTEXT("Editor D3D11-device removed. Device was reset due to badly formed commands. Trying to recreate device..."); break;
		case DXGI_ERROR_DRIVER_INTERNAL_ERROR: txt = MTEXT("Editor D3D11-device removed. Device internal error. Trying to recreate device..."); break;
		case DXGI_ERROR_INVALID_CALL: txt = MTEXT("Editor D3D11-device removed. Device encountered an invalid call. Trying to recreate device..."); break;
		default: txt = MTEXT("Editor D3D11-device removed. No reason identified. Trying to recreate device..."); break;
		}
		msg(WARN, txt);
		
		HRESULT hr = EditorD3DDevice::GetInstance()->RecreateDevice();
		if (SUCCEEDED(hr)) {
			msg(INFO, MTEXT("Succeeded recreating editor D3D11-device!"));
		}
		else {
			msg(FATAL, MTEXT("Ouch! Recreating the editor D3D11-device failed! Looks like you will have to shut down and restart SnaX Developer... Sorry about that!"));
		}
	}
	_testNext = presentResult == DXGI_PRESENT_TEST;

	if (hasFocus()) {
		ActionManager::instance().updateAction(ActionID::CUT);
		ActionManager::instance().updateAction(ActionID::COPY);
		ActionManager::instance().updateAction(ActionID::DELITE);
	}

	return S_OK;
}

void DXEditorWidget::focusInEvent (QFocusEvent *event)
{
//	QPoint p = mapFromGlobal(QCursor::pos());
//	_im->GotFocus(XMFLOAT2(p.x(), p.y()));
	QWidget::focusInEvent(event);
	emit onFocusIn();
}

void DXEditorWidget::mouseDoubleClickEvent (QMouseEvent *event)
{
	_im->mouseDoubleClickEvent(event);
}

void DXEditorWidget::mousePressEvent (QMouseEvent *event)
{
	_mouseMoved = false;
	_im->mousePressEvent(event);
}

void DXEditorWidget::mouseReleaseEvent (QMouseEvent *event)
{
	_im->mouseReleaseEvent(event);
}

void DXEditorWidget::mouseMoveEvent(QMouseEvent *event) 
{
	_mouseMoved = true;
}

void DXEditorWidget::wheelEvent(QWheelEvent *event)
{
	if (event->angleDelta().y() != 0)
		_im->wheelEvent(event);
}

static int KeyPressModifiersHack = 0;

void DXEditorWidget::keyPressEvent(QKeyEvent *event)
{
	if (!event->isAutoRepeat())
		_im->keyPressEvent(event);
	KeyPressModifiersHack = event->modifiers();
}

void DXEditorWidget::keyReleaseEvent(QKeyEvent *event)
{
	if (!event->isAutoRepeat())
		_im->keyReleaseEvent(event);

	ClassView *doc = _classView;
	if (!doc)
		return;
	if (event->key() == Qt::Key_C && KeyPressModifiersHack == 0 && event->modifiers() == 0 && doc->GetClass()->CanAddFoldersAndBackgroundItems()) {
		_mousePositionBeforeMenuExec = _classView->GetMousePos();
		_useMousePositionBeforeMenuExec = true;
		addChipFromMenu();
		_useMousePositionBeforeMenuExec = false;
	}
}

bool DXEditorWidget::IsSavable() const
{
	return _classView && _classView->GetClass()->GetDocument() != nullptr;
}

bool DXEditorWidget::TakeScreenshot(const Char *filename)
{
	ID3D11Device *device = EditorD3DDevice::GetInstance()->GetDevice();
	if (!device)
		return false;
	SID3D11DeviceContext context;
	device->GetImmediateContext(&context);
	SID3D11Resource backBuffer;
	_rtv->GetResource(&backBuffer);

	String ext = Path(filename).GetFileExtention();

	HRESULT hr = E_FAIL;
	wchar_t wpath[MAX_PATH];
	if (strUtils::compareNoCase(ext, String(MTEXT("dds"))) == 0) {
		hr = SaveDDSTextureToFile(context, backBuffer, strUtils::widen(wpath, MAX_PATH, filename));
	}
	else {
		GUID guid = {};
		if (strUtils::compareNoCase(ext, String(MTEXT("jpg"))) == 0)
			guid = GUID_ContainerFormatJpeg;
		else if (strUtils::compareNoCase(ext, String(MTEXT("png"))) == 0)
			guid = GUID_ContainerFormatPng;
		else if (strUtils::compareNoCase(ext, String(MTEXT("bmp"))) == 0)
			guid = GUID_ContainerFormatBmp;
		else if (strUtils::compareNoCase(ext, String(MTEXT("tiff"))) == 0)
			guid = GUID_ContainerFormatTiff;
		else
			return false;

		hr = SaveWICTextureToFile(context, backBuffer, guid, strUtils::widen(wpath, MAX_PATH, filename));//, &GUID_WICPixelFormat32bppBGRA);
	}

	return SUCCEEDED(hr);
}

Chip *DXEditorWidget::addChip(unsigned chipTypeIndex)
{
	if (!_classView)
		return nullptr;

	ClassExt *cge = _classView->GetClass();

	Vector2 p = _useMousePositionBeforeMenuExec ? _mousePositionBeforeMenuExec : _classView->GetMousePos();
	Chip *ch = cge->AddChip(chipTypeIndex, _classView->GetFolder(), p);
	if (ch)
		cge->SetDirty();

	return ch;
}

Chip *DXEditorWidget::addClassChip(ClassID cgid, Vector2 *pos)
{
	if (!_classView)
		return nullptr;

	Class *cg = engine->GetClassManager()->GetClass(cgid);
	if (!cg)
		return nullptr;

	ClassExt *cge = _classView->GetClass();

	Vector2 p = _useMousePositionBeforeMenuExec ? _mousePositionBeforeMenuExec : _classView->GetMousePos();

	Chip *ch = cge->AddChip(engine->GetChipManager()->GetChipTypeIndex(CLASSCHIP_GUID), _classView->GetFolder(), pos ? *pos : p);
	if (ch) {
		ClassChip *c = dynamic_cast<ClassChip*>(ch);
		if (c) {
			c->SetCG(cg);
			c->SetName(cg->GetName());
		}
		cge->SetDirty();
	}

	return ch;
}

Chip *DXEditorWidget::addFunctionCallChip(ClassID cgid, ChipID function, bool callByName)
{
	if (!_classView)
		return nullptr;

	Class *cg = engine->GetClassManager()->GetClass(cgid);
	if (!cg)
		return nullptr;

	Chip *chFunc = cg->GetChip(function);
	if (!chFunc)
		return nullptr;

	Function *f = chFunc->GetFunction();
	if (!f)
		return nullptr;

	ClassExt *cge = _classView->GetClass();

	if (f->GetAccess() == Function::Access::Private && cg != cge) {
		QMessageBox::warning(this, "Function inaccessible", QString("The function \'") + TOQSTRING(chFunc->GetName()) + "\' is declared private and can not be accessed outside \'" + TOQSTRING(cg->GetName()) + "\'.");
		return nullptr;
	}

	if (f->GetAccess() == Function::Access::Protected && !cge->IsBaseClass(cg)) {
		QMessageBox::warning(this, "Function inaccessible", QString("The function \'") + TOQSTRING(chFunc->GetName()) + "\' is declared protected and can not be accessed outside \'" + TOQSTRING(cg->GetName()) + "\' or its subgraphs.");
		return nullptr;
	}


/*	THIS IS WRONG! YOU CAN'T, BEFORE THE FUNCTION IS CALLED, DETERMIND IF CALL-BY-NAME IS VALID OR NOT!!
	if (callByName && f->GetType() != Function::STATIC && !cge->IsBaseGraph(cg)) {
		QMessageBox::warning(this, "Function can't be called by name", QString("The function \'") + TOQSTRING(chFunc->GetName()) + "\' can only be called by name from within a non-static function in \'" + TOQSTRING(cg->GetName()) + "\' or its subgraphs.");
		return;
	}*/
	
	Vector2 p = _useMousePositionBeforeMenuExec ? _mousePositionBeforeMenuExec : _classView->GetMousePos();

	Chip *ch = cge->AddChip(engine->GetChipManager()->GetChipTypeIndex(FUNCTIONCALL_GUID), _classView->GetFolder(), p);
	if (!ch)
		return nullptr;

	cge->SetDirty();

	FunctionCall *fc = dynamic_cast<FunctionCall*>(ch);
	if (!fc)
		return nullptr;

	fc->SetChipType(chFunc->GetChipType());
	fc->SetFunction(cg->GetName(), chFunc->GetName(), callByName, true);

	return ch;
}

void DXEditorWidget::ShowChipDialog(Chip *chip, bool showComment)
{
	emit showChipDialog(chip, showComment);
}

void DXEditorWidget::ShowContextMenu()
{
	ClassView *doc = _classView;
	if (!doc)
		return;

	_mousePositionBeforeMenuExec = _classView->GetMousePos();
	_useMousePositionBeforeMenuExec = true;

	if (doc->GetHoverMode() == HM_NONE) {
		ClassExt* cg = doc->GetClass();

		bool b = doc->GetClass()->CanAddFoldersAndBackgroundItems();
		_addChipAction->setEnabled(b);
		_addRectangleAction->setEnabled(b);
		_addImageAction->setEnabled(b);
		_addTextAction->setEnabled(b);
		_addFolderAction->setEnabled(b);
		_classDescAction->setEnabled(dynamic_cast<ClassDiagram*>(cg) == nullptr);

		QAction *a = _generalMenu->exec(QCursor::pos());

		if (a == _classDescAction) {
			showClassDescriptionDialog(cg);
		}
	}
	else if (doc->GetHoverMode() == HM_CHIP) {
		ChipID cvid = doc->GetHoverChip();
		ClassExt *cg = doc->GetClass();
		Chip *chip = cg->GetChip(cvid);
		if (!chip)
			return;

		_setStartAction->setEnabled(dynamic_cast<ClassDiagram*>(cg) == nullptr);
		_convertProxyAction->setEnabled(cg->CanConvertChip(chip, ClassExt::PROXY));
		_convertInstanceDataAction->setEnabled(cg->CanConvertChip(chip, ClassExt::INSTANCE_DATA));
		_convertFunctionDataAction->setEnabled(cg->CanConvertChip(chip, ClassExt::FUNCTION_DATA));
		_convertFunctionCallAction->setEnabled(cg->CanConvertChip(chip, ClassExt::FUNCTION_CALL));
		_convertParameterAction->setEnabled(cg->CanConvertChip(chip, ClassExt::PARAMETER));
		_convertStdChipAction->setEnabled(cg->CanConvertChip(chip, ClassExt::STANDARD));

		QAction *a = _chipMenu->exec(QCursor::pos());
		
		// It could actually happen that the chip is deleted until we get here...
		chip = cg->GetChip(cvid);
		if (!chip)
			return;

		if (a == _chipPropAction) {
			emit showChipDialog(chip, false);
		}
		else if (a == _commentsAction) {
			emit showChipDialog(chip, true);
		}
		else if (a == _setStartAction) {
			if (cg->GetStartChip() != chip) {
				cg->SetStartChip(chip);
				cg->SetDirty();
			}
			if (engine->GetClassManager()->GetStartClass() != cg)
				engine->GetClassManager()->SetStartClass(cg);
		}
		else if (a == _convertProxyAction) {
			if (cg->ConvertChip(chip, ClassExt::PROXY))
				cg->SetDirty();
		}
		else if (a == _convertInstanceDataAction) {
			if (cg->ConvertChip(chip, ClassExt::INSTANCE_DATA)) {
				cg->SetDirty();
				// TODO: Owners of instances of cg and derived must be marked dirty! (Same when deleting instance data!)
			}
		}
		else if (a == _convertFunctionDataAction) {
			if (cg->ConvertChip(chip, ClassExt::FUNCTION_DATA))
				cg->SetDirty();
		}
		else if (a == _convertFunctionCallAction) {
			if (cg->ConvertChip(chip, ClassExt::FUNCTION_CALL)) {
				cg->SetDirty();
				showChipDialog(chip, false);
			}
		}
		else if (a == _convertParameterAction) {
			if (cg->ConvertChip(chip, ClassExt::PARAMETER))
				cg->SetDirty();
		}
		else if (a == _convertStdChipAction) {
			if (cg->ConvertChip(chip, ClassExt::STANDARD))
				cg->SetDirty();
		}
		else if (a == _arrangeChildrenAction) {
			cg->ArrangeChildren(chip);
			cg->SetDirty();
		}
	}
	else if (doc->GetHoverMode() == HM_CHILDCONN) {
		ChipID cvid = doc->GetHoverChip();
		ClassExt* cg = doc->GetClass();
		Chip* chip = cg->GetChip(cvid);
		if (!chip)
			return;

		ConnectionID cid = chip->GetChipEditorData()->GetConnectionID(doc->GetHoverConn());
		if (cid == InvalidConnectionID)
			return;
		const ChildConnection *cl = chip->GetChildren()[cid.first];
		if (!cl)
			return;
		ChildConnectionDesc::ConnectionType cType = cl->desc.connType;
		if (cType == ChildConnectionDesc::SINGLE)
			return;
		_convertToGrowingConnectionAction->setVisible(cType == ChildConnectionDesc::MULTI);
		_convertToMultiConnectionAction->setVisible(cType == ChildConnectionDesc::GROWING);
		QAction* a = _childConnMenu->exec(QCursor::pos());
		if (a == _convertToGrowingConnectionAction) {
			ChildConnectionDesc cDesc = cl->desc;
			cDesc.connType = ChildConnectionDesc::GROWING;
			chip->SetConnection(cid.first, cDesc, true);
		}
		else if (a == _convertToMultiConnectionAction) {
			ChildConnectionDesc cDesc = cl->desc;
			cDesc.connType = ChildConnectionDesc::MULTI;
			chip->SetConnection(cid.first, cDesc, true);
		}
	}
	else if (doc->GetHoverMode() == HM_BGITEM || doc->GetHoverMode() == HM_BGITEM_SCALE) {
		QAction *a = _backgroundItemMenu->exec(QCursor::pos());
		if (a == _editItemAction) {
			UpdateBackgroundItem(doc->GetHoverBackgroundItem());
		}
	}
	else if (doc->GetHoverMode() == HM_FOLDER) {
		QAction *a = _folderMenu->exec(QCursor::pos());

		FolderID folder = doc->GetHoverFolder();

		if (a == _enterFolderAction) {
			emit enterFolder(doc->GetClass(), (unsigned)folder, doc->GetFolder() != folder);
		}
		else if (a == _renameFolderAction) {
			bool ok = false;
			QString s = QInputDialog::getText(this, "Rename Folder", "Folder Name:", QLineEdit::Normal, TOQSTRING(doc->GetClass()->GetFolderList()[folder].name), &ok);
			if (ok) {
				String s2 = FROMQSTRING(s);
				if (s2 != doc->GetClass()->GetFolderList()[folder].name) {
					doc->GetClass()->SetFolderName(folder, s2);
					doc->GetClass()->SetDirty();
				}
			}
		}
		else if (a == _unpackFolderAction) {
			doc->UnpackFolder(folder);
			doc->GetClass()->SetDirty();
		}
	}
	else if (doc->GetHoverMode() == HM_LINK) {
		_linkMenu->exec(QCursor::pos());
	}
	else if (doc->GetHoverMode() == HM_ADDCHILD) {
		Chip *c = addChipFromMenu(doc->GetDragGUID());
		if (c)
			doc->AddChildToDraggedConnection(c, _addChildMenu->type == 0); // allow rename if menu was ordinary chip, not class or function call...
	}

	_useMousePositionBeforeMenuExec = false;
}

DXEditorWidget::ListAction::ListAction(QWidget *parent) : QWidgetAction(parent)
{
	QWidget *p = new QWidget();
	QGridLayout *l = new QGridLayout();
	l->setContentsMargins(QMargins(0, 0, 0, 0));
	p->setLayout(l);
	edit = new QLineEdit(p);
	list = new QListWidget(p);
	list->setSortingEnabled(true);
	l->addWidget(edit, 0, 0);
	l->addWidget(list, 1, 0);
	setDefaultWidget(p);
	p->setMinimumSize(300, 300);
	edit->installEventFilter(this);

	connect(edit, &QLineEdit::textEdited, [=]() {
		QString str = edit->text();
		if (str.isEmpty()) {
			for (int i = 0; i < list->count(); i++) {
				list->item(i)->setHidden(false);
			}
		}
		else {
			int s = -1, t = -1;
			for (int i = 0; i < list->count(); i++) {
				QListWidgetItem *itm = list->item(i);
				bool b = itm->text().contains(str, Qt::CaseInsensitive);
				if (b && t == -1)
					t = i;
				itm->setHidden(!b);
				if (b && s == -1 && itm->text().startsWith(str, Qt::CaseInsensitive))
					s = i;
			}
			int i = s != -1 ? s : t;
			if (i != -1) {
				list->setCurrentRow(i);
				list->item(i)->setSelected(true);
			}
		}
	});

	connect(edit, &QLineEdit::returnPressed, [=]() { 
		auto a = list->selectedItems();
		if (!a.isEmpty())
			emit list->itemClicked(a.first());
	});
}

bool DXEditorWidget::ListAction::eventFilter(QObject *watched, QEvent *event)
{
	if (watched == edit && event->type() == QEvent::KeyPress) {
		QKeyEvent *e = dynamic_cast<QKeyEvent*>(event);
		if (e) {
			if (e->key() == Qt::Key_Up || e->key() == Qt::Key_Down) {
				if (list->currentRow() == -1) {
					for (int i = 0; i < list->count(); i++) {
						if (!list->item(i)->isHidden()) {
							list->setCurrentRow(i);
							break;
						}
					}
				}
				else {
					bool up = e->key() == Qt::Key_Up;
					for (int i = 1; i < list->count(); i++) {
						int k = (list->currentRow() + list->count() + i * (up ? -1 : 1)) % list->count();
						if (!list->item(k)->isHidden()) {
							list->setCurrentRow(k);
							break;
						}
					}
				}
			}
			if (list->currentItem())
				list->currentItem()->setSelected(true);
		}
	}
	return false;
}

void DXEditorWidget::ListAction::fill(ChipTypeIndex typeRequired, ClassExt *currentClass)
{
	const ChipTypeIndex proxyID = engine->GetChipManager()->GetChipTypeIndex(PROXYCHIP_GUID);
	const ChipTypeIndex classChipID = engine->GetChipManager()->GetChipTypeIndex(CLASSCHIP_GUID);

	const bool supportsClassChip = typeRequired == InvalidChipTypeIndex || engine->GetChipManager()->IsChipTypeSupported(typeRequired, classChipID);

	list->clear();
	edit->clear();


	const PacketPtrByStringMap &pm = engine->GetChipManager()->GetPacketMap();
	static const QIcon chipIcon = QIcon(":/EditorApp/Resources/chip.png");
	static const QIcon funcIcons[3] = { QIcon(":/EditorApp/Resources/static-func.png"), QIcon(":/EditorApp/Resources/nonvirt-func.png"), QIcon(":/EditorApp/Resources/virt-func.png") };
	static const QIcon classIcon = QIcon(":/EditorApp/Resources/graph.png");

	QListWidgetItem *citm = nullptr;
	for (const auto &n : pm) {
		for (const auto &m : n.second->chips) {
			if (m.second->chipDesc.usage == ChipDesc::HIDDEN)
				continue;
			if (typeRequired == InvalidChipTypeIndex || engine->GetChipManager()->IsChipTypeSupported(typeRequired, m.second->chipTypeIndex) || engine->GetChipManager()->IsChipTypeSupported(proxyID, m.second->chipTypeIndex)) {
				QListWidgetItem *itm = new QListWidgetItem(TOQSTRING(m.second->chipDesc.name), list);
				itm->setIcon(chipIcon);
				itm->setData(Qt::UserRole, 0); // chip
				itm->setData(Qt::UserRole + 1, m.second->chipTypeIndex);
				itm->setToolTip("Chip");
				if (m.second->chipTypeIndex == typeRequired)
					citm = itm;
			}
		}
	}
	for (const auto &n : engine->GetClassManager()->GetClasssByName()) {
		if (supportsClassChip) {
			QListWidgetItem* itm = new QListWidgetItem(TOQSTRING(n.first), list);
			itm->setIcon(classIcon);
			itm->setData(Qt::UserRole, 2); // class
			itm->setData(Qt::UserRole + 1, (unsigned)n.second->GetID());
			itm->setToolTip("Class");
		}
		for (const auto &m : n.second->GetFunctions()) {
			if (typeRequired == InvalidChipTypeIndex || engine->GetChipManager()->IsChipTypeSupported(typeRequired, m->GetChip()->GetChipTypeIndex())) {
				if (m->GetAccess() == Function::Access::Public || m->GetAccess() == Function::Access::Private && currentClass == n.second || m->GetAccess() == Function::Access::Protected && currentClass->IsBaseClass(n.second)) {
					QListWidgetItem* itm = new QListWidgetItem(QString("%1 (%2)").arg(TOQSTRING(m->GetChip()->GetName())).arg(TOQSTRING(n.first)), list);
					itm->setIcon(funcIcons[(uint32)m->GetType()]);
					itm->setData(Qt::UserRole, 1); // function
					itm->setData(Qt::UserRole + 1, (unsigned)n.second->GetID());
					itm->setData(Qt::UserRole + 2, (unsigned)m->GetChip()->GetID());
					itm->setToolTip("Function");
				}
			}
		}
	}

	edit->setFocus(Qt::FocusReason::ActiveWindowFocusReason);
	if (citm) {
		list->setCurrentItem(citm);
		citm->setSelected(true);
		list->scrollToItem(citm, QAbstractItemView::EnsureVisible);
	}
	else
		list->scrollToTop();
}

DXEditorWidget::AddChipMenu::AddChipMenu(QWidget *parent) : QMenu(parent)
{
	addChipAction = new ListAction(this);
	addAction(addChipAction);
	type = -1;
	chipType = InvalidChipTypeIndex;
	chipId = InvalidChipID;
	classId = InvalidClassID;
	connect(addChipAction->list, &QListWidget::itemClicked, [this](QListWidgetItem *itm) {
		if (itm) {
			type = itm->data(Qt::UserRole).toUInt();
			switch (type)
			{
			case 0: chipType = (ChipTypeIndex)itm->data(Qt::UserRole + 1).toUInt(); break;
			case 1: chipId = (ChipID)itm->data(Qt::UserRole + 2).toUInt(); // falldown..
			case 2: classId = (ClassID)itm->data(Qt::UserRole + 1).toUInt(); break;
			}

		}
		close();
	});
}

void DXEditorWidget::AddChipMenu::exe(ChipTypeIndex typeRequired, ClassExt *currentClass)
{
	addChipAction->fill(typeRequired, currentClass);
	type = -1;
	chipType = InvalidChipTypeIndex;
	chipId = InvalidChipID;
	classId = InvalidClassID;
	exec(QCursor::pos());
//	return _selected;
}

Chip *DXEditorWidget::addChipFromMenu(ChipTypeIndex chipTypeIndex)
{
	_addChildMenu->exe(chipTypeIndex, _classView->GetClass());
	Chip *c = nullptr;
	switch (_addChildMenu->type)
	{
	case 0: c = addChip(_addChildMenu->chipType); break;
	case 1: c = addFunctionCallChip(_addChildMenu->classId, _addChildMenu->chipId, false); break;
	case 2: c = addClassChip(_addChildMenu->classId); break;
	}
	return c;
}

void DXEditorWidget::FocusOnChip(ClassExt *cg, Chip *chip)
{
	emit focusOnChip(cg, chip);
}

void DXEditorWidget::EnterFolder(ClassExt *cg, FolderID folderID, bool inside)
{
	emit enterFolder(cg, (unsigned)folderID, inside);
}

void DXEditorWidget::AddBackgroundItem(CGBackgroundItem::Type type)
{
	if (!_classView)
		return;

	Vector2 p = _useMousePositionBeforeMenuExec ? _mousePositionBeforeMenuExec : _classView->GetMousePos();

	switch (type) 
	{
	case CGBackgroundItem::RECT:
		{
			QColor c = QColorDialog::getColor(QColor(0,180,0,255), this, "Color", QColorDialog::ShowAlphaChannel);
			if (c.isValid()) {
				_classView->AddRectangle(Vector4(c.redF(), c.greenF(), c.blueF(), c.alphaF()), p);
				_classView->GetClass()->SetDirty();
			}
		}
		break;
	case CGBackgroundItem::IMAGE:
		{
			QString fn = QFileDialog::getOpenFileName(this, "Image", "c:\\", "Images (*.bmp *.jpg *.png *.tif *.tiff *.gif);;All Files (*.*)", 0, QFileDialog::Options());
			if (fn.size() > 0) {
				if (!_addImage(FROMQSTRING(fn))) {
					QMessageBox::warning(this, "Error", "Failed to load image from file.");
				}
			}
		}
		break;
	case CGBackgroundItem::TEXT:
		{
			QString text;
			float s = 5;
			QColor c(0, 0, 0, 255);
			if (TextItemDialog::show(this, text, s, c)) {
				String str = FROMQSTRING(text);
				Vector2 size(10.0f, 10.0f);
				_clazzr->FormatTextItem(str, size, s * 0.1f);

				_classView->AddText(FROMQSTRING(text), size, s * 0.1f, Vector4(c.redF(), c.greenF(), c.blueF(), c.alphaF()), p);
				_classView->GetClass()->SetDirty();
			}
		}
		break;
	}
}

void DXEditorWidget::addFolder()
{
	if (!_classView)
		return;
	bool ok = false;
	QString s = QInputDialog::getText(this, "Add Folder", "Folder Name:", QLineEdit::Normal, "New Folder", &ok);
	if (ok) {
		Vector2 p = _useMousePositionBeforeMenuExec ? _mousePositionBeforeMenuExec : _classView->GetMousePos();
		_classView->AddFolder(FROMQSTRING(s), p);
	}
}

void DXEditorWidget::cut() 
{
	if (_classView && _classView->CanCopy() && _classView->CanDelete()) {
		copy();
		delite();
	}
}

void DXEditorWidget::copy() 
{
	if (_classView && _classView->CanCopy()) {
		DataBuffer db;
		_classView->Copy(db);
		if (db.getBufferSize()) {
			QMimeData *data = new QMimeData();
			QByteArray ba((const char*)db.getConstBuffer(), db.getBufferSize());
			data->setText(ba);
			QApplication::clipboard()->setMimeData(data);
		}
	}
}

void DXEditorWidget::paste() 
{
	if (_classView && _classView->CanPaste()) {
		const QMimeData *data = QApplication::clipboard()->mimeData();
		if (data)
		{
			const QByteArray ba = data->text().toUtf8();
			if (ba.size() > 0) {
				DataBuffer db((const unsigned char*)ba.constData(), ba.size(), 0);
				_classView->Paste(std::move(db));
			}
		}
	}
}

void DXEditorWidget::delite()
{
	if (_classView && _classView->CanDelete()) {
		if (QMessageBox::question(this, "Delete", "Are you sure you want to delete the selection?", QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) != QMessageBox::Yes)
			return;
		_classView->Delete();
	}
}



bool DXEditorWidget::_addImage(Path p)
{
	DataBuffer db;
	if (!LoadDataBuffer(p, db))
		return false;

	HRESULT hr = S_OK;
					
	SID3D11Device device = EditorD3DDevice::GetInstance()->GetDevice();
	if (!device)
		return false;

	SID3D11DeviceContext context;
	device->GetImmediateContext(&context);
	SID3D11Resource resource;
	SID3D11ShaderResourceView srv;
	hr = CreateWICTextureFromMemory(device, context, db.getConstBuffer(), db.getBufferSize(), &resource, &srv);
	if (FAILED(hr))
		return false;
	D3D11_RESOURCE_DIMENSION dim;
	resource->GetType(&dim);
	if (dim != D3D11_RESOURCE_DIMENSION_TEXTURE2D)
		return false;
	SID3D11Texture2D tex = (SID3D11Texture2D&)resource;
	D3D11_TEXTURE2D_DESC desc;
	tex->GetDesc(&desc);

	Vector2 pos = _useMousePositionBeforeMenuExec ? _mousePositionBeforeMenuExec : _classView->GetMousePos();

	Vector2 s(5.0f, 5.0f);
	float ar = float(desc.Width) / desc.Height;
	if (ar < 1.0f)
		s.y /= ar;
	else
		s.x *= ar;
	_classView->AddImage(s, db, pos, srv);
	_classView->GetClass()->SetDirty();

	return true;
}

void DXEditorWidget::UpdateBackgroundItem(CGBackgroundItemID id)
{
	auto n = _classView->GetClass()->GetBackgroundItems().find(id);
	if (n == _classView->GetClass()->GetBackgroundItems().end())
		return;
	switch (n->second.type)
	{
	case CGBackgroundItem::RECT:
		{
			QColor color;
			color.setRgbF(n->second.color.x, n->second.color.y, n->second.color.z, n->second.color.w);
			QColor c = QColorDialog::getColor(color, this, "Color", QColorDialog::ShowAlphaChannel);
			if (c.isValid()) {
				_classView->GetClass()->UpdateRectangle(id, Vector4(c.redF(), c.greenF(), c.blueF(), c.alphaF()));
				_classView->GetClass()->SetDirty();
			}
		}
		break;
	case CGBackgroundItem::IMAGE:
		{

		}
		break;
	case CGBackgroundItem::TEXT:
		{
			QString text = TOQSTRING(n->second.text);
			float s = n->second.textSize * 10;
			QColor c;
			c.setRgbF(n->second.color.x, n->second.color.y, n->second.color.z, n->second.color.w);
			if (TextItemDialog::show(this, text, s, c)) {
				String str = FROMQSTRING(text);
				_classView->GetClass()->UpdateText(id, Vector4(c.redF(), c.greenF(), c.blueF(), c.alphaF()), str, s * 0.1f);
				_classView->GetClass()->SetDirty();
			}
		}
		break;
	}
}

void DXEditorWidget::contextMenuEvent(QContextMenuEvent *event)
{
	if (_mouseMoved && event->reason() == QContextMenuEvent::Reason::Mouse)
		return;
	ShowContextMenu();
}

void DXEditorWidget::onTemplateDragEnter(const Vector4 &extent)
{
	_isDraggingTemplate = true;
	_templateExtent = extent;
}

void DXEditorWidget::onTemplateDragLeave()
{
	_isDraggingTemplate = false;
}

void DXEditorWidget::addTemplate(DocumentLoader &loader, String name)
{
	_isDraggingTemplate = false;
	_classView->InsertTemplate(loader, name);

	setFocus();
}
