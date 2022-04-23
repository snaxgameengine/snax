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
#include "M3DEngine/GlobalDef.h"
#include "Publisher.h"
#include "M3DEngine/Engine.h"
#include "M3DEngine/ClassManager.h"
#include "M3DEngine/ChipManager.h"
#include "M3DEngine/DocumentFileTypes.h"
#include "M3DEngine/DocumentSaver.h"
#include "M3DEngine/DocumentLoader.h"
#include "M3DEngine/DocumentSaveLoadUtil.h"
#include "M3DEngine/Document.h"
#include "M3DEngine/Environment.h"
#include <fstream>
#include <libxml/encoding.h>
#include <libxml/xmlsave.h>
#include <libxml/tree.h>
#include <ppl.h>


using namespace m3d;

#define FILE_VIEWER_EXE Path(MTEXT("SnaXViewer.exe"))
#define FILE_CORE_DLL Path(MTEXT("M3DCore.dll"))
#define FILE_ENGINE_DLL Path(MTEXT("M3DEngine.dll"))

#if defined(DEBUG) || defined(_DEBUG)
#define FILE_LIBXML2_DLL Path(MTEXT("libxml2.dll"))
#define FILE_ZLIB_DLL Path(MTEXT("zlibd1.dll"))
#define FILE_LIBICONV Path(MTEXT("iconv-2.dll"))
#define FILE_LZMA Path(MTEXT("lzmad.dll"))
//#define FILE_LIBCHARSET MTEXT("libcharset.dll")
#define FILE_SDL2 Path(MTEXT("SDL2d.dll"))
//#define FILE_MSVCP MTEXT("msvcp110d.dll")
#else
#define FILE_LIBXML2_DLL Path(MTEXT("libxml2.dll"))
#define FILE_ZLIB_DLL Path(MTEXT("zlib1.dll"))
#define FILE_LIBICONV Path(MTEXT("iconv-2.dll"))
#define FILE_LZMA Path(MTEXT("lzma.dll"))
//#define FILE_LIBCHARSET MTEXT("libcharset.dll")
#define FILE_SDL2 Path(MTEXT("SDL2.dll"))
//#define FILE_MSVCP MTEXT("msvcp140.dll") 
//#define FILE_CONCRT140 MTEXT("concrt140.dll")
//#define FILE_VCCORLIB140 MTEXT("vccorlib140.dll")
//#define FILE_UCRTBASE MTEXT("ucrtbase.dll") 
//#define FILE_API_MS_ MTEXT("api-ms-*.dll")
#endif

//TODO: Other files for other platforms?!


bool m3d::SerializeDocumentData(DocumentSaver &saver, const PublishSettings &data)
{
	SAVE(MTEXT("profile"), data.profile);
	SAVE(MTEXT("publisher"), data.publisher);
	SAVE(MTEXT("title"), data.title);
	SAVE(MTEXT("description"), data.description);
	SAVE(MTEXT("publisherID"), data.publisherID);
	SAVE(MTEXT("productID"), data.productID);
	SAVE(MTEXT("version"), data.version);
	SAVE(MTEXT("autoIncrementVersion"), data.autoIncrementVersion);

	SAVE(MTEXT("platform"), (unsigned)data.platform);
	SAVE(MTEXT("targetType"), (unsigned)data.targetType);
	SAVE(MTEXT("target"), data.target);
	SAVE(MTEXT("compression"), (unsigned)data.compression);

	SAVE(MTEXT("filters"), data.filters);
	SAVE(MTEXT("copyProject"), data.copyProject);
	SAVE(MTEXT("includeAll"), data.includeAll);
	SAVE(MTEXT("projectFileCompression"), (unsigned)data.projectFileCompression);

	SAVE(MTEXT("files"), data.projectFiles);

	return true;
}

bool m3d::DeserializeDocumentData(DocumentLoader &loader, PublishSettings &data)
{
	LOAD(MTEXT("profile"), data.profile);
	LOAD(MTEXT("publisher"), data.publisher);
	LOAD(MTEXT("title"), data.title);
	LOAD(MTEXT("description"), data.description);
	LOAD(MTEXT("publisherID"), data.publisherID);
	LOAD(MTEXT("productID"), data.productID);
	LOAD(MTEXT("version"), data.version);
	LOAD(MTEXT("autoIncrementVersion"), data.autoIncrementVersion);

	LOAD(MTEXT("platform"), (unsigned&)data.platform);
	LOAD(MTEXT("targetType"), (unsigned&)data.targetType);
	LOAD(MTEXT("target"), data.target);
	LOAD(MTEXT("compression"), (unsigned&)data.compression);

	LOAD(MTEXT("filters"), data.filters);
	LOAD(MTEXT("copyProject"), data.copyProject);
	LOAD(MTEXT("includeAll"), data.includeAll);
	LOAD(MTEXT("projectFileCompression"), (unsigned&)data.projectFileCompression);

	LOAD(MTEXT("files"), data.projectFiles);

	return true;
}


Publisher::Publisher()
{
	_warnings = 0;
}

Publisher::~Publisher()
{
}

