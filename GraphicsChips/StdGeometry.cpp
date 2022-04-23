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
#include "StdGeometry.h"
#include "M3DEngine/DocumentSaveLoadUtil.h"
#include "M3DCore/DataBuffer.h"
#include "M3DEngine/Engine.h"
#include "D3D12Formats.h"
#include "Graphics.h"

using namespace m3d;


CHIPDESCV1_DEF(StdGeometry, MTEXT("Standard Geometry"), STDGEOMETRY_GUID, GEOMETRY_GUID);



namespace m3d
{
	bool SerializeDocumentData(DocumentSaver& saver, const StdGeometry::TexCoordSet& data)
	{
		SAVE("type", (uint32)data.type);
		switch (data.type)
		{
		case StdGeometry::U: SAVE("data", data.u); break;
		case StdGeometry::UV: SAVE("data", data.uv); break;
		case StdGeometry::UVW: SAVE("data", data.uvw); break;
		case StdGeometry::UVWX: SAVE("data", data.uvwx); break;
		}
		return true;
	}

	bool DeserializeDocumentData(DocumentLoader& loader, StdGeometry::TexCoordSet& data)
	{
		LOAD("type", (uint32&)data.type);
		switch (data.type)
		{
		case StdGeometry::U: LOAD("data", data.u); break;
		case StdGeometry::UV: LOAD("data", data.uv); break;
		case StdGeometry::UVW: LOAD("data", data.uvw); break;
		case StdGeometry::UVWX: LOAD("data", data.uvwx); break;
		}
		return true;
	}
}


bool StdGeometry::CopyChip(Chip* chip)
{
	StdGeometry* c = dynamic_cast<StdGeometry*>(chip);
	B_RETURN(Geometry::CopyChip(c));
	_positions = c->_positions;
	_normals = c->_normals;
	_tangents = c->_tangents;
	_bitangents = c->_bitangents;
	_colors = c->_colors;
	for (uint32 i = 0; i < MAX_TEXCOORD_SETS; i++)
		_texcoords[i] = c->_texcoords[i];
	_blendWeights = c->_blendWeights;
	_blendIndices = c->_blendIndices;
	_indices = c->_indices;
	_streams = c->_streams;
	_tsc = c->_tsc;
	_packNormals = c->_packNormals;
	_packTexcoords = c->_packTexcoords;
	return true;
}

bool StdGeometry::LoadChip(DocumentLoader& loader)
{
	B_RETURN(Geometry::LoadChip(loader));
	LOAD("positions", _positions);
	LOAD("normals", _normals);
	LOAD("tangents", _tangents);
	if (!loader.LoadData("bitangents", _bitangents, false))
		LOAD("binormals", _bitangents); // TEMPORARY!!
	LOAD("colors", _colors);
	LOADARRAY("texcoords", _texcoords, MAX_TEXCOORD_SETS);
	LOAD("blendWeights", _blendWeights);
	LOAD("blendIndices", _blendIndices);
	LOAD("indices", _indices);
	LOADARRAY("streams", (uint32*)_streams.s, MAX_ELEMENTS);
	LOAD("tangentSpaceCompression", (uint32&)_tsc);
	LOAD("packNormals", _packNormals);
	LOAD("packTexSets", _packTexcoords);
	return true;
}

bool StdGeometry::SaveChip(DocumentSaver& saver) const
{
	B_RETURN(Geometry::SaveChip(saver));
	SAVE("positions", _positions);
	SAVE("normals", _normals);
	SAVE("tangents", _tangents);
	SAVE("bitangents", _bitangents);
	SAVE("colors", _colors);
	SAVEARRAY("texcoords", _texcoords, MAX_TEXCOORD_SETS);
	SAVE("blendWeights", _blendWeights);
	SAVE("blendIndices", _blendIndices);
	SAVE("indices", _indices);
	SAVEARRAY("streams", _streams.s, MAX_ELEMENTS);
	SAVE("tangentSpaceCompression", (uint32)_tsc);
	SAVE("packNormals", _packNormals);
	SAVE("packTexSets", _packTexcoords);
	return true;
}

void StdGeometry::Clear()
{
	Geometry::Clear();
	_positions.clear();
	_normals.clear();
	_tangents.clear();
	_bitangents.clear();
	_colors.clear();
	for (uint32 i = 0; i < MAX_TEXCOORD_SETS; i++)
		_texcoords[i].clear();
	_blendWeights.clear();
	_blendIndices.clear();
	_indices.clear();
	memset(_streams.s, 0, sizeof(_streams));
}

void StdGeometry::CommitSubset(M3D_PRIMITIVE_TOPOLOGY pt, String name)
{
	GeometrySubset ss;
	ss.name = name;
	ss.pt = pt;
	ss.baseVertexLocation = 0;
	ss.startLocation = GetSubsets().size() ? (GetSubsets().back().startLocation + GetSubsets().back().count) : 0;
	ss.count = uint32(GetAPI() == DRAW ? _positions.size() : _indices.size()) - ss.startLocation;
	_calculateBoundingBox(ss);
	AddSubset(ss);
}

