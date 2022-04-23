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
#include "Geometry.h"


namespace m3d
{

static const Guid STDGEOMETRY_GUID = { 0x402a1e54, 0x5b7e, 0x4bf5, { 0xa4, 0xd6, 0x65, 0xd7, 0x29, 0xfe, 0x70, 0x43 } };


class GRAPHICSCHIPS_API StdGeometry : public Geometry
{
	CHIPDESC_DECL;
	CHIPMSG(MSG_NO_SUBSETS, WARN, MTEXT("No subsets!"))
	CHIPMSG(MSG_NO_POSITIONS, WARN, MTEXT("No position data!"))
	CHIPMSG(MSG_NO_DATA_FOR_INDEX_BUFFER, WARN, MTEXT("No data for index buffer!"))

public:
	enum Element { POSITIONS, NORMALS, TANGENTS, BITANGENTS, COLORS, BLENDWEIGHTS, BLENDINDICES, TEXCOORD0, TEXCOORD1, TEXCOORD2, TEXCOORD3, MAX_ELEMENTS };

	static const uint32 MAX_TEXCOORD_SETS = 4;
	static const uint32 DISABLE_ELEMENT = -1;

	struct Streams
	{
		uint32 s[MAX_ELEMENTS] = {};
		uint32 operator()(uint32 e) const { return s[e]; }
		uint32& operator()(uint32 e) { return s[e]; }
		bool active(uint32 e) { return s[e] != DISABLE_ELEMENT; }
	};

	enum TexCoordSetType { NONE, U, UV, UVW, UVWX };
	enum TexCoordSetFormat { TCF_FLOAT, TCF_UNORM16 };
	struct TexCoordSet
	{
		// These are union.
		List<float32> u;
		List<XMFLOAT2> uv;
		List<XMFLOAT3> uvw;
		List<XMFLOAT4> uvwx;

		TexCoordSetType type;
		TexCoordSetFormat fmt;

		TexCoordSet() : type(NONE), fmt(TCF_FLOAT) {}

		void clear()
		{
			switch (type)
			{
			case U: u.clear(); break;
			case UV: uv.clear(); break;
			case UVW: uvw.clear(); break;
			case UVWX: uvwx.clear(); break;
			default:;
			}
			type = NONE;
		}
		size_t size() const
		{
			switch (type)
			{
			case U: return u.size();
			case UV: return uv.size();
			case UVW: return uvw.size();
			case UVWX: return uvwx.size();
			default:;
			}
			return 0;
		}
	};

	// TSC_NONE: 12 bytes (DXGI_FORMAT_R32G32B32_FLOAT). Very precise! No VS decode.
	// TSC_SNORM16: 8 bytes. (DXGI_FORMAT_R16G16B16A16_SNORM) Precise. No VS decode. Max component-wise error of 0.00002.
	// TSC_10_10_10_2: 4 bytes. (DXGI_FORMAT_R10G10B10A2_UNORM) Unprecise (But better than SNORM8). VS decode: xyz*2-1. About 130 times less precise than TSC_SNORM16, but very even error.
	// TSC_SPHEREMAP_TRANSFORM8: 8 bytes. (DXGI_FORMAT_R32G32_FLOAT) Requires VS decode func. About 5 times more precise than TSC_SNORM16 on average, but 1000 times worse at most.
	// TSC_SPHEREMAP_TRANSFORM4: 4 bytes. (DXGI_FORMAT_R16G16_SNORM) Requires VS decode func. About 3 times less precise than TSC_SNORM16 on average, but 1000 times worse at most.
	// TSC_QUAT16: 16 bytes. (DXGI_FORMAT_R32G32B32A32_FLOAT) Encodes whole tangent space. Requires VS decode. Very precise!
	// TSC_QUAT8: 8 bytes. (DXGI_FORMAT_R16G16B16A16_SNORM) Encodes whole tangent space. Requires VS decode. About half precision of TSC_SNORM16.
	// TSC_QUAT4: 4 bytes. (DXGI_FORMAT_R8G8B8A8_SNORM) Encodes whole tangent space. Requires VS decode. About 1000 less precise than TSC_SNORM16, but requires only 4 bytes to include whole tangent space!
	enum TangentSpaceCompression { TSC_NONE, TSC_SNORM16, TSC_10_10_10_2, TSC_SPHEREMAP_TRANSFORM8, TSC_SPHEREMAP_TRANSFORM4, TSC_QUAT16, TSC_QUAT8, TSC_QUAT4 };

	
	bool CopyChip(Chip* chip) override;
	bool LoadChip(DocumentLoader& loader) override;
	bool SaveChip(DocumentSaver& saver) const override;

