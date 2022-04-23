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

#include "pch.h"
#include "GlobalDef.h"
#include "DocumentManager.h"
#include "DocumentFileTypes.h"
#include "DocumentLoader.h"
#include "DocumentSaver.h"
#include "Document.h"
#include "Engine.h"
#include "ClassManager.h"
#include "Environment.h"


using namespace m3d;

namespace m3d
{
	template<typename T>
	class TCriticalBlock2
	{
	public:
		TCriticalBlock2(T& mutex) : _lock(mutex, std::defer_lock)
		{
			_lock.lock();
		}
		TCriticalBlock2(T& mutex, std::function<void()> cb, std::chrono::milliseconds ms = std::chrono::milliseconds(10)) : _lock(mutex, std::defer_lock)
		{
			while (!_lock.try_lock_for(std::chrono::milliseconds(ms)))
				cb();
		}

	private:
		std::unique_lock<T> _lock;
	};
	using CriticalBlock2 = TCriticalBlock2<std::timed_mutex>;
	using RecursiveCriticalBlock2 = TCriticalBlock2<std::recursive_timed_mutex>;
}

DocumentManager::DocumentManager() : _eventListener(nullptr), _usePreloading(true)
{
}

DocumentManager::~DocumentManager()
{
	assert(_docs.empty());
	assert(_docsByFileName.empty());
}

void DocumentManager::Clear()
{
	while (_docs.size())
		RemoveDocument(*_docs.rbegin());

	while (_docsByFileName.size()) {
		assert(_docsByFileName.begin()->second == nullptr);
		_docsByFileName.erase(_docsByFileName.begin());
	}

	for (const auto &n : _loadTasks) {
		if (n.second->task.valid())
			n.second->task.wait();
		Document *doc = n.second->task.get();
		mmdelete(doc);
	}
	_loadTasks.clear();
}

Document *DocumentManager::CreateDocument()
{
	RecursiveCriticalBlock2 lock(_lock, [this]() { if (_eventListener) _eventListener->WaitingForDocumentLoading(); });

	Document *doc = mmnew Document(engine->GetClassManager()->GetClassFactory());
	_docs.insert(doc);
	if (_eventListener)
		_eventListener->OnDocumentAdded(doc);

	return doc;
}

Document *DocumentManager::GetDocument(Path fileName, bool load, bool tryReloadIfPreviousLoadFailed)
{
	// Strip file extension!
	Path fn = Path::File(fileName.GetFileNameWithoutExtention(), fileName.GetParentDirectory());

	std::shared_ptr<LoadTask> loadTask;

	{
		RecursiveCriticalBlock2 lock(_lock, [this]() { if (_eventListener) _eventListener->WaitingForDocumentLoading(); });

		// Check if we have this file already...
		auto n = _docsByFileName.find(fn);
		if (n != _docsByFileName.end()) {
			if (n->second)
				return n->second;

			if (load && tryReloadIfPreviousLoadFailed) // Force reload?
				_docsByFileName.erase(n);
		}

		if (!load)
			return nullptr; // Not found. No loading!

		// Get the load task.
		loadTask = _getLoadTask(fileName);

		if (!loadTask) {
			_docsByFileName.insert(std::make_pair(fn, nullptr)); // Indicate failure!
			return nullptr; // Could not load this document!
		}
	}

	// Load document. Hopefully already done so we don't have to wait!
	while (loadTask->task.valid() && loadTask->task.wait_for(std::chrono::milliseconds(10)) != std::future_status::ready) {
		if (_eventListener)
			_eventListener->WaitingForDocumentLoading();
	}

	Document* doc = nullptr;

	{
		RecursiveCriticalBlock2 lock(_lock, [this]() { if (_eventListener) _eventListener->WaitingForDocumentLoading(); });

		if (!loadTask->task.valid()) {
			// The task is no longer valid and already processed.
			assert(loadTask->isProcessed);

			auto n = _docsByFileName.find(fn);
			if (n != _docsByFileName.end()) { // This SHOULD be valid by now!
				if (n->second)
					return n->second;
			}
			assert(false);
			return nullptr; // Should not happen
		}

		assert(loadTask->task.valid());

		doc = loadTask->task.get();

		fn = Path::File(loadTask->fileName.GetFileNameWithoutExtention(), loadTask->fileName.GetParentDirectory());

		// Note: Task may already have been removed from map at this point!
		if (loadTask->isProcessed)
			return doc; // In this case, the document has already been added. Return early!

		loadTask->isProcessed = true;
		for (const auto &n : loadTask->associatedFileNames)
			_loadTasks.erase(n);

		if (!doc) {
			_docsByFileName.insert(std::make_pair(fn, nullptr)); // Indicate failure!
			return nullptr; // Load task could not load document!
		}

		if (_eventListener)
			_eventListener->OnDocumentAdded(doc);

		_docsByFileName.insert(std::make_pair(fn, doc)); // Insert document WITHOUT extension!
		_docs.insert(doc);

	}

	if (!engine->GetClassManager()->OnDocumentLoaded(doc)) { // Let cg-manager know!

		// NOTE: OnDocumentLoaded() does currenly ALWAYS return true, so this section is never reach.
		//       We will have to lock the mutex again if wecome here...

		if (_eventListener)
			_eventListener->OnDocumentRemoved(doc);

		_docsByFileName.insert(std::make_pair(fn, nullptr)); // Indicate failure!
		mmdelete(doc);
		return nullptr; // Something went wrong when inserting the classes...
	}

	return doc;
}

