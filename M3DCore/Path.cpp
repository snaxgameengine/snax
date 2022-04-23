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
#include "Path.h"
#include "PlatformDef.h"
#include "Containers.h"
#include <cassert>
#include <shellapi.h>

using namespace m3d;

namespace m3d
{

	List<String> _splitAndCleanPath(String path)
	{
		path = strUtils::trim(strUtils::trim(strUtils::replaceChar(path, MCHAR('/'), MCHAR('\\'))), MCHAR('\t'));
		List<String> res;
		size_t offset = 0;
		size_t p;
		bool isDir = false;
		while (true) {
			isDir = true;
			p = path.find_first_of(MCHAR('\\'), offset);
			if (p == offset); // \\ means do nothing
			else {
				String s = strUtils::trim(strUtils::trim(path.substr(0, p)), MCHAR('\t'));
				if (s == MTEXT(".")); // do nothing
				else if (s == MTEXT("..")) {
					if (res.size() == 1 && res[0].size() == 2 && res[0][1] == MCHAR(':'))
						return List<String>(); // Invalid!
					if (res.empty() || res[res.size() - 1] == MTEXT(".."))
						res.push_back(s);
					else
						res.pop_back();
				}
				else if (s.empty()) {
					if (p != String::npos)
						return List<String>(); // eg bla\bla\   \bla -> invalid path!
				}
				else if (s.size() == 2 && (s[0] >= MCHAR('a') && s[0] <= MCHAR('z') || s[0] >= MCHAR('A') && s[0] <= MCHAR('Z')) && s[1] == MCHAR(':')) {
					if (offset == 0)
						res.push_back(s); // drive, eg c:
					else
						return List<String>(); // eg bla\bla\c:\bla -> invalid path!
				}
				else {
					// check for invalid chars...
					res.push_back(s);
					isDir = false;
				}
			}
			if (p == String::npos)
				break;
			path = path.substr(p + 1);
		}
		if (isDir)
			res.push_back(String());
		return res;
	}

	String _cleanPath2(String path)
	{
		auto a = _splitAndCleanPath(path);
		if (a.empty())
			return String();
		String r = (a[0].size() == 2 && a[0][1] == MCHAR(':')) ? a[0] : (MTEXT(".\\") + a[0]);
		for (auto i = 1; i < a.size(); i++)
			r += MTEXT("\\") + a[i];
		return r;
	}

#if defined( DEBUG ) || defined( _DEBUG )
	int32 __PathUnitTest()
	{
		Path p1 = Path(MTEXT("c:\\a/b  \\\\c\\e\\.\\..\\..\\c\\d.txt"));
		Path p2 = Path(MTEXT("  c:\\a\\b\\c/   "));
		Path p3 = Path(MTEXT("e/f/g.txt"));
		Path p4 = Path(MTEXT("a\\..\\..\\c\\d.txt"));
		Path p5 = Path(MTEXT("c:\\a\\b\\d\\e\\"));
		Path p6 = p1.GetRelativePath(p5);
		Path p7 = p5.GetRelativePath(p1);
		bool b1 = p1.AsString() == MTEXT("c:\\a\\b\\c\\d.txt");
		bool b2 = p2.AsString() == MTEXT("c:\\a\\b\\c\\");
		bool b3 = p3.AsString() == MTEXT(".\\e\\f\\g.txt");
		bool b4 = p4.AsString() == MTEXT(".\\..\\c\\d.txt");
		bool b5 = p6.AsString() == MTEXT(".\\..\\..\\c\\d.txt");
		bool b6 = p7.AsString() == MTEXT(".\\..\\d\\e\\");
		bool b7 = Path(MTEXT("c:\\a\\b\\c\\")).GetRelativePath(Path(MTEXT("c:\\a\\b\\c"))).AsString() == MTEXT(".\\c\\");
		bool b8 = Path(MTEXT("c:\\a\\b\\c")).GetRelativePath(Path(MTEXT("c:\\a\\b\\c\\"))).AsString() == MTEXT(".\\..\\c");
		bool b9 = Path(MTEXT("c:\\a")).GetRelativePath(Path(MTEXT("d:\\a"))).AsString() == MTEXT("c:\\a");
		bool ba = Path(MTEXT("c:\\a\\b\\c\\d")).GetRelativePath(Path(MTEXT("c:\\a\\"))).AsString() == MTEXT(".\\b\\c\\d");
		bool bb = Path::File(Path(MTEXT("a\\b\\c")), Path(MTEXT(".\\d\\e"))).AsString() == MTEXT(".\\d\\a\\b\\c");
		bool bc = Path::Dir(Path(MTEXT("a\\b\\c")), Path(MTEXT(".\\d\\e\\"))).AsString() == MTEXT(".\\d\\e\\a\\b\\c\\");
		assert(b1 && b2 && b3 && b4 && b5 && b6 && b7 && b8 && b9 && ba && bb && bc);
		return 0;
	}
	static int32 __PathUnitTestV = __PathUnitTest();
#endif
}