void StdGeometry::_calculateBoundingBox(GeometrySubset& ss)
{
	if (GetAPI() == DRAW)
		ss.boundingBox.Set(ss.count, &_positions[ss.startLocation + ss.baseVertexLocation], sizeof(XMFLOAT3));
	else
		ss.boundingBox.Set(ss.count, &_positions[ss.baseVertexLocation], sizeof(XMFLOAT3), &_indices[ss.startLocation]);
}

void StdGeometry::TransformPositions(CXMMATRIX m)
{
	if (_positions.size()) {
		XMVector3TransformCoordStream(&_positions.front(), sizeof(XMFLOAT3), &_positions.front(), sizeof(XMFLOAT3), (UINT)_positions.size(), m);
		for (size_t i = 0; i < GetSubsets().size(); i++)
			SetBoundingBox(GetSubsets()[i].boundingBox * m, (uint32)i);
	}
}

void StdGeometry::TransformNormals(CXMMATRIX m)
{
	if (_normals.size())
		XMVector3TransformNormalStream(&_normals.front(), sizeof(XMFLOAT3), &_normals.front(), sizeof(XMFLOAT3), (UINT)_normals.size(), m);
	if (_tangents.size())
		XMVector3TransformNormalStream(&_tangents.front(), sizeof(XMFLOAT3), &_tangents.front(), sizeof(XMFLOAT3), (UINT)_tangents.size(), m);
	if (_bitangents.size())
		XMVector3TransformNormalStream(&_bitangents.front(), sizeof(XMFLOAT3), &_bitangents.front(), sizeof(XMFLOAT3), (UINT)_bitangents.size(), m);
}

void StdGeometry::TransformTexCoordSet(CXMMATRIX m, uint32 set)
{
	switch (_texcoords[set].type)
	{
	case U: _texcoords[set].u.clear(); break;
	case UV: XMVector2TransformCoordStream(&_texcoords[set].uv.front(), sizeof(XMFLOAT2), &_texcoords[set].uv.front(), sizeof(XMFLOAT2), (UINT)_texcoords[set].uv.size(), m); break;
	case UVW: XMVector3TransformCoordStream(&_texcoords[set].uvw.front(), sizeof(XMFLOAT3), &_texcoords[set].uvw.front(), sizeof(XMFLOAT3), (UINT)_texcoords[set].uvw.size(), m); break;
	case UVWX: XMVector4TransformStream(&_texcoords[set].uvwx.front(), sizeof(XMFLOAT4), &_texcoords[set].uvwx.front(), sizeof(XMFLOAT4), (UINT)_texcoords[set].uvwx.size(), m); break;
	default:;
	}
}

void StdGeometry::SwapTexcoords(uint32 set1, uint32 set2)
{
	if (set1 == set2)
		return;
	std::swap(_texcoords[set1].type, _texcoords[set2].type);
	std::swap(_texcoords[set1].u, _texcoords[set2].u); // NOTE: This is kinda nasty, but is should work. The formal way would be to swap the correct type of course! (std::move will prevent data copy!)
}

void StdGeometry::ChangeTexCoordSetType(uint32 set, TexCoordSetType type)
{
	if (_texcoords[set].type == type)
		return;
	TexCoordSet s;
	s.type = type;
	switch (_texcoords[set].type)
	{
	case U:
	{
		switch (s.type)
		{
		case UV:
			for (size_t i = 0; i < _texcoords[set].u.size(); i++) s.uv.push_back(XMFLOAT2(_texcoords[set].u[i], 0.0f));
			break;
		case UVW:
			for (size_t i = 0; i < _texcoords[set].u.size(); i++) s.uvw.push_back(XMFLOAT3(_texcoords[set].u[i], 0.0f, 0.0f));
			break;
		case UVWX:
			for (size_t i = 0; i < _texcoords[set].u.size(); i++) s.uvwx.push_back(XMFLOAT4(_texcoords[set].u[i], 0.0f, 0.0f, 0.0f));
			break;
		}
	}
	break;
	case UV:
	{
		switch (s.type)
		{
		case U:
			for (size_t i = 0; i < _texcoords[set].uv.size(); i++) s.u.push_back(_texcoords[set].uv[i].x);
			break;
		case UVW:
			for (size_t i = 0; i < _texcoords[set].uv.size(); i++) s.uvw.push_back(XMFLOAT3(_texcoords[set].uv[i].x, _texcoords[set].uv[i].y, 0.0f));
			break;
		case UVWX:
			for (size_t i = 0; i < _texcoords[set].uv.size(); i++) s.uvwx.push_back(XMFLOAT4(_texcoords[set].uv[i].x, _texcoords[set].uv[i].y, 0.0f, 0.0f));
			break;
		}
	}
	break;
	case UVW:
	{
		switch (s.type)
		{
		case U:
			for (size_t i = 0; i < _texcoords[set].uvw.size(); i++) s.u.push_back(_texcoords[set].uvw[i].x);
			break;
		case UV:
			for (size_t i = 0; i < _texcoords[set].uvw.size(); i++) s.uv.push_back(XMFLOAT2(_texcoords[set].uvw[i].x, _texcoords[set].uvw[i].y));
			break;
		case UVWX:
			for (size_t i = 0; i < _texcoords[set].uvw.size(); i++) s.uvwx.push_back(XMFLOAT4(_texcoords[set].uvw[i].x, _texcoords[set].uvw[i].y, _texcoords[set].uvw[i].z, 0.0f));
			break;
		}
	}
	break;
	case UVWX:
	{
		switch (s.type)
		{
		case U:
			for (size_t i = 0; i < _texcoords[set].uvwx.size(); i++) s.u.push_back(_texcoords[set].uvwx[i].x);
			break;
		case UV:
			for (size_t i = 0; i < _texcoords[set].uvwx.size(); i++) s.uv.push_back(XMFLOAT2(_texcoords[set].uvwx[i].x, _texcoords[set].uvwx[i].y));
			break;
		case UVW:
			for (size_t i = 0; i < _texcoords[set].uvwx.size(); i++) s.uvw.push_back(XMFLOAT3(_texcoords[set].uvwx[i].x, _texcoords[set].uvwx[i].y, _texcoords[set].uvwx[i].z));
			break;
		}
	}
	break;
	default:;
	}
	_texcoords[set].type = type;
	std::swap(_texcoords[set].u, s.u); // NOTE: This is kinda nasty, but is should work. The formal way would be to swap the correct type of course! (std::move will prevent data copy!)
}

