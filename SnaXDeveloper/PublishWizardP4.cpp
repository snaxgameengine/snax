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
#include "PublishWizardP4.h"
#include <qdir.h>
#include "M3DEngine/Engine.h"
#include "M3DEngine/Environment.h"


using namespace m3d;



PublishWizardP4::PublishWizardP4(QWidget *parent) : PublishWizardPage(parent)
{
	ui.setupUi(this);
	setTitle("Project Files");
	setSubTitle("Select which files you want to include in the publication.");

	ui.lineEdit_filters->installEventFilter(this);
}

PublishWizardP4::~PublishWizardP4()
{
}


int PublishWizardP4::nextId() const
{
//	if (field("platform_WP8_win32").toBool() || field("platform_WP8_arm").toBool())
//		return PublishWizard::PAGE_WP8_1;
	return PublishWizard::PAGE_FINAL;
}

void PublishWizardP4::initializePage()
{
	const PublishSettings &ps = wizard()->GetPublishSettings();

	ui.lineEdit_filters->setText(TOQSTRING(ps.filters));

	updateFilters();

	bool b0 = ui.fileTreeWidget->setProjectDirectory(TOQSTRING(wizard()->GetStartGroupPath().GetDirectory().AsString()));

	QStringList libPaths;
	for (size_t i = 0; i < wizard()->GetLibraryPaths().size(); i++)
		libPaths.push_back(TOQSTRING(wizard()->GetLibraryPaths()[i].AsString()));
	bool b1 = ui.fileTreeWidget->setLibraryDirectories(libPaths);

	QString filesNotFound;
	for (const auto &n : ps.projectFiles) {
		QString f = TOQSTRING(n);
		bool b = ui.fileTreeWidget->selectFile(f); // false if file was not found...
		if (!b)
			filesNotFound += filesNotFound.isEmpty() ? f : ("\n" + f);
	}
	if (!filesNotFound.isEmpty()) {
		ui.label_2->setText("The following files were not found:");
		ui.frame_notFound->setVisible(true);
		ui.textEdit_notFound->setText(filesNotFound);
	}
	else {
		ui.frame_notFound->setVisible(false);
	}

	ui.comboBox_indivCompression->setCurrentIndex(ps.projectFileCompression);
	ui.checkBox_copyOriginals->setChecked(ps.copyProject);
	ui.checkBox_includeAll->setChecked(ps.includeAll);
}

void PublishWizardP4::updateFilters()
{
	QStringList f = ui.lineEdit_filters->text().split(';', Qt::SkipEmptyParts);
	ui.fileTreeWidget->setFilters(f);
}

void PublishWizardP4::get(PublishSettings &ps)
{
	QStringList projectFilesAndLibraries = ui.fileTreeWidget->selectedFiles();
	ps.projectFiles.clear();
	QString duplicates;
	for (QString f : projectFilesAndLibraries) {
		if (!ps.projectFiles.insert(FROMQSTRING(f)).second) // If false, then there are probably identical library files...
			duplicates += duplicates.isEmpty() ? f : ("\n" + f);
	}

	ps.copyProject = ui.checkBox_copyOriginals->isChecked();
	ps.includeAll = ui.checkBox_includeAll->isChecked();
	ps.projectFileCompression = (PublishSettings::Compression)ui.comboBox_indivCompression->currentIndex();  
}

bool PublishWizardP4::validatePage()
{
	QStringList projectFilesAndLibraries = ui.fileTreeWidget->selectedFiles();
	Set<String> s;
	QString duplicates;
	for (QString f : projectFilesAndLibraries) {
		if (!s.insert(FROMQSTRING(f)).second) { // If false, then there are probably identical library files...
			duplicates += duplicates.isEmpty() ? f : ("\n" + f);
		}
	}
	if (!duplicates.isEmpty()) {
		ui.label_2->setText("The following files have duplicates:");
		ui.frame_notFound->setVisible(true);
		ui.textEdit_notFound->setText(duplicates);
		if (QMessageBox::warning(wizard(), "Duplicate library files", "Duplicate library files were detected. Are you sure you want to continue?") != QMessageBox::Yes)
			return false;
	}

	Path startProjectFile = engine->GetEnvironment()->GetProjectRootDocument();
	Path relativeStartProjectDir = startProjectFile.GetRelativePath(startProjectFile);

	if (s.find(relativeStartProjectDir.AsString()) == s.end()) {
		QMessageBox::critical(wizard(), "Missing document", QString("The document (%1) containing the start class must be included!").arg(TOQSTRING(startProjectFile.AsString())), QMessageBox::Ok);
		return false;
	}


	return true;
}

bool PublishWizardP4::eventFilter(QObject* obj, QEvent* evt)
{
	if (evt->type() == QEvent::KeyPress && obj == ui.lineEdit_filters) {
		QKeyEvent* keyEvent = static_cast<QKeyEvent*>(evt);
		if (keyEvent->key() == Qt::Key_Enter || keyEvent->key() == Qt::Key_Return) {
			updateFilters();
			return true; // mark the event as handled
		}
	}
	return false;
}