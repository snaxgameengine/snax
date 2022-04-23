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
#include <qaction.h>
#include "M3DCore/Containers.h"


namespace m3d
{

class Document;
class ClassExt;
class Function;
class Chip;
class ClassInstance;
struct ApplicationMessage;
struct ChipMessage;

// Singleton acting as a central to distribute signals between widgets.
class AppSignals : public QObject
{
	Q_OBJECT
public:
	static AppSignals& instance();

private:
	AppSignals(QObject* parent = nullptr);

signals:
	// Mainly sent from MainWindow for distribution to various widgets...
	void initTemplates(class DocumentInfoCache* templatesCache);
	void initLibraries(const class DocumentInfoCache* librariesCache);
	void initChips();
	void updateSettings(class QSettings& settings, bool load);
	void documentAdded(Document*);
	void documentRemoved(Document*);
	void documentFileChanged(Document*, const class Path&);
	void documentDirtyFlagChanged(Document*);
	void classAdded(ClassExt*);
	void classRemoved(ClassExt*);
	void classRenamed(ClassExt*);
	void classMoved(ClassExt* clazz, Document* oldDoc, Document* newDoc);
	void classDirtyFlagChanged(ClassExt*);
	void classDescriptionChanged(ClassExt*);
	void functionAdded(Function*);
	void functionChanged(Function*);
	void functionRemoved(Function*);
	void startClassSet();
	void messageAdded(const m3d::ApplicationMessage &);
	void destroyDeviceObjects();
	void newFrame();
	void packetLoaded(QString);
	void instanceRegistered(ClassInstance* instance);
	void instanceUnregistered(ClassInstance* instance);
	void instanceNameChanged(ClassInstance* instance);
	void inheritanceChanged();
	void forceVSync(bool);
	void chipMessageAdded(m3d::Chip*, const m3d::ChipMessage&);
	void chipMessageRemoved(m3d::Chip*, const m3d::ChipMessage&);
	void setCallStack(const QList<CallStackRecord> &callStack);

	// Commands mainly to MainWindow to do different things..
	void openClass(ClassExt*, Chip*c=nullptr);
	void addClass();
	void importClass();
	void importClass2(const class Path&);
	void saveDocument(Document* doc);
	void saveDocumentAs(Document* doc);
	void deleteDocument(Document*);
	void deleteClass(ClassExt*);
	void openChipDialog(Chip*, bool = false, unsigned = 0);
	void openInstanceDialog(ClassInstance*);
	void openClassDescriptionDialog(ClassExt*);
	void folderRemoved(ClassExt*, unsigned folderID);

};

}