void StdGeometry::SetTexCoordSetFormat(uint32 set, TexCoordSetFormat fmt)
{
	_texcoords[set].fmt = fmt;
}



template<typename T>
void _addDataStream(InputLayoutDesc2& ild, UINT slot, DataBuffer* vdb, UINT* vs, const List<T>& data, UINT* offset, DXGI_FORMAT fmt, LPCSTR str, UINT index = 0)
{
	D3D12_INPUT_ELEMENT_DESC d;
	d.AlignedByteOffset = offset[slot];
	d.Format = fmt;
	d.InputSlot = slot;
	d.InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
	d.InstanceDataStepRate = 0;
	d.SemanticIndex = index;
	d.SemanticName = str;
	ild.AddElement(d);
	for (uint32 i = 0; i < data.size(); i++)
		*(T*)(vdb[slot].getBuffer() + offset[slot] + vs[slot] * i) = data[i];
	offset[slot] += dxgiformat::BitsPerPixel(d.Format) / 8;
}

XMFLOAT2 _spheremapTransformEncode(const XMFLOAT3& n)
{
	float l = sqrt(n.x * n.x + n.y * n.y + n.z * n.z);
	float p = sqrt(n.z / l * 8.0f + 8.0f) * l;
	if (p == 0.0f)
		return XMFLOAT2(0.8535533f, 0.8535533f); // (sqrt(2.0f)+2.0f)*0.25f
	return XMFLOAT2(n.x / p + 0.5f, n.y / p + 0.5f);
}

void _addDataStreamForNormals(InputLayoutDesc2& ild, UINT slot, DataBuffer* vdb, UINT* vs, const List<XMFLOAT3>& data, UINT* offset, DXGI_FORMAT fmt, LPCSTR str, UINT index = 0)
{
	switch (fmt)
	{
	case DXGI_FORMAT_R32G32B32_FLOAT: // 12 bytes
		_addDataStream(ild, slot, vdb, vs, data, offset, fmt, str, index);
		break;
	case DXGI_FORMAT_R16G16B16A16_SNORM: // 8 bytes
	{
		List<XMSHORTN4> l;
		l.reserve(data.size());
		for (size_t i = 0; i < data.size(); i++)
			l.push_back(XMSHORTN4(data[i].x, data[i].y, data[i].z, 0.0f));
		_addDataStream(ild, slot, vdb, vs, l, offset, fmt, str, index);
	}
	break;
	case DXGI_FORMAT_R10G10B10A2_UNORM: // 4 bytes
	{
		List<XMUDECN4> l;
		l.reserve(data.size());
		for (size_t i = 0; i < data.size(); i++)
			l.push_back(XMUDECN4(data[i].x * 0.5f + 0.5f, data[i].y * 0.5f + 0.5f, data[i].z * 0.5f + 0.5f, 0.0f));
		_addDataStream(ild, slot, vdb, vs, l, offset, fmt, str, index);
	}
	break;
	case DXGI_FORMAT_R32G32_FLOAT: // 8 bytes
	{
		List<XMFLOAT2> l;
		l.reserve(data.size());
		for (size_t i = 0; i < data.size(); i++)
			l.push_back(_spheremapTransformEncode(data[i]));
		_addDataStream(ild, slot, vdb, vs, l, offset, fmt, str, index);
	}
	break;
	case DXGI_FORMAT_R16G16_UNORM: // 4 bytes
	{
		List<XMUSHORTN2> l;
		l.reserve(data.size());
		for (size_t i = 0; i < data.size(); i++) {
			XMFLOAT2 f = _spheremapTransformEncode(data[i]);
			l.push_back(XMUSHORTN2((FLOAT*)&f));
		}
		_addDataStream(ild, slot, vdb, vs, l, offset, fmt, str, index);
	}
	break;
	default:
		assert(false);
		break;
	}
}