bool DocumentManager::RemoveDocument(Document *doc)
{
	assert(doc);

	RecursiveCriticalBlock2 lock(_lock, [this]() { if (_eventListener) _eventListener->WaitingForDocumentLoading(); });

	auto m = _docs.find(doc);
	if (m == _docs.end())
		return false; // Not found

	while (doc->GetClasss().size()) // Remove each classes from the document...
		engine->GetClassManager()->RemoveClass(doc->GetClasss().back()); // TODO: return value?
	
	_docs.erase(m);

	if (doc->GetFileName().IsFile()) {
		Path fn = Path::File(doc->GetFileName().GetFileNameWithoutExtention(), doc->GetFileName().GetParentDirectory());
		if (!_docsByFileName.erase(fn)) { // hmmm... this should not really happen...
			assert(false);
			for (const auto &n : _docsByFileName) {
				if (n.second == doc) {
					_docsByFileName.erase(n.first);
					break;
				}
			}
		}
	}

	if (_eventListener)
		_eventListener->OnDocumentRemoved(doc);

	mmdelete(doc);

	return true;
}

bool DocumentManager::SaveDocument(Document *doc)
{
	assert(doc);

	if (_docs.find(doc) == _docs.end())
		return false;

	Path p = doc->GetFileName();

	if (p.IsFile()) {
		// TODO: Check if doc's file name is valid?

		RecursiveCriticalBlock2 lock(_lock, [this]() { if (_eventListener) _eventListener->WaitingForDocumentLoading(); });

		if (!engine->GetEnvironment()->IsPathInsideProjectRootFolder(p) && engine->GetEnvironment()->IsPathInsideLibraryFolder(p)) {
			if (!doc->IsAllowLibraryUpdate()) {
				if (_eventListener)
					if (_eventListener->ConfirmSaveLibraryDocument(doc))
						doc->SetAllowLibraryUpdate(true);
			}
			if (!doc->IsAllowLibraryUpdate())
				return false; 
		}

		return _saveDocument(doc, p);
	}
	
	return SaveDocumentAs(doc);
}

bool DocumentManager::SaveDocumentAs(Document *doc)
{
	assert(doc);
	Path p;

	if (!_eventListener)
		return false;

	Path originalPath;

	if (doc->GetFileName().IsFile())
		p = originalPath = doc->GetFileName();
	else {
		String name = doc->GetClasss().size() > 0 ? doc->GetClasss().front()->GetName() : MTEXT("Document"); // TODO: Improve!
		String ext = DocumentFileTypes::FILE_TYPES[(uint32)DocumentFileTypes::FileType::XML].ext;
		if (_eventListener)
			p = Path(name + MTEXT(".") + ext, _eventListener->GetCurrentFilePath());
		else
			p = Path(name + MTEXT(".") + ext);
	}

	RecursiveCriticalBlock2 lock(_lock, [this]() { if (_eventListener) _eventListener->WaitingForDocumentLoading(); });

	for (bool f = false; true; f = true) {
		if (!_eventListener->GetFileName(p, f))
			return false;

		// Strip file extension!
		Path fn = Path::File(p.GetFileNameWithoutExtention(), p.GetParentDirectory());
		auto n = _docsByFileName.find(fn);
		if (n != _docsByFileName.end() && n->second != doc)
			continue; // File name already used by another doc...

		if (originalPath.IsValid() && !engine->GetEnvironment()->IsPathInsideProjectRootFolder(originalPath) && engine->GetEnvironment()->IsPathInsideLibraryFolder(originalPath)) {
			if (!doc->IsAllowLibraryUpdate()) {
				if (_eventListener)
					if (_eventListener->ConfirmMoveLibraryDocument(doc, originalPath, true))
						doc->SetAllowLibraryUpdate(true);
			}
			if (!doc->IsAllowLibraryUpdate())
				return false;
		}

		if (DocumentFileTypes::GetFileType(p) != nullptr) // Valid file ext?
			break;
	}

	return _saveDocument(doc, p);
}

