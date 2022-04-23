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

#include "stdafx.h"
#include "MainWindow.h"
#include "MainWindowUtil.h"
#include "M3DEngine/Chip.h"
#include <qmessagebox.h>
#include <qtimer.h>
#include <qfiledialog.h>
#include "M3DEngineExt/EditorState.h"
#include "EditorWidget.h"
#include "EditorCollection.h"
#include "ClassDescriptionDialog.h"
#include "AppSignals.h"

using namespace m3d;


Path MainWindow::GetApplicationFile() const
{
	return Path::File(FROMQSTRING(QApplication::applicationFilePath()));
}

void MainWindow::Quit() 
{
	msg(INFO, MTEXT("The game asked to quit. The process would shut down when running in the Viewer application!"));
}

void MainWindow::MessagedAdded(const ApplicationMessage &msg)
{
//	_messageQueue.push_back(msg);
	emit AppSignals::instance().messageAdded(msg);
}

void MainWindow::DestroyDeviceObjects()
{
	emit AppSignals::instance().destroyDeviceObjects();
	if (_dialogManager)
		_dialogManager->DestroyDeviceObject();
}

void MainWindow::ChipMessageAdded(Chip *chip, const ChipMessage &msg)
{
	emit AppSignals::instance().chipMessageAdded(chip, msg);

	if (_breakOnErrors && msg.severity == FATAL && engine->IsRunning())
		engine->Break(chip);
}

void MainWindow::ChipMessageRemoved(Chip *chip, const ChipMessage &msg)
{
	emit AppSignals::instance().chipMessageRemoved(chip, msg);
}

void MainWindow::Break(Chip *chip)
{
	if (_exitRequested)
		return;

	CallStack cs;
	CallStackWidget::DumpCurrentCallStack(cs, chip && chip->GetClass() ? chip->GetClass()->GetID() : InvalidClassID, chip ? chip->GetID() : InvalidChipID, true);
	emit AppSignals::instance().setCallStack(cs);

//	ActionManager::instance().updateAction(ActionID::KONTINUE);
//	ActionManager::instance().getAction(ActionID::KONTINUE)->setEnabled(true);

	EditorState::state = EditorState::BREAKPOINT;
	ActionManager::instance().getAction(ActionID::PAUSE)->setChecked(true);

	// Focus on chip!
	AppSignals::instance().openClass((ClassExt*)chip->GetClass(), chip);

	ActionManager::instance().updateActions();

	while (EditorState::state == EditorState::BREAKPOINT) { // Stay in this loop until user cancel it!
		QApplication::instance()->processEvents();
	}

//	ActionManager::instance().updateAction(ActionID::KONTINUE);
//	ActionManager::instance().getAction(ActionID::KONTINUE)->setEnabled(false);

	cs.clear();
	emit AppSignals::instance().setCallStack(cs); // Clear the call stack!

	ActionManager::instance().updateActions();
	//EditorState::state = EditorState::RUNNING;
}

unsigned MainWindow::GetFeatureMask() const
{
	return 0xFFFFFFFF;
}

void MainWindow::ClassMoved(Class *cg, Document *oldDoc, Document *newDoc)
{
	emit AppSignals::instance().classMoved((ClassExt*)cg, oldDoc, newDoc);
	if (cg == engine->GetClassManager()->GetStartClass())
		emit AppSignals::instance().startClassSet();
}

void MainWindow::DirtyFlagChanged(Document *doc) 
{
	if (doc->IsDirty())
		_dirtyCounter++;
	else 
		_dirtyCounter--;
	assert(_dirtyCounter != -1);
	_dirtyCounter = std::max(_dirtyCounter, 0u);
	ActionManager::instance().updateAction(ActionID::SAVE_DIRTY);
	emit AppSignals::instance().documentDirtyFlagChanged(doc);
}

bool MainWindow::WaitingForDocumentLoading()
{
	if (thread() != QThread::currentThread()) {
		return false; // This is not the main GUI thread!
	}

	QApplication::instance()->processEvents();

	return true;
}

void MainWindow::OnDocumentAboutToOpen(Path filename) 
{
	_lookForRecoveryFile(filename);
}

void MainWindow::OnDocumentAdded(Document *doc)
{
	doc->SetEventListener(this);
	if (doc->IsDirty()) {
		_dirtyCounter++;
		ActionManager::instance().updateAction(ActionID::SAVE_DIRTY);
	}
	emit AppSignals::instance().documentAdded(doc);
}

void MainWindow::OnDocumentRemoved(Document *doc)
{
	if (doc->IsDirty()) {
		_dirtyCounter--;
		ActionManager::instance().updateAction(ActionID::SAVE_DIRTY);
	}
	_deleteRecoveryFile(doc);
	doc->SetEventListener(nullptr);
	emit AppSignals::instance().documentRemoved(doc);
}

