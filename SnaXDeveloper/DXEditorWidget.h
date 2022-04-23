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

#include "Common.h"
#include "M3DEngineExt/D3DInclude.h"
#include "M3DEngineExt/ClassView.h"
#include "M3DCore/TriState.h"
#include "EditorD3DDevice.h"
#include "M3DEngineExt/Font.h"
#include "M3DEngineExt/TextWriter.h"
#include "M3DCore/HighPrecisionTimer.h"
#include "ActionManager.h"
#include <qwidget.h>

namespace m3d
{

class DocumentPage;
class DocumentLoader;
class EditorInputManager;
class TextWriter;


class DXEditorWidget : public QWidget, public ClassViewCallback, public EditorD3DDeviceListener, public ActionListener
{
	Q_OBJECT
public:
	DXEditorWidget(QWidget * parent = 0, Qt::WindowFlags f = Qt::WindowFlags());
	virtual ~DXEditorWidget();

	virtual QPaintEngine *paintEngine() const override { return nullptr; }

	virtual Chip *addChip(unsigned chipTypeIndex);
	virtual Chip *addClassChip(ClassID clazzid, Vector2 *pos = nullptr);
	virtual Chip *addFunctionCallChip(ClassID clazzid, ChipID function, bool callByName);

	virtual void onTemplateDragEnter(const Vector4 &extent);
	virtual void onTemplateDragLeave();
	virtual void addTemplate(DocumentLoader &loader, String name);

	class ClassRenderer2 *GetRenderer() { return _clazzr; }
	ClassView *GetClassView() { return _classView; }

	virtual void ShowChipDialog(Chip*, bool showComment = false);
	virtual void ShowContextMenu();
	virtual void FocusOnChip(ClassExt *clazz, Chip *chip);
	virtual void EnterFolder(ClassExt *clazz, FolderID folderID, bool inside);
	virtual void AddBackgroundItem(CGBackgroundItem::Type type);
	virtual void UpdateBackgroundItem(CGBackgroundItemID id);

	virtual bool TakeScreenshot(const Char *filename);

	bool IsSavable() const;

	virtual void OnDestroyDevice() override;

	void SetFPSProps(bool showFPS, bool limitFPS);

protected:
	virtual bool nativeEvent(const QByteArray &eventType, void *message, qintptr *result) override;
	virtual void contextMenuEvent(QContextMenuEvent * event) override;
	virtual void focusInEvent(QFocusEvent *event) override;
	virtual void mouseDoubleClickEvent(QMouseEvent *event) override;
	virtual void mousePressEvent(QMouseEvent *event) override;
	virtual void mouseReleaseEvent(QMouseEvent *event) override;
	virtual void mouseMoveEvent(QMouseEvent *event) override;
	virtual void wheelEvent(QWheelEvent *event) override;
	virtual void keyPressEvent(QKeyEvent *event) override;
	virtual void keyReleaseEvent(QKeyEvent *event) override;

	virtual HRESULT initD3D();
	virtual HRESULT newFrame();
	virtual HRESULT resizeDX();

	HRESULT _getRenderTargetView();
	bool _addImage(Path p);

	ActionMask getActions() override;
	bool isActionEnabled(ActionID id) override;
	void triggered(ActionID id) override;

signals:
	void showChipDialog(Chip*, bool);
	void focusOnChip(ClassExt*,Chip*);
	void enterFolder(ClassExt*,unsigned folderID, bool inside);
	void onFocusIn();
	void showClassDescriptionDialog(ClassExt*);

public:
	void invalidateGraphics();
	void setView(ClassView *cv);
	void SetForceVSync(bool b) { _forceVSync = b; }
	Chip *addChipFromMenu(ChipTypeIndex chipTypeIndex = InvalidChipTypeIndex);
	void addRectangle() { AddBackgroundItem(CGBackgroundItem::RECT); }
	void addImage() { AddBackgroundItem(CGBackgroundItem::IMAGE); }
	void addText() { AddBackgroundItem(CGBackgroundItem::TEXT); }
	void addFolder();
	void cut();
	void copy();
	void paste();
	void delite();


protected:
	ClassRenderer2 *_clazzr;
	ClassView *_classView;

	Vector4 _templateExtent;
	bool _isDraggingTemplate;

	EditorInputManager *_im;
	bool _mouseMoved;

	bool _isD3DInit;
	bool _testNext;
	bool _forceVSync;
	bool _showFPS = false;
	bool _limitFPS = true;

	SID3D11RenderTargetView _rtv;
	SIDXGISwapChain _swapChain;
	D3D11_TEXTURE2D_DESC _bbDesc;

	bool _useMousePositionBeforeMenuExec = false;
	Vector2 _mousePositionBeforeMenuExec;

	// For statistics
	HighPrecisionTimer _hpt;
	int64 _lastRenderTime = 0;
	unsigned _skippedFrames = -1;
	FPS _fps;
	TextWriter _textWriter;
	Map<int, Font> _fonts;

	class ListAction : public QWidgetAction
	{
	public:
		ListAction(QWidget *parent = nullptr);

		bool eventFilter(QObject *watched, QEvent *event) override;
		void fill(ChipTypeIndex typeRequired, ClassExt* currentClass);

		QLineEdit *edit;
		QListWidget *list;
	};
	class AddChipMenu : public QMenu
	{
	public:
		AddChipMenu(QWidget *parent = nullptr);

		void exe(ChipTypeIndex typeRequired, ClassExt* currentClass);

		ListAction *addChipAction;

		unsigned type; // 0=chip, 1=function, 2=class
		ChipTypeIndex chipType; // for chip
		ClassID classId; // for class and function
		ChipID chipId; // for function
	};

	QMenu *_generalMenu;
	QMenu *_chipMenu;
	QMenu* _childConnMenu;
	QMenu *_folderMenu;
	QMenu *_linkMenu;
	QMenu *_backgroundItemMenu;
	AddChipMenu *_addChildMenu;

	QAction *_convertToMultiConnectionAction;
	QAction *_convertToGrowingConnectionAction;

	QAction *_addChipAction;
	QAction *_addRectangleAction;
	QAction *_addImageAction;
	QAction *_addTextAction;
	QAction *_addFolderAction;
	QAction *_chipPropAction;
	QAction *_commentsAction;
	QAction *_setStartAction;
	QAction *_convertInstanceDataAction;
	QAction *_convertFunctionDataAction;
	QAction *_convertFunctionCallAction;
	QAction *_convertParameterAction;
	QAction *_convertProxyAction;
	QAction *_convertStdChipAction;
	QAction *_arrangeChildrenAction;
	QAction *_enterFolderAction;
	QAction *_renameFolderAction;
	QAction *_unpackFolderAction;
	QAction *_editItemAction;
	QAction* _classDescAction;

};

}