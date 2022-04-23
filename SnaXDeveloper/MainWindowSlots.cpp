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
#include <QFiledialog>
#include <QMessageBox>
#include <QClipboard>
#include <QMimeData>
#include <QInputDialog>
#include <shellapi.h>
#include <qtimer.h>
#include <qfile>
#include <qsettings>
#include "M3DEngineExt/ClassDiagram.h"
#include "M3DEngine/Application.h"
#include "StdChips\Importer.h"
#include "M3DEngine/ClassInstance.h"
#include "GraphicsChips\Graphics.h"
#include "StdChips\ProxyChip.h"
#include "PublishWizard.h"
#include "AboutDialog.h"
#include "DXEditorWidget.h"
#include "EditorWidget.h"
#include <qjsondocument.h>
#include <qjsonobject.h>
#include <qwindow.h>
#include "M3DEngineExt/EditorState.h"
#include "SettingsDialog.h"
#include "ClassDescriptionDialog.h"
#include "AppSignals.h"
#include "App.h"

using namespace m3d;

#define LIMIT_FPS 1.0/60.0


void MainWindow::onNewFrame()
{
	if (QApplication::activeWindow() == nullptr) { // temporary fix for inactive app...
		Sleep(10);
		return;
	}

//	QList<ApplicationMessage> _messageQueue;
//	for (int i = 0; i < _messageQueue.size(); i++) {
//		emit messageAdded(_messageQueue[i]);//msg.severity, msg.timestamp, msg.message, msg.cgID, msg.chipID);
//	}
//	_messageQueue.clear();


	_hpt.Tick();
	_accum += _hpt.GetDt();
//	if (_limitFPS && _accum < LIMIT_FPS) {
//		if (_accum + _hpt.GetDt() < LIMIT_FPS)
//			Sleep(1);
//		return; // <= got to find a good way to reduce cpu usage in this case...
//	}
	_accum = 0;

	_im.Update();

	emit AppSignals::instance().newFrame();
	
//	static unsigned int i =0;
//	OutputDebugStringA((String::fromNum(++i) + MTEXT("jala\n")).c_str());

	_dialogManager->UpdateChipDialogs();
	for (const auto &n : _instanceDialogs) {
		if (n.second.dialog)
			n.second.dialog->Refresh();
	}

	static DWORD dwID = GetCurrentThreadId();
	DWORD D = GetCurrentThreadId();
	

	_doRecoverySave(_recoverySaveInterval);

	if (_exitRequested) {
		close();
	}

	// NOTE: I've added this line becauses QFileDialog does not paint and refresh correctly without it. 
	// This causes mouse wheel to stop working in dx view. Got to change focus from/to to make it work.
	// Another workaround is to disable the frame timer when showing the dialog.
	//QApplication::instance()->processEvents();
}

void MainWindow::onApplicationExit()
{
	_dialogManager->CloseAllChipDialogs();
}

void MainWindow::onClipboardDataChanged()
{
	ActionManager::instance().updateAction(ActionID::PASTE);
}

void MainWindow::onNewProject()
{
	if (!_confirmCloseProject(false))
		return;

	engine->Reset();

	_unresolvedFiles.clear();

	_showStartupDialog();
}

void MainWindow::onOpenProject()
{
	if (!_confirmCloseProject(false))
		return;

	_frameTimer->stop(); // FIX: Got to stop the timer for the file-dialog to refresh correctly!
	QWidget *focus = this->focusWidget(); // FIX: Got to set the focus manually after the file-dialog for WM_MOUSEWHEEL messages to be relyed correctly to the DX-widgets!
	QString filename = QFileDialog::getOpenFileName(this, "Open Project", _currentDir, _getFilterCommon(), 0, QFileDialog::Options());
	setFocus();
	focus->setFocus();
	_frameTimer->start();

	if (filename.isEmpty())
		return;

	_currentDir = QFileInfo(filename).dir().path();

	_openProject(filename);
}

