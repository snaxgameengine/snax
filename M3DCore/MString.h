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
#include "MTypes.h"
#include <string>
#include "Allocator.h"
#include "MLocale.h"
#include <atomic>
#include <cwchar>
#include <cstdarg>
#include <string_view>
#include <algorithm>
#include <ios>
#include <sstream>


namespace m3d
{
	// Keep it UTF-8!
	using String = std::basic_string<Char, std::char_traits<Char>, Allocator<Char>>;
	using StringView = std::basic_string_view<Char>;


	class M3DCORE_API strUtils
	{
	public:

		static wchar_t* widen(wchar_t* output, int32 outputSize, const Char* input);
		static Char* narrow(Char* output, int32 outputSize, const wchar_t* input);
		static String narrow2(const wchar_t* input);

		static String toLower(const String& string);
		static String toUpper(const String& string);
		static String& replaceCharInline(String& string, Char from, Char to);
		static String replaceChar(const String& string, Char from, Char to);
		static String& replaceInline(String& string, const String& from, const String& to);
		static String replace(const String& string, const String& from, const String& to);
		static String& trimInline(String& string, Char t = MCHAR(' '));
		static String trim(const String& string, Char t = MCHAR(' '));
		static int32 compareNoCase(const String& str1, const String& str2);

		enum class NumBase { OCT, DEC, HEX };

		template<typename NumType>
		static bool __toNumFromString(const String& string, NumType& v, NumBase base)
		{
			std::basic_istringstream<Char, String::traits_type, String::allocator_type> iss(string);
			switch (base)
			{
			case NumBase::OCT: iss >> std::oct >> v; break;
			case NumBase::DEC: iss >> std::dec >> v; break;
			case NumBase::HEX: iss >> std::hex >> v; break;
			}
			return iss.eof();
		}

		template<typename NumType, typename std::enable_if<std::is_floating_point<NumType>::value>::type* = nullptr>
		static bool toNum(const String& string, NumType& v, NumBase base = NumBase::DEC)
		{
			if (string == MTEXT("nan"))
				v = std::numeric_limits<NumType>::quiet_NaN();
			else if (string == MTEXT("+inf"))
				v = std::numeric_limits<NumType>::infinity();
			else if (string == MTEXT("-inf"))
				v = -std::numeric_limits<NumType>::infinity();
			else if (string == MTEXT("+max"))
				v = std::numeric_limits<NumType>::max();
			else if (string == MTEXT("-max"))
				v = -std::numeric_limits<NumType>::max();
			else
				return __toNumFromString(string, v, base);
			return true;
		}

		template<typename NumType, typename std::enable_if<!std::is_floating_point<NumType>::value>::type* = nullptr>
		static bool toNum(const String& string, NumType& v, NumBase base = NumBase::DEC)
		{
			return __toNumFromString(string, v, base);
		}

		template<int32 BufferSize = 4096>
		static String format(const Char* str, ...)
		{
			Char tmp[BufferSize];
			va_list args;
			va_start(args, str);
			int32 i = std::vsnprintf(tmp, BufferSize, str, args);
			va_end(args);
			if (i < 0)
				return String();
			return tmp;
		}

		static String fromNum(float32 v, const Char* fmt = MTEXT("%.9g")); // can return nan, +inf, -inf
		static String fromNum(float64 v, const Char* fmt = MTEXT("%.17g")); // can return nan, +inf, -inf
		static String fromNum(int16 v, const Char* fmt = MTEXT("%hi")) { return format(fmt, v); }
		static String fromNum(uint16 v, const Char* fmt = MTEXT("%hu")) { return format(fmt, v); }
		static String fromNum(int32 v, const Char* fmt = MTEXT("%i")) { return format(fmt, v); }
		static String fromNum(uint32 v, const Char* fmt = MTEXT("%u")) { return format(fmt, v); }
		static String fromNum(int64 v, const Char* fmt = MTEXT("%I64d")) { return format(fmt, v); }
		static String fromNum(uint64 v, const Char* fmt = MTEXT("%I64u")) { return format(fmt, v); }

		struct ConstructString
		{
			String string;

			explicit ConstructString(const String& string) : string(string) {}
			explicit ConstructString(const Char* string) : string(string) {}

			ConstructString() = delete;
			ConstructString(const ConstructString&) = delete; // non construction-copyable
			ConstructString& operator=(const ConstructString&) = delete; // non copyable

			operator const String& () const { return string; }

			ConstructString arg(const String& arg) const
			{
				String ret = replace(string, String(MTEXT("%1")), arg);
				for (std::size_t i = 0; i < ret.size() - 1; i++) {
					if (ret[i] == MCHAR('%') && ret[i + 1] >= MCHAR('2') && ret[i + 1] <= MCHAR('9'))
						ret[i + 1]--;
				}
				return ConstructString(ret);
			}

			inline ConstructString arg(float32 v, const Char* fmt = MTEXT("%.9g")) const { return arg(fromNum(v, fmt)); }
			inline ConstructString arg(float64 v, const Char* fmt = MTEXT("%.17g")) const { return arg(fromNum(v, fmt)); }
			inline ConstructString arg(int16 v, const Char* fmt = MTEXT("%hi")) const { return arg(fromNum(v, fmt)); }
			inline ConstructString arg(uint16 v, const Char* fmt = MTEXT("%hu")) const { return arg(fromNum(v, fmt)); }
			inline ConstructString arg(int32 v, const Char* fmt = MTEXT("%i")) const { return arg(fromNum(v, fmt)); }
			inline ConstructString arg(uint32 v, const Char* fmt = MTEXT("%u")) const { return arg(fromNum(v, fmt)); }
			inline ConstructString arg(int64 v, const Char* fmt = MTEXT("%I64d")) const { return arg(fromNum(v, fmt)); }
			inline ConstructString arg(uint64 v, const Char* fmt = MTEXT("%I64u")) const { return arg(fromNum(v, fmt)); }
		};
	};

}

	
