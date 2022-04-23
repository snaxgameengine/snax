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
#include <QtWidgets\QMainWindow>
#include <qbasictimer.h>
#include "ui_MainWindow.h"
#include "M3DEngine/ClassManager.h"
#include "M3DEngine/ChipManager.h"
#include "M3DEngine/Engine.h"
#include "ChipDialogs/ChipDialogManager.h"
#include "ClassInstanceDialog.h"
#include "M3DEngine/ClassInstance.h"
#include "M3DEngine/Application.h"
#include "M3DEngine/DocumentManager.h"
#include "M3DEngine/Document.h"
#include "M3DEngineExt/ClassExt.h"
#include "EditorInputManager.h"
#include "ActionManager.h"
#include "ExternalProjectWidget.h"
#include "M3DCore/HighPrecisionTimer.h"
#include "DocumentInfoCache.h"

namespace m3d
{

class EditorWidget;


class MainWindow : public QMainWindow,  public Application, public ClassManagerEventListener, public ChipManagerEventListener, public ClassExtEventListener, public ChipDialogManagerCallback, public DocumentEventListener, public ClassInstanceReleaseCallback, public ClassInstanceDialogCallback, public DocumentManagerEventListener, public ActionListener
{
	Q_OBJECT

public:
	MainWindow(QWidget *parent = 0, Qt::WindowFlags flags = Qt::WindowFlags());
	~MainWindow();

	bool init(QString fileToOpen = "");
	void clear();

protected:
	// Application [MainWindowCallbacks.cpp]
	virtual ExeEnvironment GetExeEnvironment() const override { return ExeEnvironment::EXE_EDITOR; }
	virtual Path GetExeFile() const override { return GetExeFile(); }
	virtual Path GetApplicationFile() const override;
	virtual void Quit() override;
	virtual void MessagedAdded(const ApplicationMessage &msg) override;
	virtual void DestroyDeviceObjects() override;
	virtual int GetDisplayOrientation() override { return 0; } // TODO: Add some functionality to this!
	virtual void ChipMessageAdded(Chip *chip, const ChipMessage &msg) override;
	virtual void ChipMessageRemoved(Chip *chip, const ChipMessage &msg) override;
	virtual InputManager *GetInputManager() override { return &_im; }
	virtual void Break(Chip *chip) override;
	virtual bool IsBreakPointsEnabled() const override { return _breakPointsEnabled; }
	virtual unsigned GetFeatureMask() const override;

	// Document Events [MainWindowCallbacks.cpp]
	virtual void ClassMoved(Class *clazz, Document *oldDoc, Document *newDoc) override;
	virtual void DirtyFlagChanged(Document *doc) override;

	// Document Manager Events [MainWindowCallbacks.cpp]
	virtual bool WaitingForDocumentLoading() override;
	virtual void OnDocumentAboutToOpen(Path filename) override;
	virtual void OnDocumentAdded(Document *doc) override;
	virtual void OnDocumentRemoved(Document *doc) override;
	virtual void OnDocumentFileNameChanged(Document *doc, Path oldFileName) override;
	virtual void OnDocumentSaved(Document *doc) override;
	virtual bool ResolveMissingFile(Path &filename) override;
	virtual bool GetFileName(Path &filename, bool hasFailed) override;
	virtual Path GetCurrentFilePath() override;
	virtual bool ConfirmSaveLibraryDocument(Document* doc) override;
	virtual bool ConfirmMoveLibraryDocument(Document* doc, Path newFilename, bool isSaveAs) override;


	// Class Manager Events [MainWindowCallbacks.cpp]
	virtual void OnClassAdded(Class *clazz) override;
	virtual void OnClassRemoved(Class *clazz) override;
	virtual void OnClassNameChanged(Class *clazz) override;
	virtual void OnStartClassSet(Class *clazzStart) override;
	virtual bool ResolveNameConflict(String &clazzName) override;
	virtual bool ConfirmGuidChange(String clazzName) override;

	// Chip Manager Events [MainWindowCallbacks.cpp]
	virtual void ChipManagerInitiated() override;
	virtual void PacketLoaded(String packetName) override;

	// ClassExt events [MainWindowCallbacks.cpp]
	virtual void DirtyFlagChanged(Class *clazz) override;
	virtual void OnFunctionCreate(Function *function) override;
	virtual void OnFunctionRemove(Function *function) override;
	virtual void OnFunctionChange(Function *function) override;
	virtual void OnFolderRemoved(ClassExt *clazz, FolderID folder) override;
	virtual void OnInstanceRegistered(ClassInstance *instance) override;
	virtual void OnInstanceUnregistered(ClassInstance *instance) override;
	virtual void OnInstanceNameChanged(ClassInstance *instance) override;
	virtual void OnBaseClassAdded(Class *derived, Class *base) override;
	virtual void OnBaseClassRemoved(Class *derived, Class *base) override;
	virtual void OnDescriptionChanged(ClassExt* clazz) override;

