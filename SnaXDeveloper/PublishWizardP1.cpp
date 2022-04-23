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
#include "PublishWizardP1.h"
#include "M3DCore/GuidUtil.h"

using namespace m3d;



PublishWizardP1::PublishWizardP1(QWidget *parent) : PublishWizardPage(parent)
{
	ui.setupUi(this);
	setTitle("General Information");
	setSubTitle("Enter some general information about your project.");

	registerField("title*", ui.lineEdit_title);
	registerField("publisherID*", ui.lineEdit_publisherID);
	registerField("productID*", ui.lineEdit_productID);
	registerField("version*", ui.lineEdit_version);
}

PublishWizardP1::~PublishWizardP1()
{
}

Version __autoIncr(Version v)
{
	if (v.elements[0] < 9)
		v.elements[0]++;
	else if (v.elements[1] < 9) {
		v.elements[0] = 0;
		v.elements[1]++;
	}
	else if (v.elements[2] < 9) {
		v.elements[0] = 0;
		v.elements[1] = 0;
		v.elements[2]++;
	}
	else if (v.elements[3] < 255) {
		v.elements[0] = 0;
		v.elements[1] = 0;
		v.elements[2] = 0;
		v.elements[3]++;
	}
	return v;
}

bool __stringtoVersion(QString s, Version &v)
{
	QStringList vl = s.split('.');
	if (vl.size() != 4)
		return false;
	bool ok;
	for (int i = 0; i < 4; i++) {
		unsigned n = vl[i].toUInt(&ok);
		if (!ok || n > 255)
			return false;
		v.elements[3 - i] = (unsigned char)n;
	}
	return true;
}

QString __versionToString(Version v) 
{
	return v == Version(0) ? "N/A" : QString("%1.%2.%3.%4").arg(v.elements[3]).arg(v.elements[2]).arg(v.elements[1]).arg(v.elements[0]);
}

void PublishWizardP1::_autoIncrChanged()
{
	if (ui.checkBox_autoIncrement->isChecked()) {
		Version p;
		if (__stringtoVersion(ui.label_prevVersion->text(), p))
			ui.lineEdit_version->setText(__versionToString(__autoIncr(p)));
	}
	else {
		Version p, q;
		if (__stringtoVersion(ui.label_prevVersion->text(), p) && __stringtoVersion(ui.lineEdit_version->text(), q) && __autoIncr(p) == q)
			ui.lineEdit_version->setText(__versionToString(p));
	}
}

void PublishWizardP1::initializePage()
{
	const PublishSettings &ps = wizard()->GetPublishSettings();
	
	ui.lineEdit_publisher->setText(TOQSTRING(ps.publisher));
	ui.lineEdit_publisherID->setText(TOQSTRING(GuidToString(ps.publisherID)));
	ui.lineEdit_title->setText(TOQSTRING(ps.title));
	ui.lineEdit_productID->setText(TOQSTRING(GuidToString(ps.productID)));
	ui.plainTextEdit_description->setPlainText(TOQSTRING(ps.description));
	ui.lineEdit_version->setText(__versionToString(ps.version == Version(0) ? Version(1,0,0,0) : (ps.autoIncrementVersion ? __autoIncr(ps.version) : ps.version)));
	ui.checkBox_autoIncrement->setChecked(ps.autoIncrementVersion);
	ui.label_prevVersion->setText(__versionToString(ps.version));
}

void PublishWizardP1::get(PublishSettings &ps)
{
	ps.publisher = FROMQSTRING(ui.lineEdit_publisher->text().trimmed());
	ps.title = FROMQSTRING(ui.lineEdit_title->text().trimmed());
	ps.description = FROMQSTRING(ui.plainTextEdit_description->toPlainText());
	StringToGUID(FROMQSTRING(ui.lineEdit_publisherID->text()), ps.publisherID);
	StringToGUID(FROMQSTRING(ui.lineEdit_productID->text()), ps.productID);
	__stringtoVersion(ui.lineEdit_version->text(), ps.version);
	ps.autoIncrementVersion = ui.checkBox_autoIncrement->isChecked();
}

bool PublishWizardP1::isComplete() const
{
	Guid g;
	Version v;
	return !ui.lineEdit_title->text().trimmed().isEmpty() && 
		StringToGUID(FROMQSTRING(ui.lineEdit_publisherID->text()), g) &&
		StringToGUID(FROMQSTRING(ui.lineEdit_productID->text()), g) &&
		__stringtoVersion(ui.lineEdit_version->text(), v);
}
