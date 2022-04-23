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


namespace m3d
{

class M3DENGINE_API Environment
{
public:
	Environment();

	bool Init(const List<Path>& libraryPaths, const Path& projectRootDocument = Path());

	// Returns true if the given path is inside the current project root directory, if any.
	bool IsPathInsideProjectRootFolder(const Path& p) const;
	// Returns true if the given path is inside one of the library paths.
	bool IsPathInsideLibraryFolder(const Path& p) const;
	// Returns the absolute path of the project root directory.
	Path GetProjectRootFolder() const;
	// Returns the absolute path of the project root document.
	Path GetProjectRootDocument() const;
	// Returns the library paths.
	const List<Path> &GetLibraryPaths() const;
	// Returns the absolute path of the library the given path is inside. Empty if not inside any library.
	Path GetLibraryFolder(const Path& p) const;
	// Creates an absolute path to a library given the input path. The path has "lib" variable, eg $lib\somedir\somedoc.m3x.
	// The function will search all library paths, until it finds the given file. Returns empty if not found.
	Path ResolveLibraryPath(const String& libraryPath) const;
	// Create a library path, ie a relative path starting with $lib/ for the given path that should be inside one of the library paths.
	String CreateLibraryPath(const Path& libraryPath) const;
	// Returns the given path as an absolute path. Tries to parse it as a library path, or as relative to relativeTo if given.
	Path ResolveDocumentPath(const String &path, const Path& relativeTo = Path()) const;
	// Create path relative to relativeTo, if given, AND if p is inside the project root directory, OR as a library path if inside a library directory.
	String CreateDocumentPath(const Path& path, const Path& relativeTo = Path()) const;

	static bool IsLibraryPath(const String& p);
	static Path MakeLibraryPathAbsolute(const String& libraryPathStr, const Path& libraryPath);

private:
	List<Path> _libraryPaths;
	Path _projectRootDocument;
};


}