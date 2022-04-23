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
#include "FileIconProvider.h"
#include <qdir.h>
#include <qpixmapcache.h>
#include <Shellapi.h>

using namespace m3d;


extern QPixmap qt_pixmapFromWinHICON(HICON icon); // qt5

QIcon FileIconProvider::icon(QFileInfo fileInfo)
{
	SHFILEINFOA info;
	unsigned long val = 0;

	QIcon retIcon;

	//Get the small icon
	val = SHGetFileInfoA((const char *)QDir::toNativeSeparators(fileInfo.filePath()).utf16(), 0, &info,
						sizeof(SHFILEINFOA), SHGFI_ICON|SHGFI_SMALLICON|SHGFI_SYSICONINDEX|SHGFI_ADDOVERLAYS|SHGFI_OVERLAYINDEX);

	// Even if GetFileInfo returns a valid result, hIcon can be empty in some cases
	if (val && info.hIcon) {
		QPixmap pixmap = qt_pixmapFromWinHICON(info.hIcon);
//		QPixmap pixmap = QPixmap::fromWinHICON(info.hIcon); // qt4
			if (!pixmap.isNull())
				retIcon.addPixmap(pixmap);
		DestroyIcon(info.hIcon);
	}

	/* WE DONT NEED THIS I THINK....
	//Get the big icon
	val = SHGetFileInfo((const Char  *)QDir::toNativeSeparators(fileInfo.filePath()).utf16(), 0, &info,
						sizeof(SHFILEINFAO), SHGFI_ICON|SHGFI_LARGEICON|SHGFI_SYSICONINDEX|SHGFI_ADDOVERLAYS|SHGFI_OVERLAYINDEX);

	if (val && info.hIcon) {
		QPixmap pixmap = QPixmap::fromWinHICON(info.hIcon);
		if (!pixmap.isNull())
			retIcon.addPixmap(pixmap);
		DestroyIcon(info.hIcon);
	}*/
	return retIcon;
}