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
#include "PublishWizardP0.h"
#include <qinputdialog.h>
#include <qformlayout.h>
#include <qlineedit.h>
#include <qcombobox.h>
#include <QDialogButtonBox.h>
#include <QMessageBox.h>

using namespace m3d;



PublishWizardP0::PublishWizardP0(QWidget *parent) : PublishWizardPage(parent)
{
	ui.setupUi(this);
	setTitle("Publishing Profile");
	setSubTitle("Select which profile you want to use when publishing.\nAll settings you update in the wizard are stored in this profile.");

	registerField("profile_index*", ui.comboBox_profile);
	registerField("profile*", ui.comboBox_profile, "currentText");
}

PublishWizardP0::~PublishWizardP0()
{
}


void PublishWizardP0::initializePage()
{
	ui.comboBox_profile->clear();

	const Map<String, PublishSettings> &pp = wizard()->GetPublishProfiles();

	for (const auto &n : pp) {
		ui.comboBox_profile->addItem(TOQSTRING(n.first));
	}

	ui.comboBox_profile->setCurrentIndex(0);
	ui.pushButton_remove->setEnabled(ui.comboBox_profile->count() > 0);
	ui.pushButton_rename->setEnabled(ui.comboBox_profile->currentIndex() != -1);
}

void PublishWizardP0::onNew()
{
	QDialog *d = new QDialog(wizard());
	QFormLayout *l = new QFormLayout(d);
	QLineEdit *n = new QLineEdit();
	l->addRow("Name:", n);
	n->setText("Unnamed Profile");
	n->selectAll();
	QComboBox *cb = new QComboBox();
	l->addRow("Copy From:", cb);
	cb->addItem("<Default Settings>");
	const Map<String, PublishSettings> &pp = wizard()->GetPublishProfiles();
	for (const auto &n : pp) {
		cb->addItem(TOQSTRING(n.first));
	}
	cb->setCurrentIndex(0);
	QDialogButtonBox *b = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
	l->addWidget(b);
    connect(b, &QDialogButtonBox::accepted, d, &QDialog::accept);
    connect(b, &QDialogButtonBox::rejected, d, &QDialog::reject);
	l->setSizeConstraint( QLayout::SetFixedSize );
	while (true) {
		if (d->exec() == QDialog::Accepted) {
			String name = FROMQSTRING(n->text().trimmed());
			if (name.empty() || pp.find(name) != pp.end()) {
				QMessageBox::critical(wizard(), "Error", QString("The name '%1' is already used or not valid.").arg(TOQSTRING(name)));
				continue;
			}
			if (cb->currentIndex() == 0) {
				if (wizard()->AddProfile(name)) {
					ui.comboBox_profile->addItem(TOQSTRING(name));
					ui.comboBox_profile->setCurrentIndex(ui.comboBox_profile->findText(TOQSTRING(name)));
					break;
				}
			}
			else {
				QString m = cb->itemText(cb->currentIndex());
				if (wizard()->AddProfile(name, FROMQSTRING(m))) {
					ui.comboBox_profile->addItem(TOQSTRING(name));
					ui.comboBox_profile->setCurrentIndex(ui.comboBox_profile->findText(TOQSTRING(name)));
					break;
				}
			}
			QMessageBox::critical(wizard(), "Error", "Failed to create profile.");
		}
		else
			break;
	}
	ui.pushButton_remove->setEnabled(ui.comboBox_profile->count() > 0);
	ui.pushButton_rename->setEnabled(ui.comboBox_profile->currentIndex() != -1);
}

void PublishWizardP0::onDelete()
{
	int i = ui.comboBox_profile->currentIndex();
	if (i == -1)
		return;
	String s = FROMQSTRING(ui.comboBox_profile->itemText(i));
	if (wizard()->RemoveProfile(s)) {
		ui.comboBox_profile->removeItem(i);
		ui.comboBox_profile->setCurrentIndex(std::min(ui.comboBox_profile->count() - 1, i));
		ui.pushButton_remove->setEnabled(ui.comboBox_profile->count() > 0);
		ui.pushButton_rename->setEnabled(ui.comboBox_profile->currentIndex() != -1);
	}
}

void PublishWizardP0::onRename()
{
	int i = ui.comboBox_profile->currentIndex();
	if (i == -1)
		return;
	QString s = ui.comboBox_profile->itemText(i);
	bool ok = false;
	QString n = QInputDialog::getText(wizard(), "Rename", "Name:", QLineEdit::Normal, s, &ok);
	n = n.trimmed();
	if (!ok || s == n || n.isEmpty())
		return;
	if (wizard()->RenameProfile(FROMQSTRING(s), FROMQSTRING(n))) {
		ui.comboBox_profile->setItemText(i, n);
	}
}

String PublishWizardP0::getProfile()
{
	return FROMQSTRING(ui.comboBox_profile->currentText());
}

void PublishWizardP0::get(PublishSettings &ps)
{
	// N/A
}

bool PublishWizardP0::isComplete() const
{
	return ui.comboBox_profile->currentIndex() != -1;
}