bool Publisher::Publish(const PublishSettings &p, const Environment& env)
{
	Path startProjectFile = env.GetProjectRootDocument();

	if (!startProjectFile.IsFile()) {
		msg(FATAL, MTEXT("Invalid Start Group."));
		return false;
	}

	Path relativeStartProjectDir = startProjectFile.GetRelativePath(startProjectFile);

	if (p.projectFiles.find(relativeStartProjectDir.AsString()) == p.projectFiles.end()) {
		msg(FATAL, MTEXT("Start Group is not included in project files."));
		return false;
	}

	Path appDir = Path::CurrentDir();

	Path platformAppDir = GetPlatformPath(p.platform);
	if (!platformAppDir.IsDirectory()) {
		msg(FATAL, MTEXT("Invalid target platform.")); 
		return false;
	}

	if (!platformAppDir.CheckExistence()) {
		msg(FATAL, MTEXT("Target platform is not available: ") + platformAppDir.AsString() + MTEXT(".")); 
		return false; 
	}

	if (p.targetType == PublishSettings::ARCHIVE) // Publishing to archive?
		return _publishToArchive(appDir, platformAppDir, startProjectFile, p, env);
	else if (p.targetType == PublishSettings::FOLDER) // Publishing to folder?
		return _publishToFolder(platformAppDir, startProjectFile, p, env);
	else if (p.targetType == PublishSettings::VISUAL_STUDIO) { // Publishing to visual studio project (for WP8)
		if (p.platform != PublishSettings::WP8_X86 && p.platform != PublishSettings::WP8_ARM)
			return false; // This is only available for WP8 projects.
		return _publishToVS(platformAppDir, startProjectFile, p, env);
	}

	return false;
}

Path Publisher::GetPlatformPath(PublishSettings::Platform p)
{
	Path appDir = Path::CurrentDir();
	Path platformAppDir;

	switch (p)
	{
#if PLATFORM==PLATFORM_WINDESKTOP_X64
	case PublishSettings::WD_X64: platformAppDir = appDir; break;
	case PublishSettings::WD_X86: platformAppDir = Path(Path(MTEXT("WDA_x86/")), appDir); break;
#else
	case PublishSettings::WD_X64: platformAppDir = Path(MTEXT("WDA_x64/"), appDir); break;
	case PublishSettings::WD_X86: platformAppDir = appDir; break;
#endif
	case PublishSettings::WSA_X64: platformAppDir = Path(Path(MTEXT("WSA_x64/")), appDir); break;
	case PublishSettings::WSA_X86: platformAppDir = Path(Path(MTEXT("WSA_x86/")), appDir); break;
	case PublishSettings::WSA_ARM: platformAppDir = Path(Path(MTEXT("WSA_ARM/")), appDir); break;
	case PublishSettings::WP8_X86: platformAppDir = Path(Path(MTEXT("WP8_x86/")), appDir); break;
	case PublishSettings::WP8_ARM: platformAppDir = Path(Path(MTEXT("WP8_ARM/")), appDir); break;
	default:;
	}
	return platformAppDir;
}

bool Publisher::_publishToArchive(Path appDir, Path platformAppDir, Path startProjectFile, const PublishSettings &p, const Environment &env)
{
	bool publishResult;

	Path tmppath = Path::TempDir();	
	Path dir, zipDir;

	// Create the archive file name.
	Path target = Path::File(p.target, startProjectFile);
	if (!target.IsFile()) {
		msg(FATAL, MTEXT("Invalid target path: ") + target.AsString() + MTEXT("."));
		return false;
	}

	// Delete old archive if it exist.
	if (target.CheckExistence() && !target.Delete()) {
		msg(FATAL, MTEXT("Failed to delete old target file: ") + target.AsString() + MTEXT("."));
		return false;
	}

	// Set up temporary directory for project.
	for (unsigned i = 1; i < 10000; i++) {
		dir = Path::Dir(MTEXT("F3DPublish") + strUtils::fromNum(i), tmppath);
		if (CreateDirectoryA(dir.AsString().c_str(), nullptr))
			break;
	}
	// Set up temporary directory for 7-zip.
	for (unsigned i = 1; i < 10000; i++) {
		zipDir = Path::Dir(MTEXT("F3D7Zip") + strUtils::fromNum(i), tmppath);
		if (CreateDirectoryA(zipDir.AsString().c_str(), nullptr))
			break;
	}

	if (dir.IsDirectory() && zipDir.IsDirectory())
		publishResult = _publishWorker(dir, platformAppDir, startProjectFile, p, env)  && _archiveWorker(target, appDir, dir, zipDir, p);
	else {
		msg(FATAL, MTEXT("Failed to set up temporary directories."));
	} 

	// delete temp directories
	if (dir.IsValid() && !dir.Delete()) {
		msg(WARN, MTEXT("Failed to delete temporary folder: ") + dir.AsString() + MTEXT("."));
		_warnings++;
	}
	if (zipDir.IsValid() && !zipDir.Delete()) {
		msg(WARN, MTEXT("Failed to delete temporary folder: ") + zipDir.AsString() + MTEXT("."));
		_warnings++;
	}

	return publishResult;
}

bool Publisher::_publishToFolder(Path platformAppDir, Path startProjectFile, const PublishSettings &p, const Environment& env)
{
	// Create name for target directory.
	Path targetDir = Path::Dir(p.target, startProjectFile);
	if (!targetDir.IsDirectory()) {
		msg(FATAL, MTEXT("Invalid target path: ") + targetDir.AsString() + MTEXT("."));
		return false;
	}

	// Note: Parent directory MUST exist!

	// Delete old directory if it exist.
	if (targetDir.CheckExistence() && !targetDir.Delete()) { // NOTE: Dangerous! If targetDir is unintentional.. Bang!
		msg(FATAL, MTEXT("Failed to delete old target directory: ") + targetDir.AsString() + MTEXT("."));
		return false;
	}

	// Create target directory
	
	if (!targetDir.CreateDir(true)) {
		msg(FATAL, MTEXT("Failed to create directory: ") + targetDir.AsString() + MTEXT("."));
		return false;
	}
		
	if (!_publishWorker(targetDir, platformAppDir, startProjectFile, p, env)) {
		if (!targetDir.Delete()) // Delete directory
			msg(WARN, MTEXT("Failed to delete directory: ") + targetDir.AsString() + MTEXT("."));
		return false;
	}
	return true;
}

