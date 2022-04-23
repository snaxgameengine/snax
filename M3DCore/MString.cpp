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
#include "MString.h"


using namespace m3d;


wchar_t* strUtils::widen(wchar_t* output, int32 outputSize, const Char* input)
{
	MultiByteToWideChar(CP_UTF8, 0, input, -1, output, outputSize);
	return output;
}

Char* strUtils::narrow(Char* output, int32 outputSize, const wchar_t* input)
{
	WideCharToMultiByte(CP_UTF8, 0, input, -1, output, outputSize, NULL, NULL);
	return output;
}

String strUtils::narrow2(const wchar_t* input)
{
	if (!input)
		return String();
	size_t len = ::wcslen(input);
	if (len <= 0)
		return String();
	std::unique_ptr<Char[]> buffer(new Char[len + 1]);
	return String(narrow(buffer.get(), int32(len + 1), input));
}

String strUtils::toLower(const String& string)
{
	String ret = string;
	std::transform(ret.begin(), ret.end(), ret.begin(), [](uint8 c) { return std::tolower(c); });
	return ret;
}

String strUtils::toUpper(const String& string)
{
	String ret = string;
	std::transform(ret.begin(), ret.end(), ret.begin(), [](uint8 c) { return std::toupper(c); });
	return ret;
}

String& strUtils::replaceCharInline(String& string, Char from, Char to)
{
	std::replace(string.begin(), string.end(), from, to);
	return string;
}

String strUtils::replaceChar(const String& string, Char from, Char to)
{
	String str(string);
	replaceCharInline(str, from, to);
	return str;
}

String& strUtils::replaceInline(String& string, const String& from, const String& to)
{
	size_t len = String(from).length(), pos = 0, len2 = String(to).length();
	while ((pos = string.find(from, pos)) != String::npos) {
		string.replace(pos, len, to);
		pos += len2;
	}
	return string;
}

String strUtils::replace(const String& string, const String& from, const String& to)
{
	String str(string);
	replaceInline(str, from, to);
	return str;
}

String& strUtils::trimInline(String& string, Char t)
{
	string.erase(string.begin(), std::find_if(string.begin(), string.end(), [t](Char c) { return c != t; }));
	string.erase(std::find_if(string.rbegin(), string.rend(), [t](Char c) { return c != t; }).base(), string.end());
	return string;
}

String strUtils::trim(const String& string, Char t)
{
	String str(string);
	trimInline(str, t);
	return str;
}

int32 strUtils::compareNoCase(const String& str1, const String& str2)
{
	auto a = str1.begin(), b = str2.begin();
	for (; a != str1.end() && b != str2.end(); ++a, ++b) {
		Char la = std::tolower(*a), lb = std::tolower(*b);
		if (la != lb)
			return la < lb ? -1 : 1;
	}
	if (a == str1.end() && b == str2.end())
		return 0;
	return a == str1.end() ? -1 : 1;
}


String strUtils::fromNum(float32 v, const Char* fmt) // can return nan, +inf, -inf
{
	if (v != v)
		return MTEXT("nan");
	else if (v < -std::numeric_limits<float32>::max())
		return MTEXT("-inf");
	else if (v > std::numeric_limits<float32>::max())
		return MTEXT("+inf");
	else if (v == -std::numeric_limits<float32>::max())
		return MTEXT("-max");
	else if (v == std::numeric_limits<float32>::max())
		return MTEXT("+max");
	else if (v < std::numeric_limits<float32>::min() && v > -std::numeric_limits<float32>::min())
		return format(fmt, 0.0f);
	return format(fmt, v);
}

String strUtils::fromNum(float64 v, const Char* fmt) // can return nan, +inf, -inf
{
	if (v != v)
		return MTEXT("nan");
	else if (v < -std::numeric_limits<float64>::max())
		return MTEXT("-inf");
	else if (v > std::numeric_limits<float64>::max())
		return MTEXT("+inf");
	else if (v == -std::numeric_limits<float64>::max())
		return MTEXT("-max");
	else if (v == std::numeric_limits<float64>::max())
		return MTEXT("+max");
	else if (v < std::numeric_limits<float64>::min() && v > -std::numeric_limits<float64>::min())
		return format(fmt, 0.0);
	return format(fmt, v);
}
