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
#include "GraphicsBuffer.h"
#include "M3DEngine/DocumentSaveLoadUtil.h"
#include "RenderSettings.h"
#include "StdChips/Value.h"
#include "StdChips/VectorChip.h"
#include "StdChips/MatrixChip.h"
#include "StdChips/ValueArray.h"
#include "StdChips/VectorArray.h"
#include "StdChips/MatrixArray.h"
#include "SimpleLexer.h"
#include "M3DCore/DataBuffer.h"
#include "M3DEngine/Engine.h"
#include "M3DEngine/ChipManager.h"


using namespace m3d;

CHIPDESCV1_DEF(GraphicsBuffer, MTEXT("Graphics Buffer"), GRAPHICSBUFFER_GUID, GRAPHICSRESOURCECHIP_GUID);


GraphicsBuffer::GraphicsBuffer()
{
	CREATE_CHILD(0, CHIP_GUID, true, UP, MTEXT("Data"));
}

GraphicsBuffer::~GraphicsBuffer()
{
}

bool GraphicsBuffer::CopyChip(Chip* chip)
{
	GraphicsBuffer* c = dynamic_cast<GraphicsBuffer*>(chip);
	B_RETURN(GraphicsResourceChip::CopyChip(c));
	_size = c->_size;
	_flags = c->_flags;
	_gpuOnly = c->_gpuOnly;
	//ClearResource();
	return true;
}

bool GraphicsBuffer::LoadChip(DocumentLoader& loader)
{
	B_RETURN(GraphicsResourceChip::LoadChip(loader));
	LOADDEF("size", _size, 0);
	LOADDEF("flags", _flags, M3D_RESOURCE_FLAG_NONE);
	LOADDEF("gpuOnly", _gpuOnly, false);

	return true;
}

bool GraphicsBuffer::SaveChip(DocumentSaver& saver) const
{
	B_RETURN(GraphicsResourceChip::SaveChip(saver));
	SAVEDEF("size", _size, 0);
	SAVEDEF("flags", _flags, M3D_RESOURCE_FLAG_NONE);
	SAVEDEF("gpuOnly", _gpuOnly, false);
	return true;
}

void GraphicsBuffer::SetSize(UINT64 size)
{
	_size = size;
}

void GraphicsBuffer::SetFlags(M3D_RESOURCE_FLAGS flags)
{
	_flags = flags;
}

void GraphicsBuffer::SetGPUOnly(bool gpuOnly)
{
	_gpuOnly = gpuOnly;
}


void GraphicsBuffer::CallChip()
{
	// Note: We do not do any checking of refresh status here!

	try {
		if (_layoutID == InvalidBufferLayoutID)
			throw GraphicsException(this, MTEXT("This buffer has not been initialized with a layout."), FATAL);

		_update(_layoutID);
	}
	catch (const ChipException& e)
	{
		AddException(e);
	}
}

void GraphicsBuffer::ClearResource()
{
	GraphicsResourceChip::ClearResource();
	_currentSize = 0;
}

void GraphicsBuffer::UpdateChip(BufferLayoutID layoutID)
{
	RefreshT refresh(Refresh);
	if (!refresh) {
		if (!_res)
			throw ReinitPreventedException(this);
		return;
	}

	_update(layoutID);
}

