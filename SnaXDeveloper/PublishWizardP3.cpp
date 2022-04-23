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
#include "PublishWizardP3.h"
#include <qfiledialog.h>
#include <qmessagebox.h>

using namespace m3d;



PublishWizardP3::PublishWizardP3(QWidget *parent) : PublishWizardPage(parent)
{
	ui.setupUi(this);
	setTitle("Target");
	setSubTitle("Select how you want your project to be published.");

	registerField("compression*", ui.comboBox_compression);
	registerField("targetName*", ui.lineEdit_name);
	registerField("targetLocation*", ui.lineEdit_location);
}

PublishWizardP3::~PublishWizardP3()
{
}

void PublishWizardP3::initializePage()
{
	const PublishSettings &ps = wizard()->GetPublishSettings();

	bool isWP8 = field("platform_WP8_win32").toBool() || field("platform_WP8_arm").toBool();

	if (isWP8)
		ui.radioButton_archive->setText("Publish to XAP-archive");
	else
		ui.radioButton_archive->setText("Publish to executable archive");
	// TODO: Add option for Store Apps.

	ui.radioButton_archive->setChecked(ps.targetType == PublishSettings::ARCHIVE);
	ui.radioButton_folder->setChecked(ps.targetType == PublishSettings::FOLDER);
//	ui.radioButton_vs->setChecked(ps.targetType == PublishSettings::VISUAL_STUDIO);
	//ui.radioButton_vs->setEnabled(isWP8);
	ui.comboBox_compression->setCurrentIndex(ps.compression);
	ui.lineEdit_name->setText(TOQSTRING(ps.target.GetName()));
	ui.lineEdit_location->setText(TOQSTRING(Path(ps.target, wizard()->GetStartGroupPath()).GetParentDirectory().AsString()));
	targetTypeChanged(0);
}

void PublishWizardP3::get(PublishSettings &ps)
{
	ps.targetType = ui.radioButton_archive->isChecked() ? PublishSettings::ARCHIVE : PublishSettings::FOLDER;// (ui.radioButton_folder->isChecked() ? PublishSettings::FOLDER : PublishSettings::VISUAL_STUDIO);
	ps.compression = (PublishSettings::Compression)ui.comboBox_compression->currentIndex();
	PublishWizard *pw = ((PublishWizard*)wizard());
	Path name = FROMQSTRING(ui.lineEdit_name->text());
	Path location = Path::Dir(FROMQSTRING(ui.lineEdit_location->text()), wizard()->GetStartGroupPath());
	if (ui.radioButton_archive->isChecked())
		ps.target = Path::File(name, location); // check it is valid!
	else
		ps.target = Path::Dir(name, location); // check it is valid!
	if (ps.target.IsInSubFolder(wizard()->GetStartGroupPath())) // Make it relative only if in subfolder of start group!
		ps.target = ps.target.GetRelativePath(wizard()->GetStartGroupPath());
}

void PublishWizardP3::targetTypeChanged(int)
{
	ui.comboBox_compression->setEnabled(ui.radioButton_archive->isChecked());
	QString s = ui.lineEdit_name->text();
	s = s.left(s.lastIndexOf('.'));
	if (ui.radioButton_archive->isChecked()) {
		if (field("platform_WP8_win32").toBool() || field("platform_WP8_arm").toBool())
			s += ".xap";
		else
			s += ".exe";
		// TODO: Add option for Store Apps.
	}
	else {
	}
	ui.lineEdit_name->setText(s);
	emit completeChanged();
}

void PublishWizardP3::browseLocation()
{
	QString dir = QFileDialog::getExistingDirectory(this, tr("Target Location"), ui.lineEdit_location->text(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

	if (!dir.isEmpty()) {
		Path p(FROMQSTRING(dir));
		ui.lineEdit_location->setText(TOQSTRING(p.AsString()));
	}
}

bool PublishWizardP3::isComplete() const
{
	Path p(FROMQSTRING(ui.lineEdit_name->text()));
	Path q = Path::Dir(FROMQSTRING(ui.lineEdit_location->text()));

	bool a = (ui.radioButton_archive->isChecked() && p.IsFile() || !ui.radioButton_archive->isChecked() && p.IsValid());
	bool b = p.ContainsValidCharactersOnly() && p.GetParentDirectory().AsString() == MTEXT(".\\");
	bool c = q.IsDirectory() && q.ContainsValidCharactersOnly();
	bool d = ui.comboBox_compression->currentIndex() != -1;
	bool e = ui.buttonGroup->checkedButton() && ui.buttonGroup->checkedButton()->isEnabled();
	return a && b && c && d && e;
}

bool PublishWizardP3::validatePage()
{
	Path name = FROMQSTRING(ui.lineEdit_name->text());
	Path location = Path::Dir(FROMQSTRING(ui.lineEdit_location->text()), wizard()->GetStartGroupPath());
	Path target;
	if (ui.radioButton_archive->isChecked())
		target = Path::File(name, location); // check it is valid!
	else
		target = Path::Dir(name, location); // check it is valid!
	if (target.CheckExistence()) {
		if (ui.radioButton_archive->isChecked()) {
			if (QMessageBox::question(wizard(), "Confirm Overwrite", TOQSTRING((MTEXT("The file \'") + target.AsString() + MTEXT("\' does already exist. Do you want to overwrite it?")))) != QMessageBox::Yes)
				return false;
		}
		else {
			if (QMessageBox::question(wizard(), "Confirm Overwrite", TOQSTRING((MTEXT("The directory \'") + target.AsString() + MTEXT("\' does already exist. Do you want to overwrite it and all its contents?")))) != QMessageBox::Yes)
				return false;
		}
	}
	return true;
}