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
#include "MainWindowUtil.h"
#include "ChipDialogs/ChipDialog.h"
#include "M3DEngine/Chip.h"
#include "M3DEngineExt/ClassDiagram.h"
#include "M3DEngine/Class.h"
#include "M3DEngine/Function.h"
#include "StdChips/Importer.h"
#include "StdChips/Caller.h"
#include <QStandardItemModel.h>
#include <QCloseEvent>
#include <QTimer>
#include <qfiledialog.h>
#include <QInputDialog>
#include <QmessageBox.h>
#include <qclipboard>
#include <qsettings>
#include "EditorD3DDevice.h"
#include "GraphicsChips/Graphics.h"
#include "M3DEngineExt/EditorState.h"
#include <qstandardpaths.h>
#include "EditorWidget.h"
#include "EditorCollection.h"
#include "DXEditorWidget.h"
#include "Paths.h"
#include "NewProjectDialog.h"
#include "M3DEngine/DocumentLoader.h"
#include "M3DEngineExt/DocumentExtLoader.h"
#include "AppSignals.h"
#include "SaveFilesDialog.h"
#include "App.h"


#define CHIPS_PATH MTEXT(".\\Chips\\")
#define THIRD_PATH MTEXT(".\\3rd\\")
#define DIALOGS_PATH MTEXT(".\\Dialogs\\")

#define LIBRARIES_SETTINGS_KEY "settings/libraries"
#define LIBRARIES_CACHE_SETTINGS_KEY "settings/librariesCache"
#define LIBRARIES_DEFAULT_CACHE_FILE "$applocaldata\\libraries.cache"
#define TEMPLATES_SETTINGS_KEY "settings/templates"
#define TEMPLATES_CACHE_SETTINGS_KEY "settings/templateCache"
#define TEMPLATES_DEFAULT_CACHE_FILE "$applocaldata\\templates.cache"

using namespace m3d;


void LimitThreadAffinityToCurrentProc()
{
    HANDLE hCurrentProcess = GetCurrentProcess();

    // Get the processor affinity mask for this process
    DWORD_PTR dwProcessAffinityMask = 0;
    DWORD_PTR dwSystemAffinityMask = 0;

    if( GetProcessAffinityMask( hCurrentProcess, &dwProcessAffinityMask, &dwSystemAffinityMask ) != 0 &&
        dwProcessAffinityMask )
    {
        // Find the lowest processor that our process is allows to run against
        DWORD_PTR dwAffinityMask = ( dwProcessAffinityMask & ( ( ~dwProcessAffinityMask ) + 1 ) );

        // Set this as the processor that our thread must always run against
        // This must be a subset of the process affinity mask
        HANDLE hCurrentThread = GetCurrentThread();
        if( INVALID_HANDLE_VALUE != hCurrentThread )
        {
            SetThreadAffinityMask( hCurrentThread, dwAffinityMask );
            CloseHandle( hCurrentThread );
        }
    }

    CloseHandle( hCurrentProcess );
}