void GraphicsBuffer::_update(BufferLayoutID layoutID)
{
	D3D_DEBUG_REPORTER_BLOCK

		if (layoutID == InvalidBufferLayoutID) {
			// Create buffer based on size
			if (_currentSize < _size) {
				ClearResource();
				UINT64 sizeAligned = (_size + (D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT - 1)) & ~(D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT - 1);
				if (sizeAligned == 0)
					throw GraphicsException(this, MTEXT("Buffer size is zero!"), FATAL);
				_createBuffer(sizeAligned, D3D12_RESOURCE_STATE_COMMON); // throws
			}
			return;
		}

	const BufferLayout* bl = BufferLayoutManager::GetInstance().GetLayout(layoutID);

	if (!bl)
		throw GraphicsException(this, MTEXT("Invalid Buffer Layout provided."), FATAL);

	UINT64 size = bl->GetBufferSize();
	UINT64 sizeAligned = (size + (D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT - 1)) & ~(D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT - 1);

	if (sizeAligned > _currentSize)
		ClearResource();

	if (layoutID != _layoutID) {
		_layoutID = layoutID;
	}

	if (sizeAligned == 0)
		throw GraphicsException(this, MTEXT("Buffer size is zero!"), FATAL);

	if (_gpuOnly) {
		if (!_res)
			throw GraphicsException(this, MTEXT("A GPU-only buffer must be initialized before being used as a constant buffer."), FATAL);
		return;
	}

	struct S
	{
		String name;
		uint32 idx;
		bool available;
	};

	List<S> dataChips[6];

	ChipManager* cm = engine->GetChipManager();

	for (uint32 i = 0, j = GetSubConnectionCount(0); i < j; i++) {
		Chip* c = GetRawChild(0, i);
		if (!c)
			continue;
		Guid guid = c->GetChipType();

		BufferLayout::Item::Type type;
		if (cm->IsChipTypeSupported(VALUE_GUID, guid))
			type = BufferLayout::Item::VALUE;
		else if (cm->IsChipTypeSupported(VECTORCHIP_GUID, guid))
			type = BufferLayout::Item::VECTOR;
		else if (cm->IsChipTypeSupported(MATRIXCHIP_GUID, guid))
			type = BufferLayout::Item::MATRIX;
		else if (cm->IsChipTypeSupported(VALUEARRAY_GUID, guid))
			type = BufferLayout::Item::VALUE_ARRAY;
		else if (cm->IsChipTypeSupported(VECTORARRAY_GUID, guid))
			type = BufferLayout::Item::VECTOR_ARRAY;
		else if (cm->IsChipTypeSupported(MATRIXARRAY_GUID, guid))
			type = BufferLayout::Item::MATRIX_ARRAY;
		else
			continue;
		String name = strUtils::toLower(c->GetName());
		for (size_t i = 0; i < name.size(); i++)
			if (!(name[i] >= 'a' && name[i] <= 'z' || name[i] >= 'A' && name[i] <= 'Z' || name[i] >= '0' && name[i] <= '9'))
				name[i] = '_';

		dataChips[type].push_back({ name, i, true });
	}

	List<uint32> mappings(bl->GetItems().size(), (uint32)-1);
	for (size_t i = 0; i < bl->GetItems().size(); i++) {
		const auto& a = bl->GetItems()[i];
		String name = strUtils::toLower(a.name);
		for (size_t j = 0; j < dataChips[a.type].size(); j++) {
			if (dataChips[a.type][j].available && name == dataChips[a.type][j].name) {
				dataChips[a.type][j].available = false;
				mappings[i] = dataChips[a.type][j].idx;
				break;
			}
		}
	}
	for (size_t i = 0; i < bl->GetItems().size(); i++) {
		if (mappings[i] != -1)
			continue;
		const auto& a = bl->GetItems()[i];
		for (size_t j = 0; j < dataChips[a.type].size(); j++) {
			if (dataChips[a.type][j].available) {
				dataChips[a.type][j].available = false;
				mappings[i] = dataChips[a.type][j].idx;
				break;
			}
		}
	}

	UINT64 uploadHeapOffet = 0;
	BYTE* uploadHeapBuffer = nullptr;
	D3D12_GPU_VIRTUAL_ADDRESS uploadHeapAddr;
	if (FAILED(GetUploadHeap()->Allocate(sizeAligned, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT, &uploadHeapBuffer, &uploadHeapAddr, &uploadHeapOffet))) {
		throw GraphicsException(this, MTEXT("Failed to allocate memory from the upload heap."));
	}

	memset(uploadHeapBuffer, 0, sizeAligned);

	uint32 c[6] = { 0 };
	for (size_t i = 0; i < bl->GetItems().size(); i++) {
		const auto& a = bl->GetItems()[i];
		uint32 conn = mappings[i];
		int32 n = c[a.type]++;
		if (conn == -1)
			continue; // We have no data for this one....
		switch (a.type)
		{
		case BufferLayout::Item::VALUE:
		{
			ChildPtr<Value> ch = GetChild(0, conn);
			if (ch) {
				double f = (double)ch->GetValue();
				bl->SetValue(n, uploadHeapBuffer, &f);
			}
			break;
		}
		case BufferLayout::Item::VECTOR:
		{
			ChildPtr<VectorChip> ch = GetChild(0, conn);
			if (ch) {
				bl->SetVector(n, uploadHeapBuffer, &ch->GetVector());
			}
			break;
		}
		case BufferLayout::Item::MATRIX:
		{
			ChildPtr<MatrixChip> ch = GetChild(0, conn);
			if (ch) {
				bl->SetMatrix(n, uploadHeapBuffer, &ch->GetMatrix());
			}
			break;
		}
		case BufferLayout::Item::VALUE_ARRAY:
		{
			ChildPtr<ValueArray> ch = GetChild(0, conn);
			if (ch) {
				const List<value>& a = ch->GetArray();
				bl->SetValueArray(n, uploadHeapBuffer, (uint32)a.size(), a.empty() ? nullptr : &a[0]);
			}
			break;
		}
		case BufferLayout::Item::VECTOR_ARRAY:
		{
			ChildPtr<VectorArray> ch = GetChild(0, conn);
			if (ch) {
				const List<XMFLOAT4>& a = ch->GetArray();
				bl->SetVectorArray(n, uploadHeapBuffer, (uint32)a.size(), a.empty() ? nullptr : &a[0]);
			}
			break;
		}
		case BufferLayout::Item::MATRIX_ARRAY:
		{
			ChildPtr<MatrixArray> ch = GetChild(0, conn);
			if (ch) {
				const List<XMFLOAT4X4>& a = ch->GetArray();
				bl->SetMatrixArray(n, uploadHeapBuffer, (uint32)a.size(), a.empty() ? nullptr : &a[0]);
			}
			break;
		}
		}
	}

	if (_res) {
		CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(_res, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, D3D12_RESOURCE_STATE_COPY_DEST);
		rs()->ResourceBarrier(1, &barrier);
		rs()->CommitResourceBarriers();
	}
	else {
		_createBuffer(sizeAligned, D3D12_RESOURCE_STATE_COPY_DEST); // throws
	}

	rs()->CopyBufferRegion(_res, 0, GetUploadHeap()->GetBufferResource(), uploadHeapOffet, sizeAligned);
	CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(_res, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	rs()->ResourceBarrier(1, &barrier);
}

void GraphicsBuffer::_createBuffer(UINT64 size, D3D12_RESOURCE_STATES state)
{
	CD3DX12_HEAP_PROPERTIES hp(D3D12_HEAP_TYPE_DEFAULT);
	CD3DX12_RESOURCE_DESC buff = CD3DX12_RESOURCE_DESC::Buffer(size, (D3D12_RESOURCE_FLAGS)_flags, D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT);
	if (FAILED(device()->CreateCommittedResource(
		&hp,
		D3D12_HEAP_FLAG_NONE,
		&buff,
		state,
		nullptr,
		IID_PPV_ARGS(&_res))))
	{
		throw GraphicsException(this, MTEXT("Failed to create buffer resource."), FATAL);
	}
	_currentSize = size;

	String s = GetName() + MTEXT(" (") + GuidToString(GetGlobalID()) + MTEXT(")");
	_res->SetPrivateData(WKPDID_D3DDebugObjectName, (UINT)s.size(), s.c_str());

	if (_flags & D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS) {
		SResourceStateTracker t;
		HRESULT hr = CreateResourceStateTracker(_res, state, t.fill());
	}
}