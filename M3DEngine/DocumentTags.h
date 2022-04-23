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

// For XML only. When storing large arrays (eg float32*) this is the size of the array before we switch to base64 to help performance!
#define ARRAY_LIMIT_BEFORE_BASE64 (uint32)1000


namespace m3d
{

enum class DocumentEncryptionLevel {ENCLEVEL0 = 0x10, ENCLEVEL1 = 0x20, ENCLEVEL2 = 0x40}; 
enum class DocumentCompressionLevel { DCL_NONE, DCL_LOW, DCL_NORMAL, DCL_HIGH };

namespace DocumentTags
{
	enum Tag 
	{
		Document,
		Class,
		Chips,
		Chip,
		Connections,
		Connection,
		ChipData,
		Data,
		Inheritance,
		ClassDiagram,
		Ptr,
		Editor,
		ClassVisual,
		Folder,
		SelectionCopy,
		Function,
		Parameter,
		Comments,
		Comment,
		Instances,
		Instance,
		Description_old,
		Description,
		Rect,
		Image,
		Text,
		PublishProfiles,
		Profile,
		Classes,
		Parameters,
		Content,
		name = 32, // Attributes from 32-63
		startclassid,
		startchipid,
		version,
		classid,
		chipid,
		id,
		conn,
		subconn,
		x,
		y,
		inx,
		iny,
		type,
		access,
		refreshmode,
		filename,
		extent,
		multiconnection,
		sx = multiconnection + 6, // Skipping elementes that were removed...
		sy,
		color,
		textSize,
		objectType, // json only
		_content = 255 // Internal
	};

	static const Char *TagStr[] = 
	{
		MTEXT("Document"), 
		MTEXT("Class"),
		MTEXT("Chips"), 
		MTEXT("Chip"),
		MTEXT("Connections"),
		MTEXT("Connection"),
		MTEXT("ChipData"),
		MTEXT("Data"),
		MTEXT("Inheritance"),
		MTEXT("ClassDiagram"),
		MTEXT("Ptr"),
		MTEXT("Editor"),
		MTEXT("ClassVisual"),
		MTEXT("Folder"),
		MTEXT("SelectionCopy"),
		MTEXT("Function"),
		MTEXT("Parameter"),
		MTEXT("Comments"),
		MTEXT("Comment"),
		MTEXT("Instances"),
		MTEXT("Instance"),
		MTEXT("Publish"),
		MTEXT("Description"),
		MTEXT("Rect"),
		MTEXT("Image"),
		MTEXT("Text"),
		MTEXT("PublishProfiles"),
		MTEXT("Profile"),
		MTEXT("Classes"),
		MTEXT("Parameters"),
		MTEXT("Content")
		,MTEXT(""),
		MTEXT("name"),
		MTEXT("startclassid"),
		MTEXT("startchipid"),
		MTEXT("version"),
		MTEXT("classid"),
		MTEXT("chipid"),
		MTEXT("id"),
		MTEXT("conn"),
		MTEXT("subconn"),
		MTEXT("x"), 
		MTEXT("y"), 
		MTEXT("inx"),
		MTEXT("iny"),
		MTEXT("type"),
		MTEXT("access"),
		MTEXT("refreshmode"),
		MTEXT("filename"),
		MTEXT("extent"),
		MTEXT("multiconnection"),
		MTEXT("target"),
		MTEXT("filters"),
		MTEXT("compression"),
		MTEXT("copyproject"),
		MTEXT("includeall"),
		MTEXT("sx"),
		MTEXT("sy"),
		MTEXT("color"),
		MTEXT("textSize"),
		MTEXT("objectType")
	};

	static const Char* TagStr_json[] =
	{
		MTEXT("snax"),
		MTEXT("class"),
		MTEXT("chips"),
		MTEXT("chip"),
		MTEXT("connections"),
		MTEXT("connection"),
		MTEXT("chipData"),
		MTEXT("data"),
		MTEXT("inheritance"),
		MTEXT("classDiagram"),
		MTEXT("ptr"),
		MTEXT("editor"),
		MTEXT("graph"),
		MTEXT("folder"),
		MTEXT("selectionCopy"),
		MTEXT("function"),
		MTEXT("parameter"),
		MTEXT("comments"),
		MTEXT("comment"),
		MTEXT("instances"),
		MTEXT("instance"),
		MTEXT(""),
		MTEXT("description"),
		MTEXT("rect"),
		MTEXT("image"),
		MTEXT("text"),
		MTEXT("publishProfiles"),
		MTEXT("profile"),
		MTEXT("classes"),
		MTEXT("parameters"),
		MTEXT("content")
		,MTEXT(""),
		MTEXT("name"),
		MTEXT("startClassId"),
		MTEXT("startChipId"),
		MTEXT("version"),
		MTEXT("classId"),
		MTEXT("chipId"),
		MTEXT("id"),
		MTEXT("connection"),
		MTEXT("subConnection"),
		MTEXT("x"),
		MTEXT("y"),
		MTEXT("inx"),
		MTEXT("iny"),
		MTEXT("type"),
		MTEXT("access"),
		MTEXT("recalculation"),
		MTEXT("fileName"),
		MTEXT("extent"),
		MTEXT("multiConnection"),
		MTEXT("target"),
		MTEXT("filters"),
		MTEXT("compression"),
		MTEXT("copyProject"),
		MTEXT("includeAll"),
		MTEXT("sx"),
		MTEXT("sy"),
		MTEXT("color"),
		MTEXT("textSize"),
		MTEXT("objectType")
	};
}

}


 