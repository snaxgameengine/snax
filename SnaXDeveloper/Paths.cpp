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

#include "StdAfx.h"
#include "Paths.h"
#include "M3DEngine/ProjectDependencies.h" // Need this to avoid linker errors.. I really should hvae a look at template containers :/

using namespace m3d;


bool Paths::Resolve(QString inPath, Path& outPath, bool forceDir)
{
	inPath = inPath.trimmed();
	if (inPath.startsWith("$app"))
		inPath = QApplication::applicationDirPath() + "\\" + inPath.mid(4);
	else if (inPath.startsWith("$applocaldata"))
		inPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "\\" + inPath.mid(13);
	else if (inPath.startsWith("$"))
		return false;
	outPath = forceDir ? Path::Dir(FROMQSTRING(inPath)) : Path(FROMQSTRING(inPath));
	return outPath.IsAbsolute();
}

bool Paths::SplitAndResolve(QString inPaths, List<Path>& outPaths, bool forceDir)
{
	outPaths.clear();
	QStringList pList = inPaths.split(';', Qt::SkipEmptyParts);
	bool b = true;
	Set<Path> ps;
	for (QString s : pList) {
		Path p;
		if (Resolve(s, p, forceDir)) {
			if (ps.insert(p).second)
				outPaths.push_back(p);
		}
		else
			b = false;
	}
	return b;
}