MainWindow::MainWindow(QWidget* parent, Qt::WindowFlags flags)
	: QMainWindow(parent, flags),
	_templatesCache(TEMPLATES_SETTINGS_KEY, TEMPLATES_PATH, TEMPLATES_CACHE_SETTINGS_KEY, TEMPLATES_DEFAULT_CACHE_FILE, false),
	_librariesCache(LIBRARIES_SETTINGS_KEY, LIBRARIES_PATH, LIBRARIES_CACHE_SETTINGS_KEY, LIBRARIES_DEFAULT_CACHE_FILE, true)
{
	// We need these for passing the types as signal/slot arguments from multiple threads!
	qRegisterMetaType<m3d::ApplicationMessage>();
	qRegisterMetaType<m3d::ChipMessage>();

	ui.setupUi(this);

	_actions.recentProjects = new QAction("&Recent Projects", this);
	_actions.enableBreakpoints = new QAction(QIcon(":/EditorApp/Resources/disk.png"), "Enable Break Points", this);
	_actions.enableBreakpoints->setCheckable(true);
	_actions.breakOnError = new QAction(QIcon(":/EditorApp/Resources/close.png"), "Break on Errors", this);
	_actions.breakOnError->setCheckable(true);
	_actions.render_fps = new QAction("Show Frame rate (FPS)", this);
	_actions.render_fps->setCheckable(true);
	_actions.render_World_Grid = new QAction("World Grid", this);
	_actions.render_World_Grid->setCheckable(true);
	_actions.render_World_Space_AABB = new QAction("World Space AABB", this);
	_actions.render_World_Space_AABB->setCheckable(true);
	_actions.render_Local_Space_AABB = new QAction("Local Space AABB", this);
	_actions.render_Local_Space_AABB->setCheckable(true);
	_actions.profilingDisable = new QAction("Disable", this);
	_actions.profilingDisable->setCheckable(true);
	_actions.profilingPrFrame = new QAction("Average", this);
	_actions.profilingPrFrame->setCheckable(true);
	_actions.profilingAccumulate = new QAction("Floating", this);
	_actions.profilingAccumulate->setCheckable(true);
	_actions.profilingReset = new QAction("Reset", this);
	_actions.options = new QAction(QIcon(":/EditorApp/Resources/gears.png"), "Settings...", this);
	_actions.limit_Frame_Rate = new QAction("Limit Frame Rate to V-Sync", this);
	_actions.limit_Frame_Rate->setCheckable(true);
	_actions.help = new QAction(QIcon(":/EditorApp/Resources/help.png"), "View Help", this);
	_actions.about = new QAction(QIcon(":/EditorApp/Resources/about.png"), "About", this);
	_actions.exit = new QAction("E&xit", this);

	connect(_actions.breakOnError, &QAction::triggered, this, &MainWindow::onEnableBreakOnErrors);
	connect(_actions.enableBreakpoints, &QAction::triggered, this, &MainWindow::onEnableBreakPoints);
	connect(_actions.render_fps, &QAction::triggered, this, [](bool b) {
		Graphics* g = engine->GetGraphics();
		if (g)
			g->SetRenderFPS(b);
		}); 
	connect(_actions.render_World_Grid, &QAction::triggered, this, &MainWindow::updateDebugGeometryFromMenu);
	connect(_actions.render_World_Space_AABB, &QAction::triggered, this, &MainWindow::updateDebugGeometryFromMenu);
	connect(_actions.render_Local_Space_AABB, &QAction::triggered, this, &MainWindow::updateDebugGeometryFromMenu);
	connect(_actions.profilingReset, &QAction::triggered, this, &MainWindow::onResetProfiling);
	connect(_actions.options, &QAction::triggered, this, &MainWindow::onOptions);
	connect(_actions.limit_Frame_Rate, &QAction::triggered, this, &MainWindow::onLimitFPSChanged);
	connect(_actions.help, &QAction::triggered, this, &MainWindow::showHelp);
	connect(_actions.about, &QAction::triggered, this, &MainWindow::showAboutDialog);
	connect(_actions.exit, &QAction::triggered, this, &MainWindow::close);

	ui.mainToolBar->addAction(ActionManager::instance().getAction(ActionID::NEW_PROJECT));
	ui.mainToolBar->addAction( ActionManager::instance().getAction(ActionID::OPEN_PROJECT));
	ui.mainToolBar->addSeparator();
	ui.mainToolBar->addAction(ActionManager::instance().getAction(ActionID::ADD_CLASS));
	ui.mainToolBar->addAction(ActionManager::instance().getAction(ActionID::IMPORT_CLASS));
	ui.mainToolBar->addSeparator();
	ui.mainToolBar->addAction(ActionManager::instance().getAction(ActionID::SAVE));
	ui.mainToolBar->addAction(ActionManager::instance().getAction(ActionID::SAVE_DIRTY));
	ui.mainToolBar->addAction(ActionManager::instance().getAction(ActionID::SAVE_ALL));
	ui.mainToolBar->addSeparator();
	ui.mainToolBar->addAction(ActionManager::instance().getAction(ActionID::LOAD_ALL));
	ui.mainToolBar->addSeparator();
	ui.mainToolBar->addAction(ActionManager::instance().getAction(ActionID::CUT));
	ui.mainToolBar->addAction(ActionManager::instance().getAction(ActionID::COPY));
	ui.mainToolBar->addAction(ActionManager::instance().getAction(ActionID::PASTE));
	ui.mainToolBar->addAction(ActionManager::instance().getAction(ActionID::DELITE));
	ui.mainToolBar->addSeparator();
	ui.mainToolBar->addAction(ActionManager::instance().getAction(ActionID::KONTINUE));
	ui.mainToolBar->addAction(ActionManager::instance().getAction(ActionID::PAUSE));
	ui.mainToolBar->addSeparator();
	ui.mainToolBar->addAction(_actions.enableBreakpoints);
	ui.mainToolBar->addAction(_actions.breakOnError);
	ui.mainToolBar->addSeparator();
	ui.mainToolBar->addAction(ActionManager::instance().getAction(ActionID::SCREENSHOT));
	ui.mainToolBar->addSeparator();
	ui.mainToolBar->addAction(ActionManager::instance().getAction(ActionID::CLASS_DIAGRAM));
	ui.mainToolBar->addSeparator();
	ui.mainToolBar->addAction(ActionManager::instance().getAction(ActionID::EXTERNALVIEW));
	ui.mainToolBar->addAction(ActionManager::instance().getAction(ActionID::EXTERNALVIEWFULLSCREEN));
	
	ui.menuFile->addAction(ActionManager::instance().getAction(ActionID::NEW_PROJECT));
	ui.menuFile->addAction(ActionManager::instance().getAction(ActionID::OPEN_PROJECT));
	ui.menuFile->addSeparator();
	ui.menuFile->addAction(ActionManager::instance().getAction(ActionID::ADD_CLASS));
	ui.menuFile->addAction(ActionManager::instance().getAction(ActionID::IMPORT_CLASS));
	ui.menuFile->addSeparator();
	ui.menuFile->addAction(ActionManager::instance().getAction(ActionID::SAVE));
	ui.menuFile->addAction(ActionManager::instance().getAction(ActionID::SAVE_AS));
	ui.menuFile->addAction(ActionManager::instance().getAction(ActionID::SAVE_DIRTY));
	ui.menuFile->addAction(ActionManager::instance().getAction(ActionID::SAVE_ALL));
	ui.menuFile->addSeparator();
	ui.menuFile->addAction(ActionManager::instance().getAction(ActionID::LOAD_ALL));
	ui.menuFile->addSeparator();
	ui.menuFile->addAction(ActionManager::instance().getAction(ActionID::PUBLISH));
	ui.menuFile->addSeparator();
	ui.menuFile->addAction(_actions.recentProjects);
	ui.menuFile->addSeparator();
	ui.menuFile->addAction(_actions.exit);

	ui.menuEdit->addAction(ActionManager::instance().getAction(ActionID::CUT));
	ui.menuEdit->addAction(ActionManager::instance().getAction(ActionID::COPY));
	ui.menuEdit->addAction(ActionManager::instance().getAction(ActionID::PASTE));
	ui.menuEdit->addAction(ActionManager::instance().getAction(ActionID::DELITE));
	ui.menuEdit->addSeparator();
	ui.menuEdit->addAction(_actions.options);

	ui.menuView->addAction(ActionManager::instance().getAction(ActionID::CLASS_DIAGRAM));
	ui.menuView->addSeparator();
	QAction *debugGeometryAction = ui.menuView->addAction(QIcon(":/EditorApp/Resources/geometry.png"), "Debug Geometry");
	QMenu *debugGeometryMenu = new QMenu();
	debugGeometryAction->setMenu(debugGeometryMenu);
	connect(debugGeometryMenu, &QMenu::aboutToShow, this, &MainWindow::updateDebugGeometryMenu);
	debugGeometryMenu->addAction(_actions.render_World_Grid);
	debugGeometryMenu->addAction(_actions.render_World_Space_AABB);
	debugGeometryMenu->addAction(_actions.render_Local_Space_AABB);
	QAction *profilingAction = ui.menuView->addAction(QIcon(":/EditorApp/Resources/watch.png"), "Profiling");
	QMenu *profilingMenu = new QMenu();
	profilingAction->setMenu(profilingMenu);
	profilingMenu->addAction(_actions.profilingDisable);
	profilingMenu->addAction(_actions.profilingPrFrame);
	profilingMenu->addAction(_actions.profilingAccumulate);
	profilingMenu->addSeparator();
	profilingMenu->addAction(_actions.profilingReset);
	ui.menuView->addSeparator();
	QAction *externalAction = ui.menuView->addAction(QIcon(":/EditorApp/Resources/windows-couple.png"), "External Project View");
	QMenu *externalMenu = new QMenu();
	externalMenu->addAction(ActionManager::instance().getAction(ActionID::EXTERNALVIEW));
	externalMenu->addAction(ActionManager::instance().getAction(ActionID::EXTERNALVIEWFULLSCREEN));
	externalAction->setMenu(externalMenu);
	ui.menuView->addSeparator();
	ui.menuView->addAction(ActionManager::instance().getAction(ActionID::KONTINUE));
	ui.menuView->addAction(ActionManager::instance().getAction(ActionID::PAUSE));
	ui.menuView->addSeparator();
	ui.menuView->addAction(_actions.enableBreakpoints);
	ui.menuView->addAction(_actions.breakOnError);
	ui.menuView->addSeparator();
	ui.menuView->addAction(ActionManager::instance().getAction(ActionID::SCREENSHOT));
	ui.menuView->addAction(_actions.render_fps);
	ui.menuView->addAction(_actions.limit_Frame_Rate);

	ui.menuHelp->addAction(_actions.help);
	ui.menuHelp->addSeparator();
	ui.menuHelp->addAction(_actions.about);


	// Connect custom signals
	{
		// Signals from widgets to us
		connect(ui.mainTabWidget, &QTabWidget::currentChanged, this, [this]() { this->ui.projectWidget1->setFocus(); });
		connect(ui.tabWidget_2, &QTabWidget::currentChanged, this, [this]() { this->ui.projectWidget2->setFocus(); });

		connect(&AppSignals::instance(), &AppSignals::addClass, this, &MainWindow::addClass); // Adds a class to the project and opens it in current editor.
		connect(&AppSignals::instance(), &AppSignals::importClass, this, &MainWindow::importClass);
		connect(&AppSignals::instance(), &AppSignals::importClass2, this, &MainWindow::importClass2);
		connect(&AppSignals::instance(), &AppSignals::saveDocument, this, &MainWindow::saveDocument);
		connect(&AppSignals::instance(), &AppSignals::saveDocumentAs, this, &MainWindow::saveDocumentAs);
		connect(&AppSignals::instance(), &AppSignals::deleteDocument, this, &MainWindow::deleteDocument); // Delete document. Signals are emitted.
		connect(&AppSignals::instance(), &AppSignals::deleteClass, this, &MainWindow::deleteClass); // Delete class. Signals are emitted.
		connect(&AppSignals::instance(), &AppSignals::openChipDialog, this, &MainWindow::showChipDialog); // Ask us to open a given chip dialog.
		connect(&AppSignals::instance(), &AppSignals::openInstanceDialog, this, &MainWindow::showInstanceDialog); // Ask us to open a given instance dialog.
		connect(&AppSignals::instance(), &AppSignals::openClassDescriptionDialog, this, &MainWindow::showClassDescriptionDialog); // Ask us to open a given instance dialog.
	}

	Engine::Create();

	setWindowTitle(QCoreApplication::applicationName());

	engine->GetDocumentManager()->SetEventListener(this);
	engine->GetClassManager()->SetEventListener(this);
	engine->GetChipManager()->SetEventListener(this);

	_dialogManager = new ChipDialogManager(this);

	_frameTimer = new QTimer(this);
   connect(_frameTimer, &QTimer::timeout, this, &MainWindow::onNewFrame);
	_frameTimer->start(0);
	//connect(QAbstractEventDispatcher::instance(), SIGNAL(aboutToBlock()), this, SLOT(OnIdle()));
	_accum = 0;
//	_hpt.Start();
	_limitFPS = true;
	_actions.limit_Frame_Rate->setChecked(_limitFPS);
	_actions.render_fps->setChecked(false);

	_breakPointsEnabled = true;
	_breakOnErrors = false;
	_exitRequested = false;

	// Hackish...
	_runningStateActionGroup = new QActionGroup(this);
	_runningStateActionGroup->addAction(ActionManager::instance().getAction(ActionID::KONTINUE));
	_runningStateActionGroup->addAction(ActionManager::instance().getAction(ActionID::PAUSE));
	ActionManager::instance().getAction(ActionID::KONTINUE)->setChecked(true);
	ActionManager::instance().getAction(ActionID::KONTINUE)->setShortcutContext(Qt::ShortcutContext::ApplicationShortcut);
	ActionManager::instance().getAction(ActionID::PAUSE)->setShortcutContext(Qt::ShortcutContext::ApplicationShortcut);
	ActionManager::instance().getAction(ActionID::SCREENSHOT)->setShortcutContext(Qt::ShortcutContext::ApplicationShortcut);
	ActionManager::instance().getAction(ActionID::EXTERNALVIEW)->setShortcutContext(Qt::ShortcutContext::ApplicationShortcut);
	ActionManager::instance().getAction(ActionID::EXTERNALVIEWFULLSCREEN)->setShortcutContext(Qt::ShortcutContext::ApplicationShortcut);

	ui.mainWidget->init();

	connect(QApplication::instance(), &QApplication::aboutToQuit, this, &MainWindow::onApplicationExit);
	connect(QApplication::clipboard(), &QClipboard::dataChanged, this, &MainWindow::onClipboardDataChanged);

	connect(this, &MainWindow::showRecoveryDlg, this, &MainWindow::onShowRecoveryDlg, Qt::BlockingQueuedConnection);
	connect(this, &MainWindow::resolveMissingFile, this, &MainWindow::onResolveMissingFile, Qt::BlockingQueuedConnection);

	_actions.enableBreakpoints->setChecked(_breakPointsEnabled);
	_actions.breakOnError->setChecked(_breakOnErrors);

//	_clazzg = new ClassExt(NullGUID);

	_recentProjectsMenu = new QMenu(this);
	connect(_recentProjectsMenu, &QMenu::aboutToShow,this, &MainWindow::onShowRecentProjectsMenu);
	connect(_recentProjectsMenu, &QMenu::triggered, this,  &MainWindow::onOpenRecentProject);
	_actions.recentProjects->setMenu(_recentProjectsMenu);

	//_tim.start(0, Qt::CoarseTimer, this);
	
	_dirtyCounter = 0;

	//ui.actionNew_Project->setParent(ui.dxEditorWidget);

	_currentDir = QDir::homePath();

	_profilingActionGroup = new QActionGroup(this);
	_profilingActionGroup->addAction(_actions.profilingDisable);
	_profilingActionGroup->addAction(_actions.profilingPrFrame);
	_profilingActionGroup->addAction(_actions.profilingAccumulate);
	_actions.profilingDisable->setChecked(true);
	connect(_profilingActionGroup, &QActionGroup::triggered, this, &MainWindow::onProfilingChanged);

	//LimitThreadAffinityToCurrentProc();

	_recoverySaveInterval = 5*60; // 5 min

	_restoringClasss = false;
	_externalProjectView = nullptr;
	_externalViewOpenAtStartup = false;

	ui.mainToolBar->setIconSize(QSize(16, 16));

	msg(INFO, MTEXT("Starting Editor..."));
}