xmlNodePtr xmlGetChild(xmlNodePtr node, const xmlChar *name) // Gets the first child of given name.
{
	for (xmlNodePtr n = node->children; n; n = n->next)
		if (xmlStrcmp(n->name, name) == 0)
			return n;
	return nullptr;
}
/*
xmlNodePtr xmlGetChild(xmlNodePtr node, const xmlChar *name, const xmlChar *propertyName, const xmlChar *propertyValue) // Gets the first child of given name and property.
{
	for (xmlNodePtr n = node->children; n; n = n->next) {
		if (xmlStrcmp(n->name, name) == 0) {
			xmlChar *a = xmlGetProp(n, BAD_CAST propertyName);
			if (!a)
				continue;
			int32 c = xmlStrcmp(a, BAD_CAST propertyValue);
			xmlFree(a);
			if (c == 0)
				return n;
		}
	}
	return nullptr;
}
*/

// %1 Project name
// %2 Project file (xxx.vcxproj)
// %3 Project id (guid)
// %4 Debug/Release
// %5 Platform (ARM/Win32/x64)
static const Char VS_SOLUTION_TXT[] = 
	MTEXT("Microsoft Visual Studio Solution File, Format Version 12.00\n") \
	MTEXT("# Visual Studio 2012\n") \
	MTEXT("Project(\"{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}\") = \"%1\", \"%2\", \"%3\"\n") \
	MTEXT("EndProject\n") \
	MTEXT("Global\n") \
	MTEXT("	GlobalSection(SolutionConfigurationPlatforms) = preSolution\n") \
	MTEXT("		%4|%5 = %4|%5\n") \
	MTEXT("	EndGlobalSection\n") \
	MTEXT("	GlobalSection(ProjectConfigurationPlatforms) = postSolution\n") \
	MTEXT("		%3.%4|%5.ActiveCfg = %4|%5\n") \
	MTEXT("		%3.%4|%5.Build.0 = %4|%5\n") \
	MTEXT("		%3.%4|%5.Deploy.0 = %4|%5\n") \
	MTEXT("	EndGlobalSection\n") \
	MTEXT("	GlobalSection(SolutionProperties) = preSolution\n") \
	MTEXT("		HideSolutionNode = FALSE\n") \
	MTEXT("	EndGlobalSection\n") \
	MTEXT("EndGlobal\n");

// %1 Platform (ARM/Win32/x64)
// %2 Project id (guid)
// %3 Project name
// %4 item groups
static const Char VS_PROJECT_TXT[] = 
	MTEXT("<?xml version=\"1.0\" encoding=\"utf-8\"?>\n") \
	MTEXT("<Project DefaultTargets=\"Build\" ToolsVersion=\"4.0\" xmlns=\"http://schemas.microsoft.com/developer/msbuild/2003\">\n") \
	MTEXT("  <ItemGroup Label=\"ProjectConfigurations\">\n") \
	MTEXT("    <ProjectConfiguration Include=\"Release|%1\">\n") \
	MTEXT("      <Configuration>Release</Configuration>\n") \
	MTEXT("      <Platform>%1</Platform>\n") \
	MTEXT("    </ProjectConfiguration>\n") \
	MTEXT("  </ItemGroup>\n") \
	MTEXT("  <PropertyGroup Label=\"Globals\">\n") \
	MTEXT("    <ProjectGuid>%2</ProjectGuid>\n") \
	MTEXT("    <RootNamespace>%3</RootNamespace>\n") \
	MTEXT("    <DefaultLanguage>en-US</DefaultLanguage>\n") \
	MTEXT("    <MinimumVisualStudioVersion>11.0</MinimumVisualStudioVersion>\n") \
	MTEXT("    <XapOutputs>true</XapOutputs>\n") \
	MTEXT("    <XapFilename>%3_$(Configuration)_$(Platform).xap</XapFilename>\n") \
	MTEXT("    <WinMDAssembly>true</WinMDAssembly>\n") \
	MTEXT("  </PropertyGroup>\n") \
	MTEXT("  <Import Project=\"$(VCTargetsPath)\\Microsoft.Cpp.Default.props\" />\n") \
	MTEXT("  <PropertyGroup Condition=\"\'$(Configuration)|$(Platform)\'==\'Release|%1\'\" Label=\"Configuration\">\n") \
	MTEXT("    <ConfigurationType>Application</ConfigurationType>\n") \
	MTEXT("    <UseDebugLibraries>false</UseDebugLibraries>\n") \
	MTEXT("    <WholeProgramOptimization>true</WholeProgramOptimization>\n") \
	MTEXT("    <PlatformToolset>v110_wp80</PlatformToolset>\n") \
	MTEXT("  </PropertyGroup>\n") \
	MTEXT("  <Import Project=\"$(VCTargetsPath)\\Microsoft.Cpp.props\" />\n") \
	MTEXT("  <ImportGroup Label=\"PropertySheets\">\n") \
	MTEXT("    <Import Project=\"$(UserRootDir)\\Microsoft.Cpp.$(Platform).user.props\" Condition=\"exists(\'$(UserRootDir)\\Microsoft.Cpp.$(Platform).user.props\')\" Label=\"LocalAppDataPlatform\" />\n") \
	MTEXT("  </ImportGroup>\n") \
	MTEXT("  <PropertyGroup Label=\"UserMacros\" />\n") \
	MTEXT("%4") \
	MTEXT("  <Import Project=\"$(VCTargetsPath)\\Microsoft.Cpp.targets\" />\n") \
	MTEXT("  <Import Project=\"$(MSBuildExtensionsPath)\\Microsoft\\WindowsPhone\\v8.0\\Microsoft.Cpp.WindowsPhone.8.0.targets\" />\n") \
	MTEXT("  <ImportGroup Label=\"ExtensionTargets\">\n") \
	MTEXT("  </ImportGroup>\n") \
	MTEXT("</Project>\n");
//	MTEXT("  <Import Project=\"$(MSBuildExtensionsPath)\\Microsoft\\WindowsPhone\\v$(TargetPlatformVersion)\\Microsoft.Cpp.WindowsPhone.$(TargetPlatformVersion).targets\" />\n") \




