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

#pragma once

#include "Common.h"
#include <qwizard.h>
#include "M3DEngineExt/Publisher.h"

namespace m3d
{

class PublishWizard;

class PublishWizardPage : public QWizardPage
{
public:
	PublishWizardPage(QWidget *parent = 0) : QWizardPage(parent) {}

	PublishWizard *wizard() const { return (PublishWizard*)QWizardPage::wizard(); }

	virtual void get(PublishSettings &ps) = 0;
};

class ClassExt;

class PublishWizard : public QWizard
{
	Q_OBJECT
public:
	enum Pages { PAGE_PROFILE, PAGE_GENERAL, PAGE_PLATFORM, PAGE_TARGET, PAGE_PROJECT_FILES, PAGE_WP8_1, PAGE_FINAL };

	PublishWizard(QWidget *parent = 0);
	~PublishWizard();

	const PublishSettings &GetPublishSettings() const;
	Path GetStartGroupPath() const { return _startGroup; }

	const List<Path> &GetLibraryPaths() const { return _libraryPaths; }

	const Map<String, PublishSettings> &GetPublishProfiles() const { return _profiles; }
	bool AddProfile(String name);
	bool AddProfile(String name, String copyFrom);
	bool RemoveProfile(String name);
	bool RenameProfile(String oldName, String newName);

	void publish();

	void accept();

	PublishSettings getSettingsFromWizard();


private:
	Map<String, PublishSettings> _profiles;
	ClassExt *_clazz;
	Path _startGroup;
	List<Path> _libraryPaths;
};


}