MainWindow::~MainWindow()
{
	msg(DINFO, MTEXT("MainWindow::~MainWindow() called."));

//	emit destroyDeviceObjects();

//	engine->GetClassManager()->Clear();

	engine->GetClassManager()->SetEventListener(nullptr);
	engine->GetChipManager()->SetEventListener(nullptr);

	SAFE_DELETE(_dialogManager);

	if (ClassDiagram::Singleton()) {
		OnClassRemoved(ClassDiagram::Singleton()); // Do this to close any view of this...
		ClassDiagram::DestroySingleton();
	}

//	_frameTimer->stop();
//	engine->Clear();
}

ActionMask MainWindow::getActions()
{
	return ActionMask(ActionID::NEW_PROJECT) | ActionMask(ActionID::OPEN_PROJECT) | ActionMask(ActionID::ADD_CLASS) | ActionMask(ActionID::IMPORT_CLASS) | 
		ActionMask(ActionID::SAVE_ALL) | ActionMask(ActionID::SAVE_DIRTY) | 
		ActionMask(ActionID::LOAD_ALL) | ActionMask(ActionID::PUBLISH) | ActionMask(ActionID::CLASS_DIAGRAM) | ActionMask(ActionID::KONTINUE) | ActionMask(ActionID::SCREENSHOT)| 
		ActionMask(ActionID::PAUSE) | ActionMask(ActionID::EXTERNALVIEW) | ActionMask(ActionID::EXTERNALVIEWFULLSCREEN);
}