bool Publisher::_publishToVS(Path platformAppDir, Path startProjectFile, const PublishSettings &p, const Environment& env)
{
	/*
	// Create name for target directory.
	Path targetDir = Path::Dir(p.target, startProjectFile);
	if (!targetDir.IsDirectory()) {
		msg(FATAL, MTEXT("Invalid target path: ") + targetDir.AsString() + MTEXT("."));
		return false;
	}

	// Note: Parent directory MUST exist!

	// Delete old directory if it exist.
	if (targetDir.CheckExistence() && !targetDir.Delete()) { // NOTE: Dangerous! If targetDir is unintentional.. Bang!
		msg(FATAL, MTEXT("Failed to delete old target directory: ") + targetDir.AsString() + MTEXT("."));
		return false;
	}

	// Create target directory
	if (CreateDirectory(targetDir.AsString().c_str(), nullptr) == FALSE) {
		msg(FATAL, MTEXT("Failed to create directory: ") + targetDir.AsString() + MTEXT("."));
		return false;
	}*/
	if (!_publishToFolder(platformAppDir, startProjectFile, p, env))
		return false;

	Path targetDir = Path::Dir(p.target, startProjectFile);
	Guid projectID;
	GenerateGuid(projectID);
	String projectName = targetDir.GetName();
	String platformStr;
	String configStr = MTEXT("Release");
	String relativeProjectFileName = /*projectName + MTEXT("\\") +*/ projectName + MTEXT(".vcxproj");
	Path projectDir = targetDir;//Path::Dir(projectName, targetDir);
	Path projectFile = Path::File(relativeProjectFileName, targetDir);

	switch (p.platform)
	{
	case PublishSettings::WD_X64: platformStr = MTEXT("x64"); break;
	case PublishSettings::WD_X86: platformStr = MTEXT("Win32"); break;
	case PublishSettings::WSA_X64: platformStr = MTEXT("x64"); break;
	case PublishSettings::WSA_X86: platformStr = MTEXT("Win32"); break;
	case PublishSettings::WSA_ARM: platformStr = MTEXT("ARM"); break;
	case PublishSettings::WP8_X86: platformStr = MTEXT("Win32"); break;
	case PublishSettings::WP8_ARM: platformStr = MTEXT("ARM"); break;
	default:;
	}
/*
	// Create target directory
	if (CreateDirectory(projectDir.AsString().c_str(), nullptr) == FALSE) {
		msg(FATAL, MTEXT("Failed to create directory: ") + projectDir.AsString() + MTEXT("."));
		return false;
	}

	if (!_publishWorker(projectDir, platformAppDir, startProjectFile, p)) {
		// delete folder.
		return false;
	}
*/

	// Build and create project (.vcxproj) file.
	{
		String itemGroups, itemGroupImages, itemGroupNone, itemGroupXML;

		Set<Path> projectFiles;
		_search(projectFiles, projectDir);
		for (const auto& n : projectFiles) {
			String ext = strUtils::toLower(n.GetFileExtention());
			String localPath = n.GetRelativePath(projectDir).AsString();
			if (ext == MTEXT("xml")) {
				itemGroupXML += strUtils::ConstructString(MTEXT("    <Xml Include=\"%1\" />\n")).arg(localPath);
			}
			else {
				itemGroupNone += strUtils::ConstructString(	MTEXT("    <None Include=\"%1\">\n") \
											MTEXT("      <DeploymentContent Condition=\"\'$(Configuration)|$(Platform)\'==\'Release|%2\'\">true</DeploymentContent>\n") \
											MTEXT("    </None>\n")).arg(localPath).arg(platformStr);
			}
		}
		if (!itemGroupImages.empty())
			itemGroupImages = MTEXT("  <ItemGroup>\n") + itemGroupImages + MTEXT("  </ItemGroup>\n");
		if (!itemGroupNone.empty())
			itemGroupNone = MTEXT("  <ItemGroup>\n") + itemGroupNone + MTEXT("  </ItemGroup>\n");
		if (!itemGroupXML.empty())
			itemGroupXML = MTEXT("  <ItemGroup>\n") + itemGroupXML + MTEXT("  </ItemGroup>\n");
		itemGroups = itemGroupImages + itemGroupNone + itemGroupXML;

		String projectFileTxt = strUtils::ConstructString(VS_PROJECT_TXT).arg(platformStr).arg(GuidToString(projectID)).arg(projectName).arg(itemGroups);

		std::wofstream projectFileStream;
		projectFileStream.open(Path(projectName + MTEXT(".vcxproj"), projectDir).AsString().c_str());
		if (!projectFileStream.is_open()) {
			msg(FATAL, MTEXT("Failed create Visual Studio project file."));
			return false;
		}
		projectFileStream << projectFileTxt.c_str();
		projectFileStream.close();
	}

	// Build and create solution (.sln) file.
	{
		String solutionFileTxt = strUtils::ConstructString(VS_SOLUTION_TXT).arg(projectName).arg(relativeProjectFileName).arg(GuidToString(projectID)).arg(configStr).arg(platformStr);

		std::wofstream solutionFile;
		solutionFile.open(Path(String(projectName) + MTEXT(".sln"), targetDir).AsString().c_str());
		if (!solutionFile.is_open()) {
			msg(FATAL, MTEXT("Failed create Visual Studio solution file."));
			return false;
		}
		solutionFile << solutionFileTxt.c_str();
		solutionFile.close();
	}

	return true;
}

// Search p for all matching files. p can be file or directory and wildcards are allowed!
void Publisher::_search(Set<Path> &files, Path p)
{
	if (!p.IsValid())
		return;
	if (p.IsDirectory())
		p = Path(Path(MTEXT("*")), p);
	Path dir = p.GetDirectory();
	WIN32_FIND_DATAA FindFileData;
	HANDLE hFind = FindFirstFileA(p.AsString().c_str(), &FindFileData);
	if (hFind != INVALID_HANDLE_VALUE) {
		for (BOOL b = TRUE; b; b = FindNextFileA(hFind, &FindFileData)) {
			String filename(FindFileData.cFileName);
			if (filename == MTEXT(".") || filename == MTEXT(".."))
				continue; // no dot and dotdot!
			if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				_search(files, Path::Dir(filename, dir));
			else
				files.insert(Path(filename, dir));
		}
		FindClose(hFind);
	}
}