Path::Path(String name)
{
	_path = _cleanPath2(name);
}

Path::Path(Char*name)
{
	_path = _cleanPath2(name);
}

Path::Path(Path p, Path relativeTo)
{
	if (relativeTo.IsValid() && p.IsRelative())
		_path = _cleanPath2(relativeTo.GetDirectory().AsString() + p._path);
	else
		_path = p._path;
}

Path Path::File(Path name, Path relativeTo)
{
	Path p(name, relativeTo);
	if (p.IsDirectory())
		return Path(); // original name specified a directory: Not valid!
	return p;
}

Path Path::Dir(Path name, Path relativeTo)
{
	Path p(name, relativeTo);
	if (p.IsFile())
		p._path += MTEXT("\\"); // this will make it a directory!
	return p; // p will now be invalid or a directory (not a file!)
}

Path Path::TempDir()
{
#ifdef WINDESKTOP
	Char tmppath[MAX_PATH] = {MCHAR('\0')};
	if (!::GetTempPathA(MAX_PATH, tmppath))
		return Path();
	return Dir(tmppath);
#else
	return Path(); // Not implemented yet.
#endif
}

Path Path::CurrentDir()
{
#ifdef WINDESKTOP
	Char dir[MAX_PATH] = {MCHAR('\0')};
	if (!::GetCurrentDirectoryA(MAX_PATH, dir))
		return Path();
	return Dir(dir);
#else
	return Dir(String(MTEXT(".\\"))); // App root!
#endif
}

bool Path::operator==(const Path &rhs) const
{
	return strUtils::compareNoCase(_path, rhs._path) == 0;//CompareStringOrdinal(_path.c_str(), (int32)_path.length(), rhs._path.c_str(), (int32)rhs._path.length(), TRUE) == CSTR_EQUAL;
}

bool Path::operator!=(const Path &rhs) const
{
	return strUtils::compareNoCase(_path, rhs._path) != 0;//CompareStringOrdinal(_path.c_str(), (int32)_path.length(), rhs._path.c_str(), (int32)rhs._path.length(), TRUE) != CSTR_EQUAL;
}

bool Path::operator<(const Path &rhs) const
{
	return strUtils::compareNoCase(_path, rhs._path) < 0;//CompareStringOrdinal(_path.c_str(), (int32)_path.length(), rhs._path.c_str(), (int32)rhs._path.length(), TRUE) == CSTR_LESS_THAN;
}

bool Path::IsValid() const
{ 
	return _path.length() > 0; 
}

bool Path::IsDirectory() const 
{ 
	return IsValid() && _path[_path.size() - 1] == MCHAR('\\'); 
}

bool Path::IsDetermined() const
{
	return IsValid() && _path.find_first_of(MTEXT("\\..\\")) == String::npos;
}

bool Path::IsRoot() const
{
	return IsDirectory() && _path.find_first_of(MCHAR('\\')) == _path.find_last_of(MCHAR('\\'));
}