	// Chip Dialog Manager Callbacks [MainWindowCallbacks.cpp]
	virtual void SetDirty(Class *clazz) override;
	virtual bool HasComments(Chip *ch) override;
	virtual String GetComment(Chip *ch) override;
	virtual void SetComment(Chip *ch, String comment) override;
	virtual void DisableFrameTimer(bool disable) override;
	virtual void OpenInstanceDialog(ClassInstance *instance) override;
	virtual Path GetCurrentPath() const override { return Path(FROMQSTRING(_currentDir)); }
	virtual void SetCurrentPath(Path p) override { _currentDir = TOQSTRING(p.GetDirectory().AsString()); }
	virtual QWidget *GetParentWidget() override { return this; }
	virtual ChipDialogContainer *GetEmbeddedContainer(unsigned id) override;

	void timerEvent(QTimerEvent *) override;
	QBasicTimer _tim;

	// ClassInstance Callback
	virtual void OnRelease(ClassInstance *instance) override;

	// ClassInstanceDialog Callbacks
	virtual void CloseDialog(ClassInstanceDialog *dialog) override;
	virtual void ShowChipDialog(Chip *chip) override;
//	virtual void SetDirty(Class *clazz) override; [duplicate]

	// Overrided from QMainWindow
	virtual bool nativeEvent(const QByteArray &eventType, void *message, qintptr *result) override;
	virtual void closeEvent(QCloseEvent *event) override;
	virtual QMenu *createPopupMenu() override { return nullptr; } // No right-click-on-toolbar-menu!

	// Utils
	void _openProject(QString filename);
	void _recentlyOpened(QString filename);
	bool _createInitClass(QString filename, QString clazz);
	bool _queryCGName(QString &s, QString title);
	bool _confirmCloseProject(bool isQuitting);
	void _updateSettings(bool load);
	void _showStartupDialog();

	void _doRecoverySave(long recoverySaveInterval);
	void _deleteRecoveryFile(Document *doc);
	void _lookForRecoveryFile(Path filename);
	void _importDocument(Path filename, bool focus);

	ActionMask getActions() override;
	bool isActionEnabled(ActionID id) override;
	void triggered(ActionID id) override;

private:
	Ui::MainWindow ui;

	struct {
		QAction *recentProjects;
		QAction *enableBreakpoints;
		QAction *breakOnError;
		QAction *render_fps;
		QAction* render_World_Grid;
		QAction *render_World_Space_AABB;
		QAction *render_Local_Space_AABB;
		QAction *profilingDisable;
		QAction *profilingPrFrame;
		QAction *profilingAccumulate;
		QAction *profilingReset;
		QAction *options;
		QAction *limit_Frame_Rate;
		QAction *help;
		QAction *about;
		QAction *exit;
	} _actions;

	QActionGroup *_profilingActionGroup;
	QActionGroup *_runningStateActionGroup;


	QTimer *_frameTimer;
	HighPrecisionTimer _hpt;
	double _accum;
	bool _limitFPS;

	bool _breakPointsEnabled;
	bool _breakOnErrors;
	bool _exitRequested;

	unsigned _dirtyCounter;

	ChipDialogManager *_dialogManager;

	DocumentInfoCache _templatesCache;
	DocumentInfoCache _librariesCache;

	List<Guid> _importers;

	long _recoverySaveInterval;

	EditorInputManager _im;

	struct ClassInstanceDialogDesc
	{
		ClassInstanceDialog *dialog;
		int x,y,w,h;
	};

	Map<ClassInstance*, ClassInstanceDialogDesc> _instanceDialogs;

	ClassPtrSet _classesToRestore;
	bool _restoringClasss;

	//	ClassExt *_clazzg;

	QMenu *_recentProjectsMenu;

	QString _currentDir;

	ExternalProjectWidget *_externalProjectView;
	QByteArray _externalViewState;
	bool _externalViewOpenAtStartup;

	QMap<ClassExt*, class ClassDescriptionDialog*> _classDialogs;

	// Missing files that we asked the user to find.
	// If it was not found, add it to this set so we don't ask more than once.
	Set<Path> _unresolvedFiles;

private:
	void onNewFrame();
	void onApplicationExit();
	void onClipboardDataChanged();
	void onNewProject();
	void onOpenProject();
	void addClass();
	void importClass();
	void importClass2(const Path &file);
	void onSaveDirty();
	void onSaveAll();
	void takeScreenshot();
	void onOpenCGG();
	void onShowRecentProjectsMenu();
	void onOpenRecentProject(QAction*);
	void onProfilingChanged(QAction*);
	void onResetProfiling();
	void onLimitFPSChanged();
	void onPublish();
	void showChipDialog(Chip*, bool=false, unsigned=0);
	void showClassDescriptionDialog(ClassExt*);
	void showInstanceDialog(ClassInstance*);
	void onLoadAllProjectFiles();
	void updateDebugGeometryMenu();
	void updateDebugGeometryFromMenu();
	void onShowRecoveryDlg(QString msg, int *result);
	void onContinue();
	void onPause();
	void onEnableBreakPoints(bool);
	void onEnableBreakOnErrors(bool);
	void onOptions();
	void showHelp();
	void showAboutDialog();
	void externalViewToggled();
	void externalFulscreenToggled();

	void renameClass(ClassExt*);
	void renameDoc(Document*);
	void saveDocument(Document *doc);
	void saveDocumentAs(Document *doc);
	void deleteDocument(Document*);
	void deleteClass(ClassExt*);

	void onResolveMissingFile(Path& filename, bool &bSuccess);

signals:
	void showRecoveryDlg(QString msg, int *result);
	void resolveMissingFile(Path& filename, bool &bSuccess);
};
}