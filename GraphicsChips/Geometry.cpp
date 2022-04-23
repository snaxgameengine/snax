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
#include "Geometry.h"
#include "M3DEngine/DocumentSaveLoadUtil.h"
#include "M3DCore/DataBuffer.h"
#include "M3DCore/Frustum.h"
#include "RenderSettings.h"


using namespace m3d;

CHIPDESCV1_DEF(Geometry, MTEXT("Geometry"), GEOMETRY_GUID, CHIP_GUID);


namespace m3d
{
	bool SerializeDocumentData(DocumentSaver& saver, const GeometrySubset& data)
	{
		SAVE("name", data.name);
		SAVE("primitiveTopology", data.pt);
		SAVE("count", data.count);
		SAVE("startLocation", data.startLocation);
		SAVE("baseVertexLocation", data.baseVertexLocation);
		SAVE("boundingBox", data.boundingBox);
		return true;
	}

	bool DeserializeDocumentData(DocumentLoader& loader, GeometrySubset& data)
	{
		LOAD("name", data.name);
		LOAD("primitiveTopology|pt", data.pt);
		LOAD("count", data.count);
		LOAD("startLocation|start", data.startLocation);
		LOAD("baseVertexLocation|base", data.baseVertexLocation);
		LOAD("boundingBox|bbox", data.boundingBox);
		return true;
	}
}

bool Geometry::CopyChip(Chip* chip)
{
	Geometry* c = dynamic_cast<Geometry*>(chip);
	B_RETURN(Chip::CopyChip(c));
	_subsets = c->_subsets;
	DestroyDeviceObjects();
	return true;
}

bool Geometry::LoadChip(DocumentLoader& loader)
{
	B_RETURN(Chip::LoadChip(loader));
	LOAD("subsets", _subsets);
	DestroyDeviceObjects();
	return true;
}

bool Geometry::SaveChip(DocumentSaver& saver) const
{
	B_RETURN(Chip::SaveChip(saver));
	SAVE("subsets", _subsets);
	return true;
}

void Geometry::Clear()
{
	_subsets.clear();
	DestroyDeviceObjects();
}

void Geometry::Prepare()
{
	D3D_DEBUG_REPORTER_BLOCK

	if (!_isInit)
		CreateDeviceObjects(); // Throws!

	rs()->SetPipelineInputStateDesc(_pisID);

	if (_vertexBuffers.size()) {
		List<D3D12_VERTEX_BUFFER_VIEW> vbViews;
		for (size_t i = 0; i < _vertexBuffers.size(); i++)
			vbViews.push_back(_vertexBuffers[i].view);
		rs()->IASetVertexBuffers(0, (UINT)_vertexBuffers.size(), &vbViews.front());
	}

	if (GetAPI() == DRAW_INDEXED && _indexBuffer)
		rs()->IASetIndexBuffer(&_indexBufferView);
}