bool MainWindow::isActionEnabled(ActionID id) 
{
	switch (id)
	{
	case ActionID::NEW_PROJECT:
	case ActionID::OPEN_PROJECT:
	case ActionID::ADD_CLASS:
	case ActionID::IMPORT_CLASS:
	case ActionID::LOAD_ALL:
	case ActionID::PUBLISH:
	case ActionID::EXTERNALVIEW:
		return EditorState::state != EditorState::BREAKPOINT && EditorState::state != EditorState::LOADING;
	case ActionID::SAVE_DIRTY:
		return _dirtyCounter != 0;
	case ActionID::SAVE_ALL:
		return false;
	case ActionID::CLASS_DIAGRAM:
		return true;
	case ActionID::KONTINUE:
		return true;
	case ActionID::SCREENSHOT:
		return dynamic_cast<DXEditorWidget*>(QApplication::focusWidget()) != nullptr || dynamic_cast<ProjectViewWidget*>(QApplication::focusWidget()) != nullptr;
	case ActionID::PAUSE:
		return true;
	case ActionID::EXTERNALVIEWFULLSCREEN:
		return _externalProjectView != nullptr;
	}
	return false;
}

void MainWindow::triggered(ActionID id) 
{
	switch (id) 
	{
	case ActionID::NEW_PROJECT:
		return onNewProject();
	case ActionID::OPEN_PROJECT:
		return onOpenProject();
	case ActionID::ADD_CLASS:
		return addClass();
	case ActionID::IMPORT_CLASS:
		return importClass();
	case ActionID::SAVE_DIRTY:
		return onSaveDirty();
	case ActionID::SAVE_ALL:
		return onSaveAll();
	case ActionID::LOAD_ALL:
		return onLoadAllProjectFiles();
	case ActionID::PUBLISH:
		return onPublish();
	case ActionID::CLASS_DIAGRAM:
		return onOpenCGG();
	case ActionID::KONTINUE:
		return onContinue();
	case ActionID::SCREENSHOT:
		return takeScreenshot();
	case ActionID::PAUSE:
		return onPause();
	case ActionID::EXTERNALVIEW:
		return externalViewToggled();
	case ActionID::EXTERNALVIEWFULLSCREEN:
		return externalFulscreenToggled();
	}
}


