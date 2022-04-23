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
#include "PublishWizardP2.h"

using namespace m3d;



PublishWizardP2::PublishWizardP2(QWidget *parent) : PublishWizardPage(parent)
{
	ui.setupUi(this);
	setTitle("Platform");
	setSubTitle("Select which platform you want to target.");
	connect(ui.buttonGroup, &QButtonGroup::idClicked, this, &PublishWizardP2::platformChanged);

	registerField("platform_Desktop_x64", ui.radioButton_platform0);
	registerField("platform_Desktop_x32", ui.radioButton_platform1);
	registerField("platform_WinStore_x64", ui.radioButton_platform2);
	registerField("platform_WinStore_x32", ui.radioButton_platform3);
	registerField("platform_WinStore_arm", ui.radioButton_platform4);
	registerField("platform_WP8_win32", ui.radioButton_platform5);
	registerField("platform_WP8_arm", ui.radioButton_platform6);
}

PublishWizardP2::~PublishWizardP2()
{
}

void PublishWizardP2::initializePage()
{
	const PublishSettings &ps = wizard()->GetPublishSettings();

	ui.radioButton_platform0->setEnabled(Publisher::GetPlatformPath(PublishSettings::WD_X64).CheckExistence());
	ui.radioButton_platform1->setEnabled(Publisher::GetPlatformPath(PublishSettings::WD_X86).CheckExistence());
	ui.radioButton_platform2->setEnabled(Publisher::GetPlatformPath(PublishSettings::WSA_X64).CheckExistence());
	ui.radioButton_platform3->setEnabled(Publisher::GetPlatformPath(PublishSettings::WSA_X86).CheckExistence());
	ui.radioButton_platform4->setEnabled(Publisher::GetPlatformPath(PublishSettings::WSA_ARM).CheckExistence());
	ui.radioButton_platform5->setEnabled(Publisher::GetPlatformPath(PublishSettings::WP8_X86).CheckExistence());
	ui.radioButton_platform6->setEnabled(Publisher::GetPlatformPath(PublishSettings::WP8_ARM).CheckExistence());

	ui.radioButton_platform0->setChecked(ps.platform == PublishSettings::WD_X64);
	ui.radioButton_platform1->setChecked(ps.platform == PublishSettings::WD_X86);
	ui.radioButton_platform2->setChecked(ps.platform == PublishSettings::WSA_X64);
	ui.radioButton_platform3->setChecked(ps.platform == PublishSettings::WSA_X86);
	ui.radioButton_platform4->setChecked(ps.platform == PublishSettings::WSA_ARM);
	ui.radioButton_platform5->setChecked(ps.platform == PublishSettings::WP8_X86);
	ui.radioButton_platform6->setChecked(ps.platform == PublishSettings::WP8_ARM);
}

void PublishWizardP2::get(PublishSettings &ps)
{
	switch (ui.buttonGroup->checkedId())
	{
	case -2: ps.platform = PublishSettings::WD_X64; break;
	case -3: ps.platform = PublishSettings::WD_X86; break;
	case -4: ps.platform = PublishSettings::WSA_X64; break;
	case -5: ps.platform = PublishSettings::WSA_X86; break;
	case -6: ps.platform = PublishSettings::WSA_ARM; break;
	case -7: ps.platform = PublishSettings::WP8_X86; break;
	case -8: ps.platform = PublishSettings::WP8_ARM; break;
	default: ps.platform = PublishSettings::WD_X64; break;
	}
}

bool PublishWizardP2::isComplete() const
{
	return ui.buttonGroup->checkedButton() && ui.buttonGroup->checkedButton()->isEnabled();
}

void PublishWizardP2::platformChanged()
{
	emit completeChanged();
}