void MainWindow::onShowRecentProjectsMenu()
{
	QSettings s = ((const App*)QApplication::instance())->GetSettings();
	QStringList recentProjects = s.value("settings/recentProjects", QStringList()).toStringList();
	_recentProjectsMenu->clear();
	for (QList<QString>::iterator itr = recentProjects.begin(); itr != recentProjects.end(); itr++)
		if (QFile::exists(*itr))
			_recentProjectsMenu->addAction(*itr);
}

void MainWindow::onOpenRecentProject(QAction*a)
{
	if (!_confirmCloseProject(false))
		return;
	QString filename = a->text();
	_openProject(filename);
}

void MainWindow::addClass()
{
	QString s;
	if (!_queryCGName(s, "New Class"))
		return;
	
	Document *doc = nullptr;

	ProjectTreeWidget *ptw = dynamic_cast<ProjectTreeWidget*>(focusWidget());
	if (ptw) {
		ProjectTreeWidgetItem *i = dynamic_cast<ProjectTreeWidgetItem*>(ptw->currentItem());
		if (i) {
			if (i->wt == ProjectTreeWidgetItem::WT_DOC)
				doc = i->doc;
		}
	}

	if (!doc)
		doc = engine->GetDocumentManager()->CreateDocument();

	Class *cg = engine->GetClassManager()->CreateClass(FROMQSTRING(s), doc);
	if (cg) {
		((ClassExt*)cg)->SetDirty();
		AppSignals::instance().openClass((ClassExt*)cg);
	}
	else
		engine->GetDocumentManager()->RemoveDocument(doc);
}

void MainWindow::importClass()
{
	Map<QString, Guid> filters, filters2;

	QString allImportables;
	for (size_t i = 0; i < _importers.size(); i++) {
		const ChipInfo *nfo = engine->GetChipManager()->GetChipInfo(_importers[i]);
		QStringList lst = TOQSTRING(nfo->chipDesc.filters).split(';', Qt::SkipEmptyParts);
		if (lst.empty())
			continue;
		QString t;
		for (int j = 0; j < lst.size(); j++) {
			if (t.size() > 0)
				t += " ";
			QString ext = lst[j].toLower();
			t += "*." + ext;
			filters2.insert(std::make_pair(ext, _importers[i]));
		}
		if (allImportables.size() > 0)
			allImportables += " ";
		allImportables += t;
		filters.insert(std::make_pair(TOQSTRING(nfo->chipDesc.name) + " (" + t + ")", _importers[i]));
	}
	QString stdFilter = _getFilterCommon();
	allImportables = stdFilter.mid(stdFilter.indexOf(QChar('(')) + 1, stdFilter.length() - stdFilter.indexOf(QChar('(')) - 2) + (!allImportables.isEmpty() ? (" " + allImportables) : "");
	QString filterString = "All Importables (" + allImportables + ");;" + stdFilter;
	for (const auto &n : filters) {
		filterString += ";;" + n.first;
	}
	QString selectedFilter;

	_frameTimer->stop(); // FIX: Got to stop the timer for the file-dialog to refresh correctly!
//	QWidget *focus = this->focusWidget(); // FIX: Got to set the focus manually after the file-dialog for WM_MOUSEWHEEL messages to be relyed correctly to the DX-widgets!
	QString filename = QFileDialog::getOpenFileName(this, "Import Documents", _currentDir, filterString, &selectedFilter, QFileDialog::Options());
//	setFocus();
//	focus->setFocus();
	_frameTimer->start();

	if (filename.isEmpty())
		return;

	QFileInfo fNfo(filename);

	QString ext = fNfo.suffix().toLower();
	bool stdFile = false;
	for (unsigned i = 0; i < (uint32)DocumentFileTypes::FileType::FILE_TYPE_COUNT; i++) {
		if (TOQSTRING(DocumentFileTypes::FILE_TYPES[i].ext).toLower() == ext)
			stdFile = true;
	}

	_currentDir = QFileInfo(filename).dir().path();

	if (stdFile) {
		_importDocument(FROMQSTRING(filename), true);
	}
	else {
		auto n = filters.find(selectedFilter);
		if (n == filters.end()) {
			int m = filename.lastIndexOf('.');
			if (m != -1) {
				QString ext = filename.right(filename.length() - m - 1).toLower();
				n = filters2.find(ext);

				if (n == filters2.end()) {
					QMessageBox::warning(this, "Import Failed", "No importer found for this file format.");
					return;
				}
			}
			else
			{
				QMessageBox::warning(this, "Import Failed", "No importer found for this file format.");
				return;
			}
		}
		Importer *c = (Importer*)engine->GetChipManager()->CreateChip(n->second);
		if (!c) {
			QMessageBox::warning(this, "Import Failed", "Failed to Load Importer.");
			return;
		}
		c->SetFilename(Path(FROMQSTRING(filename)));
		int modalResult = 0;
		_dialogManager->OpenChipDialog(c, false, 0, &modalResult);
		if (modalResult == QDialog::Accepted) {
			Chip *chip = nullptr;
			Class* cg = nullptr;
			{
				ScopedOverrideCursor oc(Qt::WaitCursor);
				cg = c->Import(&chip);
			}
			if (cg) {
				((ClassExt*)cg)->SetDirty();
				if (chip)
					((ClassExt*)cg)->ArrangeChildren(chip);
				AppSignals::instance().openClass((ClassExt*)cg);
			}
			else
				QMessageBox::warning(this, "Import failed", "Import failed!");
		}
		c->Release();
	}
}

