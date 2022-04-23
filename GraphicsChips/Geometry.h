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
#include "GraphicsDefines.h"
#include "M3DEngine/Chip.h"
#include "M3DCore/AxisAlignedBox.h"
#include "M3DCore/Containers.h"
#include "M3DCore/DataBuffer.h"
#include "M3DCore/Frustum.h"
#include "Graphics.h"

namespace m3d
{


static const Guid GEOMETRY_GUID = { 0xf75cc4d4, 0xbb15, 0x40de, { 0xb7, 0x18, 0x07, 0xc3, 0xbc, 0x40, 0x66, 0x1b } };

enum DrawApi { DRAW, DRAW_INDEXED };

struct GeometrySubset
{
	String name; // Name of the subset. does not have to be unique.
	AxisAlignedBox boundingBox; // Subset bounding box.
	M3D_PRIMITIVE_TOPOLOGY pt = M3D_PRIMITIVE_TOPOLOGY_UNDEFINED; // The primitive topology for this subset. They must belong to the same group as set for the Geometry (points, lines, triangles, patch)
	UINT count = 0u; // vertexCount if api is DRAW, else indexCount
	UINT startLocation = 0u; // startVertexLocation if api is DRAW, else startIndexLocation
	INT baseVertexLocation = 0; // Offset into the vertex array
	mutable uint32 _cullingPlane = 0u; // This MAY improve performance in some cases by testing the frustum plane we failed last frame first.. Works good in theory :)
};

typedef List<GeometrySubset> GeometrySubsetList;
//template class GRAPHICSCHIPS_API List<GeometrySubset>;

class GRAPHICSCHIPS_API Geometry : public Chip, public GraphicsUsage
{
	CHIPDESC_DECL;
	CHIPMSG(MSG_VB_FAIL, FATAL, MTEXT("Failed to create vertex buffer!"))
	CHIPMSG(MSG_IB_FAIL, FATAL, MTEXT("Failed to create index buffer!"))
public:
	bool CopyChip(Chip* chip) override;
	bool LoadChip(DocumentLoader& loader) override;
	bool SaveChip(DocumentSaver& saver) const override;

	void OnDestroyDevice() override { return DestroyDeviceObjects(); }

	// Prepares GPU before rendering. Will create device objects if they do not exist!
	virtual void Prepare();
	// Updates anything neccessary in the geometry!
	virtual void Update() {}
	// Create gpu objects. Most chips overrides this.
	virtual void CreateDeviceObjects() {}
	// Creates gpu objects using given buffer data.
	virtual void CreateDeviceObjects(D3D12_PRIMITIVE_TOPOLOGY_TYPE ptt, const D3D12_INPUT_LAYOUT_DESC& ild, const List<DataBuffer>& vertexBufferData, const List<UINT> vertexBufferStrides, const DataBuffer& indexBufferData = DataBuffer(), DXGI_FORMAT indexFormat = DXGI_FORMAT_R32_UINT);
	// Destroy gpu objects
	virtual void DestroyDeviceObjects();
	// true if gpu objects is created.
	bool IsGPUInit() const { return _isInit; }

	// Clears all settings in the chip.
	virtual void Clear();
	// Get the draw api. (If we are to use index buffer or not) To be overridden!
	virtual DrawApi GetAPI() const { return DRAW; }
	// Sets the subsets.
	virtual void SetSubsets(const GeometrySubsetList& subsets) { _subsets = subsets; }
	// Adds a new subset to the end of the list.
	virtual void AddSubset(const GeometrySubset& subset) { _subsets.push_back(subset); }
	// Sets a subset at given index.
	virtual void SetSubset(const GeometrySubset& subset, uint32 index) { _subsets[index] = subset; }
	// Gets the list of subsets.
	virtual const GeometrySubsetList& GetSubsets() const { return _subsets; }

	// Set bounding box for given subset.
	virtual void SetBoundingBox(const AxisAlignedBox& aabb, uint32 subset) { _subsets[subset].boundingBox = aabb; }

	// When uploaded to gpu, this is the InputElementDescsID.
	virtual PipelineStateDescID GetPipelineInputStateDescID() const { return _pisID; }
	// Only available after device objects are created.
	virtual D3D12_INDEX_BUFFER_VIEW GetIndexBufferView() const { return _indexBufferView; }

protected:
	GeometrySubsetList _subsets;

	// These are generated!
	uint32 _cullingPlane = Frustum::LEFTP; // This MAY improve performance in some cases by testing the frustum plane we failed last frame first.. Works good in theory :)

	struct VertexBuffer
	{
		RID3D12Resource resource;
		D3D12_VERTEX_BUFFER_VIEW view;
	};

	// These are generated!
	PipelineStateDescID _pisID = 0;
	List<VertexBuffer> _vertexBuffers;
	RID3D12Resource _indexBuffer;
	D3D12_INDEX_BUFFER_VIEW _indexBufferView;

	bool _isInit = false;
};




}