// This function can be called from multiple threads....
void _projectFileConverter(const PublishSettings &p, String originalRelativePath, Path startProjectFile, Path projectDir, Path libDir, ProjectDependencies &third, ProjectDependencies &packets, Guid signature, std::atomic_uint32_t &warnings, const Environment &env)
{
	Path originalAbsolutePath = env.ResolveDocumentPath(originalRelativePath, startProjectFile);

	if (!originalAbsolutePath.IsFile()) {
		msg(WARN, MTEXT("File not found: ") + originalRelativePath + MTEXT("."));
		warnings++;
		return;
	}
	bool isInProjectFolder = env.IsPathInsideProjectRootFolder(originalAbsolutePath);
	bool isInLibFolder = !isInProjectFolder && env.IsPathInsideLibraryFolder(originalAbsolutePath);

	if (!(isInProjectFolder || isInLibFolder)) {
		msg(WARN, MTEXT("This file is excluded because it is not in the project or library directory: ") + originalAbsolutePath.AsString() + MTEXT("."));
		warnings++;
		return;
	}
	if (!DocumentFileTypes::CheckIfProjectFileExist(originalAbsolutePath)) { // Project file exist? NOTE: The project file could have a different file extention!
		msg(WARN, MTEXT("Could not find project file: ") + originalAbsolutePath.AsString() + MTEXT("."));
		warnings++;
		return;
	}
	DocumentLoader *loader = DocumentFileTypes::CreateLoader(originalAbsolutePath);
	if (!loader) {
		msg(WARN, MTEXT("Failed to load project file: ") + originalAbsolutePath.AsString() + MTEXT("."));
		warnings++;
		return;
	}
	if (loader->OpenFile(originalAbsolutePath)) { // Load project file.
		Document *doc = mmnew Document(&DefaultClassFactory);
		doc->SetFileName(originalAbsolutePath);
		if (loader->LoadDocument(doc)) {
			if (!p.includeAll) { // If not including ALL chips and 3rds: Search for dependencies.
				for (size_t i = 0; i < doc->GetClasss().size(); i++)
					doc->GetClasss()[i]->AddDependencies(packets, third);
			}
			if (!p.copyProject) { // If not copying project: Save as binary and encrypt.
				Path newAbsolutePath;
				if (isInLibFolder)
					newAbsolutePath = Path::File(Environment::MakeLibraryPathAbsolute(originalRelativePath, libDir)).ChangeFileExtention(DocumentFileTypes::FILE_TYPES[(uint32)DocumentFileTypes::FileType::BINARY].ext);
				else
					newAbsolutePath = Path::File(originalRelativePath, projectDir).ChangeFileExtention(DocumentFileTypes::FILE_TYPES[(uint32)DocumentFileTypes::FileType::BINARY].ext);
				if (!newAbsolutePath.CheckExistence()) {
					DocumentSaver *saver = DocumentFileTypes::CreateSaver(newAbsolutePath);
					if (saver)
						saver->SetSaveEditorData(false);
					saver->SetCompressionLevel((DocumentCompressionLevel)p.projectFileCompression); // TODO: !!
					if (!(saver && saver->Initialize() && saver->SaveClasss(doc) && saver->SaveToFile(newAbsolutePath, DocumentEncryptionLevel::ENCLEVEL2, signature))) { // encrypt at ENCLEVEL2 (not openable in editor!)
						msg(WARN, MTEXT("Failed to save project file to ") + newAbsolutePath.AsString() + MTEXT("."));
						warnings++;
					}
					DocumentFileTypes::Free(saver);
				}
				else {
					msg(WARN, MTEXT("The published project already contain a file called ") + newAbsolutePath.GetRelativePath(projectDir).AsString() + MTEXT("."));
					warnings++;
				}
			}
		}
		else {
			msg(WARN, MTEXT("Failed to load project file: ") + originalAbsolutePath.AsString() + MTEXT("."));
			warnings++;
		}
		mmdelete(doc);
	}
	else {
		msg(WARN, MTEXT("Failed to load project file: ") + originalAbsolutePath.AsString() + MTEXT("."));
		warnings++;
	}
	DocumentFileTypes::Free(loader);
}