void MainWindow::OnDocumentFileNameChanged(Document *doc, Path oldFileName)
{
	Class *startcg = engine->GetClassManager()->GetStartClass();
	if (doc->GetFileName().IsFile() && startcg && doc == startcg->GetDocument())
		_recentlyOpened(TOQSTRING(doc->GetFileName().AsString()));
	emit AppSignals::instance().documentFileChanged(doc, oldFileName);
	const ClassPtrList &lst = doc->GetClasss();
	for (size_t i = 0; i < lst.size(); i++) {
		((ClassExt*)lst[i])->SetRelationsDirty(DR_DERIVED | DR_FUNCTION_CALLS | DR_INSTANCES | DR_CG);
	}
	if (engine->GetClassManager()->GetStartClass() && engine->GetClassManager()->GetStartClass()->GetDocument() == doc)
		emit AppSignals::instance().startClassSet();
}

void MainWindow::OnDocumentSaved(Document *doc) 
{
	_deleteRecoveryFile(doc);
	doc->SetDirty(false);
}

void MainWindow::OnClassAdded(Class *cg)
{
	ClassExt *cge = (ClassExt*)cg;
	cge->SetEventListener(this);
	emit AppSignals::instance().classAdded(cge);

	for (const auto &n : cg->GetInstances())
		emit AppSignals::instance().instanceRegistered(n);

	if (_restoringClasss)
		_classesToRestore.insert(cg); // If we clicked "Load all project files", we are in the middle of such a process.
}

void MainWindow::OnClassRemoved(Class *cg)
{
	ClassExt* cge = (ClassExt*)cg;

	auto itr = _classDialogs.find(cge);
	if (itr != _classDialogs.end()) {
		itr.value()->reject();
		assert(_classDialogs.find(cge) == _classDialogs.end());
	}

	cge->SetEventListener(nullptr);
	emit AppSignals::instance().classRemoved(cge);
	_classesToRestore.erase(cg);
}

void MainWindow::OnClassNameChanged(Class* cg)
{
	ClassExt* cge = (ClassExt*)cg;

	emit AppSignals::instance().classRenamed(cge);

	auto itr = _classDialogs.find(cge);
	if (itr != _classDialogs.end()) {
		itr.value()->UpdateWindowTitle();
	}
}

bool MainWindow::ResolveMissingFile(Path &filename)
{
	bool bSuccess = false;
	if (this->thread() == QThread::currentThread())
		onResolveMissingFile(filename, bSuccess);
	else
		emit resolveMissingFile(filename, bSuccess); // Blocks until ui-thread is done.
	return bSuccess;
}

bool MainWindow::ResolveNameConflict(String &cgName)
{
	ScopedOverrideCursor oc(Qt::ArrowCursor);
	QMessageBox::warning(this, "Name taken", "The class name \'" + TOQSTRING(cgName) + "\' already exist.");

	QString s = TOQSTRING(cgName);
	if (_queryCGName(s, "Conflicting class name")) {
		cgName = FROMQSTRING(s);
		return true;
	}
	return false;
}

bool MainWindow::ConfirmGuidChange(String cgName)
{
	ScopedOverrideCursor oc(Qt::ArrowCursor);
	bool b = QMessageBox::question(this, "ID Conflict", "A class with an ID equal to that of \'" + TOQSTRING(cgName) + "\' being loaded already exist in the project. Do you want to give the new class a new ID?", QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) == QMessageBox::Yes;
	return b;
}

bool MainWindow::GetFileName(Path &filename, bool hasFailed)
{
	ScopedOverrideCursor oc(Qt::ArrowCursor);

	if (hasFailed)
		QMessageBox::critical(this, "Invalid File Name", QString("\'") + TOQSTRING(filename.AsString()) + "\' is not valid.");

	_frameTimer->stop(); // FIX: Got to stop the timer for the file-dialog to refresh correctly!
	QWidget *focus = this->focusWidget(); // FIX: Got to set the focus manually after the file-dialog for WM_MOUSEWHEEL messages to be relyed correctly to the DX-widgets!
	QString currentExt = _getFilterFromFileType(TOQSTRING(filename.GetFileExtention()));
	QString fn = QFileDialog::getSaveFileName(this, "Save Document As", TOQSTRING(filename.AsString()), _getFilter(), &currentExt, QFileDialog::Options());
	setFocus();
	focus->setFocus();
	_frameTimer->start();

	if (fn.isEmpty())
		return false;

	_currentDir = QFileInfo(fn).dir().path();

	filename = Path(FROMQSTRING(fn));
	return true;
}

void MainWindow::OnStartClassSet(Class *cgStart)
{
	emit AppSignals::instance().startClassSet();
}

Path MainWindow::GetCurrentFilePath()
{
	return Path::Dir(FROMQSTRING(_currentDir));
}


bool MainWindow::ConfirmSaveLibraryDocument(Document* doc)
{
	ScopedOverrideCursor oc(Qt::ArrowCursor);
	
	int r = QMessageBox::question((QWidget*)nullptr, "Save Library Document", QString("Do you really want to save the library document\n\'%1\'?").arg(TOQSTRING(doc->GetFileName().AsString())), (QMessageBox::StandardButtons)(QMessageBox::Yes | QMessageBox::No), QMessageBox::No);
	return r == QMessageBox::Yes;
}

