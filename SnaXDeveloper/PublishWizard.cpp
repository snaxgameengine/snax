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
#include "PublishWizard.h"
#include "PublishWizardP0.h"
#include "PublishWizardP1.h"
#include "PublishWizardP2.h"
#include "PublishWizardP3.h"
#include "PublishWizardP4.h"
#include "PublishWizardP5.h"
#include "PublishWizardWP8P1.h"
#include "M3DEngine/Document.h"
#include "M3DEngineExt/Publisher.h"
#include "M3DEngine/Engine.h"
#include "M3DEngineExt/ClassExt.h"
#include "M3DEngine/DocumentFileTypes.h"
#include "M3DEngine/Environment.h"
#include <qmessagebox.h>


using namespace m3d;




PublishWizard::PublishWizard(QWidget *parent) : QWizard(parent)
{
	setWindowTitle("Publish Project");

	setPage(PAGE_PROFILE, new PublishWizardP0(this));
	setPage(PAGE_GENERAL, new PublishWizardP1(this));
	// No multiplatform at this time!
	//setPage(PAGE_PLATFORM, new PublishWizardP2(this));
	setPage(PAGE_TARGET, new PublishWizardP3(this));
	setPage(PAGE_PROJECT_FILES, new PublishWizardP4(this));
	setPage(PAGE_WP8_1, new PublishWizardWP8P1(this));
	setPage(PAGE_FINAL, new PublishWizardP5(this));

	resize(600, 500);

	_clazz = (ClassExt*)engine->GetClassManager()->GetStartClass();
	_profiles = _clazz->GetPublishProfiles();
	if (_profiles.empty())
		AddProfile(MTEXT("Default"));

	_startGroup = _clazz->GetDocument()->GetFileName();
	_libraryPaths = engine->GetEnvironment()->GetLibraryPaths();

	//connect(this, SIGNAL(accepted()), this, SLOT(onFinished()));
}

PublishWizard::~PublishWizard()
{
}


const PublishSettings &PublishWizard::GetPublishSettings() const
{
	String profile = FROMQSTRING(field("profile").toString());

	auto n = _profiles.find(profile);
	if (n == _profiles.end()) {
		n = _profiles.begin(); // should not happend....
	}

	return n->second;
}

bool PublishWizard::AddProfile(String name)
{
	if (_profiles.find(name) != _profiles.end())
		return false;

	PublishSettings ps;

	{
		ps.profile = name;
		ps.title = _clazz->GetName();
		GenerateGuid(ps.productID);
		GenerateGuid(ps.publisherID);
		ps.target = _clazz->GetName() + MTEXT(".exe");
		ps.filters = MTEXT("*.");
		for (unsigned i = 0; i < (uint32)DocumentFileTypes::FileType::FILE_TYPE_COUNT; i++) {
			if (i > 0)
				ps.filters += MTEXT(";*.");
			ps.filters += DocumentFileTypes::FILE_TYPES[i].ext;
		}
		const ClassPtrByStringMap &m = engine->GetClassManager()->GetClasssByName();
		for (const auto &n : m) {
			ps.projectFiles.insert(engine->GetEnvironment()->CreateDocumentPath(n.second->GetDocument()->GetFileName(), _clazz->GetDocument()->GetFileName()));
		}
	}

	_profiles.insert(std::make_pair(name, ps));
	return true;
}

bool PublishWizard::AddProfile(String name, String copyFrom)
{
	if (_profiles.find(name) != _profiles.end())
		return false;

	auto n  = _profiles.find(copyFrom);
	if (n == _profiles.end())
		return false;

	PublishSettings ps;
	ps.profile = name;

	_profiles.insert(std::make_pair(name, ps));
	return true;
}

bool PublishWizard::RemoveProfile(String name)
{
	return _profiles.erase(name) > 0;
}

bool PublishWizard::RenameProfile(String oldName, String newName)
{
	auto n  = _profiles.find(oldName);
	if (n == _profiles.end() || _profiles.find(newName) != _profiles.end())
		return false;
	PublishSettings ps = n->second;
	ps.profile = newName;
	_profiles.erase(n);
	_profiles.insert(std::make_pair(newName, ps));
	return true;
}

void PublishWizard::publish()
{

	show();

}

PublishSettings PublishWizard::getSettingsFromWizard()
{
	String profile = FROMQSTRING(field("profile").toString());

	auto n = _profiles.find(profile);

	if (n == _profiles.end())
		return PublishSettings();

	PublishSettings ps = n->second;

	QList<int> pages = visitedIds();
	for (int i = 0; i < pages.size(); i++) {
		PublishWizardPage* p = (PublishWizardPage*)page(pages[i]);
		p->get(ps);
	}
	return ps;
}

void PublishWizard::accept()
{
	String profile = FROMQSTRING(field("profile").toString());

	auto n = _profiles.find(profile);

	if (n == _profiles.end())
		return;

	QList<int> pages = visitedIds();
	for (int i = 0; i < pages.size(); i++) {
		PublishWizardPage *p = (PublishWizardPage*)page(pages[i]);
		p->get(n->second);
	}

	QApplication::setOverrideCursor(Qt::WaitCursor);

	Publisher p;

	ClassExt *cg = (ClassExt*)engine->GetClassManager()->GetStartClass();

	// Save profiles.
	if (cg->GetPublishProfiles() != _profiles) {
		cg->SetPublishProfiles(_profiles);
		cg->SetDirty();
	}

	if (p.Publish(n->second, *engine->GetEnvironment())) {
		QApplication::restoreOverrideCursor();
		if (p.GetWarningCount() == 0)
			QMessageBox::information(this, "Publish", QString("Congratulations! Your project published correctly!"));
		else
			QMessageBox::warning(this, "Publish", QString("There were one or more warnings during publishing. Your project may not work correctly. Check the message log for details."));
	}
	else {
		QApplication::restoreOverrideCursor();
		QMessageBox::warning(this, "Publish", "Publish Failed! Check the message log for details.");
	}

	QWizard::accept();
}