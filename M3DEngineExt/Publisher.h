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

#include "M3DCore/Path.h"
#include "M3DEngine/ProjectDependencies.h"
#include "M3DEngine/Class.h"
#include "M3DCore/GuidUtil.h"

namespace m3d
{

struct PublishSettings
{
	enum Platform { WD_X64, WD_X86, WSA_X64, WSA_X86, WSA_ARM, WP8_X86, WP8_ARM };
	enum Compression { NONE, LOW, NORMAL, HIGH };
	enum TargetType { ARCHIVE, FOLDER, VISUAL_STUDIO };

	// Name of the profile.
	String profile;
	// Name of publisher.
	String publisher;
	// The project title.
	String title;
	// Simple description of project.
	String description;
	// Project version.
	Version version;
	// Auto increment version for each publish.
	bool autoIncrementVersion;
	// Unique id for the project.
	Guid productID;
	// Unique id for the publisher.
	Guid publisherID;

	// Which platform to target. 
	Platform platform;

	// How you want to publish your project. To archive or folder.
	TargetType targetType;
	// The target file/folder.
	Path target; 
	// How to compress the archive.
	Compression compression;
	// Icon
	//RelativeFileName icon;

	// For desktop: One .ico file? Set it to viewer and/or exezip.

	/* For WP8:
	We need an application icon. 100x100? (png)
	We have three options for tiles:
	flip:159x159, 336x336 optional: 691x336 (if support for lage tiles)
	cycle:one small:159x159, op to nine 336x336/691x336
	iconic: 110x110, 202x202
	*/

	// Filters for project files.
	String filters;
	// Project files (relative to project root folder) to include in the publication, including libraries (path starting with $lib)
	Set<String> projectFiles;
	// How to compress the individual project files.
	Compression projectFileCompression;
	// true if to include all 3rds and chips.
	bool includeAll;
	// true if to copy project files as they are.
	bool copyProject;

	PublishSettings()
	{
		version = Version(0,0,0,0);
		autoIncrementVersion = true;
		productID = NullGUID;
		publisherID = NullGUID;
		platform = WD_X64;
		targetType = ARCHIVE;
		compression = NORMAL;
		projectFileCompression = NONE;
		includeAll = false;
		copyProject = false;
	}
	
	bool operator==(const PublishSettings &rhs) const
	{
		return 
			profile == rhs.profile &&
			publisher == rhs.publisher &&
			title == rhs.title && 
			description == rhs.description &&
			version == rhs.version &&
			autoIncrementVersion == rhs.autoIncrementVersion &&
			publisherID == rhs.publisherID &&
			productID == rhs.productID &&
			platform == rhs.platform &&
			targetType == rhs.targetType &&
			target == rhs.target && 
			compression == rhs.compression && 
			filters == rhs.filters && 
			projectFiles == rhs.projectFiles &&
			projectFileCompression == rhs.projectFileCompression &&
			includeAll == rhs.includeAll && 
			copyProject == rhs.copyProject;
	}
	bool operator!=(const PublishSettings &rhs) const { return !(*this == rhs); }
	
};

extern bool SerializeDocumentData(DocumentSaver &saver, const PublishSettings &data);
extern bool DeserializeDocumentData(DocumentLoader &loader, PublishSettings &data);


class Publisher
{
public:
	Publisher();
	~Publisher();

	bool Publish(const PublishSettings &p, const class Environment& env);

	unsigned GetWarningCount() const { return _warnings; }

	static Path GetPlatformPath(PublishSettings::Platform p);

protected:
	bool _publishToArchive(Path appDir, Path platformAppDir, Path startProjectFile, const PublishSettings &p, const class Environment& env);
	bool _publishToFolder(Path platformAppDir, Path startProjectFile, const PublishSettings &p, const class Environment& env);
	bool _publishToVS(Path platformAppDir, Path startProjectFile, const PublishSettings &p, const class Environment& env);
	void _search(Set<Path> &files, Path p);
	bool _publishWorker(Path dir, Path platformAppDir, Path startProjectFile, const PublishSettings &p, const class Environment &Env);
	bool _archiveWorker(Path target, Path appDir, Path dir, Path zipDir, const PublishSettings &p);

	unsigned _warnings;
};

}