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

#include "Exports.h"
#include "M3DCore/Path.h"
#include "M3DCore/Containers.h"
#include "M3DCore/GuidUtil.h"
#include <mutex>
#include <future>

namespace m3d
{

class Document;
class ClassFactory;

typedef Map<Path, Document*> DocumentPtrByPathMap;
typedef Set<Document*> DocumentPtrSet;

class DocumentManagerEventListener
{
public:
	virtual bool WaitingForDocumentLoading() = 0;
	virtual void OnDocumentAboutToOpen(Path filename) = 0;
	virtual void OnDocumentAdded(Document *doc) = 0;
	virtual void OnDocumentRemoved(Document *doc) = 0;
	virtual void OnDocumentFileNameChanged(Document *doc, Path oldFileName) = 0;
	virtual void OnDocumentSaved(Document *doc) = 0;
	virtual bool ResolveMissingFile(Path &filename) = 0;
	virtual bool GetFileName(Path &filename, bool hasFailed) = 0;
	virtual Path GetCurrentFilePath() = 0;
	virtual bool ConfirmSaveLibraryDocument(Document* doc) = 0;
	virtual bool ConfirmMoveLibraryDocument(Document* doc, Path newFilename, bool isSaveAs) = 0;
};

class M3DENGINE_API DocumentManager
{
public:
	DocumentManager();
	~DocumentManager();

	// Sets the event listener. 
	void SetEventListener(DocumentManagerEventListener *listener) { _eventListener = listener; }

	// Removes all documents
	void Clear();

	// Returns all documents.
	const DocumentPtrSet &GetDocuments() const { return _docs; }
	// Creates a new document.
	Document *CreateDocument();
	// Gets a document with the given file name. May load it.
	Document *GetDocument(Path fileName, bool load = true, bool tryReloadIfPreviousLoadFailed = false);
	// Removes a document and all its classes.
	bool RemoveDocument(Document *doc);
	// Saves document to its current file. Event listener may be asked for file name.
	bool SaveDocument(Document *doc);
	// Save document to a new file. Event listener is asked for new file name.
	bool SaveDocumentAs(Document *doc);
	bool RenameDocument(Document* doc, Path newFileName);

	// true if we are to use preloading.
	bool IsUsingPreloading() const { return _usePreloading; }
	// Set if we should use preloading.
	void SetUsePreloading(bool b) { _usePreloading = b; }
	// Starts to load the given document async. When GetDocument(...) is later called, it will hopefully already be available, saving us time!
	// The (optional) clazzGUID/clazzName is our reason for asking for the file. If it is found, we do not load anything...
	Path PreloadDocument(Path fileName, const Guid *clazzGUID = nullptr);
	Path PreloadDocument(Path fileName, String clazzName);

private:
	// Callback for events.
	DocumentManagerEventListener *_eventListener;

	// Documents. Need this because some documents may not have a file.
	DocumentPtrSet _docs;
	// Documents ordered by file name WITHOUT file extension! Two documents in the same folder can NOT have the same name even if the extension is different!
	DocumentPtrByPathMap _docsByFileName;
	// true if we should allow documents to be preloaded, ie loaded async immediately when a file name is found during the loading process of a document.
	bool _usePreloading;
	// true if chips should do preparation (load data to GPU etc) on load. This makes for shorter waiting time when we are to use the chips!
	//bool _useInstantPrepare;

	std::recursive_timed_mutex _lock;

	struct LoadTask
	{
		std::future<Document*> task;
		Path fileName;
		Set<Path> associatedFileNames;
		bool isProcessed = false;
	};


// Class internal only
#pragma warning(push)
#pragma warning(disable:4251)

	Map<Path, std::shared_ptr<LoadTask>> _loadTasks;

	std::shared_ptr<LoadTask> _getLoadTask(Path fileName);

#pragma warning(pop)

	bool _saveDocument(Document* doc, Path p);
};


}