template<typename T>
void _addDataStreamForTangentSpace(InputLayoutDesc2& ild, UINT slot, DataBuffer* vdb, UINT* vs, const List<XMFLOAT3>& normals, const List<XMFLOAT3>& tangents, const List<XMFLOAT3>& bitangents, uint32 I, void(__vectorcall* XMStoreFunction)(T*, FXMVECTOR), UINT* offset, DXGI_FORMAT fmt, LPCSTR str, UINT index = 0)
{
	const List<XMFLOAT3>* TNB[3] = { &tangents, &normals, &bitangents };

	List<T> tangentSpaceQuats(TNB[I]->size(), T());
	XMMATRIX tangentSpace;
	tangentSpace.r[3] = XMVectorSet(0, 0, 0, 1);

	for (size_t i = 0; i < tangentSpaceQuats.size(); i++) {
		tangentSpace.r[(I + 0) % 3] = XMVector3Normalize(XMLoadFloat3(&(*TNB[(I + 0) % 3])[i]));
		tangentSpace.r[(I + 1) % 3] = XMVector3Normalize(XMLoadFloat3(&(*TNB[(I + 1) % 3])[i]));
		tangentSpace.r[(I + 2) % 3] = XMVector3Normalize(XMVector3Cross(tangentSpace.r[(I + 0) % 3], tangentSpace.r[(I + 1) % 3])); // b=txn, t=nxb, n=bxt
		(*XMStoreFunction)(&tangentSpaceQuats[i], XMQuaternionRotationMatrix(tangentSpace));
	}
	_addDataStream(ild, slot, vdb, vs, tangentSpaceQuats, offset, fmt, str, index);
}

// Almost hackish...
inline void XM_CALLCONV XMStoreUShort(USHORT* pDestination, FXMVECTOR V)
{
	XMUSHORT2 s;
	XMStoreUShort2(&s, V);
	*pDestination = s.x;
}

template<typename T, typename S>
void _addDataStreamForTexCoords(InputLayoutDesc2& ild, UINT slot, DataBuffer* vdb, UINT* vs, const List<T>& data, XMVECTOR(__vectorcall* XMLoadFunction)(const T*), void(__vectorcall* XMStoreFunction)(S*, FXMVECTOR), UINT* offset, DXGI_FORMAT fmt, LPCSTR str, UINT index = 0)
{
	List<S> l(data.size(), S());
	for (size_t i = 0; i < data.size(); i++)
		(*XMStoreFunction)(&l[i], (*XMLoadFunction)(&data[i]));
	_addDataStream(ild, slot, vdb, vs, l, offset, fmt, str, index);
}

void _addDataStreamForTexCoords(InputLayoutDesc2& ild, UINT slot, DataBuffer* vdb, UINT* vs, const StdGeometry::TexCoordSet& data, UINT* offset, DXGI_FORMAT fmt, LPCSTR str, UINT index = 0)
{
	switch (data.fmt)
	{
	case StdGeometry::TCF_FLOAT:
		switch (data.type)
		{
		case StdGeometry::U: _addDataStream(ild, slot, vdb, vs, data.u, offset, fmt, str, index); break;
		case StdGeometry::UV: _addDataStream(ild, slot, vdb, vs, data.uv, offset, fmt, str, index); break;
		case StdGeometry::UVW: _addDataStream(ild, slot, vdb, vs, data.uvw, offset, fmt, str, index); break;
		case StdGeometry::UVWX: _addDataStream(ild, slot, vdb, vs, data.uvwx, offset, fmt, str, index); break;
		}
		break;
	case StdGeometry::TCF_UNORM16:
		switch (data.type)
		{
		case StdGeometry::U: _addDataStreamForTexCoords(ild, slot, vdb, vs, data.u, XMLoadFloat, XMStoreUShort, offset, fmt, str, index); break;
		case StdGeometry::UV: _addDataStreamForTexCoords(ild, slot, vdb, vs, data.uv, XMLoadFloat2, XMStoreUShort2, offset, fmt, str, index); break;
		case StdGeometry::UVW: _addDataStreamForTexCoords(ild, slot, vdb, vs, data.uvw, XMLoadFloat3, XMStoreUShort4, offset, fmt, str, index); break;
		case StdGeometry::UVWX: _addDataStreamForTexCoords(ild, slot, vdb, vs, data.uvwx, XMLoadFloat4, XMStoreUShort4, offset, fmt, str, index); break;
		}
		break;
	default:
		assert(false);
		break;
	}
}