void MainWindow::importClass2(const Path& file)
{
	_importDocument(file, false);
}


void MainWindow::onSaveDirty()
{
	_saveDirty();
}

void MainWindow::onSaveAll()
{
	if (QMessageBox::question(this, "Save All", "Are you sure you want to save all Documents in the project?", QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::Yes) {
		const DocumentPtrSet &m = engine->GetDocumentManager()->GetDocuments();
		for (const auto &n : m)
			if (!_save(n))
				break; // Cancel operation!
	}
}

void MainWindow::onPublish()
{
	if (engine->GetClassManager()->GetClasssByName().empty()) {
		QMessageBox::critical(this, "Publish", "There are no Documents to publish!");
		return;
	}
	Class *startCG = engine->GetClassManager()->GetStartClass();
	if (!startCG || !startCG->GetStartChip()) {
		QMessageBox::critical(this, "Publish", "You must set a start-chip for your project before publishing!");
		return;
	}

	while (true) {
		List<Document*> dirtyList;
		_enumerateDirty(dirtyList);
		bool isUnsaved = false;
		for (unsigned i = 0; i < dirtyList.size(); i++) {
			if (!dirtyList[i]->GetFileName().IsFile()) {
				isUnsaved = true;
				break;
			}
		}
		if (isUnsaved) {
			if (QMessageBox::warning(this, "Publish", "Please save all classes before publishing! Do you want to do it now?", QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) == QMessageBox::No)
				return;
			if (!_saveDirty())
				return;
			continue;
		}
		if (dirtyList.size()) {
			int i = QMessageBox::question(this, "Publish", "Do you want to save modified classes before publishing?", QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel, QMessageBox::Yes);
			if (i == QMessageBox::Cancel)
				return;
			if (i == QMessageBox::Yes) {
				if (!_saveDirty())
					return;
				continue;
			}
		}
		break;
	}

	PublishWizard *pw = new PublishWizard(this);
	pw->publish();
}


void MainWindow::takeScreenshot()
{
	DXEditorWidget *e = dynamic_cast<DXEditorWidget*>(QApplication::focusWidget());
	if (e) {
		_frameTimer->stop(); // FIX: Got to stop the timer for the file-dialog to refresh correctly!
		QWidget *focus = this->focusWidget(); // FIX: Got to set the focus manually after the file-dialog for WM_MOUSEWHEEL messages to be relyed correctly to the DX-widgets!
		QString initfn = _currentDir + "\\EditorView.png";
		QString fn = QFileDialog::getSaveFileName(this, "Save Screenshot of Editor View", initfn, "PNG (*.png);;JPEG (*.jpg);;BMP (*.bmp);;TIFF (*.tiff)"/*;;DDS (*.dds)"*/, 0, QFileDialog::Options()); // Disabling DDS because alpha is not correct at the moment. (needs separate alpha blending!)
		setFocus();
		focus->setFocus();
		_frameTimer->start();
		if (fn.isEmpty())
			return;
		if (!e->TakeScreenshot(FROMQSTRING(fn).c_str())) {
			QMessageBox::critical(this, "Screenshot Failed", "Failed to take screenshot of editor view.");
		}
	} else
	{
		ProjectViewWidget *e = dynamic_cast<ProjectViewWidget*>(QApplication::focusWidget());
		if (e) {
			_frameTimer->stop(); // FIX: Got to stop the timer for the file-dialog to refresh correctly!
			QWidget *focus = this->focusWidget(); // FIX: Got to set the focus manually after the file-dialog for WM_MOUSEWHEEL messages to be relyed correctly to the DX-widgets!
			QString initfn = _currentDir + "\\ProjectView.png";
			QString fn = QFileDialog::getSaveFileName(this, "Save Screenshot of Project View", initfn, "PNG (*.png);;JPEG (*.jpg);;BMP (*.bmp);;TIFF (*.tiff);;DDS (*.dds)", 0, QFileDialog::Options());
			setFocus();
			if (focus)
				focus->setFocus();
			_frameTimer->start();
			if (fn.isEmpty())
				return;
			if (!e->TakeScreenshot(FROMQSTRING(fn).c_str())) {
				QMessageBox::critical(this, "Screenshot Failed", "Failed to take screenshot of project view.");
			}
		}
	}
}



void MainWindow::showInstanceDialog(ClassInstance *instance)
{
	auto n = _instanceDialogs.find(instance);
	if (n == _instanceDialogs.end()) {
		ClassInstanceDialogDesc d;
		d.dialog = nullptr;
		d.x = d.y = -1;
		d.w = 800;
		d.h = 600;

		n = _instanceDialogs.insert(std::make_pair(instance, d)).first;
		instance->SetReleaseCallback(this);
	}
	if (n->second.dialog == nullptr) {
		ClassInstanceDialog *d = new ClassInstanceDialog(this);
		d->SetCallback(this);
		if (n->second.x != -1) 
			d->move(n->second.x, n->second.y);
		d->resize(n->second.w, n->second.h);
		d->Init(instance);
		n->second.dialog = d;
		n->second.dialog->show();
	}
	else {
		n->second.dialog->activateWindow();
		n->second.dialog->Flash();
	}

}

void MainWindow::onLoadAllProjectFiles()
{
	ScopedOverrideCursor oc(Qt::WaitCursor);

	_classesToRestore.clear();
	for (const auto &n : engine->GetClassManager()->GetClasssByName())
		_classesToRestore.insert(n.second);

	_restoringClasss = true;

	while (_classesToRestore.size()) {
		Class *cg = *_classesToRestore.begin();
		_classesToRestore.erase(_classesToRestore.begin());
		cg->RestoreChips(); // This may trigger loading of other classes. These are inserted into _classesToRestore.
	}

	_restoringClasss = false;
}

void MainWindow::showChipDialog(Chip *chip, bool showComment, unsigned embeddedID)
{
	// If asked for dialog for instance data, check if an instance is selected in instance list (If selected in tab!), and show dialog for its instance data instead......
	if (ui.tabWidget_1->currentIndex() == 3 && chip->AsInstanceData()) {
		ClassInstance *instance = ui.treeWidget_instances->GetCurrentInstance();
		if (instance) {
			Chip *ch = instance->GetData(chip->AsInstanceData());
			if (ch)
				chip = ch;
		}
	}
	_dialogManager->OpenChipDialog(chip, showComment, embeddedID);
}

void MainWindow::showClassDescriptionDialog(ClassExt *cl)
{
	auto itr = _classDialogs.find(cl);
	if (itr != _classDialogs.end()) {
		itr.value()->activateWindow();
		FLASHWINFO finfo;
		finfo.cbSize = sizeof(FLASHWINFO);
		finfo.hwnd = (HWND)itr.value()->winId();
		finfo.uCount = 6;
		finfo.dwTimeout = 50;
		finfo.dwFlags = FLASHW_CAPTION;
		::FlashWindowEx(&finfo);
		return;
	}
	ClassDescriptionDialog* dlg = new ClassDescriptionDialog(this);
	dlg->SetClass(cl);
	_classDialogs.insert(cl, dlg);
	dlg->show();
	connect(dlg, &QDialog::finished, this, [this, dlg, cl](int) { 
		_classDialogs.remove(cl);
		dlg->deleteLater();
	});

}

void MainWindow::onOpenCGG()
{
	AppSignals::instance().openClass(ClassDiagram::Singleton());
}

void MainWindow::onProfilingChanged(QAction*a)
{
	if (a == _actions.profilingDisable)
		functionStack.SetPerfMon(FunctionStack::PerfMon::PERF_NONE);
	else if (a == _actions.profilingPrFrame)
		functionStack.SetPerfMon(FunctionStack::PerfMon::PERF_FRAME);
	else if (a == _actions.profilingAccumulate)
		functionStack.SetPerfMon(FunctionStack::PerfMon::PERF_ACCUM);
}

void MainWindow::onResetProfiling()
{
	functionStack.ResetPerfFrame();
}

void MainWindow::onLimitFPSChanged()
{
	_limitFPS = _actions.limit_Frame_Rate->isChecked();
	emit AppSignals::instance().forceVSync(_actions.limit_Frame_Rate->isChecked());
}

void MainWindow::updateDebugGeometryMenu()
{
	Graphics *g = engine->GetGraphics();
	_actions.render_World_Grid->setChecked(g->IsRenderWorldGrid());
	_actions.render_World_Space_AABB->setChecked(g->IsRenderWorldSpaceAABB());
	_actions.render_Local_Space_AABB->setChecked(g->IsRenderLocalAABB());
}

void MainWindow::updateDebugGeometryFromMenu()
{
	Graphics *g = engine->GetGraphics();
	g->SetRenderWorldGrid(_actions.render_World_Grid->isChecked());
	g->SetRenderWorldSpaceAABB(_actions.render_World_Space_AABB->isChecked());
	g->SetRenderLocalAABB(_actions.render_Local_Space_AABB->isChecked());
}

void MainWindow::onContinue()
{
	if (EditorState::state == EditorState::BREAKPOINT || EditorState::state == EditorState::PAUSE)
		EditorState::state = EditorState::RUNNING;
}

void MainWindow::onPause()
{
	if (EditorState::state == EditorState::RUNNING)
		EditorState::state = EditorState::PAUSE;
}

void MainWindow::onEnableBreakPoints(bool b)
{
	_breakPointsEnabled = b;
	if (_actions.enableBreakpoints->isChecked() != b)
		_actions.enableBreakpoints->setChecked(b);
}

void MainWindow::onEnableBreakOnErrors(bool b)
{
	_breakOnErrors = b;
	if (_actions.breakOnError->isChecked() != b)
		_actions.breakOnError->setChecked(b);
}

void MainWindow::onOptions()
{
	SettingsDialog* dialog = new SettingsDialog(this);
	dialog->exec();
	dialog->deleteLater();
}

void MainWindow::showHelp()
{
	ShellExecuteA(GetDesktopWindow(), MTEXT("open"), MTEXT("SnaXManual.pdf"), NULL, NULL, SW_SHOWMAXIMIZED);
}

void MainWindow::showAboutDialog()
{
	AboutDialog *dialog = new AboutDialog(this);
	dialog->exec();
	dialog->deleteLater();
}

void MainWindow::externalViewToggled()
{
	if (EditorState::state == EditorState::BREAKPOINT || EditorState::state == EditorState::LOADING) {
		QMessageBox::information(this, "Unavailable Operation", "This operation is not available when a Break Point has triggered!");
		return;
	}

	bool goExternal = ActionManager::instance().getAction(ActionID::EXTERNALVIEW)->isChecked();

	if (goExternal) {
		if (_externalProjectView)
			return; // Already in this state!

		ui.mainTabWidget->setCurrentIndex(0);
		ui.mainTabWidget->setTabEnabled(1, false);
		ui.tabWidget_2->setCurrentIndex(0);
		ui.tabWidget_2->setTabEnabled(1, false);

		_externalProjectView = new ExternalProjectWidget(this, Qt::Window);
		QGridLayout *l = new QGridLayout(_externalProjectView);
		l->setContentsMargins(QMargins(0, 0, 0, 0));
		_externalProjectView->setLayout(l);
		ProjectViewWidget *ew = new ProjectViewWidget(_externalProjectView);
		l->addWidget(ew);
		connect(&AppSignals::instance(), &AppSignals::newFrame, ew, &ProjectViewWidget::invalidateGraphics);

		connect(_externalProjectView, &ExternalProjectWidget::closing, [this]() {  
			ActionManager::instance().getAction(ActionID::EXTERNALVIEW)->setChecked(false);
			externalViewToggled();
		});
		_externalProjectView->setMinimumSize(100, 100);
		_externalProjectView->resize(640, 480);
		_externalProjectView->setWindowTitle("SnaX Developer - Project View");
		if (!_externalViewState.isEmpty())
			_externalProjectView->restoreGeometry(_externalViewState);
		if (_externalProjectView->isFullScreen())
			ActionManager::instance().getAction(ActionID::EXTERNALVIEWFULLSCREEN)->setChecked(true);

		_externalProjectView->show();
	}
	else {
		if (!_externalProjectView)
			return;

		ExternalProjectWidget *w = _externalProjectView;
		_externalProjectView = nullptr;
		_externalViewState = w->saveGeometry();
		w->close();
		w->deleteLater();
		w = nullptr;

		ui.mainTabWidget->setTabEnabled(1, true);
		ui.tabWidget_2->setTabEnabled(1, true);
		ActionManager::instance().getAction(ActionID::EXTERNALVIEWFULLSCREEN)->setChecked(false);
	}

	ActionManager::instance().updateAction(ActionID::EXTERNALVIEWFULLSCREEN);
}

void MainWindow::externalFulscreenToggled()
{
	if (!_externalProjectView)
		return;
	bool isFullscreen = ActionManager::instance().getAction(ActionID::EXTERNALVIEWFULLSCREEN)->isChecked();
	
	if (isFullscreen) {
		if (_externalProjectView->isMinimized())
			_externalProjectView->setWindowState(Qt::WindowNoState);
		_externalProjectView->setWindowState(_externalProjectView->windowState() | Qt::WindowFullScreen);
	}
	else {
		_externalProjectView->setWindowState(_externalProjectView->windowState() & ~QFlags<Qt::WindowState>(Qt::WindowFullScreen));
	}
}

void MainWindow::renameClass(ClassExt *c)
{
	while (true) {
		QString s = TOQSTRING(c->GetName());
		if (!_queryCGName(s, "Rename class"))
			break;
		if (engine->GetClassManager()->SetClassName(c, FROMQSTRING(s))) {
			c->SetDirty();
			c->SetRelationsDirty(DR_FUNCTION_CALLS);
			break;
		}
		QMessageBox::information(this, "Name taken", QString("The name \'") + s + "\' already exist.");
	}
}

void MainWindow::renameDoc(Document *doc)
{
	if (!doc->GetFileName().IsFile())
		return;

}

void MainWindow::saveDocument(Document *doc)
{
	_save(doc);
}

void MainWindow::saveDocumentAs(Document *doc)
{
	_saveAs(doc);
}

void MainWindow::deleteDocument(Document *doc)
{
	if (doc->GetFileName().IsFile()) {
		QMessageBox box(this);
		box.setIcon(QMessageBox::Question);
		box.setWindowTitle("Delete");
		box.setText("Are you sure you want to remove the document \'" + TOQSTRING(doc->GetFileName().GetName()) + "\'" + (doc->GetClasss().size() ? " and all its classes" : "") + " from the project?\n\nChoose Remove to remove it from the project.\n\nChoose Delete to delete it permanently.");
		QPushButton *btnRemove = box.addButton("Remove", QMessageBox::YesRole);
		QPushButton *btnDelete = box.addButton("Delete", QMessageBox::YesRole);
		QPushButton *btnCancel = box.addButton("Cancel", QMessageBox::RejectRole);
		box.exec();
		QAbstractButton *res = box.clickedButton();
		if (res == btnCancel)
			return;
		if (res == btnRemove) {
			if (doc->IsDirty()) {
				if (QMessageBox::question(this, "Save", "Do you want to save the document \'" + TOQSTRING(doc->GetFileName().GetName()) + "\' before removing it?", QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) == QMessageBox::Yes) {
					if (!_save(doc))
						return; // something failed....
				}
			}
		}
		Path p = doc->GetFileName();

		const ClassPtrList &lst = doc->GetClasss();
		for (size_t i = 0; i < lst.size(); i++)
			((ClassExt*)lst[i])->SetRelationsDirty(DR_ALL);

		engine->GetDocumentManager()->RemoveDocument(doc);
		if (res == btnDelete) {
			if (!p.Delete()) {
				QMessageBox::warning(this, "Delete", "Failed to delete the document from disk.");
			}
		}
	}
	else {
		if (QMessageBox::question(this, "Delete", QString("Are you sure you want to remove the unsaved document") + (doc->GetClasss().size() ? " and all its classes" : "") + " from the project?", QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) != QMessageBox::Yes)
			return;

		const ClassPtrList &lst = doc->GetClasss();
		for (size_t i = 0; i < lst.size(); i++)
			((ClassExt*)lst[i])->SetRelationsDirty(DR_ALL);

		engine->GetDocumentManager()->RemoveDocument(doc);
	}
}

void MainWindow::deleteClass(ClassExt *c)
{
	if (QMessageBox::question(this, "Delete", "Are you sure you want to remove the Class \'" + TOQSTRING(c->GetName()) + "\' from the project?", QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) != QMessageBox::Yes)
		return;
	if (c->IsDirty()) {} // wanna save?
	c->SetRelationsDirty(DR_ALL);
	engine->GetClassManager()->RemoveClass(c);
}


void MainWindow::onResolveMissingFile(Path& filename, bool& bSuccess)
{
	bSuccess = false;

	if (_unresolvedFiles.find(filename) != _unresolvedFiles.end())
		return;

	ScopedOverrideCursor oc(Qt::ArrowCursor);
	_frameTimer->stop(); // FIX: Got to stop the timer for the file-dialog to refresh correctly!
	QWidget* focus = this->focusWidget(); // FIX: Got to set the focus manually after the file-dialog for WM_MOUSEWHEEL messages to be relyed correctly to the DX-widgets!
	QString currentExt = _getFilterFromFileType(TOQSTRING(filename.GetFileExtention()));
	QString fn = QFileDialog::getOpenFileName(this, "File Not Found", TOQSTRING(filename.AsString()), _getFilter(), &currentExt, QFileDialog::Options());
	setFocus();
	if (focus)
		focus->setFocus();
	_frameTimer->start();

	if (!fn.isEmpty())
	{
		_currentDir = QFileInfo(fn).dir().path();

		filename = Path(FROMQSTRING(fn));

		bSuccess = true;
	}
	else
	{
		_unresolvedFiles.insert(filename);
	}
}
