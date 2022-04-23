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
#include "M3DCore/SlimRWLock.h"


namespace m3d
{

typedef Set<Path> PathSet;
//template class M3DENGINE_API Set<Path>;

class M3DENGINE_API ProjectDependencies
{
public:
	enum Platform { WD_X64, WD_X86, WSA_X64, WSA_X86, WSA_ARM, WP8_X86, WP8_ARM }; // duplicate of that in publisher...

	ProjectDependencies();
	ProjectDependencies(Path baseDirectory, uint32 platform);
	~ProjectDependencies();

	bool AddDependency(String file);

	const PathSet &GetDependencies() const { return _deps; }
	Path GetBaseDirectory() const { return _baseDirectory; }

	uint32 GetPlatform() const { return _platform; }

private:
	// base directory for all files. They myst be in this directory or subdirectories.
	Path _baseDirectory;
	PathSet _deps;
	SlimRWLock _lock;
	uint32 _platform;
};

}