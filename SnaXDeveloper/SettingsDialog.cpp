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
#include "SettingsDialog.h"
#include <qapplication.h>
#include "App.h"

using namespace m3d;


SettingsDialog::SettingsDialog(QWidget* parent, Qt::WindowFlags f) : QDialog(parent, f | Qt::MSWindowsFixedSizeDialogHint)
{
	ui.setupUi(this);
	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

	QSettings s = ((const App*)QApplication::instance())->GetSettings();
	{
		QString libraries = s.value("settings/libraries", "$app\\Libraries\\").toString();
		libraries = libraries.replace(';', '\n');
		ui.plainTextEdit_libs->setPlainText(_libsOriginal = libraries);
	}
	{
		QString templates = s.value("settings/templates", "$app\\Templates\\").toString();
		templates = templates.replace(';', '\n');
		ui.plainTextEdit_templates->setPlainText(_templatesOriginal = templates);
	}
}

SettingsDialog::~SettingsDialog()
{
}

void SettingsDialog::accept()
{
	QSettings s = ((const App*)QApplication::instance())->GetSettings();
	{
		QString libraries = ui.plainTextEdit_libs->toPlainText();
		if (libraries != _libsOriginal) {
			libraries = libraries.replace('\n', ';');
			s.setValue("settings/libraries", libraries);
		}
	}
	{
		QString templates = ui.plainTextEdit_templates->toPlainText();
		if (templates != _templatesOriginal) {
			templates = templates.replace('\n', ';');
			s.setValue("settings/templates", templates);
		}
	}

	QDialog::accept();
}