void MainWindow::timerEvent(QTimerEvent *)
{
	onNewFrame();
	
}

bool MainWindow::init(QString fileToOpen)
{
	static ClassExtFactory cgExtFactory;
	engine->GetClassManager()->SetClassFactory(&cgExtFactory);

	// Write log.txt to AppData-folder!
	engine->SetMessageFile(Path(Path(MTEXT("log.txt")), Path::Dir(FROMQSTRING(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)))));

#if PLATFORM == PLATFORM_WINDESKTOP_X86 
	String platform(MTEXT("x86"));
#elif PLATFORM == PLATFORM_WINDESKTOP_X64
	String platform(MTEXT("x64"));
#else
	String platform(MTEXT("Unknown"));
#endif

	msg(INFO, MTEXT("Starting Snax Developer ") + FROMQSTRING(QApplication::applicationVersion()) + MTEXT(" (") + platform + MTEXT(")..."));

	bool _useDebugDevice = true;
	bool _useD3D9LevelForEditorDevice = false;

	if (FAILED(EditorD3DDevice::GetInstance()->CreateDevice(_useDebugDevice, _useD3D9LevelForEditorDevice))) {
		msg(FATAL, MTEXT("Failed to create D3D Device for SnaX Developer graphics. Quitting!"));
		return false;
	}

	Path applicationDir = Path::Dir(FROMQSTRING(QApplication::applicationDirPath()));

	List<Path> libPaths;
	QSettings s = ((const App*)QApplication::instance())->GetSettings();
	QString libraries = s.value(LIBRARIES_SETTINGS_KEY, LIBRARIES_PATH).toString();
	Paths::SplitAndResolve(libraries, libPaths);

	if (!engine->Init(this, Path::Dir(Path(CHIPS_PATH), applicationDir), Path::Dir(Path(THIRD_PATH), applicationDir), libPaths)) {
		msg(FATAL, MTEXT("Failed to initialize the engine. Quitting!"));
		return false;
	}

	if (!engine->GetGraphics()->Init()) {
		msg(FATAL, MTEXT("Failed to initialize the graphics engine. Quitting!"));
		return false;
	}

	engine->GetGraphics()->SetRequestDebugDevice(_useDebugDevice); // TODO: Add option to enable/disable this request!

	//	engine->SetThirdDepsDirectory((Path(MTEXT(".\\3rd\\"))));

	// These MUST be included for dynamic libraries (eg chips) to find their deps!