	void Clear() override;

	virtual DrawApi GetAPI() const { return _indices.size() > 0 ? DRAW_INDEXED : DRAW; }

	// Rember to set the API (DRAW or DRAW_INDEXED) before calling this method!
	virtual void CommitSubset(M3D_PRIMITIVE_TOPOLOGY pt, String name = MTEXT(""));

	virtual void AddPosition(const XMFLOAT3& pos) { _positions.push_back(pos); }
	virtual void AddNormal(const XMFLOAT3& normal) { _normals.push_back(normal); }
	virtual void AddTangent(const XMFLOAT3& tangent) { _tangents.push_back(tangent); }
	virtual void AddBitangent(const XMFLOAT3& bitangent) { _bitangents.push_back(bitangent); }
	virtual void AddColor(const XMCOLOR& color) { _colors.push_back(color); }
	virtual void AddTexCoord(float32 tc, uint32 set = 0) { if (_texcoords[set].type == NONE || _texcoords[set].type == U) { _texcoords[set].u.push_back(tc); _texcoords[set].type = U; } }
	virtual void AddTexCoord(const XMFLOAT2& tc, uint32 set = 0) { if (_texcoords[set].type == NONE || _texcoords[set].type == UV) { _texcoords[set].uv.push_back(tc); _texcoords[set].type = UV; } }
	virtual void AddTexCoord(const XMFLOAT3& tc, uint32 set = 0) { if (_texcoords[set].type == NONE || _texcoords[set].type == UVW) { _texcoords[set].uvw.push_back(tc); _texcoords[set].type = UVW; } }
	virtual void AddTexCoord(const XMFLOAT4& tc, uint32 set = 0) { if (_texcoords[set].type == NONE || _texcoords[set].type == UVWX) { _texcoords[set].uvwx.push_back(tc); _texcoords[set].type = UVWX; } }
	virtual void AddBlendWeights(const XMUBYTEN4& weights) { _blendWeights.push_back(weights); }
	virtual void AddBlendIndices(const XMUSHORT4& indices) { _blendIndices.push_back(indices); }
	virtual void AddIndex(UINT index) { _indices.push_back(index); }

	virtual const List<XMFLOAT3>& GetPositions() const { return _positions; }
	virtual const List<XMFLOAT3>& GetNormals() const { return _normals; }
	virtual const List<XMFLOAT3>& GetTangents() const { return _tangents; }
	virtual const List<XMFLOAT3>& GetBitangents() const { return _bitangents; }
	virtual const List<XMCOLOR>& GetColors() const { return _colors; }
	virtual const TexCoordSet& GetTexCoords(uint32 set = 0) const { return _texcoords[set]; }
	virtual const TexCoordSet *GetTexCoordsArray() const { return _texcoords; }
	virtual const List<XMUBYTEN4>& GetBlendWeights() const { return _blendWeights; }
	virtual const List<XMUSHORT4>& GetBlendIndices() const { return _blendIndices; }
	virtual const List<UINT>& GetIndices() const { return _indices; }

	virtual void ClearPositions() { _positions.clear(); }
	virtual void ClearNormals() { _normals.clear(); }
	virtual void ClearTangents() { _tangents.clear(); }
	virtual void ClearBitangents() { _bitangents.clear(); }
	virtual void ClearColors() { _colors.clear(); }
	virtual void ClearTexcoords(uint32 set = 0) { _texcoords[set].clear(); }
	virtual void ClearBlendWeights() { _blendWeights.clear(); }
	virtual void ClearBlendIndices() { _blendIndices.clear(); }
	virtual void ClearIndices() { _indices.clear(); }

