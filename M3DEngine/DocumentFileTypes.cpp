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
#include "DocumentFileTypes.h"
#include "DocumentXMLLoader.h"
#include "DocumentXMLSaver.h"
#include "DocumentBINLoader.h"
#include "DocumentBINSaver.h"
#include "DocumentJSONLoader.h"
#include "DocumentJSONSaver.h"

using namespace m3d;

DocumentLoader* __createJSONLoader() { return mmnew DocumentJSONLoader(); }
DocumentSaver* __createJSONSaver() { return mmnew DocumentJSONSaver(); }

DocumentLoader *__createXMLLoader() { return mmnew DocumentXMLLoader(); }
DocumentSaver *__createXMLSaver() { return mmnew DocumentXMLSaver(); }

DocumentLoader *__createBINLoader() { return mmnew DocumentBINLoader(); }
DocumentSaver *__createBINSaver() { return mmnew DocumentBINSaver(); }

const DocumentFileTypes::FileTypeDesc DocumentFileTypes::FILE_TYPES[] =     { { DocumentFileTypes::FileType::JSON, MTEXT("m3j"), MTEXT("JSON SnaX Document"), &__createJSONLoader, &__createJSONSaver },
																			{ DocumentFileTypes::FileType::BINARY, MTEXT("m3b"), MTEXT("Binary SnaX Document"), &__createBINLoader, &__createBINSaver },
																			{ DocumentFileTypes::FileType::XML, MTEXT("m3x"), MTEXT("DEPRECATED: XML-Based SnaX Document"), &__createXMLLoader, &__createXMLSaver } };

bool DocumentFileTypes::CheckIfProjectFileExist(Path &filename)
{
	if (filename.CheckExistence())
		return true; // no change!
	for (size_t i = 0; i < (size_t)FileType::FILE_TYPE_COUNT; i++) {
		Path p = filename.ChangeFileExtention(FILE_TYPES[i].ext);
		if (p == filename || !p.CheckExistence())
			continue;
		filename = p;
		return true;
	}
	return false;
}

const DocumentFileTypes::FileTypeDesc *DocumentFileTypes::GetFileType(Path filename)
{
	String ext = filename.GetFileExtention();
	for (size_t i = 0; i < (size_t)FileType::FILE_TYPE_COUNT; i++) {
		if (strUtils::compareNoCase(FILE_TYPES[i].ext, ext) == 0)
			return &FILE_TYPES[i];
	}
	return nullptr;
}

DocumentLoader *DocumentFileTypes::CreateLoader(Path filename)
{
	for (size_t i = 0; i < (size_t)FileType::FILE_TYPE_COUNT; i++)
		if (filename.CompareFileExtention(FILE_TYPES[i].ext))
			return (*FILE_TYPES[i].loader)();
	return nullptr;
}

DocumentSaver *DocumentFileTypes::CreateSaver(Path filename)
{
	for (size_t i = 0; i < (size_t)FileType::FILE_TYPE_COUNT; i++)
		if (filename.CompareFileExtention(FILE_TYPES[i].ext))
			return (*FILE_TYPES[i].saver)();
	return nullptr;
}

void DocumentFileTypes::Free(DocumentLoader *loader)
{
	mmdelete(loader);
}

void DocumentFileTypes::Free(DocumentSaver *saver)
{
	mmdelete(saver);
}