//	SetDllDirectory(MTEXT(".\\chips\\"));
//	SetDllDirectory(MTEXT(".\\3rd\\"));

//	if (!engine->GetChipManager()->FindChips(Path(MTEXT(".\\Chips\\"))))
//		return false;

	if (!_dialogManager->FindDialogs(Path(Path(DIALOGS_PATH), applicationDir))) {
		msg(FATAL, MTEXT("Failed to initialize chip dialogs."));
		return false;
	}

	ClassDiagram::CreateSingleton();

//	engine->GetInputManager()->Init((HWND)ui.projectWidget1->winId());//winId());

	emit AppSignals::instance().forceVSync(_actions.limit_Frame_Rate->isChecked());

	_updateSettings(true);

	AppSignals::instance().initTemplates(&_templatesCache);
	AppSignals::instance().initLibraries(&_librariesCache);

	_importers = engine->GetChipManager()->GetDerivedChips(IMPORTER_GUID);

	msg(INFO, MTEXT("Snax Developer ") + FROMQSTRING(QApplication::applicationVersion()) + MTEXT(" (") + platform + MTEXT(") started!"));

	if (_externalViewOpenAtStartup) {
		EditorState::state = EditorState::PAUSE;
		ActionManager::instance().getAction(ActionID::PAUSE)->setChecked(true);
		ActionManager::instance().getAction(ActionID::EXTERNALVIEW)->setChecked(true);
		externalViewToggled();
	}

	if (fileToOpen.isEmpty())
		_showStartupDialog();
	else
		_openProject(fileToOpen);

	setFocus();

	return true;
}

void MainWindow::clear()
{
	_frameTimer->stop();
	engine->Clear();

	EditorD3DDevice::GetInstance()->DestroyDevice();
}