// dir is the directory we are copying the files to.
// platformAppDir is the directory where binaries for the selected platofrm is found.
bool Publisher::_publishWorker(Path dir, Path platformAppDir, Path startProjectFile, const PublishSettings &p, const Environment& env)
{
	// BIG NOTE: Placing chips in a subdirectory when publising to WP8 causes problems because
	// of the way WP8 apps searches for dependent DLLs. Eg: StdChips.dll will load fine,
	// but the D3DChips.dll will not because it depends on StdChips.dll which is not found
	// when located in a subdirectory (If not already loaded). This is because the functions to 
	// spcecify which directories ([Set/Add]DllDirectory) to search for DLLs are not available in WP8. 
	// Therefore, when publishing to WP8, place all chips and thirds in the root folder. 
	// It's messy, but currently I have no other solution to this issue. :-/
	Path thirdDir = Path::Dir(Path(MTEXT("3rd")), dir);
	Path chipsDir = Path::Dir(Path(MTEXT("Chips")), dir);
	Path projectDir = Path::Dir(Path(MTEXT("Project")), dir);
	Path libDir = Path::Dir(Path(MTEXT("Libraries")), dir);

	// if/else: See the BIG NOTE above!
	if (p.platform != PublishSettings::WP8_X86 && p.platform != PublishSettings::WP8_ARM) {
		if (!thirdDir.CreateDir()) {
			msg(FATAL, MTEXT("Failed to create directory: ") + thirdDir.AsString() + MTEXT("."));
			return false;
		}
		if (!chipsDir.CreateDir()) {
			msg(FATAL, MTEXT("Failed to create directory: ") + chipsDir.AsString() + MTEXT("."));
			return false;
		}
	}
	else {
		thirdDir = chipsDir = dir;
	}
	
	if (!projectDir.CreateDir()) {
		msg(FATAL, MTEXT("Failed to create directory: ") + projectDir.AsString() + MTEXT("."));
		return false;
	}

	for (const auto &n : p.projectFiles) {
		Path newFile;
		if (Environment::IsLibraryPath(n)) {
			newFile = Environment::MakeLibraryPathAbsolute(n, libDir);
		}
		else
			newFile = Path::File(n, projectDir);
		newFile.GetParentDirectory().CreateDir(true); // make sure directory exist!
	}

	ProjectDependencies third(engine->GetThirdDepsDirectory(), p.platform);
	ProjectDependencies packets(engine->GetChipManager()->GetChipsDirectory(), p.platform);

	if (p.includeAll) {
		third.AddDependency(MTEXT("*"));
		packets.AddDependency(MTEXT("*"));
	}

	// If not raw copy of project files OR not including ALL chips and 3rd dependencies.
	if (!p.copyProject || !p.includeAll) {
		// The signature is to ensure that published documents can't be opened togehter with 3rd-party malicious documents in a viewer (that can be used to extract content of encrypted document!)
		Guid signature; 
		GenerateGuid(signature);

		std::atomic_uint32_t warnings = 0; // Use atomic to make sure warnings are counted correctly!

#if 1 // Lets save a couple of seconds and use multiple threads... :)
		List<String> pp;
		for (const auto &n : p.projectFiles)
			pp.push_back(n);
		
		concurrency::parallel_for(size_t(0), pp.size(), [&](size_t i) 
		{
			_projectFileConverter(p, pp[i], startProjectFile, projectDir, libDir, third, packets, signature, warnings, env);
		});
#else
		for (Set<String>::cNode n = p.projectFiles.front(); n.valid(); n++) { // Iterate project files
			_projectFileConverter(p, n.get(), startProjectFile, projectDir, libDir, third, packets, signature, warnings, env);
		}
#endif
		_warnings += warnings;
	}

	// Copy project files
	if (p.copyProject) { // Raw copy of project files?
		for (const auto &n : p.projectFiles) {
			Path originalFile = env.ResolveDocumentPath(n, startProjectFile);
			if (!originalFile.IsValid() || !DocumentFileTypes::CheckIfProjectFileExist(originalFile)) {
				msg(WARN, MTEXT("Could not find project file: ") + (originalFile.IsValid() ? originalFile.AsString() : n) + MTEXT("."));
				_warnings++;
				continue;
			}	
			Path newFile;

			if (Environment::IsLibraryPath(n))
				newFile = Environment::MakeLibraryPathAbsolute(n, libDir);
			else
				newFile = Path::File(n, projectDir);
			if (!originalFile.Copy(newFile)) {
				msg(WARN, MTEXT("Failed to copy project file: ") + originalFile.AsString() + MTEXT("."));
				_warnings++;
			}
		}
	}
	else 
		startProjectFile = startProjectFile.ChangeFileExtention(DocumentFileTypes::FILE_TYPES[(uint32)DocumentFileTypes::FileType::BINARY].ext);

	auto cpy = [=](Path from, Path to, m3d::MessageSeverity s = FATAL) -> bool
	{ 
		if (!from.CheckExistence())
			msg(s, strUtils::ConstructString(MTEXT("File does not exist: \'%1\'.")).arg(from.AsString()));
		else if (!from.Copy(to))
			msg(s, strUtils::ConstructString(MTEXT("Failed to copy file \'%1\' to \'%2\'.")).arg(from.AsString()).arg(to.AsString()));
		else
			return true;
		return false;
	};

	// Copy standard files
	{
		if (!cpy(Path::File(FILE_VIEWER_EXE, platformAppDir), Path::File(FILE_VIEWER_EXE, dir)))
			return false;
		if (!cpy(Path::File(FILE_CORE_DLL, platformAppDir), Path::File(FILE_CORE_DLL, dir)))
			return false;
		if (!cpy(Path::File(FILE_ENGINE_DLL, platformAppDir), Path::File(FILE_ENGINE_DLL, dir)))
			return false;
		if (!cpy(Path::File(FILE_LIBXML2_DLL, platformAppDir), Path::File(FILE_LIBXML2_DLL, dir)))
			return false;
		if (!cpy(Path::File(FILE_ZLIB_DLL, platformAppDir), Path::File(FILE_ZLIB_DLL, dir)))
			return false;
		if (!cpy(Path::File(FILE_LIBICONV, platformAppDir), Path::File(FILE_LIBICONV, dir)))
			return false;
		if (!cpy(Path::File(FILE_LZMA, platformAppDir), Path::File(FILE_LZMA, dir)))
			return false;
//		if (!cpy(Path::File(FILE_LIBCHARSET, platformAppDir), Path::File(FILE_LIBCHARSET, dir)))
//			return false;
		if (!cpy(Path::File(FILE_SDL2, platformAppDir), Path::File(FILE_SDL2, dir)))
			return false;

		if (p.platform == PublishSettings::WD_X64 || p.platform == PublishSettings::WD_X86) {
#ifdef FILE_MSVCR
		if (!cpy(Path::File(FILE_MSVCR, platformAppDir), Path::File(FILE_MSVCR, dir), WARN))
			_warnings++;
#endif 
#ifdef FILE_MSVCP
		if (!cpy(Path::File(FILE_MSVCP, platformAppDir), Path::File(FILE_MSVCP, dir), WARN))
			_warnings++;
#endif
#ifdef FILE_CONCRT140
		if (!cpy(Path::File(FILE_CONCRT140, platformAppDir), Path::File(FILE_CONCRT140, dir), WARN))
			_warnings++;
#endif
#ifdef FILE_VCCORLIB140
		if (!cpy(Path::File(FILE_VCCORLIB140, platformAppDir), Path::File(FILE_VCCORLIB140, dir), WARN))
			_warnings++;
#endif
#ifdef FILE_UCRTBASE
		if (!cpy(Path::File(FILE_UCRTBASE, platformAppDir), Path::File(FILE_UCRTBASE, dir), WARN))
			_warnings++;
#endif

#ifdef FILE_API_MS_
			{
				Path p = Path::File(FILE_API_MS_, platformAppDir);

				WIN32_FIND_DATA FindFileData;
				HANDLE hFind = FindFirstFileEx(p.AsString().c_str(), FindExInfoStandard, &FindFileData, FindExSearchNameMatch, NULL, 0);

				if (hFind != INVALID_HANDLE_VALUE) {
					do {
						Path q = Path::File(FindFileData.cFileName, platformAppDir);
						if (!cpy(q, Path::File(q.GetName(), dir), WARN))
							_warnings++;
					}
					while (FindNextFile(hFind, &FindFileData) != 0);
					FindClose(hFind);
				}
				else {
					msg(WARN, String(MTEXT("No such files exist: \'%1\'.")).arg(p.AsString()));
					_warnings++;
				}
			}
#endif
		}
	}

	// Copy chips
	{
		Path platformChipsDir = Path::Dir(Path(MTEXT("Chips")), platformAppDir);
		Set<Path> files;
		const Set<Path> &a = packets.GetDependencies();
		for (const auto &n : a)
			_search(files, Path::File(n.GetRelativePath(packets.GetBaseDirectory()), platformChipsDir));
		for (const auto &n : files) {
			if (!cpy(n, Path::File(n.GetName(), Path::File(n.GetRelativePath(platformChipsDir).AsString(), chipsDir)))) {
				return false;
			}
		}		
	}

	// Copy 3rd dependencies
	{
		Path platform3rdDir = Path::Dir(Path(MTEXT("3rd")), platformAppDir);
		Set<Path> files;
		const Set<Path> &a = third.GetDependencies();
		for (const auto &n : a)
			_search(files, Path(n.GetRelativePath(third.GetBaseDirectory()), platform3rdDir));
		for (const auto &n : files) {
			if (!cpy(n, Path::File(n.GetName(), Path::File(n.GetRelativePath(platform3rdDir).AsString(), thirdDir)))) {
				msg(WARN, MTEXT("Failed to copy 3rd-party dependency: ") + n.AsString() + MTEXT("."));
				_warnings++;
			}
		}		
	}

	// Do stuff specific to Windows Phone.
	if (p.platform == PublishSettings::WP8_X86 || p.platform == PublishSettings::WP8_ARM) {
		Path::File(Path(MTEXT("Assets\\ApplicationIcon.png")), platformAppDir).Copy(Path(Path(MTEXT("ApplicationIcon.png")), dir));
		Path::File(Path(MTEXT("Assets\\Tiles\\FlipCycleTileSmall.png")), platformAppDir).Copy(Path(Path(MTEXT("FlipCycleTileSmall.png")), dir));
		Path::File(Path(MTEXT("Assets\\Tiles\\FlipCycleTileMedium.png")), platformAppDir).Copy(Path(Path(MTEXT("FlipCycleTileMedium.png")), dir));

		Path wp8Manifest = Path::File(Path(MTEXT("WMAppManifest.xml")), dir);
		
		xmlDocPtr doc = xmlReadFile(Path::File(Path(MTEXT("WMAppManifest.xml")), platformAppDir).AsString().c_str(), nullptr, 0);
		
		xmlNodePtr nDeployment = doc->children; // verify Deployment node!
		xmlNodePtr nApp = xmlGetChild(nDeployment, BAD_CAST "App");
		xmlSetProp(nApp, BAD_CAST "ProductID", BAD_CAST GuidToString(p.productID).c_str());
		xmlSetProp(nApp, BAD_CAST "Title", BAD_CAST p.title.c_str());
		xmlSetProp(nApp, BAD_CAST "Version", BAD_CAST (strUtils::fromNum(p.version.elements[0]) + "." + strUtils::fromNum(p.version.elements[1]) + "." + strUtils::fromNum(p.version.elements[2]) + "." + strUtils::fromNum(p.version.elements[3])).c_str());
		xmlSetProp(nApp, BAD_CAST "Author", BAD_CAST p.publisher.c_str());
		xmlSetProp(nApp, BAD_CAST "Description", BAD_CAST p.description.c_str());
		xmlSetProp(nApp, BAD_CAST "Publisher", BAD_CAST p.publisher.c_str());
		xmlSetProp(nApp, BAD_CAST "PublisherID", BAD_CAST GuidToString(p.publisherID).c_str());

		xmlSaveFormatFile(wp8Manifest.AsString().c_str(), doc, 1);
	}

	// Create startup file (This is a file named 'startup' in the same directory as the Viewer-app)
	std::ofstream startup;
	startup.open(Path(Path(MTEXT("startup")), dir).AsString().c_str());
	if (!startup.is_open()) {
		msg(FATAL, MTEXT("Failed create startup file."));
		return false;
	}
	String s = MTEXT(".\\Project\\") + startProjectFile.GetName();
	startup << s.c_str();
	startup.close();

	return true; // SUCCESS!
}