	virtual void TransformPositions(CXMMATRIX m);
	virtual void TransformNormals(CXMMATRIX m);
	virtual void TransformTexCoordSet(CXMMATRIX m, uint32 set = 0);

	virtual void SwapTexcoords(uint32 set1, uint32 set2);
	virtual void ChangeTexCoordSetType(uint32 set, TexCoordSetType type);
	virtual void SetTexCoordSetFormat(uint32 set, TexCoordSetFormat fmt);

	// Set/Get DISABLE_ELEMENT disables element. 
	// stream must be < D3D12_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT (32). 
	virtual uint32 GetElementStream(Element element) const { return _streams(element); }
	virtual void SetElementStream(Element element, uint32 stream) { _streams(element) = stream; }
	virtual const Streams& GetElementStreams() const { return _streams; }

	virtual TangentSpaceCompression GetTangentSpaceCompression() const { return _tsc; }
	virtual void SetTangentSpaceCompression(TangentSpaceCompression tsc) { _tsc = tsc; }

	/*
	// Function for decoding spheremap transform: (Method #4: Spheremap Transform from http://aras-p.info/texts/CompactNormalStorage.html)
	half3 decode(half2 enc)
	{
	half2 fenc = enc*4.0-2.0;
	half f = dot(fenc,fenc);
	return half3(fenc*sqrt(max(0,1.0-f*0.25)), 1.0-f*0.5);
	}

	// This is the encoding function:
	half2 encode(half3 n)
	{
	half p = sqrt(n.z * 8.0 + 8.0);
	if (p == 0.0)
	return float2(0.8535533, 0.8535533f);
	return half2(n.xy / p + 0.5);
	}
	*/

	/*
	// Tangent space quaternion compression decode function:
	// TIPS: Instead of transforming n, t and b using world matrix, rotate q using world quaternion!
	void decode(float4 q, out float3 n, out float3 t, out float3 b)
	{
	t = float3(1,0,0)+float3(-2,2,-2)q.yxw*q.y*+float3(-2,2,2)*q.zwx*q.z;
	n = float3(0,1,0)+float3(-2,-2,2)q.wzy*q.z*+float3(2,-2,2)*q.yxw*q.x;
	b = float3(0,0,1)+float3(2,-2,-2)q.zwx*q.x*+float3(2,2,-2)*q.wzy*q.y;
	}
	*/

	virtual bool IsPackNormals() const { return _packNormals; }
	virtual void SetPackNormals(bool b) { _packNormals = b; }
	virtual bool IsPackTextureSets() const { return _packTexcoords; }
	virtual void SetPackTextureSets(bool b) { _packTexcoords = b; }

	void CreateDeviceObjects() override;

protected:
	List<XMFLOAT3> _positions; // 12 bytes pr vertex.
	List<XMFLOAT3> _normals; // GPU format depends on TangentSpaceCompression.
	List<XMFLOAT3> _tangents; // GPU format depends on TangentSpaceCompression.
	List<XMFLOAT3> _bitangents; // GPU format depends on TangentSpaceCompression.
	List<XMCOLOR> _colors; // 4 bytes pr vertex.
	TexCoordSet _texcoords[MAX_TEXCOORD_SETS];
	List<XMUBYTEN4> _blendWeights; // 8-bits: 0.2% error (0.5 / 256)
	List<XMUSHORT4> _blendIndices; // Can be compressed to 8-bits when uploaded to gpu.
	List<UINT> _indices; // Can be compressed to 16-bits when uploaded to gpu.

	Streams _streams;

	TangentSpaceCompression _tsc = TSC_NONE;
	bool _packNormals = false;
	bool _packTexcoords = false;

	void _calculateBoundingBox(GeometrySubset& ss);
};


}