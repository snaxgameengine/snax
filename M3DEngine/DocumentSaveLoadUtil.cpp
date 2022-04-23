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
#include "DocumentSaveLoadUtil.h"
#include "M3DCore/DataBuffer.h"
#include "M3DCore/AxisAlignedBox.h"


using namespace m3d;



bool m3d::SerializeDocumentData(DocumentSaver &saver, const Guid &data) 
{ 
	if (saver.IsBinary())
		return saver.WriteData((void*)&data, sizeof(Guid));
	else
		return saver.WriteData(GuidToString(data));
}

bool m3d::DeserializeDocumentData(DocumentLoader &loader, Guid &data) 
{ 
	if (loader.IsBinary())
		return loader.ReadData((void*)&data, sizeof(Guid));
	else {
		String s;
		return loader.ReadData(s) && StringToGUID(s, data);
	}
}

bool m3d::SerializeDocumentData(DocumentSaver &saver, Version data)
{
	if (saver.IsBinary())
		return saver.WriteData(data.version);
	return saver.WriteData(strUtils::ConstructString(MTEXT("%1.%2.%3.%4"))
		.arg(strUtils::fromNum(uint32(data.elements[3])))
		.arg(strUtils::fromNum(uint32(data.elements[2])))
		.arg(strUtils::fromNum(uint32(data.elements[1])))
		.arg(strUtils::fromNum(uint32(data.elements[0]))).string);
}

bool m3d::DeserializeDocumentData(DocumentLoader &loader, Version &data)
{
	if (loader.IsBinary())
		return loader.ReadData(data.version);
	String v;
	if (!loader.ReadData(v))
		return false;
	v += MTEXT(".");
	uint32 k = 0;
	for (size_t i = 0, j = 0; i < v.size(); i++) {
		if (v[i] == MCHAR('.')) {
			uint32 n = 0;
			if (k > 3 || !strUtils::toNum(v.substr(j, i - j), n) || n > 255)
				return false;
			data.elements[3 - k++] = (uint8)n;
			j = i + 1;
		}
	}
	return k == 4;
}

bool m3d::SerializeDocumentData(DocumentSaver &saver, Path data)
{
	return saver.WriteData(data.AsString());
}

bool m3d::DeserializeDocumentData(DocumentLoader &loader, Path &data)
{
	String s;
	if (!loader.ReadData(s))
		return false;
	data = s;
	return true;
}


bool m3d::SerializeDocumentData(DocumentSaver &saver, const Chip *chip)
{
	return saver.SaveChip((Chip*)chip, nullptr); // <========================================= OBS OBS! (just casting to non const for now...)
}

bool m3d::DeserializeDocumentData(DocumentLoader &loader, Chip *&chip)
{
	return loader.LoadChip(&chip);
}

bool m3d::SerializeDocumentData(DocumentSaver &saver, const DataBuffer &data)
{
	SAVE(MTEXT("size"), (uint32)data.getBufferSize());
	if (data.getBufferSize())
		SAVEARRAY(MTEXT("data"), (void*)data.getConstBuffer(), (uint32)data.getBufferSize());
	return true;
}

bool m3d::DeserializeDocumentData(DocumentLoader &loader, DataBuffer &data)
{
	uint32 size = 0;
	data.clear();
	LOAD(MTEXT("size"), size);
	if (size == 0)
		return true;
	data.realloc(size);
	LOADARRAY(MTEXT("data"), (void*)data.getBuffer(), size);
	return true;
}

bool m3d::DeserializeDocumentData(DocumentLoader &loader, m3d::Function::Access &data)
{
	if (loader.GetDocumentVersion() < Version(1, 2, 7, 0))
	{
		static const Char* FunctionAccessNames[3] = { MTEXT("public"), MTEXT("protected"), MTEXT("private") };
		if (loader.IsBinary())
			return loader.ReadData((uint32&)data);
		String s;
		if (!loader.ReadData(s))
			return false;
		if (s == FunctionAccessNames[0])
			data = Function::Access::Public;
		else if (s == FunctionAccessNames[1])
			data = Function::Access::Protected;
		else if (s == FunctionAccessNames[2])
			data = Function::Access::Private;
		else
			return false;
		return true;
	}
	return DeserializeDocumentDataEnum(loader, data);
}

bool m3d::DeserializeDocumentData(DocumentLoader &loader, m3d::Function::Type &data)
{
	if (loader.GetDocumentVersion() < Version(1, 2, 7, 0))
	{
		static const Char* FunctionTypeNames[3] = { MTEXT("static"), MTEXT("nonvirtual"), MTEXT("virtual") };
		if (loader.IsBinary())
			return loader.ReadData((uint32&)data);
		String s;
		if (!loader.ReadData(s))
			return false;
		if (s == FunctionTypeNames[0])
			data = Function::Type::Static;
		else if (s == FunctionTypeNames[1])
			data = Function::Type::NonVirtual;
		else if (s == FunctionTypeNames[2])
			data = Function::Type::Virtual;
		else
			return false;
		return true;
	}
	return DeserializeDocumentDataEnum(loader, data);
}

bool m3d::DeserializeDocumentData(DocumentLoader &loader, RefreshManager::RefreshMode &data)
{
	if (loader.GetDocumentVersion() < Version(1, 2, 7, 0))
	{
		static const Char* RefreshModeNames[5] = { MTEXT("always"), MTEXT("function"), MTEXT("frame"), MTEXT("once"), MTEXT("never") };
		if (loader.IsBinary())
			return loader.ReadData((uint32&)data);
		String s;
		if (!loader.ReadData(s))
			return false;
		if (s == RefreshModeNames[0])
			data = RefreshManager::RefreshMode::Always;
		else if (s == RefreshModeNames[1] || s == MTEXT("stack")) // ||TEMPORARY!
			data = RefreshManager::RefreshMode::OncePerFunctionCall;
		else if (s == RefreshModeNames[2])
			data = RefreshManager::RefreshMode::OncePerFrame;
		else if (s == RefreshModeNames[3])
			data = RefreshManager::RefreshMode::Once;
		else if (s == RefreshModeNames[3])
			data = RefreshManager::RefreshMode::Never;
		else
			return false;
		return true;
	}
	return DeserializeDocumentDataEnum(loader, data);
}

bool m3d::SerializeDocumentData(DocumentSaver &saver, const AxisAlignedBox &data)
{
	SAVE(MTEXT("isinf"), data.IsInfinite());
	if (!data.IsInfinite()) {
		SAVE(MTEXT("min"), data.GetMin());
		SAVE(MTEXT("max"), data.GetMax());
	}
	return true;
}

bool m3d::DeserializeDocumentData(DocumentLoader &loader, AxisAlignedBox &data)
{
	bool b = false;
	LOAD(MTEXT("isinf"), b);
	if (b)
		data.SetInfinite();
	else {
		XMFLOAT3 m, n;
		LOAD(MTEXT("min"), m);
		LOAD(MTEXT("max"), n);
		data.Set(m, n);
	}
	return true;
}


