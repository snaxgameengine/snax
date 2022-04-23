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

namespace m3d
{

class DocumentLoader;
class DocumentSaver;

typedef DocumentLoader *(*CreateDocumentLoaderFunc)();
typedef DocumentSaver *(*CreateDocumentSaverFunc)();

struct M3DENGINE_API DocumentFileTypes
{
	// NOTE: XML saver/loader is deprecated, and should be removed!
	enum class FileType { JSON, BINARY, XML, FILE_TYPE_COUNT };

	struct FileTypeDesc
	{
		FileType filetype;
		String ext;
		String desc;
		CreateDocumentLoaderFunc loader;
		CreateDocumentSaverFunc saver;
	};

	static const FileTypeDesc FILE_TYPES[];

	static bool CheckIfProjectFileExist(Path &filename);
	static const FileTypeDesc *GetFileType(Path filename);

	static DocumentLoader *CreateLoader(Path filename);
	static DocumentSaver *CreateSaver(Path filename);

	static DocumentLoader *CreateLoader(FileType ft) { return (*FILE_TYPES[(uint32)ft].loader)(); }
	static DocumentSaver *CreateSaver(FileType ft) { return (*FILE_TYPES[(uint32)ft].saver)(); }

	static void Free(DocumentLoader *loader);
	static void Free(DocumentSaver *saver);
};



}