bool Publisher::_archiveWorker(Path target, Path appDir, Path dir, Path zipDir, const PublishSettings &p)
{
	String cmd; // The command line for executing 7-zip

	// 0(none),1(fastest),3,5(normal),7,9(ultra)
	int32 compression = 0;
	switch (p.compression) {
		case PublishSettings::NONE: compression = 0; break;
		case PublishSettings::LOW: compression = 1; break;
		case PublishSettings::NORMAL: compression = 5; break;
		case PublishSettings::HIGH: compression = 9; break;
		default: compression = 5; break;
	}

	// Set up 7-zip 
	{
		if (!Path::File(Path(MTEXT("Publish\\7zG.exe")), appDir).Copy(Path::File(Path(MTEXT("7zG.exe")), zipDir), true)) {
			msg(FATAL, MTEXT("Failed to copy 7zG.exe."));
			return false;
		}
		if (!Path::File(Path(MTEXT("Publish\\7z.dll")), appDir).Copy(Path::File(Path(MTEXT("7z.dll")), zipDir), true)) {
			msg(FATAL, MTEXT("Failed to copy 7z.dll"));
			return false;
		}

		// Just a few words about the .sfx files:
		// Im using a specialt built .sfx (using the 7-zip sdk). It is very like the original one except:
		// - The temp. file name is prefixed F3D
		// - a special command line argument is added:  __exeFile=NameOfArchiveExe. This is to make is possible to find the original file the user executed. 
		// Note: Command line arguments passed to the archive is forwarded to the program executed when extracting is done.
		// - Built for x64 AND x86. (I could do with x86 only, but its ok to have pure x64!)
		// Icon is changed!

		// Publishing to desktop application? Then create a self extracting archive!
		if (p.platform == PublishSettings::WD_X64 || p.platform == PublishSettings::WD_X86) {
			// Select the right .sfx file. x86 or x64!			
			if (!Path::File(Path(p.platform == PublishSettings::WD_X64 ? MTEXT("Publish\\SFXNoConfig_x64.sfx") : MTEXT("Publish\\SFXNoConfig_x86.sfx")), appDir).Copy(Path::File(Path(MTEXT("M3D.sfx")), zipDir), true)) {
				msg(FATAL, MTEXT("Failed to copy SFXNoConfig.sfx"));
				return false;
			}

			// Create config file for sfx
			std::ofstream confFile;
			confFile.open(Path(Path(MTEXT("M3D.sfx")), zipDir).AsString().c_str(), std::ios_base::binary | std::ios_base::app);
			if (confFile.is_open()) {
				String startCG;// = MTEXT("\\\")%%T\\Project\\" + startGroup.GetFileName() + MTEXT("\\\")";  Using startup file instead!
				startCG = strUtils::replace(startCG, MTEXT("\\"), MTEXT("\\\\"));
				String conf = MTEXT(";!@Install@!UTF-8!\r\nTitle=\"") + p.title + MTEXT("\"\r\nRunProgram=\"SnaXViewer.exe ") + startCG + MTEXT(" \\\"__sfxFile=%%S\\\"\"\r\n;!@InstallEnd@!\r\n");
				// SetEnvironment="F3DSFX=%%S"  this can be used instead of __sfxFile?
				//IMPORTANT! Certain two-symbol sequences in the Value are parsed as follows:
				//\\ = \
				//\" = "
				//\n = caret return
				//\t = tabulation (TAB)
				Char bom[4] = {Char(0xEF), Char(0xBB), Char(0xBF), Char(0x00)};
				confFile << bom << conf.c_str();
				confFile.close();
			}
			else {
				msg(FATAL, MTEXT("Failed create sfx configuration."));
				return false;
			} 
			cmd = zipDir.AsString() + MTEXT("7zG.exe a -sfxM3D.sfx \"") + target.AsString() + MTEXT("\" \"") + dir.AsString() + MTEXT("*\" -mx=") + strUtils::fromNum(compression);
		}
		else { // The other option is currently XAP-archive for WP8
			cmd = zipDir.AsString() + MTEXT("7zG.exe a -tzip ") + target.AsString() + MTEXT(" ") + dir.AsString() + MTEXT("* -mx=") + strUtils::fromNum(compression);
		}
	}

	// Create the archive!
	{
		STARTUPINFOA siStartupInfo;
		PROCESS_INFORMATION piProcessInfo;

		memset(&siStartupInfo, 0, sizeof(siStartupInfo));
		memset(&piProcessInfo, 0, sizeof(piProcessInfo));

		siStartupInfo.cb = sizeof(siStartupInfo);
			
		if(CreateProcessA(0, (Char*)cmd.c_str(), 0, 0, FALSE, CREATE_DEFAULT_ERROR_MODE, 0, zipDir.AsString().c_str(), &siStartupInfo, &piProcessInfo)) {
			// Wait until application has terminated
			WaitForSingleObject(piProcessInfo.hProcess, INFINITE);
			DWORD returnCode;
			if (GetExitCodeProcess(piProcessInfo.hProcess, &returnCode)) {
				// 0 No error 
				// 1 Warning (Non fatal error(s)). For example, one or more files were locked by some other application, so they were not compressed. 
				// 2 Fatal error 
				// 7 Command line error 
				// 8 Not enough memory for operation 
				// 255 User stopped the process 
				switch (returnCode) 
				{
				case 0:
					return true; // SUCCESS!
					break;
				case 1:
					msg(WARN, MTEXT("7-Zip reported one or more warnings. Project did probably not publish correctly."));
					break;
				case 2:
					msg(FATAL, MTEXT("7-Zip reported a nonspecified fatal error. Publish failed!"));
					break;
				case 7:
					msg(FATAL, MTEXT("7-Zip reported a command line error. Publish failed!"));
					break;
				case 8:
					msg(FATAL, MTEXT("7-Zip ran out of memory. Publish failed!"));
					break;
				case 255:
					msg(WARN, MTEXT("User cancelled publishing."));
					break;
				default:
					msg(FATAL, strUtils::format(MTEXT("7-Zip has exited with code %1. Project did probably not publish correctly."), returnCode));
					break;
				}
			}
			else {
				msg(FATAL, MTEXT("7-Zip finished unexpectedly. Project did probably not publish correctly."));
			}
		}
		else {
			msg(FATAL, MTEXT("Failed to start 7-Zip. Publish failed!"));
		}
	}
	return false; // FAIL!
}