bool DocumentManager::RenameDocument(Document* doc, Path newFileName)
{
	Path q = doc->GetFileName();

	if (!doc || !q.IsFile())
		return false;

	if (newFileName.CheckExistence())
		return false;

	// Strip file extension!
	Path fn = Path::File(newFileName.GetFileNameWithoutExtention(), newFileName.GetParentDirectory());
	auto n = _docsByFileName.find(fn);
	if (n != _docsByFileName.end() && n->second != doc)
		return false; // file name already exist in project!

	if (DocumentFileTypes::GetFileType(newFileName) == nullptr) // Valid file ext?
		return false;

	if (q.IsValid() && !engine->GetEnvironment()->IsPathInsideProjectRootFolder(q) && engine->GetEnvironment()->IsPathInsideLibraryFolder(q)) {
		if (!doc->IsAllowLibraryUpdate()) {
			if (_eventListener)
				if (_eventListener->ConfirmMoveLibraryDocument(doc, q, false))
					doc->SetAllowLibraryUpdate(true);
		}
		if (!doc->IsAllowLibraryUpdate())
			return false;
	}

	if (!q.Move(newFileName))
		return false;

	_docsByFileName.erase(Path::File(q.GetFileNameWithoutExtention(), q.GetParentDirectory()));

	_docsByFileName.insert(std::make_pair(Path::File(newFileName.GetFileNameWithoutExtention(), newFileName.GetParentDirectory()), doc));
	doc->SetFileName(newFileName);
	if (_eventListener)
		_eventListener->OnDocumentFileNameChanged(doc, q);

	return true;
}

Path DocumentManager::PreloadDocument(Path fileName, const Guid* clazzGUID)
{
	RecursiveCriticalBlock2 lock(_lock, [this]() { if (_eventListener) _eventListener->WaitingForDocumentLoading(); });

	if (clazzGUID) {
		Path p;
		if (engine->GetClassManager()->HasClass(*clazzGUID, p)) {
			return p;
		}
	}

	// Strip file extension!
	Path fn = Path::File(fileName.GetFileNameWithoutExtention(), fileName.GetParentDirectory());

	auto n = _docsByFileName.find(fn);
	if (n != _docsByFileName.end())
		return n->second ? n->second->GetFileName() : Path(); // Document already (tried) loaded!

	auto a = _getLoadTask(fileName); // Start async loading process!
	if (a)
		return a->fileName; // Update the actual filename to load.

	return Path(); // No such file...
}

Path DocumentManager::PreloadDocument(Path fileName, String clazzName)
{
	{
		RecursiveCriticalBlock2 lock(_lock, [this]() { if (_eventListener) _eventListener->WaitingForDocumentLoading(); });

		Path p;
		if (engine->GetClassManager()->HasClass(clazzName, p)) {
			return p;
		}
	}
	return PreloadDocument(fileName);
}