D3D12_PRIMITIVE_TOPOLOGY_TYPE __getPTT(D3D12_PRIMITIVE_TOPOLOGY pt)
{
	switch (pt)
	{
	case D3D_PRIMITIVE_TOPOLOGY_UNDEFINED: return D3D12_PRIMITIVE_TOPOLOGY_TYPE_UNDEFINED;
	case D3D_PRIMITIVE_TOPOLOGY_POINTLIST: return D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
	case D3D_PRIMITIVE_TOPOLOGY_LINELIST: return D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
	case D3D_PRIMITIVE_TOPOLOGY_LINESTRIP: return D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
	case D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST: return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	case D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP: return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	case D3D_PRIMITIVE_TOPOLOGY_LINELIST_ADJ: return D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
	case D3D_PRIMITIVE_TOPOLOGY_LINESTRIP_ADJ: return D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
	case D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST_ADJ: return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	case D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP_ADJ: return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	default: break;
	}
	return D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
}

void StdGeometry::CreateDeviceObjects()
{
	// Destroy old data
	DestroyDeviceObjects();

	const auto& positions = GetPositions();
	const auto& colors = GetColors();
	const auto& normals = GetNormals();
	const auto& tangents = GetTangents();
	const auto& bitangents = GetBitangents();
	const auto& texcoords = GetTexCoordsArray();
	const auto& indices = GetIndices();
	const auto& blendIndices = GetBlendIndices();
	const auto& blendWeights = GetBlendWeights();
	const GeometrySubsetList& subsets = GetSubsets();

	Streams streams = GetElementStreams();


	const auto& ss = GetSubsets();

	// Make sure data is valid
	if (ss.size() == 0) // No subsets?
		throw GraphicsException(this, MTEXT("No subsets defined."), WARN);

	D3D12_PRIMITIVE_TOPOLOGY_TYPE ptt = __getPTT((D3D12_PRIMITIVE_TOPOLOGY)ss.front().pt);

	if (positions.size() == 0 || !streams.active(POSITIONS)) { // No positions?
		if (positions.empty() && colors.empty() && normals.empty() && tangents.empty() && bitangents.empty() && texcoords[0].size() == 0 && texcoords[1].size() == 0
			&& texcoords[2].size() == 0 && texcoords[3].size() == 0 && indices.empty() && blendIndices.empty() && blendWeights.empty())
		{
			// This is for geometryless primitives to work....
			_isInit = true;
			return  Geometry::CreateDeviceObjects(ptt, { (const D3D12_INPUT_ELEMENT_DESC*)nullptr, 0u }, List<DataBuffer>(), List<UINT>());;
		}
		else
			throw GraphicsException(this, MTEXT("A POSITION stream is required."), WARN);
	}

	if (GetAPI() == DRAW_INDEXED && indices.size() == 0) // No indices for indexed draw?
		throw GraphicsException(this, MTEXT("No data specified for index buffer."), WARN);

	if (ptt == D3D12_PRIMITIVE_TOPOLOGY_TYPE_UNDEFINED)
		throw GraphicsException(this, MTEXT("No primitive topology specified."), WARN);

	for (size_t i = 0; i < subsets.size(); i++) {
		if (__getPTT((D3D12_PRIMITIVE_TOPOLOGY)ss[i].pt) != ptt)
			throw GraphicsException(this, MTEXT("Primitive topology mismatch in subsets."), WARN);
	}

	bool use8bitBlendIndices = false; // We use 16-bits blend indices by default (Up to 64k joints), but we may manage with 8-bits (Up to 256 joints).
	if (streams.active(BLENDINDICES)) {
		size_t i = 0;
		for (; i < blendIndices.size(); i++)
			if ((blendIndices[i].x | blendIndices[i].y | blendIndices[i].z | blendIndices[i].w) & 0xFF00)
				break;
		use8bitBlendIndices = i == blendIndices.size();
	}

	// Tips for compressing vertex: (http://humus.name/Articles/Persson_CreatingVastGameWorlds.pdf)

	// Position compression:
	// - Uncompressed 3x4 bytes (12 bytes) floating point (DXGI_FORMAT_R32G32B32_FLOAT)
	// - Unsigned normalized 4x2 bytes (8 bytes) with bias&scale (DXGI_FORMAT_R16G16B16A16_SNORM)

	// Texture coord compression:
	// - Uncompressed 4 bytes floating point pr comp
	// - Unsigned normalized 2 bytes pr comp. (possibly with bias&scale if out of [0,1])

	// Normal&tangent space compressions:
	//	1. Raw normals 3x4bytes=12bytes (DXGI_FORMAT_R32G32B32_FLOAT)
	//	2. Signed normalized 4x2bytes=8bytes (DXGI_FORMAT_R16G16B16A16_SNORM)
	//	3. 10-10-10-2 uint32 (needs x*2-1 in VS): 4 bytes (DXGI_FORMAT_R10G10B10A2_UNORM)
	//	4. enc/dec: 2x2bytes=4bytes (DXGI_FORMAT_R16G16_SNORM)  (Method #4: Spheremap Transform from (http://aras-p.info/texts/CompactNormalStorage.html)
	//	5. Tangent space quaternion: 4x1(2)bytes=4/8bytes (DXGI_FORMAT_R8G8B8A8_SNORM/DXGI_FORMAT_R16G16B16A16_SNORM) (From the humus link above)
	//  For method 1-4 with tangent space we could skip 1 of the three comps and do a cross product in shader to get third.

	// TODO: New feature: Pack texcoords and possibly also normals into as few elements as possible (xy xy => xyzw)

	DXGI_FORMAT tangentSpaceFormat = DXGI_FORMAT_UNKNOWN;

	switch (GetTangentSpaceCompression())
	{
	case TSC_NONE: tangentSpaceFormat = DXGI_FORMAT_R32G32B32_FLOAT; break;
	case TSC_SNORM16: tangentSpaceFormat = DXGI_FORMAT_R16G16B16A16_SNORM; break;
	case TSC_10_10_10_2: tangentSpaceFormat = DXGI_FORMAT_R10G10B10A2_UNORM; break;
	case TSC_SPHEREMAP_TRANSFORM8: tangentSpaceFormat = DXGI_FORMAT_R32G32_FLOAT; break;
	case TSC_SPHEREMAP_TRANSFORM4: tangentSpaceFormat = DXGI_FORMAT_R16G16_UNORM; break;
	case TSC_QUAT16: tangentSpaceFormat = DXGI_FORMAT_R32G32B32A32_FLOAT; break;
	case TSC_QUAT8: tangentSpaceFormat = DXGI_FORMAT_R16G16B16A16_SNORM; break;
	case TSC_QUAT4: tangentSpaceFormat = DXGI_FORMAT_R8G8B8A8_SNORM; break;
	}

	// Formats for the different types
	const DXGI_FORMAT position_fmt = DXGI_FORMAT_R32G32B32_FLOAT; // scale&bias compressed: DXGI_FORMAT_R16G16B16A16_SNORM; // 12/8(2 unused) bytes
	const DXGI_FORMAT normal_fmt = tangentSpaceFormat;
	const DXGI_FORMAT tangent_fmt = tangentSpaceFormat;
	const DXGI_FORMAT bitangent_fmt = tangentSpaceFormat;
	const DXGI_FORMAT color_fmt = DXGI_FORMAT_B8G8R8A8_UNORM; // 4 bytes
	const DXGI_FORMAT u_fmt = DXGI_FORMAT_R32_FLOAT; // Compressed: DXGI_FORMAT_R16_SNORM; // 4/2 bytes
	const DXGI_FORMAT uv_fmt = DXGI_FORMAT_R32G32_FLOAT; // Compressed: DXGI_FORMAT_R16G16_SNORM; // 8/4 bytes
	const DXGI_FORMAT uvw_fmt = DXGI_FORMAT_R32G32B32_FLOAT; // Compressed: DXGI_FORMAT_R16G16B16A16_SNORM; 12/8(2 unused) bytes
	const DXGI_FORMAT uvwx_fmt = DXGI_FORMAT_R32G32B32A32_FLOAT; // Compressed: DXGI_FORMAT_R16G16B16A16_SNORM; 16/8 bytes
	const DXGI_FORMAT blendWeight_fmt = DXGI_FORMAT_R8G8B8A8_UNORM; // 4 bytes
	const DXGI_FORMAT blendIndices_fmt = use8bitBlendIndices ? DXGI_FORMAT_R8G8B8A8_UINT : DXGI_FORMAT_R16G16B16A16_UINT; // 4/8 bytes

	static const DXGI_FORMAT TexFormat[5][2] = { {DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_UNKNOWN}, {DXGI_FORMAT_R32_FLOAT, DXGI_FORMAT_R16_UNORM}, {DXGI_FORMAT_R32G32_FLOAT, DXGI_FORMAT_R16G16_UNORM}, {DXGI_FORMAT_R32G32B32_FLOAT, DXGI_FORMAT_R16G16B16A16_UNORM}, {DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_R16G16B16A16_UNORM} };

	uint32 maxSlot = 0; // This is the highest VS-stream slot number in use.
	for (uint32 i = 0; i < MAX_ELEMENTS; i++)
		if (streams.active(i))
			maxSlot = std::max(maxSlot, streams(i));

	assert(maxSlot < D3D12_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT);

	List<UINT> vs(maxSlot + 1, (UINT)0); // Vertex size of the streams

	vs[streams(POSITIONS)] += dxgiformat::BitsPerPixel(position_fmt) / 8; // Add size of positions

	if (streams.active(NORMALS)) { // Active normals?
		if (normals.size() != positions.size()) { // Valid size?
			streams(NORMALS) = DISABLE_ELEMENT;
			if (normals.size())
				AddMessage(ChipMessage(MTEXT("InvalidStreamCount"), MTEXT("Invalid number of components: Normals. Skipping it."), WARN));
		}
		else if (GetTangentSpaceCompression() < TSC_QUAT16) // Not using quat compression?
			vs[streams(NORMALS)] += dxgiformat::BitsPerPixel(normal_fmt) / 8; // Add size of normals
	}

	if (streams.active(TANGENTS)) { // Active tangents?
		if (tangents.size() != positions.size()) { // Valid size?
			streams(TANGENTS) = DISABLE_ELEMENT;
			if (tangents.size())
				AddMessage(ChipMessage(MTEXT("InvalidStreamCount"), MTEXT("Invalid number of components: Tangents. Skipping it."), WARN));
		}
		else if (GetTangentSpaceCompression() < TSC_QUAT16) // Not using quat compression?
			vs[streams(TANGENTS)] += dxgiformat::BitsPerPixel(tangent_fmt) / 8; // Add size of tangents
	}

	if (streams.active(BITANGENTS)) { // Active bitangents?
		if (bitangents.size() != positions.size()) { // Valid size?
			streams(BITANGENTS) = DISABLE_ELEMENT;
			if (bitangents.size())
				AddMessage(ChipMessage(MTEXT("InvalidStreamCount"), MTEXT("Invalid number of components: Bitangents. Skipping it."), WARN));
		}
		else if (GetTangentSpaceCompression() < TSC_QUAT16) // Not using quat compression?
			vs[streams(BITANGENTS)] += dxgiformat::BitsPerPixel(bitangent_fmt) / 8; // Add size of bitangents
	}

	uint32 tangentSpaceQuatCompressionType = -1;
	UINT tangentSpaceQuatStream = 0;

	if (streams.active(NORMALS) || streams.active(TANGENTS) || streams.active(BITANGENTS)) { // Active normals, tangents and/or bitangents?
		if (GetTangentSpaceCompression() >= TSC_QUAT16 && GetTangentSpaceCompression() <= TSC_QUAT4) { // Using tangent space quaternion compression?
													   // Note: Even if we have all three components, we only use two and derive the third using cross product.
			if (streams.active(NORMALS) && streams.active(TANGENTS)) { // Has normals and tangents?
				tangentSpaceQuatCompressionType = 0; // TN(B)
				tangentSpaceQuatStream = streams(NORMALS);
			}
			else if (streams.active(NORMALS) && streams.active(BITANGENTS)) { // Has normals and bitangents?
				tangentSpaceQuatCompressionType = 1; // NB(T)
				tangentSpaceQuatStream = streams(NORMALS);
			}
			else if (streams.active(TANGENTS) && streams.active(BITANGENTS)) {// Has tangents and bitangents?
				tangentSpaceQuatCompressionType = 2; // BT(N)
				tangentSpaceQuatStream = streams(TANGENTS);
			}

			if (tangentSpaceQuatCompressionType == -1) { // Invalid?
				AddMessage(ChipMessage(MTEXT("TangentSpace"), MTEXT("Not enough data for tangent space. Skipping it."), WARN));
			}
			else {
				vs[0] += dxgiformat::BitsPerPixel(tangentSpaceFormat); // Add size of tangent space quaternion.
			}
			streams(NORMALS) = streams(TANGENTS) = streams(BITANGENTS) = DISABLE_ELEMENT; // Disable the individual streams.
		}
	}

	if (streams.active(COLORS)) { // Active colors?
		if (colors.size() != positions.size()) { // Valid size?
			streams(COLORS) = DISABLE_ELEMENT;
			if (colors.size())
				AddMessage(ChipMessage(MTEXT("InvalidStreamCount"), MTEXT("Invalid number of components: Colors. Skipping it."), WARN));
		}
		else
			vs[streams(COLORS)] += dxgiformat::BitsPerPixel(color_fmt) / 8; // Add size of colors.
	}

	for (uint32 i = 0; i < MAX_TEXCOORD_SETS; i++) { // Iterate texture sets
		if (streams.active(TEXCOORD0 + i)) { // Active texture set?
			if (texcoords[i].size() != positions.size()) { // Valid size?
				streams(TEXCOORD0 + i) = DISABLE_ELEMENT;
				if (texcoords[i].size())
					AddMessage(ChipMessage(MTEXT("InvalidStreamCount"), strUtils::format(MTEXT("Invalid number of components: Texcoord%i. Skipping it."), i), WARN));
			}
			else
				vs[streams(TEXCOORD0 + i)] += dxgiformat::BitsPerPixel(TexFormat[texcoords[i].type][texcoords[i].fmt]) / 8; // Add size of texture set.
		}
	}

	if (streams.active(BLENDWEIGHTS)) { // Active blend weights?
		if (blendWeights.size() != positions.size()) { // Valid size?
			streams(BLENDWEIGHTS) = DISABLE_ELEMENT;
			if (blendWeights.size())
				AddMessage(ChipMessage(MTEXT("InvalidStreamCount"), MTEXT("Invalid number of components: BlendWeights. Skipping it."), WARN));
		}
		else
			vs[streams(BLENDWEIGHTS)] += dxgiformat::BitsPerPixel(blendWeight_fmt) / 8; // Add size of blend weights
	}

	if (streams.active(BLENDINDICES)) { // Active blend indices?
		if (blendIndices.size() != positions.size()) { // Valid size?
			streams(BLENDINDICES) = DISABLE_ELEMENT;
			if (blendIndices.size())
				AddMessage(ChipMessage(MTEXT("InvalidStreamCount"), MTEXT("Invalid number of components: BlendIndices. Skipping it."), WARN));
		}
		else
			vs[streams(BLENDINDICES)] += dxgiformat::BitsPerPixel(blendIndices_fmt) / 8; // Add size of blend indices
	}

	List<UINT> offset(maxSlot + 1, (UINT)0); // Byte offset of next element to add to streams
	List<DataBuffer> vdb(maxSlot + 1, DataBuffer()); // Vertex data to be pushed to gpu for the different streams.
	for (size_t i = 0; i < vdb.size(); i++)
		if (vs[i] > 0) // Data in stream?
			vdb[i].realloc(vs[i] * positions.size()); // Allocate stream size

	InputLayoutDesc2 ild; // Vertex descriptor

	// Add data to hw stream(s)
	DataBuffer* vertexData = &vdb.front();
	UINT* strides = &vs.front();
	UINT* offsets = &offset.front();

	_addDataStream(ild, 0, vertexData, strides, positions, offsets, position_fmt, "POSITION"); // Add positions

	if (streams.active(NORMALS)) _addDataStreamForNormals(ild, streams(NORMALS), vertexData, strides, normals, offsets, normal_fmt, "NORMAL"); // Add normals
	if (streams.active(TANGENTS)) _addDataStreamForNormals(ild, streams(TANGENTS), vertexData, strides, tangents, offsets, tangent_fmt, "TANGENT"); // Add tangents
	if (streams.active(BITANGENTS)) _addDataStreamForNormals(ild, streams(BITANGENTS), vertexData, strides, bitangents, offsets, bitangent_fmt, "BITANGENT"); // Add bitangents

	if (tangentSpaceQuatCompressionType != -1) { // Using tangent space quaternion compression?
		switch (GetTangentSpaceCompression())
		{
		case TSC_QUAT16:
			_addDataStreamForTangentSpace(ild, tangentSpaceQuatStream, vertexData, strides, normals, tangents, bitangents, tangentSpaceQuatCompressionType, XMStoreFloat4, offsets, tangentSpaceFormat, "TANGENTSPACE");
			break;
		case TSC_QUAT8:
			_addDataStreamForTangentSpace(ild, tangentSpaceQuatStream, vertexData, strides, normals, tangents, bitangents, tangentSpaceQuatCompressionType, XMStoreShortN4, offsets, tangentSpaceFormat, "TANGENTSPACE");
			break;
		case TSC_QUAT4:
			_addDataStreamForTangentSpace(ild, tangentSpaceQuatStream, vertexData, strides, normals, tangents, bitangents, tangentSpaceQuatCompressionType, XMStoreByteN4, offsets, tangentSpaceFormat, "TANGENTSPACE");
			break;
		default:
			assert(false);
			break;
		}
	}

	for (uint32 i = 0; i < MAX_TEXCOORD_SETS; i++) { // Iterate and add texture sets
		if (streams.active(TEXCOORD0 + i)) { // Texture set active?
			_addDataStreamForTexCoords(ild, streams(TEXCOORD0 + i), vertexData, strides, texcoords[i], offsets, TexFormat[texcoords[i].type][texcoords[i].fmt], "TEXCOORD", i); // Add texture coord
		}
	}

	if (streams.active(COLORS)) _addDataStream(ild, streams(COLORS), vertexData, strides, colors, offsets, color_fmt, "COLOR"); // Add colors
	if (streams.active(BLENDWEIGHTS)) _addDataStream(ild, streams(BLENDWEIGHTS), vertexData, strides, blendWeights, offsets, blendWeight_fmt, "BLENDWEIGHTS"); // Add blend weights

	if (streams.active(BLENDINDICES)) { // Using blend indices?
		if (use8bitBlendIndices) { // 8 bits pr comp?
			List<XMUBYTE4> blendIndices8;
			blendIndices8.reserve(blendIndices.size());
			for (size_t i = 0; i < blendIndices.size(); i++)
				blendIndices8.push_back(XMUBYTE4((BYTE)blendIndices[i].x, (BYTE)blendIndices[i].y, (BYTE)blendIndices[i].z, (BYTE)blendIndices[i].w));
			_addDataStream(ild, streams(BLENDINDICES), vertexData, strides, blendIndices8, offsets, blendIndices_fmt, "BLENDINDICES");
		}
		else
			_addDataStream(ild, streams(BLENDINDICES), vertexData, strides, blendIndices, offsets, blendIndices_fmt, "BLENDINDICES");
	}


	DXGI_FORMAT ifmt = DXGI_FORMAT_R32_UINT;
	DataBuffer idata;

	// Set up index buffer if using it
	if (GetAPI() == DRAW_INDEXED) {
		bool i32 = positions.size() > 65535;
		UINT byteWidth = (uint32)indices.size() * (i32 ? sizeof(UINT) : sizeof(USHORT));
		ifmt = i32 ? DXGI_FORMAT_R32_UINT : DXGI_FORMAT_R16_UINT;
		idata.realloc(byteWidth);
		if (i32) {
			std::memcpy(idata.getBuffer(), &indices.front(), byteWidth);
		}
		else {
			for (uint32 i = 0; i < indices.size(); i++)
				((USHORT*)idata.getBuffer())[i] = (USHORT)indices[i];
		}
	}

	return Geometry::CreateDeviceObjects(ptt, ild, vdb, vs, idata, ifmt);
}