bool Path::IsDrive() const
{
	return IsValid() && _path.length() == 3 && _path[1] == MCHAR(':');
}

bool Path::IsFile() const
{
	return IsValid() && _path[_path.size() - 1] != MCHAR('\\'); 
}

bool Path::IsRelative() const
{
	return IsValid() && _path[0] == MCHAR('.');
}

bool Path::IsAbsolute() const
{
	return IsValid() && _path[0] != MCHAR('.');
}

Path Path::GetAbsolute() const
{
	return IsAbsolute() ? *this : Path(*this, CurrentDir());
}

Path Path::GetDrive() const
{
	return IsAbsolute() ? Path(_path.substr(0, 3)) : Path();
}

Path Path::GetDirectory() const
{
	if (IsDirectory())
		return *this;
	if (IsFile())
		return Path(_path.substr(0, _path.find_last_of(MCHAR('\\')) + 1));
	return Path();
}

Path Path::GetParentDirectory() const
{
	if (IsFile())
		return Path(_path.substr(0, _path.find_last_of(MCHAR('\\')) + 1));
	if (IsDirectory() && !IsRoot())
		return Path(_path.substr(0, _path.find_last_of(MCHAR('\\'), _path.size() - 2) + 1));
	return Path();
}

String Path::GetName() const
{
	if (IsFile())
		return _path.substr(_path.find_last_of(MCHAR('\\')) + 1);
	if (IsDirectory() && !IsRoot())
		return _path.substr(_path.find_last_of(MCHAR('\\'), _path.size() - 2) + 1, _path.size() - _path.find_last_of(MCHAR('\\'), _path.size() - 2) - 2);
	return MTEXT("");
}

String Path::GetFileExtention() const
{
	if (!IsFile())
		return MTEXT("");
	String s = GetName();
	if (s.find_last_of('.') == String::npos)
		return MTEXT("");
	return s.substr(s.find_last_of('.') + 1);
}

String Path::GetFileNameWithoutExtention() const
{
	if (!IsFile())
		return GetName();
	String s = GetName();
	return s.substr(0, s.find_last_of('.'));
}

bool Path::IsInSubFolder(Path checkIfInSubFolderToThis) const
{
	if (!IsValid() || !checkIfInSubFolderToThis.IsValid() || IsAbsolute() != checkIfInSubFolderToThis.IsAbsolute())
		return false;

	String relDir = checkIfInSubFolderToThis.GetDirectory().AsString();
	String myDir = GetDirectory().AsString();

	return myDir.size() >= relDir.size() && strUtils::compareNoCase(myDir.substr(0, relDir.size()), relDir) == 0;
}

Path Path::GetRelativePath(Path relativeTo) const
{
	if (!IsValid() || !relativeTo.IsValid() || !IsAbsolute() || !relativeTo.IsAbsolute())
		return Path();

	relativeTo = relativeTo.GetDirectory();

	List<String> elements0, elements1;
	{
		size_t offset = 0;
		while (true) {
			size_t i = _path.find_first_of(MCHAR('\\'), offset);
			if (i == String::npos) {
				elements0.push_back(_path.substr(offset));
				break;
			}
			elements0.push_back(_path.substr(offset, i - offset));
			offset = i + 1;
		}
	}
	{
		size_t offset = 0;
		while (true) {
			size_t i = relativeTo._path.find_first_of(MCHAR('\\'), offset);
			if (i == String::npos) {
				elements1.push_back(relativeTo._path.substr(offset));
				break;
			}
			elements1.push_back(relativeTo._path.substr(offset, i - offset));
			offset = i + 1;
		}
	}

	size_t i = 0;
	for (; i < elements0.size() && i < elements1.size(); i++) {
		if (strUtils::compareNoCase(elements0[i], elements1[i]) != 0) 
			break;
	}
	if (i == 0)
		return *this; // Different drives...

	String res = MTEXT(".");

	if (i == elements0.size() && (i + 1) == elements1.size())
		i--;

	for (size_t j = i; j < elements1.size() - 1; j++) {
		res += MTEXT("\\..");
	}
	for (size_t j = i; j < elements0.size(); j++) {
		res += MTEXT("\\") + elements0[j];
	}

	return Path(res);
}