std::shared_ptr<DocumentManager::LoadTask> DocumentManager::_getLoadTask(Path fileName)
{
	// Strip file extension!
	Path fn = Path::File(fileName.GetFileNameWithoutExtention(), fileName.GetParentDirectory()), fn2 = fn;

	auto n = _loadTasks.find(fn); // Check if load task already exist for this file...
	if (n != _loadTasks.end())
		return n->second; // Yes!

	Path fileNameToLoad = fileName;

	// Need to check different file extensions when searching for file.
	bool fileFound = DocumentFileTypes::CheckIfProjectFileExist(fileNameToLoad);

	if (!fileFound) {
		// First, see if any of the documents we are loading got the same file name...
		String baseName = fileName.GetFileNameWithoutExtention();
		for (const auto &m : _loadTasks) {
			if (strUtils::compareNoCase(m.second->fileName.GetFileNameWithoutExtention(), baseName) == 0) {
				fileFound = true;
				fileNameToLoad = m.second->fileName;
				break;
			}
		}
	}
	if (!fileFound) {
		// Try ask the user finding it manually then..
		if (!(_eventListener && _eventListener->ResolveMissingFile(fileNameToLoad) && fileNameToLoad.CheckExistence())) {
			msg(FATAL, MTEXT("File not found: ") + fileName.AsString() + MTEXT("."));
			return nullptr; // No such file...
		}
	}

	if (fileName != fileNameToLoad) {
		msg(WARN, MTEXT("File not found: \'") + fileName.AsString() + MTEXT("\'. Trying \'") + fileNameToLoad.AsString() + MTEXT("\'."));
		fn2 = Path::File(fileNameToLoad.GetFileNameWithoutExtention(), fileNameToLoad.GetParentDirectory());
		n = _loadTasks.find(fn2); // Check if load task already exist for this file...
		if (n != _loadTasks.end()) {
			// Others looking for the original fileName will automatically be redirected
			_loadTasks.insert(std::make_pair(fn, n->second)); 
			n->second->associatedFileNames.insert(fn); 

			return n->second; // Yes!
		}
	}

	if (_eventListener)
		_eventListener->OnDocumentAboutToOpen(fileNameToLoad); // This gives a chance to check for a recovery file..

	std::shared_ptr<LoadTask> lt(new LoadTask());
	lt->fileName = fileNameToLoad;
	lt->associatedFileNames.insert(fn);
	_loadTasks.insert(std::make_pair(fn, lt)).first;
	if (fn != fn2) {
		_loadTasks.insert(std::make_pair(fn, lt)).first;
		lt->associatedFileNames.insert(fn2);
	}

	lt->task = std::async(std::launch::async, [lt]
	{
		Document *doc = nullptr;

	//	msg(INFO, String(MTEXT("Starting load at thread %1.")).arg(String::fromNum((uint32)GetCurrentThreadId())));

		// Find loader based on file extension
		DocumentLoader *loader = DocumentFileTypes::CreateLoader(lt->fileName); 

		if (loader == nullptr) {
			msg(FATAL, MTEXT("Failed to load document because of unknown file extension (") + lt->fileName.AsString() + MTEXT(")."));
			return (Document*)nullptr; 
		}

		loader->SetLoadRelatedDocumentsAsync(true); // Let us also load the base graphs right away!

		// Load cg using the loader.
		if (loader->OpenFile(lt->fileName)) {
			doc = mmnew Document(engine->GetClassManager()->GetClassFactory());
			doc->SetFileName(lt->fileName);

			if (!loader->LoadDocument(doc)) {
				msg(FATAL, MTEXT("Failed to open document \'") + lt->fileName.AsString() + MTEXT("\'. Failed on parsing."));
				mmdelete(doc);
				doc = nullptr;
			}
		}
		else
			msg(FATAL, MTEXT("Failed to open document \'") + lt->fileName.AsString() + MTEXT("\'. Failed on loading."));

		DocumentFileTypes::Free(loader);

	//	msg(INFO, String(MTEXT("Ending load at thread %1.")).arg(String::fromNum((uint32)GetCurrentThreadId())));

		return doc;
	});

	return lt;
}

bool DocumentManager::_saveDocument(Document *doc, Path p)
{
	DocumentSaver *saver = DocumentFileTypes::CreateSaver(p);

	if (!saver) {
		msg(FATAL, MTEXT("Failed to save document because of unknown file extension (") + p.AsString() + MTEXT(")."));
		return false;
	}

	saver->SetCompressionLevel(DocumentCompressionLevel::DCL_NORMAL);

	if (!saver->Initialize()) {
		msg(FATAL, MTEXT("Failed to save document (") + p.AsString() + MTEXT(") because saver failed to initialize."));
		DocumentFileTypes::Free(saver);
		return false;
	}

	bool fileChange = p != doc->GetFileName();

	Path q = doc->GetFileName();
	doc->SetFileName(p); // We must set the file name here because chips may use it when refering other files.
	if (!saver->SaveClasss(doc)) {
		doc->SetFileName(q); // Restore file name...
		msg(FATAL, MTEXT("Failed to save document \'") + doc->GetFileName().AsString() + MTEXT("\'."));
		DocumentFileTypes::Free(saver);
		return false;
	}
	if (!saver->SaveToFile(p)) {
		doc->SetFileName(q); // Restore file name...
		msg(FATAL, MTEXT("Failed to write document to file \'") + p.AsString() + MTEXT("\'."));
		DocumentFileTypes::Free(saver);
		return false;
	}
	DocumentFileTypes::Free(saver);

	if (fileChange) {
		if (q.IsFile())
			_docsByFileName.erase(Path::File(q.GetFileNameWithoutExtention(), q.GetParentDirectory()));
		_docsByFileName.insert(std::make_pair(Path::File(p.GetFileNameWithoutExtention(), p.GetParentDirectory()), doc));
		if (_eventListener)
			_eventListener->OnDocumentFileNameChanged(doc, q);
	}
	if (_eventListener)
		_eventListener->OnDocumentSaved(doc);

	return true;
}