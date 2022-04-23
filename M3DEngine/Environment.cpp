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
#include "Environment.h"
#include "ClassManager.h"
#include "Engine.h"
#include "Document.h"
#include "DocumentFileTypes.h"


using namespace m3d;


Environment::Environment()
{
}

bool Environment::Init(const List<Path>& libraryPaths, const Path& projectRootDocument)
{
	_projectRootDocument = projectRootDocument;
	if (_projectRootDocument.IsValid()) {
		if (!(_projectRootDocument.IsAbsolute() && _projectRootDocument.IsFile()))
			return false;
	}
	_libraryPaths.clear();
	for (size_t i = 0; i < libraryPaths.size(); i++) {
		Path p = libraryPaths[i];
		if (!p.IsAbsolute())
			continue;
		for (size_t j = 0; j < _libraryPaths.size(); j++)
			if (p.IsInSubFolder(_libraryPaths[j]) || _libraryPaths[j].IsInSubFolder(p))
				continue;
		_libraryPaths.push_back(p);
	}

	return true;
}

bool Environment::IsPathInsideProjectRootFolder(const Path& p) const
{
	Path root = GetProjectRootFolder();
	return root.IsDirectory() ? p.IsInSubFolder(root) : false;
}

bool Environment::IsPathInsideLibraryFolder(const Path& p) const
{
	return GetLibraryFolder(p).IsDirectory();
}

Path Environment::GetProjectRootFolder() const
{
	Path p = GetProjectRootDocument();
	if (p.IsFile())
		return p.GetDirectory();
	return Path();
}

Path Environment::GetProjectRootDocument() const
{
	if (_projectRootDocument.IsAbsolute())
		return _projectRootDocument.GetDirectory();
	Class* clazzStart = engine->GetClassManager()->GetStartClass();
	Document* start = clazzStart ? clazzStart->GetDocument() : nullptr;
	return start && start->GetFileName().IsFile() ? start->GetFileName() : Path();
}

const List<Path> &Environment::GetLibraryPaths() const
{
	return _libraryPaths;
}

Path Environment::GetLibraryFolder(const Path& p) const
{
	for (size_t i = 0; i < _libraryPaths.size(); i++) {
		if (p.IsInSubFolder(_libraryPaths[i]))
			return _libraryPaths[i];
	}
	return Path();
}

Path Environment::ResolveLibraryPath(const String& libraryPath) const
{
	String l = strUtils::replaceChar(strUtils::trim(libraryPath), MCHAR('/'), MCHAR('\\'));
	if (l.substr(0, 5) != MTEXT("$lib\\"))
		return Path();
	l = MTEXT(".") + l.substr(4);
	for (size_t i = 0; i < _libraryPaths.size(); i++) {
		Path p = Path(l, _libraryPaths[i]);
		if (DocumentFileTypes::CheckIfProjectFileExist(p))
			return p;
	}

	// If the file is not found, what should we do?!
	// For now, just use the first library path, create the file name and return.
	// When the file is tried being loaded - and not found, the user (in developer) will be
	// given a chance to browse for the file... This is better than returning nothing at all.
	if (_libraryPaths.size() > 0)
		return Path(l, _libraryPaths[0]);
	else
		return Path(l, Path(MTEXT("c:\\")));

	return Path(); // No such file in libraries!
}

String Environment::CreateLibraryPath(const Path& libraryPath) const
{
	if (!libraryPath.IsAbsolute())
		return String();
	for (size_t i = 0; i < _libraryPaths.size(); i++) {
		if (libraryPath.IsInSubFolder(_libraryPaths[i])) {
			String str = libraryPath.GetRelativePath(_libraryPaths[i]).AsString();
			return MTEXT("$lib") + str.substr(1); // substr skips leading .
		}
	}
	return String(); // Not inside any library folders...
}

Path Environment::ResolveDocumentPath(const String& path, const Path& relativeTo) const
{
	if (IsLibraryPath(path))
		return ResolveLibraryPath(path);
	Path p(path);
	if (!p.IsValid())
		return Path();
	if (p.IsAbsolute())
		return p;
//	if (!relativeTo.IsAbsolute())
//		return Path(); // Nothing to relate a relative path to...
	return Path(p, relativeTo);
}

String Environment::CreateDocumentPath(const Path& path, const Path& relativeTo) const
{
	if (path.IsAbsolute()) {
		if (IsPathInsideProjectRootFolder(path)) {
			if (relativeTo.IsAbsolute())
				return path.GetRelativePath(relativeTo).AsString();
		}
		if (IsPathInsideLibraryFolder(path)) {
			return CreateLibraryPath(path);
		}
		// Should we allow creating absolute paths in all situations?
	}
	return path.AsString();
}

bool Environment::IsLibraryPath(const String& p)
{
	return strUtils::replaceChar(strUtils::trim(p).substr(0, 5), MCHAR('/'), MCHAR('\\')) == MTEXT("$lib\\");
}


Path Environment::MakeLibraryPathAbsolute(const String& libraryPathStr, const Path& libraryPath)
{
	String str = strUtils::replaceChar(strUtils::trim(libraryPathStr), MCHAR('/'), MCHAR('\\'));
	if (str.substr(0, 5) != MTEXT("$lib\\"))
		return Path(); // Not a library path!

	return Path(MTEXT(".") + str.substr(4), libraryPath);
}