bool Path::CompareName(String name) const
{
	return strUtils::compareNoCase(GetName(), name) == 0;
}

bool Path::CompareFileExtention(String ext) const
{
	return strUtils::compareNoCase(GetFileExtention(), ext) == 0;
}

Path Path::ChangeFileExtention(String ext) const
{
	return Path(GetFileNameWithoutExtention() + MTEXT(".") + ext + (IsFile() ? MTEXT("") : MTEXT("\\")), GetDirectory()); // <=== may be improved!!!
}

Path Path::ChangeBaseName(String baseName) const
{
	return Path(baseName + MTEXT(".") + GetFileExtention() + (IsFile() ? MTEXT("") : MTEXT("\\")), GetDirectory()); // <=== may be improved!!!
}

bool Path::ContainsValidCharactersOnly() const
{
	static const String inv = MTEXT("|*?<>:\"");
	if (!IsValid())
		return false;
	for (size_t i = 0; i < _path.size(); i++)
		if (_path[i] < 32)
			return false;
	if (_path.substr(2).find_first_of(inv) != String::npos)
		return false;
	return true;
}


bool Path::Copy(Path newPath, bool failIfExists) const
{
#ifdef WINDESKTOP
	return CopyFileA(AsString().c_str(), newPath.AsString().c_str(), failIfExists ? TRUE : FALSE) == TRUE;
#else
	return false;
#endif
}

bool Path::Move(Path newPath) const
{
#ifdef WINDESKTOP
	return MoveFileA(AsString().c_str(), newPath.AsString().c_str()) == TRUE;
#else
	return false;
#endif
}

bool Path::CreateDir(bool recursive) const
{
	if (!IsDirectory())
		return false;
	if (recursive) {
		Path p = GetParentDirectory();
		if (!p.CheckExistence())
			if (!p.CreateDir(true))
				return false;
	}
	DWORD a = GetFileAttributesA(_path.c_str());
	if (a == INVALID_FILE_ATTRIBUTES)
		return CreateDirectoryA(AsString().c_str(), nullptr) == TRUE;
	return (a & FILE_ATTRIBUTE_DIRECTORY) != 0;
}

bool Path::CheckExistence() const
{
#ifdef WINDESKTOP
	return GetFileAttributesA(_path.c_str()) != INVALID_FILE_ATTRIBUTES;
#else
	 WIN32_FILE_ATTRIBUTE_DATA nfo;
	return GetFileAttributesEx(_path.c_str(), GetFileExInfoStandard, &nfo) == TRUE;
#endif
}

bool __deleteDir(Path name)
{
#ifdef WINDESKTOP
	// string got to be float64 null terminated!
	Char*tmp = new Char[name.AsString().size() + 2];
	std::memcpy(tmp, name.AsString().c_str(), sizeof(Char) * (name.AsString().size() + 1));
	tmp[name.AsString().size() + 1] = MCHAR('\0');
	SHFILEOPSTRUCTA fileop;
	fileop.hwnd   = NULL;    // no status display
	fileop.wFunc  = FO_DELETE;  // delete operation
	fileop.pFrom  = tmp;  // source file name as float64 null terminated string
	fileop.pTo    = NULL;    // no destination needed
	fileop.fFlags = FOF_NOCONFIRMATION|FOF_SILENT|FOF_ALLOWUNDO;  // do not prompt the user
	fileop.fAnyOperationsAborted = FALSE;
	fileop.lpszProgressTitle     = NULL;
	fileop.hNameMappings         = NULL;
	int32 r = SHFileOperationA(&fileop);
	delete[] tmp;
	return r == 0;
#else
	return false;
#endif
}

bool Path::Delete() const
{
	if (IsDirectory())
		return __deleteDir(*this);
	if (IsFile())
		return DeleteFileA(AsString().c_str()) == TRUE;
	return false;
}
