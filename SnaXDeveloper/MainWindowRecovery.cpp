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
#include "M3DEngine/DocumentSaver.h"
#include <time.h>
#include <qdatetime.h>
#include "M3DEngine/DocumentFileTypes.h"
#include <qfileinfo.h>
#include <qmessagebox.h>


using namespace m3d;


void MainWindow::_doRecoverySave(long recoverySaveInterval)
{
	// Recovery saving!
	const DocumentPtrSet &m = engine->GetDocumentManager()->GetDocuments();
	long currentTime = engine->GetClockTime();
	for (const auto &n : m) { // Iterate all documents
		if (!n->GetFileName().IsFile())
			continue; // We do only recovery save for documents with file.
		if (n->GetRecoverySaveTime() != 0 && ((currentTime - n->GetRecoverySaveTime()) / CLOCKS_PER_SEC) > recoverySaveInterval) {
			DocumentSaver *saver = DocumentFileTypes::CreateSaver(n->GetFileName());
			if (saver) {
				if (saver->Initialize()) {
					if (saver->SaveClasss(n)) {
						Path recoveryFile(n->GetFileName().AsString() + MTEXT(".backup"));
						Path oldRecoveryFile;
						if (recoveryFile.CheckExistence()) {
							// Rename old .backup to .backup.old
							oldRecoveryFile = Path(recoveryFile.AsString() + MTEXT(".old"));
							if (!recoveryFile.Move(oldRecoveryFile)) {
								oldRecoveryFile = Path();
								msg(WARN, MTEXT("Failed to rename old recovery file (") + recoveryFile.AsString() + MTEXT("). Continues recovery save anyway!"));
							}
						}
						if (saver->SaveToFile(recoveryFile)) {
							n->SetRecoveryFileName(recoveryFile);
							//n->ClearRecoveryFlag(); <= This one is moved below...
							msg(INFO, MTEXT("Recovery file created for \'") + n->GetFileName().AsString() + MTEXT("\'."));
							// Delete .backup.old!
							if (oldRecoveryFile.IsFile()) {
								if (!oldRecoveryFile.Delete()) {
									msg(WARN, MTEXT("Failed to delete old recovery file: \'") + oldRecoveryFile.AsString() + MTEXT("\'."));
								}
							}
						}
						else {
							// Rename .backup.old!
							if (oldRecoveryFile.IsFile()) {
								if (!oldRecoveryFile.Move(recoveryFile)) {
									msg(WARN, MTEXT("Failed to delete old recovery file: \'") + oldRecoveryFile.AsString() + MTEXT("\'."));
								}
							}
							msg(WARN, MTEXT("Failed to write recovery file to disk: \'") + recoveryFile.AsString() + MTEXT("\'."));
						}
					}
					else
						msg(WARN, MTEXT("Failed to do recovery save of \'") + n->GetFileName().AsString() + MTEXT("\'."));
				}
				else
					msg(WARN, MTEXT("Saver failed to initialize on recovery save of \'") + n->GetFileName().AsString() + MTEXT("\'."));
				DocumentFileTypes::Free(saver);
			}
			else
				msg(WARN, MTEXT("Failed to create saver for recovery save of \'") + n->GetFileName().AsString() + MTEXT("\'."));
			n->ClearRecoveryFlag(); // Clear the recovery time flag if it fails. This way, it does not try to redo the recovery save right away (most likly this happens if target folder is read-only)
		}
	}
}

void MainWindow::_deleteRecoveryFile(Document *doc)
{
	if (doc->GetRecoveryFileName().IsFile()) {
		if (!doc->GetRecoveryFileName().Delete()) {
			msg(WARN, MTEXT("Failed to delete recovery file \'") + doc->GetRecoveryFileName().AsString() + MTEXT("\'."));
		}
		doc->SetRecoveryFileName(Path());
	}
	doc->ClearRecoveryFlag();
}

QString _fileSizeToString(qint64 s)
{
	if (s < 1024)
		return QString::number(s) + " bytes";
	if (s < 1024*1024)
		return QString::number(s/1024) + " KB";
	if (s < 1024*1024*1024)
		return QString::number(s/1024/1024) + " MB";
	return QString::number(s/1024/1024/1024) + " GB";
}

void MainWindow::_lookForRecoveryFile(Path filename)
{
	Path recoveryFile(filename.AsString() + MTEXT(".backup"));
	if (recoveryFile.CheckExistence()) { // found recovery file?
		QFileInfo a(TOQSTRING(filename.AsString())), b(TOQSTRING(recoveryFile.AsString()));
		if (b.lastModified() > a.lastModified()) {
			QString q = "A recovery file \n\n" + b.absoluteFilePath() + " (" + _fileSizeToString(b.size()) + ", modified " + QLocale::system().toString(b.lastModified(), QLocale::ShortFormat) +
				") \n\nwas found for \n\n" + a.absoluteFilePath() + " (" + _fileSizeToString(a.size()) + ", modified " + QLocale::system().toString(a.lastModified(), QLocale::ShortFormat) +
				").\n\nDo you want to load the recovery file instead?";

			int result = 0;

			if (thread() == QThread::currentThread())
				onShowRecoveryDlg(q, &result);
			else
				emit showRecoveryDlg(q, &result); // This will block until the gui-thread has responed to the signal!

			if (result == QMessageBox::Yes) {
				int i = 1;
				Path jicFile(filename.AsString() + MTEXT(".old"));
				while (jicFile.CheckExistence()) {
					jicFile = Path(filename.AsString() + MTEXT(".old") + strUtils::fromNum(++i));
				}
				if (filename.Move(jicFile)) {
					if (recoveryFile.Move(filename)) {
						msg(INFO, MTEXT("Using recovery file. Old project file renamed to \'") + jicFile.AsString() + MTEXT("\', just in case. Delete in manually!"));
					}
					else {
						msg(FATAL, MTEXT("Failed to rename recovery file \'") + recoveryFile.AsString() + MTEXT("\'. Recovery file will NOT be used."));
						if (!jicFile.Move(filename)) {
							msg(FATAL, MTEXT("Failed to rename renamed project file \'") + jicFile.AsString() + MTEXT("\' back again. What a mess!"));
						}
					}
				}
				else {
					msg(FATAL, MTEXT("Failed to rename project file \'") + filename.AsString() + MTEXT("\'. Recovery file will NOT be used."));
				}
			}
			else {
				// User refused to use recovery file!
				msg(DINFO, MTEXT("Deleting recovery file: \'") + recoveryFile.AsString() + MTEXT("\'."));
				if (!recoveryFile.Delete()) {
					msg(WARN, MTEXT("Failed to delete recovery file: \'") + recoveryFile.AsString() + MTEXT("\'. Do it manually!"));
				}
			}
			
		}
		else {
			msg(DINFO, MTEXT("Found an out-of-date recovery file : \'") + recoveryFile.AsString() + MTEXT("\'. Deleting it!"));
			// The recovery file looks outdated. Delete it!
			if (!recoveryFile.Delete()) {
				msg(WARN, MTEXT("Failed to delete old recovery file: \'") + recoveryFile.AsString() + MTEXT("\'. Do it manually!"));
			}
		}
	}
}

void MainWindow::onShowRecoveryDlg(QString msg, int *result)
{
	ScopedOverrideCursor oc(Qt::ArrowCursor);
	*result = QMessageBox::question(this, "Recovery File Found", msg, QMessageBox::Yes | QMessageBox::No);
}