bool MainWindow::ConfirmMoveLibraryDocument(Document* doc, Path newFilename, bool isSaveAs)
{
	ScopedOverrideCursor oc(Qt::ArrowCursor);

	QString msg;
	if (isSaveAs)
		msg = QString("Do you really want to save the library document\n\'%1\'\nunder the new file name\n\'%2\'?");
	else
		msg = QString("Do you really want to rename the library document\n\'%1\'\nto the new file name\n\'%2\'?");

	int r = QMessageBox::question((QWidget*)nullptr, "Save Library Document", msg.arg(TOQSTRING(doc->GetFileName().AsString())).arg(TOQSTRING(newFilename.AsString())), (QMessageBox::StandardButtons)(QMessageBox::Yes | QMessageBox::No), QMessageBox::No);
	return r == QMessageBox::Yes;
}

void MainWindow::ChipManagerInitiated()
{
	emit AppSignals::instance().initChips();
}

void MainWindow::PacketLoaded(String packetName)
{
	emit AppSignals::instance().packetLoaded(TOQSTRING(packetName));
}

void MainWindow::DirtyFlagChanged(Class *cg)
{
	emit AppSignals::instance().classDirtyFlagChanged((ClassExt*)cg);
}

void MainWindow::OnFunctionCreate(Function *function)
{
	emit AppSignals::instance().functionAdded(function);
}

void MainWindow::OnFunctionRemove(Function *function)
{
	emit AppSignals::instance().functionRemoved(function);
}

void MainWindow::OnFunctionChange(Function *function)
{
	emit AppSignals::instance().functionChanged(function);
}

void MainWindow::OnFolderRemoved(ClassExt *cg, FolderID folder)
{
	emit AppSignals::instance().folderRemoved(cg, folder);
}

void MainWindow::OnInstanceRegistered(ClassInstance *instance) 
{
	emit AppSignals::instance().instanceRegistered(instance);
}

void MainWindow::OnInstanceUnregistered(ClassInstance *instance) 
{
	emit AppSignals::instance().instanceUnregistered(instance);
}

void MainWindow::OnInstanceNameChanged(ClassInstance *instance)
{
	emit AppSignals::instance().instanceNameChanged(instance);
	auto n = _instanceDialogs.find(instance);
	if (n != _instanceDialogs.end() && n->second.dialog)
		n->second.dialog->UpdateTitle();
}

void MainWindow::OnBaseClassAdded(Class *derived, Class *base)
{
	emit AppSignals::instance().inheritanceChanged();
}

void MainWindow::OnBaseClassRemoved(Class *derived, Class *base) 
{
	emit AppSignals::instance().inheritanceChanged();
}

void MainWindow::OnDescriptionChanged(ClassExt* clazz)
{
	emit AppSignals::instance().classDescriptionChanged(clazz);
}

void MainWindow::SetDirty(Class *cg)
{
	ClassExt *c = dynamic_cast<ClassExt*>(cg);
	if (c)
		c->SetDirty(true);
}

bool MainWindow::HasComments(Chip *ch)
{
	return ch->GetChipEditorData() != nullptr;
}

String MainWindow::GetComment(Chip *ch)
{
	return ch->GetChipEditorData() ? ch->GetChipEditorData()->comment : MTEXT("");
}

void MainWindow::SetComment(Chip *ch, String comment)
{
	if (ch->GetChipEditorData())
		ch->GetChipEditorData()->comment = comment;
}

void MainWindow::DisableFrameTimer(bool disable)
{
	if (disable) {
		_frameTimer->stop(); // FIX: Got to stop the timer for the file-dialog to refresh correctly!
	}
	else
		_frameTimer->start();

}

void MainWindow::OpenInstanceDialog(ClassInstance *instance)
{
	showInstanceDialog(instance);
}

ChipDialogContainer *MainWindow::GetEmbeddedContainer(unsigned id)
{
	EditorWidget *ew = nullptr;
	if (id != 0)
		ew = EditorCollection::GetInstance()->GetEditorByID(id);
	if (ew == nullptr)
		ew = EditorCollection::GetInstance()->GetCurrentEditor();
	if (!ew)
		return nullptr;
	return ew->createChipDialogContainer();
}

void MainWindow::OnRelease(ClassInstance *instance)
{
	auto n = _instanceDialogs.find(instance);
	if (n != _instanceDialogs.end()) {
		n->second.dialog->deleteLater();
		_instanceDialogs.erase(n);
	}
}

void MainWindow::CloseDialog(ClassInstanceDialog *dialog) 
{
	auto n = _instanceDialogs.find(dialog->GetInstance());
	if (n != _instanceDialogs.end()) {
		n->second.dialog = nullptr;
		n->second.x = dialog->pos().x();
		n->second.y = dialog->pos().y();
		n->second.w = dialog->width();
		n->second.h = dialog->height();
	}
	dialog->deleteLater();
}

void MainWindow::ShowChipDialog(Chip *chip)
{
	showChipDialog(chip, false);
}
