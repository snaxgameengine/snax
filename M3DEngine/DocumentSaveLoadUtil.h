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

#include "DocumentSaver.h"
#include "DocumentLoader.h"
#include "Engine.h"
#include "M3DCore/MMath.h"
#include "DocumentJSONSaver.h"
#include "DocumentJSONLoader.h"
#include "M3DCore/MagicEnum.h"

namespace m3d
{ 


template<typename T>
bool SerializeDocumentData(DocumentSaver &saver, const T *t, uint32 size) 
{
	if (saver.IsJson()) {
		DocumentJSONSaver *jsaver = dynamic_cast<DocumentJSONSaver*>(&saver);
		if (!jsaver->WriteData(t, size))
			return false;
	}
	else {
		for (uint32 i = 0; i < size; i++) {
			SAVE(strUtils::format(MTEXT("d%i"), i), t[i]);
		}
	}
	return true;
}

template<typename T>
bool DeserializeDocumentData(DocumentLoader &loader, T *t, uint32 size) 
{
	if (loader.IsJson()) {
		DocumentJSONLoader* jloader = dynamic_cast<DocumentJSONLoader*>(&loader);
		if (!jloader->ReadData(t, size))
			return false;
	}
	else {
		for (uint32 i = 0; i < size; i++) {
			LOAD(strUtils::format(MTEXT("d%i"), i), t[i]);
		}
	}
	return true;
}

template<typename T, typename S>
bool SerializeDocumentData(DocumentSaver &saver, const std::pair<T, S> &data)
{
	SAVE(MTEXT("first"), data.first);
	SAVE(MTEXT("second"), data.second);
	return true;
}

template<typename T, typename S>
bool DeserializeDocumentData(DocumentLoader &loader, std::pair<T, S> &data)
{
	LOAD(MTEXT("first"), data.first);
	LOAD(MTEXT("second"), data.second);
	return true;
}

template<typename T>
bool SerializeDocumentData(DocumentSaver &saver, const List<T> &data)
{
	SAVE(MTEXT("count"), (uint32)data.size());
	if (data.size())
		SAVEARRAY(MTEXT("array"), &data.front(), (uint32)data.size());
	return true;
}

template<typename T>
bool DeserializeDocumentData(DocumentLoader &loader, List<T> &data)
{
	uint32 size = 0;
	LOAD(MTEXT("count"), size);
	data.resize(size);
	if (size)
		LOADARRAY(MTEXT("array"), &data.front(), size);
	return true;
}


template<typename T, typename CMP>
bool SerializeDocumentData(DocumentSaver &saver, const Set<T, CMP> &data)
{
	SAVE(MTEXT("count"), (uint32)data.size());
	if (saver.IsJson()) {
		if (data.size()) {
			List<T> tmp(data.begin(), data.end());
			SAVEARRAY(MTEXT("set"), &tmp.front(), (uint32)tmp.size());
		}
	}
	else {
		uint32 i = 0;
		for (const auto& n : data) {
			SAVE(strUtils::format(MTEXT("d%i"), i), n);
			i++;
		}
	}
	return true;
}

template<typename T, typename CMP>
bool DeserializeDocumentData(DocumentLoader &loader, Set<T, CMP> &data)
{
	data.clear();
	uint32 size = 0;
	LOAD(MTEXT("count"), size);
	if (loader.IsJson()) {
		if (size) {
			List<T> tmp;
			tmp.resize(size);
			LOADARRAY(MTEXT("set"), &tmp.front(), size);
			data.insert(tmp.begin(), tmp.end());
			if (data.size() != size)
				return false;
		}
	}
	else {
		T p;
		for (uint32 i = 0; i < size; i++) {
			LOAD(strUtils::format(MTEXT("d%i"), i), p);
			B_RETURN(data.insert(p).second);
		}
	}
	return true;
}

template<typename T, typename CMP>
bool SerializeDocumentData(DocumentSaver& saver, const MultiSet<T, CMP>& data)
{
	SAVE(MTEXT("count"), (uint32)data.size());
	if (saver.IsJson()) {
		if (data.size()) {
			List<T> tmp(data.begin(), data.end());
			SAVEARRAY(MTEXT("multiset"), &tmp.front(), (uint32)tmp.size());
		}
	}
	else {
		uint32 i = 0;
		for (const auto& n : data) {
			SAVE(strUtils::format(MTEXT("d%i"), i), n);
			i++;
		}
	}
	return true;
}

template<typename T, typename CMP>
bool DeserializeDocumentData(DocumentLoader& loader, MultiSet<T, CMP>& data)
{
	data.clear();
	uint32 size = 0;
	LOAD(MTEXT("count"), size);
	if (loader.IsJson()) {
		if (size) {
			List<T> tmp;
			tmp.resize(size);
			LOADARRAY(MTEXT("multiset"), &tmp.front(), size);
			data.insert(tmp.begin(), tmp.end());
			if (data.size() != size)
				return false;
		}
	}
	else {
		T p;
		for (uint32 i = 0; i < size; i++) {
			LOAD(strUtils::format(MTEXT("d%i"), i), p);
			B_RETURN(data.insert(p).second);
		}
	}
	return true;
}

template<typename T, typename S, typename CMP>
bool SerializeDocumentData(DocumentSaver &saver, const Map<T, S, CMP> &data)
{
	SAVE(MTEXT("count"), (uint32)data.size());
	if (saver.IsJson()) {
		if (data.size()) {
			List<std::pair<T, S>> tmp(data.begin(), data.end());
			SAVEARRAY(MTEXT("map"), &tmp.front(), (uint32)tmp.size());
		}
	}
	else {
		uint32 i = 0;
		for (const auto& n : data) {
			SAVE(strUtils::format(MTEXT("d%i"), i), n);
			i++;
		}
	}
	return true;
}

template<typename T, typename S, typename CMP>
bool DeserializeDocumentData(DocumentLoader &loader, Map<T, S, CMP> &data)
{
	data.clear();
	uint32 size = 0;
	LOAD(MTEXT("count"), size);
	if (loader.IsJson()) {
		if (size) {
			List<std::pair<T, S>> tmp;
			tmp.resize(size);
			LOADARRAY(MTEXT("map"), &tmp.front(), size);
			data.insert(tmp.begin(), tmp.end());
			if (data.size() != size)
				return false;
		}
	}
	else {
		std::pair<T, S> p;
		for (uint32 i = 0; i < size; i++) {
			LOAD(strUtils::format(MTEXT("d%i"), i), p);
			B_RETURN(data.insert(p).second);
		}
	}
	return true;
}

template<typename T, typename S, typename CMP>
bool SerializeDocumentData(DocumentSaver& saver, const MultiMap<T, S, CMP>& data)
{
	SAVE(MTEXT("count"), (uint32)data.size());
	if (saver.IsJson()) {
		if (data.size()) {
			List<std::pair<T, S>> tmp(data.begin(), data.end());
			SAVEARRAY(MTEXT("multimap"), &tmp.front(), (uint32)tmp.size());
		}
	}
	else {
		uint32 i = 0;
		for (const auto& n : data) {
			SAVE(strUtils::format(MTEXT("d%i"), i), n);
			i++;
		}
	}
	return true;
}

template<typename T, typename S, typename CMP>
bool DeserializeDocumentData(DocumentLoader& loader, MultiMap<T, S, CMP>& data)
{
	data.clear();
	uint32 size = 0;
	LOAD(MTEXT("count"), size);
	if (loader.IsJson()) {
		if (size) {
			List<std::pair<T, S>> tmp;
			tmp.resize(size);
			LOADARRAY(MTEXT("multimap"), &tmp.front(), size);
			data.insert(tmp.begin(), tmp.end());
			if (data.size() != size)
				return false;
		}
	}
	else {
		std::pair<T, S> p;
		for (uint32 i = 0; i < size; i++) {
			LOAD(strUtils::format(MTEXT("d%i"), i), p);
			B_RETURN(data.insert(p).second);
		}
	}
	return true;
}


// Enum
template<class T, class = typename std::enable_if< std::is_enum<T>::value >::type>
bool SerializeDocumentData(DocumentSaver &saver, T t) 
{
	if (saver.IsBinary())
		return saver.WriteData((uint32)t);

	constexpr auto emin = magic_enum::customize::enum_range<T>::min;
	constexpr auto emax = magic_enum::customize::enum_range<T>::max;

	if constexpr (emin >= MAGIC_ENUM_RANGE_MIN && emax <= MAGIC_ENUM_RANGE_MAX)
	{
		String enumName;
		if (magic_enum::enum_index(t))
		{
			enumName = String(magic_enum::enum_name(t));
		}
		else
		{
			enumName = String(magic_enum::flags::enum_name(t));
		}
		if (!enumName.empty())
			return saver.WriteData(String(enumName));
	}
	else
	{
		engine->Message(WARN, MTEXT("Can not convert enum to string. Consider expanding magic-enum range, or shrink your enum range!"));
	}
	return saver.WriteData((uint32)t); // fallback
}

// Enum
template<class T, class = typename std::enable_if< std::is_enum<T>::value >::type>
bool DeserializeDocumentData(DocumentLoader &loader, T &t) 
{
	if (loader.GetDocumentVersion() < Version(1, 2, 7, 0))
		return loader.ReadData((uint32&)t);
	return DeserializeDocumentDataEnum(loader, t);
}

template<class T>
bool DeserializeDocumentDataEnum(DocumentLoader& loader, T& t)
{
	if (loader.IsBinary())
		return loader.ReadData((uint32&)t);
	String enumName;
	if (loader.ReadData(enumName)) 
	{
		if (loader.IsJson() || !(enumName.size() > 0 && enumName[0] >= MCHAR('0') && enumName[0] <= MCHAR('9'))) // XML loader can read numbers as string. That's why we need this check! Remove when removing XML loader.
		{
			if (enumName.find_first_of(MCHAR('|')) != String::npos)
			{
				auto e = magic_enum::flags::enum_cast<T>(enumName);
				if (!e)
					return false;
				t = *e;
			}
			else
			{
				auto e = magic_enum::enum_cast<T>(enumName);
				if (!e)
					return false;
				t = *e;
			}
			return true;
		}
	}
	return loader.ReadData((uint32&)t); // fallback
}

static bool SerializeDocumentData(DocumentSaver &saver, const XMFLOAT2 &data) { return saver.WriteData((const float32*)&data, 2); }
static bool DeserializeDocumentData(DocumentLoader &loader, XMFLOAT2 &data) { return loader.ReadData((float32*)&data, 2); }
static bool SerializeDocumentData(DocumentSaver &saver, const XMFLOAT2 *data, uint32 size) { return saver.WriteData((const float32*)data, 2 * size); }
static bool DeserializeDocumentData(DocumentLoader &loader, XMFLOAT2 *data, uint32 size) { return loader.ReadData((float32*)data, 2 * size); }

static bool SerializeDocumentData(DocumentSaver &saver, const XMFLOAT3 &data) { return saver.WriteData((const float32*)&data, 3); }
static bool DeserializeDocumentData(DocumentLoader &loader, XMFLOAT3 &data) { return loader.ReadData((float32*)&data, 3); }
static bool SerializeDocumentData(DocumentSaver &saver, const XMFLOAT3 *data, uint32 size) { return saver.WriteData((const float32*)data, 3 * size); }
static bool DeserializeDocumentData(DocumentLoader &loader, XMFLOAT3 *data, uint32 size) { return loader.ReadData((float32*)data, 3 * size); }

static bool SerializeDocumentData(DocumentSaver &saver, const XMFLOAT4 &data) { return saver.WriteData((const float32*)&data, 4); }
static bool DeserializeDocumentData(DocumentLoader &loader, XMFLOAT4 &data) { return loader.ReadData((float32*)&data, 4); }
static bool SerializeDocumentData(DocumentSaver &saver, const XMFLOAT4 *data, uint32 size) { return saver.WriteData((const float32*)data, 4 * size); }
static bool DeserializeDocumentData(DocumentLoader &loader, XMFLOAT4 *data, uint32 size) { return loader.ReadData((float32*)data, 4 * size); }

static bool SerializeDocumentData(DocumentSaver &saver, const XMFLOAT4X4 &data) { return saver.WriteData((const float32*)&data, 16); }
static bool DeserializeDocumentData(DocumentLoader &loader, XMFLOAT4X4 &data) { return loader.ReadData((float32*)&data, 16); }
static bool SerializeDocumentData(DocumentSaver &saver, const XMFLOAT4X4 *data, uint32 size) { return saver.WriteData((const float32*)data, 16 * size); }
static bool DeserializeDocumentData(DocumentLoader &loader, XMFLOAT4X4 *data, uint32 size) { return loader.ReadData((float32*)data, 16 * size); }

static bool SerializeDocumentData(DocumentSaver &saver, const XMUSHORT4 &data) { return saver.WriteData((const uint16*)&data, 4); }
static bool DeserializeDocumentData(DocumentLoader &loader, XMUSHORT4 &data) { return loader.ReadData((uint16*)&data, 4); }
static bool SerializeDocumentData(DocumentSaver &saver, const XMUSHORT4 *data, uint32 size) { return saver.WriteData((const uint16*)data, 4 * size); }
static bool DeserializeDocumentData(DocumentLoader &loader, XMUSHORT4 *data, uint32 size) { return loader.ReadData((uint16*)data, 4 * size); }

static bool SerializeDocumentData(DocumentSaver &saver, const XMCOLOR &data) { return saver.WriteData((const uint8*)&data, 4); }
static bool DeserializeDocumentData(DocumentLoader &loader, XMCOLOR &data) { return loader.ReadData((uint8*)&data, 4); }
static bool SerializeDocumentData(DocumentSaver &saver, const XMCOLOR *data, uint32 size) { return saver.WriteData((const uint8*)data, 4 * size); }
static bool DeserializeDocumentData(DocumentLoader &loader, XMCOLOR *data, uint32 size) { return loader.ReadData((uint8*)data, 4 * size); }

static bool SerializeDocumentData(DocumentSaver &saver, const XMUBYTEN4 &data) { return saver.WriteData(data.v); }
static bool DeserializeDocumentData(DocumentLoader &loader, XMUBYTEN4 &data) { return loader.ReadData(data.v); }
static bool SerializeDocumentData(DocumentSaver &saver, const XMUBYTEN4 *data, uint32 size) { return saver.WriteData((uint32*)data, size); }
static bool DeserializeDocumentData(DocumentLoader &loader, XMUBYTEN4 *data, uint32 size) { return loader.ReadData((uint32*)data, size); }

bool M3DENGINE_API SerializeDocumentData(DocumentSaver &saver, const Guid &data); 
bool M3DENGINE_API DeserializeDocumentData(DocumentLoader &loader, Guid &data);

bool M3DENGINE_API SerializeDocumentData(DocumentSaver &saver, const Chip *chip);
bool M3DENGINE_API DeserializeDocumentData(DocumentLoader &loader, Chip *&chip);

bool M3DENGINE_API SerializeDocumentData(DocumentSaver &saver, const DataBuffer &data);
bool M3DENGINE_API DeserializeDocumentData(DocumentLoader &loader, DataBuffer &data);

bool M3DENGINE_API DeserializeDocumentData(DocumentLoader &loader, Function::Access &data);
bool M3DENGINE_API DeserializeDocumentData(DocumentLoader &loader, Function::Type &data);
bool M3DENGINE_API DeserializeDocumentData(DocumentLoader &loader, RefreshManager::RefreshMode &data);

class AxisAlignedBox;

bool M3DENGINE_API SerializeDocumentData(DocumentSaver &saver, const AxisAlignedBox &data);
bool M3DENGINE_API DeserializeDocumentData(DocumentLoader &loader, AxisAlignedBox &data);

bool M3DENGINE_API SerializeDocumentData(DocumentSaver &saver, Version data); 
bool M3DENGINE_API DeserializeDocumentData(DocumentLoader &loader, Version &data);

bool M3DENGINE_API SerializeDocumentData(DocumentSaver &saver, Path data); 
bool M3DENGINE_API DeserializeDocumentData(DocumentLoader &loader, Path &data);


}