void MainWindow::_openProject(QString filename)
{
	if (!QFile::exists(filename)) {
		QMessageBox::warning(this, "Open Project", "File not found: " + filename + ".");
		return;
	}

	engine->Reset();

	_unresolvedFiles.clear();

	Document* doc = nullptr;
	{
		ScopedOverrideCursor oc(Qt::WaitCursor);
		doc = engine->GetDocumentManager()->GetDocument(Path(FROMQSTRING(filename)));
	}
	if (doc) {
		_recentlyOpened(filename);
		Class *cg = doc->GetStartClass();
		if (cg) { // Document points out a start class?
			engine->GetClassManager()->SetStartClass(cg);
			AppSignals::instance().openClass((ClassExt*)cg);
		}
		else {
			if (doc->GetClasss().size())
				AppSignals::instance().openClass((ClassExt*)doc->GetClasss().front()); // Just open the first class in the editor!
		}
	}
	else
		QMessageBox::warning(this, "Open Project", "Failed to open the project \'" + filename + "\'");
}

void MainWindow::_recentlyOpened(QString filename)
{
	QSettings s = ((const App*)QApplication::instance())->GetSettings();
	QStringList recentProjects = s.value("settings/recentProjects", QStringList()).toStringList();

	filename.replace('/', '\\');

	int i = -1;
	for (QList<QString>::iterator itr = recentProjects.begin(); itr != recentProjects.end(); itr++) {
		if (filename.compare(*itr, Qt::CaseInsensitive) == 0) {
			i = itr - recentProjects.begin();
			break;
		}
	}

	if (i != -1)
		recentProjects.move(i, 0);
	else 
		recentProjects.push_front(filename);
	while (recentProjects.size() > 10)
		recentProjects.pop_back();

	s.setValue("settings/recentProjects", recentProjects);
}

void MainWindow::_importDocument(Path filename, bool focus)
{
	Document* doc = nullptr;
	{
		ScopedOverrideCursor oc(Qt::WaitCursor);
		doc = engine->GetDocumentManager()->GetDocument(filename, true, true);
	}
	if (doc) {
		if (focus)
			if (doc->GetClasss().size())
				AppSignals::instance().openClass((ClassExt*)doc->GetClasss().front());
	}
	else
		QMessageBox::warning(this, "Import Failed", "Import failed!");
}

void MainWindow::_showStartupDialog()
{
	NewProjectDialog dlg(this);
	dlg.init(_templatesCache);
	if (dlg.exec() != QDialog::Accepted)
		return;
	QString selectedClass = dlg.selectedClass();
	QString selectedFile = dlg.selectedFile();
	if (!selectedClass.isEmpty() || selectedClass.isEmpty() && selectedFile.isEmpty()) { // Create from template file...
		if (!_createInitClass(selectedFile, selectedClass)) {
			msg(FATAL, MTEXT("Failed to initialize class from template."));
		}
	}
	else if (!selectedFile.isEmpty())
	{
		_openProject(selectedFile);
	}
}

bool MainWindow::_createInitClass(QString filename, QString clazz)
{
	Document* doc = engine->GetDocumentManager()->CreateDocument();
	if (!doc)
		return false;
	ClassExt* cg = (ClassExt*)engine->GetClassManager()->CreateClass(MTEXT("Default"), doc);
	if (!cg)
		return false;
	
	if (filename.isEmpty()) {
		if (!clazz.isEmpty()) {
			msg(WARN, MTEXT("Failed to build startup class"));
		}
		Chip* ch = cg->AddChip(engine->GetChipManager()->GetChipTypeIndex(CALLER_GUID));
		ch->SetName(MTEXT("Project Start"));
		cg->SetStartChip(ch);
	}
	else {
		const DocumentFileTypes::FileTypeDesc* ft = DocumentFileTypes::GetFileType(FROMQSTRING(filename));
		if (!ft) {
			msg(WARN, MTEXT("Could not find template loader."));
			return false;
		}

		DocumentLoader* loader = (*ft->loader)();
		if (!loader) {
			msg(WARN, MTEXT("Failed to create template loader."));
			return false;
		}
		if (!loader->OpenFile(FROMQSTRING(filename))) {
			msg(WARN, MTEXT("Failed to open template document \'") + FROMQSTRING(filename) + MTEXT("\'."));
			DocumentFileTypes::Free(loader);
			loader = nullptr;
			return false;
		}

		loader->SetAllowIrregularLoading(true);

		if (!(loader->GetDocumentVersion() > Version(1, 2, 5, 0) ? loader->EnterGroup(DocumentTags::Classes) : true) || !loader->EnterGroup(DocumentTags::Class, DocumentTags::name, clazz.toLatin1().constData()) || !loader->EnterGroup(DocumentTags::Editor) || !loader->EnterGroup(DocumentTags::ClassVisual) || !loader->Reset()) {
			msg(WARN, MTEXT("Could not create template \'") + FROMQSTRING(clazz) + MTEXT("\' from \'") + FROMQSTRING(filename) + MTEXT("\'."));
			DocumentFileTypes::Free(loader);
			loader = nullptr;
			return false;
		}

		Set<ChipID> sChips;
		Set<CGBackgroundItemID> sBackgroundItems;
		Set<FolderID> sFolders;
		DirectX::SimpleMath::Vector2 pos(0, 0);
		bool b = DocumentExtLoader::InsertTemplate(cg, *loader, FROMQSTRING(clazz), MainFolderID, pos, true, sChips, sBackgroundItems, sFolders);

		DocumentFileTypes::Free(loader);
		loader = nullptr;
	}

	cg->SetDirty();
	engine->GetClassManager()->SetStartClass(cg);
	AppSignals::instance().openClass(cg);

	return true;
}

