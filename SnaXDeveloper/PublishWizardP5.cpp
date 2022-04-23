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
#include "PublishWizardP5.h"
#include "PublishWizard.h"
#include "M3DEngine/Engine.h"
#include "M3DEngine/Environment.h"

using namespace m3d;



PublishWizardP5::PublishWizardP5(QWidget *parent) : PublishWizardPage(parent)
{
	ui.setupUi(this);
	setTitle("Summary");
	setSubTitle("Start the build process by clicking Finish");
	//setFinalPage(true);
}

PublishWizardP5::~PublishWizardP5()
{
}

/*
int PublishWizardP3::nextId() const
{
	if (field("platform_WP8_win32").toBool() || field("platform_WP8_arm").toBool())
		return PublishWizard::PAGE_WP8_1;
	return PublishWizard::PAGE_FINAL;
}*/

void PublishWizardP5::initializePage()
{
	PublishSettings ps = wizard()->getSettingsFromWizard();

	QString summary;
	
	summary += "Profile:      " + TOQSTRING(ps.profile) + "\n";
	summary += "Publisher:    " + TOQSTRING(ps.publisher) + "\n";
	summary += "Title:        " + TOQSTRING(ps.title) + "\n";
	summary += "Description:  " + TOQSTRING(ps.description) + "\n";
	summary += "Publisher ID: " + TOQSTRING(GuidToString(ps.publisherID)) + "\n";
	summary += "Product ID:   " + TOQSTRING(GuidToString(ps.productID)) + "\n";
	summary += "Version:      " + QString("%1.%2.%3.%4\n").arg(ps.version.elements[0]).arg(ps.version.elements[1]).arg(ps.version.elements[2]).arg(ps.version.elements[3]);
	summary += "Platform:     Windows 10 (x64) - DirectX 12\n";
	summary += "Build type:   " + QString(ps.targetType == PublishSettings::ARCHIVE ? "Exexutable archive\n" : "Folder\n");
	summary += "Target:       " + TOQSTRING(Path(ps.target.AsString(), wizard()->GetStartGroupPath()).AsString()) + "\n";
	if (ps.targetType == PublishSettings::ARCHIVE)
		summary += "Archive Compression:                 " + QString((ps.compression == PublishSettings::NONE) ? "None\n" : (ps.compression == PublishSettings::LOW ? "Low\n" : (ps.compression == PublishSettings::NORMAL ? "Normal\n" : "High\n")));
	summary += "Individial project file compression: " + QString((ps.projectFileCompression == PublishSettings::NONE) ? "None\n" : (ps.projectFileCompression == PublishSettings::LOW ? "Low\n" : (ps.projectFileCompression == PublishSettings::NORMAL ? "Normal\n" : "High\n")));
	summary += "Copy original project files:         " + QString((ps.copyProject ? "YES\n" : "No\n"));
	summary += "Include all chips and dependencies:  " + QString((ps.includeAll  ? "YES\n" : "No\n"));
	summary += "\n";
	summary += "The following files will be included:\n";
	for (const auto &n : ps.projectFiles) {
		Path p = engine->GetEnvironment()->ResolveDocumentPath(n, engine->GetEnvironment()->GetProjectRootFolder());
		if (p.IsValid()) // TODO: else..
			summary += TOQSTRING(p.AsString()) + "\n";
	}

	ui.textEdit_summary->setText(summary);
}