void Geometry::CreateDeviceObjects(D3D12_PRIMITIVE_TOPOLOGY_TYPE ptt, const D3D12_INPUT_LAYOUT_DESC& ild, const List<DataBuffer>& vertexBufferData, const List<UINT> vertexBufferStrides, const DataBuffer& indexBufferData, DXGI_FORMAT indexFormat)
{
	D3D_DEBUG_REPORTER_BLOCK

		PipelineInputStateDesc pisDesc;
	pisDesc.IBStripCutValue = indexFormat == DXGI_FORMAT_R32_UINT ? D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_0xFFFFFFFF : D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_0xFFFF;
	pisDesc.InputLayout = ild;
	pisDesc.PrimitiveTopologyType = ptt;
	_pisID = GetPipelineStatePool()->RegisterPipelineInputStateDesc(pisDesc);

	UINT64 size = 0;
	for (size_t i = 0; i < vertexBufferData.size(); i++)
		size += vertexBufferData[i].getBufferSize();
	size += indexBufferData.getBufferSize();

	if (size > 0 || indexBufferData.getBufferSize() > 0) {

		RID3D12Resource uploadHeap;

		CD3DX12_HEAP_PROPERTIES hp(D3D12_HEAP_TYPE_UPLOAD);
		CD3DX12_RESOURCE_DESC buff = CD3DX12_RESOURCE_DESC::Buffer(size);
		if (FAILED(device()->CreateCommittedResource(
			&hp,
			D3D12_HEAP_FLAG_NONE,
			&buff,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&uploadHeap))))
		{
			throw GraphicsException(this, MTEXT("Failed to create upload heap."), FATAL);
		}

		UINT64 heapOffset = 0;

		for (size_t i = 0; i < vertexBufferData.size(); i++) {
			VertexBuffer vb;
			CD3DX12_HEAP_PROPERTIES hp(D3D12_HEAP_TYPE_DEFAULT);
			CD3DX12_RESOURCE_DESC buff = CD3DX12_RESOURCE_DESC::Buffer(vertexBufferData[i].getBufferSize());
			if (FAILED(device()->CreateCommittedResource(
				&hp,
				D3D12_HEAP_FLAG_NONE,
				&buff,
				D3D12_RESOURCE_STATE_COPY_DEST,
				nullptr,
				IID_PPV_ARGS(&vb.resource))))
			{
				throw GraphicsException(this, strUtils::ConstructString(MTEXT("Failed to create vertex buffer %1.")).arg(i), FATAL);
			}

			// Copy data to the intermediate upload heap and then schedule a copy from the upload heap to the vertex buffer.
			D3D12_SUBRESOURCE_DATA vertexData = {};
			vertexData.pData = (const void*)vertexBufferData[i].getConstBuffer();
			vertexData.RowPitch = vertexBufferData[i].getBufferSize();
			vertexData.SlicePitch = vertexData.RowPitch;

			rs()->UpdateSubresources(vb.resource, uploadHeap, heapOffset, 0, 1, &vertexData);
			D3D12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(vb.resource, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
			rs()->ResourceBarrier(1, &barrier);

			vb.view.BufferLocation = vb.resource->GetGPUVirtualAddress();
			vb.view.SizeInBytes = (UINT)vertexData.RowPitch;
			vb.view.StrideInBytes = vertexBufferStrides[i];

			_vertexBuffers.push_back(vb);

			heapOffset += vertexData.RowPitch;
		}

		if (indexBufferData.getBufferSize() > 0) {
			CD3DX12_HEAP_PROPERTIES hp(D3D12_HEAP_TYPE_DEFAULT);
			D3D12_RESOURCE_DESC buff = CD3DX12_RESOURCE_DESC::Buffer(indexBufferData.getBufferSize());
			if (FAILED(device()->CreateCommittedResource(
				&hp,
				D3D12_HEAP_FLAG_NONE,
				&buff,
				D3D12_RESOURCE_STATE_COPY_DEST,
				nullptr,
				IID_PPV_ARGS(&_indexBuffer))))
			{
				throw GraphicsException(this, MTEXT("Failed to create index buffer."), FATAL);
			}

			// Copy data to the intermediate upload heap and then schedule a copy from the upload heap to the vertex buffer.
			D3D12_SUBRESOURCE_DATA indexData = {};
			indexData.pData = (const void*)indexBufferData.getConstBuffer();
			indexData.RowPitch = indexBufferData.getBufferSize();
			indexData.SlicePitch = indexData.RowPitch;

			rs()->UpdateSubresources(_indexBuffer, uploadHeap, heapOffset, 0, 1, &indexData);
			D3D12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(_indexBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_INDEX_BUFFER);
			rs()->ResourceBarrier(1, &barrier);

			_indexBufferView.BufferLocation = _indexBuffer->GetGPUVirtualAddress();
			_indexBufferView.Format = indexFormat;
			_indexBufferView.SizeInBytes = (UINT)indexData.RowPitch;

			heapOffset += indexData.RowPitch;
		}
	}

	_isInit = true;
}

void Geometry::DestroyDeviceObjects()
{
	_pisID = 0;
	for (size_t i = 0; i < _vertexBuffers.size(); i++) {
		_vertexBuffers[i].resource = nullptr;
		_vertexBuffers[i].view = { };
	}
	_vertexBuffers.clear();
	_indexBuffer = nullptr;
	_indexBuffer = nullptr;
	_indexBufferView = { };
	_isInit = false;
}