bool MainWindow::_queryCGName(QString &s, QString title)
{
	while (true) {
		bool ok = false;
		QString t = QInputDialog::getText(this, title, "Name:", QLineEdit::Normal, s, &ok);
		if (!ok)
			break;
		t = t.trimmed();
		if (t.isEmpty() || t.length() > 64)
			QMessageBox::information(this, "Invalid Class Name", "Please enter a valid Class name.");
		else {
			s = t;
			return true;
		}
	}
	return false;
}

bool MainWindow::_confirmCloseProject(bool isQuitting)
{
	if (EditorState::state == EditorState::BREAKPOINT || EditorState::state == EditorState::LOADING) {
		QMessageBox::information(this, "Unavailable Operation", "This operation is not available when a Break Point has triggered!");
		return false;
	}

	List<Document*> dirtyList;
	_enumerateDirty(dirtyList);

	if (dirtyList.empty())
		return QMessageBox::question(this, isQuitting ? "Exit" : "Close Project", isQuitting ? "Are you sure you want to exit the application?" : "Are you sure you want to close the current project?", QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::Yes;

	SaveFilesDialog* sfd = new SaveFilesDialog(this);
	sfd->setWindowTitle(isQuitting ? "Quit Application" : "Close Project");
	if (sfd->showDialog() == QDialog::Rejected)
		return false;

	dirtyList = sfd->getDocumentsToSave();
	if (dirtyList.empty())
		return true;

	return _saveThese(dirtyList);
}

void MainWindow::_updateSettings(bool load)
{
	QSettings s = ((const App*)QApplication::instance())->GetSettings();
	if (load) {
		restoreGeometry(s.value("mainWindow/windowState", QByteArray()).toByteArray());
		ui.splitter_1->restoreState(s.value("mainWindow/splitter1", QByteArray()).toByteArray());
		ui.splitter_2->restoreState(s.value("mainWindow/splitter2", QByteArray()).toByteArray());
		ui.splitter_3->restoreState(s.value("mainWindow/splitter3", QByteArray()).toByteArray());
//		ui.tabWidget_1->setCurrentIndex(s.value("tab1", 0).toInt());
		_currentDir = s.value("currentDir", QDir::currentPath()).toString();
		_externalViewOpenAtStartup = s.value("mainWindow/externalViewEnabled", false).toBool();
		_externalViewState = s.value("mainWindow/externalViewState", QByteArray()).toByteArray();
	}
	else {
		s.setValue("mainWindow/windowState", this->saveGeometry());
		s.setValue("mainWindow/splitter1", ui.splitter_1->saveState());
		s.setValue("mainWindow/splitter2", ui.splitter_2->saveState());
		s.setValue("mainWindow/splitter3", ui.splitter_3->saveState());
//		s.setValue("tab1", ui.tabWidget_1->currentIndex());

		s.setValue("currentDir", _currentDir);
		s.setValue("mainWindow/externalViewEnabled", _externalProjectView != nullptr);
		s.setValue("mainWindow/externalViewState", _externalProjectView ? _externalProjectView->saveGeometry() : _externalViewState);
	}

	emit AppSignals::instance().updateSettings(s, load);
}

bool MainWindow::nativeEvent(const QByteArray &eventType, void *message, qintptr *result)
{
	MSG *msg = (MSG*)message;
	
//	if (message->message == WM_INPUT)
//		return true;
//	bool b = engine->GetInputManager()->MsgProc(msg);

	return QMainWindow::nativeEvent(eventType, message, result);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
	if (EditorState::state != EditorState::RUNNING && EditorState::state != EditorState::PAUSE) {
		onContinue();
		_exitRequested = true;
		event->ignore();
		return;
	}

	bool cancel = false;
	//emit onClosing(&cancel);

	if (cancel) {
		event->ignore();
		_exitRequested = false;
		return;
	}

	if (_confirmCloseProject(true)) {
		_updateSettings(false);
		event->accept();

		if (ActionManager::instance().getAction(ActionID::EXTERNALVIEW)->isChecked())
			ActionManager::instance().getAction(ActionID::EXTERNALVIEW)->trigger();
	}
	else
		event->ignore();
}







