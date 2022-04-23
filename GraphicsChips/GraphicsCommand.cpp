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
#include "GraphicsCommand.h"
#include "M3DEngine/Engine.h"
#include "StdChips/Value.h"
#include "StdChips/VectorChip.h"
#include "M3DEngine/DocumentSaveLoadUtil.h"
#include "StdChips/Text.h"
#include "StdChips/MatrixChip.h"
#include "ReadbackBuffer.h"
#include "GraphicsResourceChip.h"
#include "Texture.h"
#include "GraphicsChips/GraphicsException.h"
#include "M3DEngine/RenderWindowManager.h"
#//include "ChipManager.h"
#include "GraphicsChips/DebugGeometry.h"
#include "RenderSettings.h"
//#include "Texture.h"
#include "Utils.h"
#include "ReadbackBuffer.h"

using namespace m3d;

extern SIZE_T __align(SIZE_T location, UINT alignment);
/*{
	return (location + alignment - 1) & ~UINT64(alignment - 1);
}*/

template<typename T>
T clamp(T v, T minimum, T maximum) { return v < minimum ? minimum : (v > maximum ? maximum : v); }


CHIPDESCV1_DEF(GraphicsCommand, MTEXT("Graphics Command"), GRAPHICSCOMMAND_GUID, CHIP_GUID);


GraphicsCommand::GraphicsCommand()
{
}

GraphicsCommand::~GraphicsCommand()
{
}

bool GraphicsCommand::CopyChip(Chip* chip)
{
	GraphicsCommand* c = dynamic_cast<GraphicsCommand*>(chip);
	B_RETURN(Chip::CopyChip(c));
	SetCommand(c->_cmd);
	return true;
}

bool GraphicsCommand::LoadChip(DocumentLoader& loader)
{
	B_RETURN(Chip::LoadChip(loader));
	OperatorType cmd;
	LOAD("operatorType|cmd", cmd);
	SetCommand(cmd);
	return true;
}

bool GraphicsCommand::SaveChip(DocumentSaver& saver) const
{
	B_RETURN(Chip::SaveChip(saver));
	SAVE("operatorType", _cmd);
	return true;
}


void GraphicsCommand::SetCommand(OperatorType cmd)
{
	if (_cmd == cmd)
		return;

	_cmd = cmd;

	switch (_cmd) {
	case OperatorType::CLEAR_RTV:
	{
		CREATE_CHILD_KEEP(0, VECTORCHIP_GUID, false, UP, MTEXT("Color"));
		CREATE_CHILD_KEEP(1, GRAPHICSRESOURCECHIP_GUID, true, DOWN, MTEXT("Render Targets"));
		ClearConnections(2);
	}
	break;
	case OperatorType::SET_RTV:
	{
		CREATE_CHILD_KEEP(0, GRAPHICSRESOURCECHIP_GUID, false, UP, MTEXT("Depth Stencil View"));
		CREATE_CHILD_KEEP(1, GRAPHICSRESOURCECHIP_GUID, true, UP, MTEXT("Render Targets"));
		ClearConnections(2);
	}
	break;
	case OperatorType::SET_VIEWPORTS:
	case OperatorType::SET_VIEWPORTS_ABS:
		CREATE_CHILD_KEEP(0, VALUE_GUID, false, UP, MTEXT("Min Depth (Common)"));
		CREATE_CHILD_KEEP(1, VALUE_GUID, false, UP, MTEXT("Max Depth (Common)"));
		CREATE_CHILD_KEEP(2, VECTORCHIP_GUID, true, UP, MTEXT("Viewports"));
		ClearConnections(3);
		break;
	case OperatorType::CLEAR_DSV:
		CREATE_CHILD_KEEP(0, VALUE_GUID, false, UP, MTEXT("Depth Value"));
		CREATE_CHILD_KEEP(1, VALUE_GUID, false, UP, MTEXT("Stencil Value"));
		CREATE_CHILD_KEEP(2, GRAPHICSRESOURCECHIP_GUID, true, DOWN, MTEXT("Depth Stencil Views"));
		ClearConnections(3);
		break;
	case OperatorType::CREATE_DEVICE:
		CREATE_CHILD_KEEP(0, VALUE_GUID, false, UP, MTEXT("Adapter Index"));
		CREATE_CHILD_KEEP(1, VALUE_GUID, false, UP, MTEXT("Request Debug Device?"));
		ClearConnections(2);
		break;
	case OperatorType::GET_ADAPTER_INFO:
		CREATE_CHILD_KEEP(0, VALUE_GUID, false, UP, MTEXT("Adapter Index"));
		CREATE_CHILD_KEEP(1, TEXT_GUID, false, DOWN, MTEXT("Description"));
		CREATE_CHILD_KEEP(2, VECTORCHIP_GUID, false, DOWN, MTEXT("Memory"));
		CREATE_CHILD_KEEP(3, VECTORCHIP_GUID, false, DOWN, MTEXT("PCI IDs"));
		CREATE_CHILD_KEEP(4, VALUE_GUID, false, DOWN, MTEXT("Num of Outputs"));
		ClearConnections(5);
		break;
	case OperatorType::GET_OUTPUT_INFO:
		CREATE_CHILD_KEEP(0, VALUE_GUID, false, UP, MTEXT("Adapter Index"));
		CREATE_CHILD_KEEP(1, VALUE_GUID, false, UP, MTEXT("Output Index"));
		CREATE_CHILD_KEEP(2, TEXT_GUID, false, DOWN, MTEXT("Name"));
		CREATE_CHILD_KEEP(3, VECTORCHIP_GUID, false, DOWN, MTEXT("Desktop Coordinates"));
		ClearConnections(4);
		break;
	case OperatorType::FULLSCREEN:
		CREATE_CHILD_KEEP(0, VALUE_GUID, false, UP, MTEXT("Output Index"));
		CREATE_CHILD_KEEP(1, VECTORCHIP_GUID, false, UP, MTEXT("Size (w,h) and Refresh Rate (Num/Denum)"));
		CREATE_CHILD_KEEP(2, VALUE_GUID, false, UP, MTEXT("VSync 0/1"));
		ClearConnections(3);
		break;
	case OperatorType::WINDOWED:
		CREATE_CHILD_KEEP(0, VECTORCHIP_GUID, false, UP, MTEXT("Position and Size"));
		CREATE_CHILD_KEEP(1, VALUE_GUID, false, UP, MTEXT("VSync 0/1"));
		ClearConnections(2);
		break;
	case OperatorType::UNBIND_RTV:
	case OperatorType::UNBIND_SRV:
		ClearConnections(0);
		break;
	case OperatorType::UPLOAD_SUBRESOURCE:
		CREATE_CHILD_KEEP(0, GRAPHICSRESOURCECHIP_GUID, false, DOWN, MTEXT("Resource"));
		CREATE_CHILD_KEEP(1, VALUE_GUID, false, UP, MTEXT("Subresource Index"));
		CREATE_CHILD_KEEP(2, VECTORCHIP_GUID, false, UP, MTEXT("Left/Upper/Front Corner"));
		CREATE_CHILD_KEEP(3, VECTORCHIP_GUID, false, UP, MTEXT("Width, Height, Depth"));
		CREATE_CHILD_KEEP(4, CHIP_GUID, false, DOWN, MTEXT("Called pr Index"));
		CREATE_CHILD_KEEP(5, VECTORCHIP_GUID, false, DOWN, MTEXT("Coordinate"));
		CREATE_CHILD_KEEP(6, VECTORCHIP_GUID, false, UP, MTEXT("Texel Data"));
		break;
	case OperatorType::RESOLVE_SUBRESOURCE:
		CREATE_CHILD_KEEP(0, GRAPHICSRESOURCECHIP_GUID, false, UP, MTEXT("Source (Multisampled)"));
		CREATE_CHILD_KEEP(1, VALUE_GUID, false, UP, MTEXT("Source Mip Slice"));
		CREATE_CHILD_KEEP(2, VALUE_GUID, false, UP, MTEXT("Source Array Slice"));
		CREATE_CHILD_KEEP(3, GRAPHICSRESOURCECHIP_GUID, false, DOWN, MTEXT("Destination"));
		CREATE_CHILD_KEEP(4, VALUE_GUID, false, UP, MTEXT("Destination Mip Slice"));
		CREATE_CHILD_KEEP(5, VALUE_GUID, false, UP, MTEXT("Destination Array Slice"));
		ClearConnections(6);
		break;
	case OperatorType::COPY_RESOURCE:
		CREATE_CHILD_KEEP(0, GRAPHICSRESOURCECHIP_GUID, false, UP, MTEXT("Source"));
		CREATE_CHILD_KEEP(1, GRAPHICSRESOURCECHIP_GUID, false, DOWN, MTEXT("Destination"));
		ClearConnections(2);
		break;
	case OperatorType::COPY_SUBRESOURCE_REGION:
		CREATE_CHILD_KEEP(0, GRAPHICSRESOURCECHIP_GUID, false, UP, MTEXT("Source"));
		CREATE_CHILD_KEEP(1, VALUE_GUID, false, UP, MTEXT("Source Subresource Index"));
		CREATE_CHILD_KEEP(2, VECTORCHIP_GUID, false, UP, MTEXT("Source Left/Upper/Front Corner"));
		CREATE_CHILD_KEEP(3, VECTORCHIP_GUID, false, UP, MTEXT("Width, Height, Depth"));
		CREATE_CHILD_KEEP(4, GRAPHICSRESOURCECHIP_GUID, false, DOWN, MTEXT("Destination"));
		CREATE_CHILD_KEEP(5, VALUE_GUID, false, UP, MTEXT("Destination Subresource Index"));
		CREATE_CHILD_KEEP(6, VECTORCHIP_GUID, false, UP, MTEXT("Destination Left/Upper/Front Corner"));
		ClearConnections(7);
		break;
	case OperatorType::DOWNLOAD_SUBRESOURCE_REGION:
		CREATE_CHILD_KEEP(0, GRAPHICSRESOURCECHIP_GUID, false, UP, MTEXT("Source"));
		CREATE_CHILD_KEEP(1, VALUE_GUID, false, UP, MTEXT("Source Subresource Index"));
		CREATE_CHILD_KEEP(2, READBACKBUFFER_GUID, false, DOWN, MTEXT("Destination"));
		ClearConnections(3);
		break;
	case OperatorType::DESTROY_RESOURCE:
		CREATE_CHILD_KEEP(0, GRAPHICSRESOURCECHIP_GUID, true, DOWN, MTEXT("Resources"));
		ClearConnections(1);
		break;
	case OperatorType::SET_TEXTURE_PROPS:
		CREATE_CHILD_KEEP(0, TEXTURE_GUID, false, DOWN, MTEXT("Texture"));
		CREATE_CHILD_KEEP(1, VALUE_GUID, false, UP, MTEXT("Width"));
		CREATE_CHILD_KEEP(2, VALUE_GUID, false, UP, MTEXT("Height"));
		CREATE_CHILD_KEEP(3, VALUE_GUID, false, UP, MTEXT("Depth"));
		CREATE_CHILD_KEEP(4, VALUE_GUID, false, UP, MTEXT("Mip Levels"));
		CREATE_CHILD_KEEP(5, VALUE_GUID, false, UP, MTEXT("Array Size"));
		ClearConnections(6);
		break;

		/*	case OperatorType::MAP_RESOURCE_READ:
			case OperatorType::MAP_RESOURCE_WRITE:
			case OperatorType::MAP_RESOURCE_READ_WRITE:
			case OperatorType::MAP_RESOURCE_WRITE_DISCARD:
				CREATE_CHILD_KEEP(0, D3DRESOURCE_GUID, false, BOTH, MTEXT("Resource"));
				CREATE_CHILD_KEEP(1, VALUE_GUID, false, UP, MTEXT("Subresource Index"));
				CREATE_CHILD_KEEP(2, VALUE_GUID, false, UP, MTEXT("Do Not Wait For GPU?"));
				CREATE_CHILD_KEEP(3, D3DMAPPEDRESOURCE_GUID, false, DOWN, MTEXT("Result"));
				ClearConnections(4);
				break;
			case OperatorType::UNMAP_RESOURCE:
				CREATE_CHILD_KEEP(0, D3DMAPPEDRESOURCE_GUID, false, DOWN, MTEXT("Mapped Resource"));
				ClearConnections(1);
				break;
			case OperatorType::LOAD_TEXTURE_FROM_FILE:
				CREATE_CHILD_KEEP(0, TEXTURE_GUID, false, UP, MTEXT("Texture"));
				CREATE_CHILD_KEEP(1, TEXT_GUID, true, UP, MTEXT("Filename"));
				ClearConnections(2);
				break;
			case OperatorType::SET_GLOBAL_SHADER_RESOURCES:
				CREATE_CHILD_KEEP(0, TEXT_GUID, false, UP, MTEXT("Semantic"));
				CREATE_CHILD_KEEP(1, D3DRESOURCE_GUID, true, UP, MTEXT("Textures"));
				ClearConnections(2);
				break;*/
	case OperatorType::SET_CAMERA_MATRIES:
		CREATE_CHILD_KEEP(0, MATRIXCHIP_GUID, false, UP, MTEXT("View Matrix"));
		CREATE_CHILD_KEEP(1, MATRIXCHIP_GUID, false, UP, MTEXT("Projection Matrix"));
		CREATE_CHILD_KEEP(2, MATRIXCHIP_GUID, false, UP, MTEXT("Culling Matrix"));
		CREATE_CHILD_KEEP(3, MATRIXCHIP_GUID, false, UP, MTEXT("Shadow Matrix"));
		ClearConnections(4);
		break;
	case OperatorType::DEBUG_DRAW:
		ClearConnections();
		break;
	case OperatorType::DEBUG_ADD_POINT:
		CREATE_CHILD_KEEP(0, VECTORCHIP_GUID, false, UP, MTEXT("Position"));
		CREATE_CHILD_KEEP(1, VECTORCHIP_GUID, false, UP, MTEXT("Color"));
		ClearConnections(2);
		break;
	case OperatorType::DEBUG_ADD_LINE:
		CREATE_CHILD_KEEP(0, VECTORCHIP_GUID, false, UP, MTEXT("Position 1"));
		CREATE_CHILD_KEEP(1, VECTORCHIP_GUID, false, UP, MTEXT("Position 2"));
		CREATE_CHILD_KEEP(2, VECTORCHIP_GUID, false, UP, MTEXT("Color 1"));
		CREATE_CHILD_KEEP(3, VECTORCHIP_GUID, false, UP, MTEXT("Color 2"));
		ClearConnections(4);
		break;
	case OperatorType::DEBUG_ADD_TRIANGLE:
		CREATE_CHILD_KEEP(0, VECTORCHIP_GUID, false, UP, MTEXT("Position 1"));
		CREATE_CHILD_KEEP(1, VECTORCHIP_GUID, false, UP, MTEXT("Position 2"));
		CREATE_CHILD_KEEP(2, VECTORCHIP_GUID, false, UP, MTEXT("Position 3"));
		CREATE_CHILD_KEEP(3, VECTORCHIP_GUID, false, UP, MTEXT("Color 1"));
		CREATE_CHILD_KEEP(4, VECTORCHIP_GUID, false, UP, MTEXT("Color 2"));
		CREATE_CHILD_KEEP(5, VECTORCHIP_GUID, false, UP, MTEXT("Color 3"));
		ClearConnections(6);
		break;
	};
}

void GraphicsCommand::CallChip()
{
	D3D_DEBUG_REPORTER_BLOCK

		ChipExceptionScope ces(this);

	UINT vpWMult = 1, vpHMult = 1; // Only for setting viewports!

	try
	{
		switch (_cmd)
		{
		case OperatorType::CLEAR_RTV:
		{
			ChildPtr<VectorChip> v = GetChild(0);
			XMFLOAT4 f = v ? v->GetVector() : XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);

			List<D3D12_CPU_DESCRIPTOR_HANDLE> rtvList;

			for (uint32 i = 0; i < GetSubConnectionCount(1); i++) {
				ChildPtr<GraphicsResourceChip> chRes = GetChild(1, i);
				if (chRes) {
					chRes->UpdateChip();
					const Descriptor& desc = chRes->GetRenderTargetViewDescriptor();
					rtvList.push_back(desc.GetCPUHandle());
					SResourceStateTracker rst;
					GetResourceStateTracker(desc.resource, &rst);
					if (rst)
						rs()->EnterState(rst, desc.rtv); // Can throw! Unlocks resource immediately.
				}
			}

			if (!rtvList.empty()) {
				rs()->CommitResourceBarriers();
				for (size_t i = 0; i < rtvList.size(); i++)
					rs()->ClearRenderTargetView(rtvList[i], (const FLOAT*)&f, 0, nullptr);
			}
		}
		break;
		case OperatorType::SET_RTV:
		{
			ChildPtr<GraphicsResourceChip> chDSV = GetChild(0);
			D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(CD3DX12_DEFAULT());
			D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT] = { CD3DX12_CPU_DESCRIPTOR_HANDLE(CD3DX12_DEFAULT()) };
			if (chDSV) {
				chDSV->UpdateChip();
				dsvHandle = chDSV->GetDepthStencilViewDescriptor().GetCPUHandle();
			}
			uint32 numRenderTargets = 0;
			for (uint32 i = 0, j = std::min(GetSubConnectionCount(1), (UINT)D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT); i < j; i++) {
				ChildPtr<GraphicsResourceChip> chRes = GetChild(1, i);
				if (chRes) {
					if (i > numRenderTargets)
						throw MissingChildException(this, numRenderTargets);
					chRes->UpdateChip();
					rtvHandles[numRenderTargets++] = chRes->GetRenderTargetViewDescriptor().GetCPUHandle();
				}
			}
			if (CD3DX12_CPU_DESCRIPTOR_HANDLE(CD3DX12_DEFAULT()) == dsvHandle && numRenderTargets == 0)
				throw GraphicsException(this, MTEXT("At least one render target or depth stencil view must be set. (Use \'Unbind Render Targets Views\' instead if this was on purpose)."), FATAL);

			rs()->OMSetRenderTargets(numRenderTargets, rtvHandles, CD3DX12_CPU_DESCRIPTOR_HANDLE(CD3DX12_DEFAULT()) != dsvHandle ? &dsvHandle : nullptr);
		}
		break;
		case OperatorType::SET_VIEWPORTS:
			vpWMult = rs()->GetRenderTargetWidth();
			vpHMult = rs()->GetRenderTargetHeight();
#ifdef DEVCHECKS
			if (vpWMult == 0 || vpWMult == 0)
				throw GraphicsException(this, MTEXT("Can not set relative viewport size when no render targets have been set."), FATAL);
#endif
			// Fall down! (No break!)
		case OperatorType::SET_VIEWPORTS_ABS:
		{
			D3D12_VIEWPORT vp[D3D12_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];
			D3D12_RECT sr[D3D12_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];
			float minDepth = 0.0f, maxDepth = 1.0f;
			ChildPtr<Value> ch0 = GetChild(0);
			if (ch0)
				minDepth = (float)ch0->GetValue();
			ChildPtr<Value> ch1 = GetChild(1);
			if (ch1)
				maxDepth = (float)ch1->GetValue();
			uint32 i = 0;
			for (uint32 j = 0, k = std::min(GetSubConnectionCount(2), (UINT)D3D12_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE); j < k; j++) {
				ChildPtr<VectorChip> chV = GetChild(2, j);
				if (chV) {
					XMFLOAT4 v = chV->GetVector();
					vp[i].TopLeftX = v.x * vpWMult;
					vp[i].TopLeftY = v.y * vpHMult;
					vp[i].Width = v.z * vpWMult;
					vp[i].Height = v.w * vpHMult;
					vp[i].MinDepth = minDepth;
					vp[i].MaxDepth = maxDepth;
					sr[i].left = (LONG)vp[i].TopLeftX;
					sr[i].top = (LONG)vp[i].TopLeftY;
					sr[i].right = sr[i].left + (LONG)vp[i].Width;
					sr[i].bottom = sr[i].top + (LONG)vp[i].Height;
					i++;
				}
			}
			if (i > 0)
				rs()->RSSetViewportsAndScissorRects(i, vp, sr);
		}
		break;
		case OperatorType::CLEAR_DSV:
		{
			ChildPtr<Value> ch0 = GetChild(0);
			ChildPtr<Value> ch1 = GetChild(1);
			D3D12_CLEAR_FLAGS flags = ch0 && !ch1 ? D3D12_CLEAR_FLAG_DEPTH : (ch1 && !ch0 ? D3D12_CLEAR_FLAG_STENCIL : (D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL));
			FLOAT depth = ch0 ? (float)ch0->GetValue() : 1.0f;
			UINT8 stencil = ch1 ? (UINT8)std::min(std::max((INT)ch1->GetValue(), 0), 0xFF) : 0xFF;

			List<D3D12_CPU_DESCRIPTOR_HANDLE> dsvList;

			for (uint32 i = 0; i < GetSubConnectionCount(2); i++) {
				ChildPtr<GraphicsResourceChip> chRes = GetChild(2, i);
				if (chRes) {
					chRes->UpdateChip();
					D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = chRes->GetDepthStencilViewDescriptor().GetCPUHandle();
					const Descriptor& desc = GetDSVHeapManager()->GetDescriptor(dsvHandle);
					dsvList.push_back(dsvHandle);
					SResourceStateTracker rst;
					GetResourceStateTracker(desc.resource, &rst);
					if (rst)
						rs()->EnterState(rst, desc.dsv); // Can throw! Unlocks resource immediately.
				}
			}

			if (!dsvList.empty()) {
				rs()->CommitResourceBarriers();

				for (size_t i = 0; i < dsvList.size(); i++)
					rs()->ClearDepthStencilView(dsvList[i], flags, depth, stencil, 0, nullptr);
			}
		}
		break;
		case OperatorType::CREATE_DEVICE:
		{
			ChildPtr<Value> ch0 = GetChild(0);
			uint32 adapterIndex = 0;
			if (ch0) {
				adapterIndex = (uint32)ch0->GetValue();
				if (adapterIndex >= graphics()->GetAdaptersAndOutputs().size()) {
					AddMessage(ChipMessage(MTEXT("Invalid Adapter Index"), strUtils::ConstructString(MTEXT("Invalid adapter index: %1. There are no more than %2 adapters available. Using primary (0) adapter.")).arg(adapterIndex).arg(graphics()->GetAdaptersAndOutputs().size()), WARN));
					adapterIndex = 0;
				}
			}
			graphics()->SetAdapterIndex(adapterIndex); // This will remove old device if present on another adapter!
			ChildPtr<Value> ch1 = GetChild(1);
			bool requestDebugDevice = false;
			if (ch1)
				requestDebugDevice = ch1->GetValueAsBool();
			graphics()->SetRequestDebugDevice(requestDebugDevice);
			device(); // This will create the new device!
			// TODO: Remove old device and recreate!!
		}
		break;
		case OperatorType::GET_ADAPTER_INFO:
		{
			ChildPtr<Value> ch0 = GetChild(0);
			CHECK_CHILD(ch0, 0);
			UINT idx = (UINT)ch0->GetValue();
			auto a = graphics()->GetAdaptersAndOutputs();
			String description;
			XMFLOAT4 pciIds(0.0f, 0.0f, 0.0f, 0.0f), memory(0.0f, 0.0f, 0.0f, 0.0f);
			value ocount = 0.0;
			if (idx >= a.size())
				throw GraphicsException(strUtils::ConstructString(MTEXT("Invalid adapter index: %1. There are no more than %2 adapters available.")).arg(idx).arg(a.size()), FATAL);

			auto b = a[idx].second;
			DXGI_ADAPTER_DESC1 desc;
			if (SUCCEEDED(a[idx].first->GetDesc1(&desc))) {
				description = strUtils::narrow2(desc.Description);
				pciIds.x = (float)desc.VendorId;
				pciIds.y = (float)desc.DeviceId;
				pciIds.z = (float)desc.SubSysId;
				pciIds.w = (float)desc.Revision;
				memory.x = (float)desc.DedicatedVideoMemory;
				memory.y = (float)desc.DedicatedSystemMemory;
				memory.z = (float)desc.SharedSystemMemory;
			}
			ocount = (value)a[idx].second.size();

			ChildPtr<Text> ch1 = GetChild(1);
			if (ch1)
				ch1->SetText(description);
			ChildPtr<VectorChip> ch2 = GetChild(2);
			if (ch2)
				ch2->SetVector(memory);
			ChildPtr<VectorChip> ch3 = GetChild(3);
			if (ch3)
				ch3->SetVector(pciIds);
			ChildPtr<Value> ch4 = GetChild(4);
			if (ch4)
				ch4->SetValue(ocount);
		}
		break;
		case OperatorType::GET_OUTPUT_INFO:
		{
			ChildPtr<Value> ch0 = GetChild(0);
			CHECK_CHILD(ch0, 0);
			ChildPtr<Value> ch1 = GetChild(1);
			CHECK_CHILD(ch1, 1);
			UINT idx = (UINT)ch0->GetValue();
			UINT oidx = (UINT)ch1->GetValue();
			auto a = graphics()->GetAdaptersAndOutputs();
			String name;
			XMFLOAT4 coords(0.0f, 0.0f, 0.0f, 0.0f);

			if (idx >= a.size())
				throw GraphicsException(strUtils::ConstructString(MTEXT("Invalid adapter index: %1. There are no more than %2 adapters available.")).arg(idx).arg(a.size()), FATAL);
			auto b = a[idx].second;

			if (oidx >= b.size())
				throw GraphicsException(strUtils::ConstructString(MTEXT("Invalid index: %1. Adapter has only %2 outputs.")).arg(oidx).arg(b.size()), FATAL);

			DXGI_OUTPUT_DESC desc;
			if (FAILED(b[oidx]->GetDesc(&desc)))
				throw GraphicsException(MTEXT("Failed to get DXGI_OUTPUT_DESC."), FATAL);

			name = strUtils::narrow2(desc.DeviceName);
			coords.x = (float)desc.DesktopCoordinates.left;
			coords.y = (float)desc.DesktopCoordinates.top;
			coords.z = (float)(desc.DesktopCoordinates.right - desc.DesktopCoordinates.left);
			coords.w = (float)(desc.DesktopCoordinates.top - desc.DesktopCoordinates.bottom);

			ChildPtr<Text> ch2 = GetChild(2);
			if (ch2)
				ch2->SetText(name);
			ChildPtr<VectorChip> ch3 = GetChild(3);
			if (ch3)
				ch3->SetVector(coords);
		}
		break;
		case OperatorType::FULLSCREEN:
		{
			UINT width = 0, height = 0;
			DXGI_RATIONAL refreshRate = { 0, 0 };
			uint32 outputIndex = 0;
			bool vSync = false;
			ChildPtr<Value> ch0 = GetChild(0);
			if (ch0) // Select output!
				outputIndex = (uint32)ch0->GetValue();
			ChildPtr<VectorChip> ch1 = GetChild(1);
			if (ch1) { // Size and refresh rate!
				XMFLOAT4 v = ch1->GetVector();
				width = (UINT)v.x;
				height = (UINT)v.y;
				refreshRate.Numerator = (UINT)v.z;
				refreshRate.Denominator = (UINT)v.w;
			}
			ChildPtr<Value> ch2 = GetChild(2);
			if (ch2) // vSync?
				vSync = ch2->GetValueAsBool();
			graphics()->GetRenderWindowManager()->GoFullscreen(width, height, refreshRate.Numerator, refreshRate.Denominator, outputIndex, 1, 0, false, vSync);
		}
		break;
		case OperatorType::WINDOWED:
		{
			INT left = INT_MIN, top = INT_MIN;
			UINT width = 0, height = 0;
			bool vSync = false;
			ChildPtr<VectorChip> ch0 = GetChild(0);
			if (ch0) { // Position and size
				XMFLOAT4 v = ch0->GetVector();
				left = (INT)v.x;
				top = (INT)v.y;
				width = (UINT)v.z;
				height = (UINT)v.w;
			}
			ChildPtr<Value> ch1 = GetChild(1);
			if (ch1) // vSync?
				vSync = ch1->GetValueAsBool();
			graphics()->GetRenderWindowManager()->GoWindowed(left, top, width, height, 1, 0, false, vSync);
		}
		break;
		case OperatorType::UNBIND_RTV:
		{
			rs()->OMSetRenderTargets(0, nullptr, nullptr);
		}
		break;
		case OperatorType::UNBIND_SRV:
			rs()->ClearGraphicsRootDescriptorTables();
			break;
		case OperatorType::UPLOAD_SUBRESOURCE:
		{
			ChildPtr<GraphicsResourceChip> ch0 = GetChild(0);
			CHECK_CHILD(ch0, 0);
			ch0->UpdateChip();
			ID3D12Resource* res = ch0->GetResource(); // Throws!
			const auto rDesc = res->GetDesc();

			D3D12_BOX resBox = { 0u, 0u, 0u, (UINT)rDesc.Width, rDesc.Height, std::max(1u, (UINT)rDesc.DepthOrArraySize) };
			ChildPtr<Value> ch1 = GetChild(1);
			ChildPtr<VectorChip> ch2 = GetChild(2);
			ChildPtr<VectorChip> ch3 = GetChild(3);

			UINT subresource = ch1 ? (UINT)ch1->GetValue() : 0;
			if (ch2) {
				XMFLOAT4 resBoxA = ch2->GetVector();
				resBox.left = (UINT)resBoxA.x;
				resBox.top = (UINT)resBoxA.y;
				resBox.front = (UINT)resBoxA.z;
			}
			if (ch3) {
				XMFLOAT4 resBoxB = ch3->GetVector();
				resBox.right = resBox.left + std::max(1u, (UINT)resBoxB.x);
				resBox.bottom = resBox.top + std::max(1u, (UINT)resBoxB.y);
				resBox.back = resBox.front + std::max(1u, (UINT)resBoxB.z);
			}

			D3D12_TEXTURE_COPY_LOCATION resLocation = { 0 };
			resLocation.pResource = res;
			resLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
			resLocation.SubresourceIndex = subresource;

			SResourceStateTracker rst;
			GetResourceStateTracker(res, &rst);

#ifdef DEVCHECKS
			// Check source
			{
				UINT m, a, p, w, h, d;
				UINT subresourceCount = rDesc.MipLevels * rDesc.DepthOrArraySize * (UINT)D3D12GetFormatPlaneCount(rs()->device(), rDesc.Format);
				D3D12DecomposeSubresource(subresource, rDesc.MipLevels, rDesc.DepthOrArraySize, m, a, p);
				CalculateMipLevelSize(w, h, d, m, res);
				if (subresource >= subresourceCount)
					throw GraphicsException(String(MTEXT("Invalid subresource specified for source resource.")), FATAL);
				if (resBox.right > w || resBox.bottom > h || resBox.back > d)
					throw GraphicsException(String(MTEXT("Source does not fit the region being copied.")), FATAL);
			}

			if (resBox.right <= resBox.left || resBox.bottom <= resBox.top || resBox.back <= resBox.front)
				throw GraphicsException(String(MTEXT("The region being specified is not valid.")), FATAL);
#endif

			if (rst)
				rs()->EnterState(rst, D3D12_RESOURCE_STATE_COPY_DEST); // Can throw! Unlocks resource immediately.

			DXGI_FORMAT fmt = rDesc.Format;
			UINT w = resBox.right - resBox.left;
			UINT h = resBox.bottom - resBox.top;
			UINT d = resBox.back - resBox.front;
			UINT bpp = dxgiformat::BitsPerPixel(fmt);
			BYTE* data = nullptr;

			UINT i, j, k;
			XMFLOAT4 v;
			BYTE* pixel;
			std::function<void()> convertAndAssign;
			switch (fmt)
			{
			case DXGI_FORMAT_R32G32B32A32_TYPELESS: break;
			case DXGI_FORMAT_R32G32B32A32_FLOAT: convertAndAssign = [&]() { *(XMFLOAT4*)(pixel) = v; }; break;
			case DXGI_FORMAT_R32G32B32A32_UINT: convertAndAssign = [&]() { *(XMUINT4*)(pixel) = XMUINT4((UINT)v.x, (UINT)v.y, (UINT)v.z, (UINT)v.w); }; break;
			case DXGI_FORMAT_R32G32B32A32_SINT: convertAndAssign = [&]() { *(XMINT4*)(pixel) = XMINT4((INT)v.x, (INT)v.y, (INT)v.z, (INT)v.w); }; break;
			case DXGI_FORMAT_R32G32B32_TYPELESS: break;
			case DXGI_FORMAT_R32G32B32_FLOAT: convertAndAssign = [&]() { *(XMFLOAT3*)(pixel) = XMFLOAT3((const FLOAT*)&v); }; break;
			case DXGI_FORMAT_R32G32B32_UINT: convertAndAssign = [&]() { *(XMUINT3*)(pixel) = XMUINT3((UINT)v.x, (UINT)v.y, (UINT)v.z); }; break;
			case DXGI_FORMAT_R32G32B32_SINT: convertAndAssign = [&]() { *(XMINT3*)(pixel) = XMINT3((INT)v.x, (INT)v.y, (INT)v.z); }; break;
			case DXGI_FORMAT_R16G16B16A16_TYPELESS: break;
			case DXGI_FORMAT_R16G16B16A16_FLOAT: convertAndAssign = [&]() { *(XMHALF4*)(pixel) = XMHALF4((const FLOAT*)&v); }; break;
			case DXGI_FORMAT_R16G16B16A16_UNORM: break;
			case DXGI_FORMAT_R16G16B16A16_UINT: break;
			case DXGI_FORMAT_R16G16B16A16_SNORM: break;
			case DXGI_FORMAT_R16G16B16A16_SINT: break;
			case DXGI_FORMAT_R32G32_TYPELESS: break;
			case DXGI_FORMAT_R32G32_FLOAT: convertAndAssign = [&]() { *(XMFLOAT2*)(pixel) = XMFLOAT2((const FLOAT*)&v); }; break;
			case DXGI_FORMAT_R32G32_UINT: convertAndAssign = [&]() { *(XMUINT2*)(pixel) = XMUINT2((UINT)v.x, (UINT)v.y); }; break;
			case DXGI_FORMAT_R32G32_SINT: convertAndAssign = [&]() { *(XMINT2*)(pixel) = XMINT2((INT)v.x, (INT)v.y); }; break;
			case DXGI_FORMAT_R32G8X24_TYPELESS: break;
			case DXGI_FORMAT_D32_FLOAT_S8X24_UINT: break;
			case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS: break;
			case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT: break;
			case DXGI_FORMAT_R10G10B10A2_TYPELESS: break;
			case DXGI_FORMAT_R10G10B10A2_UNORM: break;
			case DXGI_FORMAT_R10G10B10A2_UINT: break;
			case DXGI_FORMAT_R11G11B10_FLOAT: convertAndAssign = [&]() { *(XMFLOAT3PK*)(pixel) = XMFLOAT3PK(v.x, v.y, v.z); }; break;
			case DXGI_FORMAT_R8G8B8A8_TYPELESS: break;
			case DXGI_FORMAT_R8G8B8A8_UNORM:
			case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB: convertAndAssign = [&]() { *(XMUBYTE4*)(pixel) = XMUBYTE4((BYTE)clamp(v.x * 255.0f, 0.0f, 255.0f), (BYTE)clamp(v.y * 255.0f, 0.0f, 255.0f), (BYTE)clamp(v.z * 255.0f, 0.0f, 255.0f), (BYTE)clamp(v.w * 255.0f, 0.0f, 255.0f)); }; break;
			case DXGI_FORMAT_R8G8B8A8_UINT: break;
			case DXGI_FORMAT_R8G8B8A8_SNORM: break;
			case DXGI_FORMAT_R8G8B8A8_SINT: break;
			case DXGI_FORMAT_R16G16_TYPELESS: break;
			case DXGI_FORMAT_R16G16_FLOAT: convertAndAssign = [&]() { *(XMHALF2*)(pixel) = XMHALF2((const FLOAT*)&v); }; break;
			case DXGI_FORMAT_R16G16_UNORM: break;
			case DXGI_FORMAT_R16G16_UINT: break;
			case DXGI_FORMAT_R16G16_SNORM: break;
			case DXGI_FORMAT_R16G16_SINT: break;
			case DXGI_FORMAT_R32_TYPELESS: break;
			case DXGI_FORMAT_D32_FLOAT:
			case DXGI_FORMAT_R32_FLOAT: convertAndAssign = [&]() { *(FLOAT*)(pixel) = v.x; }; break;
			case DXGI_FORMAT_R32_UINT: convertAndAssign = [&]() { *(UINT*)(pixel) = (UINT)v.x; }; break;
			case DXGI_FORMAT_R32_SINT: convertAndAssign = [&]() { *(INT*)(pixel) = (INT)v.x; }; break;
			case DXGI_FORMAT_R24G8_TYPELESS: break;
			case DXGI_FORMAT_D24_UNORM_S8_UINT: break;
			case DXGI_FORMAT_R24_UNORM_X8_TYPELESS: break;
			case DXGI_FORMAT_X24_TYPELESS_G8_UINT: break;
			case DXGI_FORMAT_R8G8_TYPELESS: break;
			case DXGI_FORMAT_R8G8_UNORM: break;
			case DXGI_FORMAT_R8G8_UINT: break;
			case DXGI_FORMAT_R8G8_SNORM: break;
			case DXGI_FORMAT_R8G8_SINT: break;
			case DXGI_FORMAT_R16_TYPELESS: break;
			case DXGI_FORMAT_R16_FLOAT: break;
			case DXGI_FORMAT_D16_UNORM: break;
			case DXGI_FORMAT_R16_UNORM: break;
			case DXGI_FORMAT_R16_UINT: break;
			case DXGI_FORMAT_R16_SNORM: break;
			case DXGI_FORMAT_R16_SINT: break;
			case DXGI_FORMAT_R8_TYPELESS: break;
			case DXGI_FORMAT_R8_UNORM: break;
			case DXGI_FORMAT_R8_UINT: break;
			case DXGI_FORMAT_R8_SNORM: break;
			case DXGI_FORMAT_R8_SINT: break;
			case DXGI_FORMAT_A8_UNORM: break;
			default: break;
			};

			if (!convertAndAssign)
				throw GraphicsException(String(MTEXT("The resource's format is not supported for this operation.")), FATAL);

			D3D12_PLACED_SUBRESOURCE_FOOTPRINT placedTexture = {};
			placedTexture.Footprint = CD3DX12_SUBRESOURCE_FOOTPRINT(fmt, w, h, d, (UINT)__align(w * bpp / 8, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT));
			rs()->GetUploadHeap()->Allocate(placedTexture.Footprint.Height * placedTexture.Footprint.RowPitch * placedTexture.Footprint.Depth, D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT, &data, nullptr, &placedTexture.Offset);

			for (k = 0; k < d; k++) {
				for (j = 0; j < h; j++) {
					for (i = 0; i < w; i++) {
						ChipChildPtr ch4 = GetChild(4);
						ChildPtr<VectorChip> ch5 = GetChild(5);
						ChildPtr<VectorChip> ch6 = GetChild(6);
						CHECK_CHILD(ch6, 6);

						if (ch5)
							ch5->SetVector(XMFLOAT4((float)i, (float)j, (float)k, 0.0f));
						if (ch4)
							ch4->CallChip();
						v = ch6->GetVector();
						pixel = data + (placedTexture.Footprint.RowPitch * (h * k + j)) + i * bpp / 8;
						convertAndAssign();
					}
				}
			}

			rs()->CommitResourceBarriers();
			CD3DX12_TEXTURE_COPY_LOCATION l0(res, subresource);
			CD3DX12_TEXTURE_COPY_LOCATION l1(rs()->GetUploadHeap()->GetBufferResource(), placedTexture);
			CD3DX12_BOX box(0, 0, 0, w, h, d);
			rs()->CopyTextureRegion(&l0, resBox.left, resBox.top, resBox.front, &l1, &box);

			break;
		}
		case OperatorType::RESOLVE_SUBRESOURCE:
		{
			ChildPtr<GraphicsResourceChip> ch0 = GetChild(0);
			ChildPtr<Value> ch1 = GetChild(1);
			ChildPtr<Value> ch2 = GetChild(2);
			ChildPtr<GraphicsResourceChip> ch3 = GetChild(3);
			ChildPtr<Value> ch4 = GetChild(4);
			ChildPtr<Value> ch5 = GetChild(5);
			CHECK_CHILD(ch0, 0);
			ch0->UpdateChip();
			ID3D12Resource* source = ch0->GetResource(); // throws
			CHECK_CHILD(ch3, 3);
			ch3->UpdateChip();
			ID3D12Resource* dest = ch3->GetResource(); // throws

			auto sDesc = source->GetDesc();
			auto dDesc = dest->GetDesc();

			UINT sm = 0, sa = 0, dm = 0, da = 0;
			if (ch1) {
				value tmp = ch1->GetValue();
				sm = (UINT)std::max(tmp, 0.0);
			}
			if (ch2) {
				value tmp = ch2->GetValue();
				sa = (UINT)std::max(tmp, 0.0);
			}
			UINT ssr = D3D12CalcSubresource(sm, sa, 0, sDesc.MipLevels, sDesc.DepthOrArraySize);
			if (ch4) {
				value tmp = ch4->GetValue();
				dm = (UINT)std::max(tmp, 0.0);
			}
			if (ch5) {
				value tmp = ch5->GetValue();
				da = (UINT)std::max(tmp, 0.0);
			}
			UINT dsr = D3D12CalcSubresource(dm, da, 0, dDesc.MipLevels, dDesc.DepthOrArraySize);

#ifdef DEVCHECKS
			if (sDesc.Dimension != D3D12_RESOURCE_DIMENSION_TEXTURE2D || sDesc.SampleDesc.Count == 1)
				throw GraphicsException(String(MTEXT("Source must be a multisampled 2D texture.")), FATAL);
			if (dDesc.Dimension != D3D12_RESOURCE_DIMENSION_TEXTURE2D || dDesc.SampleDesc.Count != 1)
				throw GraphicsException(String(MTEXT("Destination must be a non-multisampled 2D texture.")), FATAL);
			if (dxgiformat::MakeTypeless(sDesc.Format) != dxgiformat::MakeTypeless(dDesc.Format))
				throw GraphicsException(strUtils::ConstructString(MTEXT("Source and destination formats are not compatible: %1 vs %2.")).arg(dxgiformat::ToString(sDesc.Format)).arg(dxgiformat::ToString(dDesc.Format)), FATAL);
			if (dxgiformat::IsTypeless(sDesc.Format))
				throw GraphicsException(strUtils::ConstructString(MTEXT("Source format can not be typeless: %1.")).arg(dxgiformat::ToString(sDesc.Format)), FATAL);
			if (sm >= sDesc.MipLevels)
				throw GraphicsException(strUtils::ConstructString(MTEXT("Invalid mip level supplied: %1. Source does not have more than %2 mip levels.")).arg(sm).arg(sDesc.MipLevels), FATAL);
			if (sa >= sDesc.DepthOrArraySize)
				throw GraphicsException(strUtils::ConstructString(MTEXT("Invalid array slice supplied: %1. Source does not have more than %2 array slices.")).arg(sa).arg(sDesc.DepthOrArraySize), FATAL);
			if (dm >= dDesc.MipLevels)
				throw GraphicsException(strUtils::ConstructString(MTEXT("Invalid mip level supplied: %1. Destination does not have more than %2 mip levels.")).arg(dm).arg(dDesc.MipLevels), FATAL);
			if (da >= dDesc.DepthOrArraySize)
				throw GraphicsException(strUtils::ConstructString(MTEXT("Invalid array slice supplied: %1. Destination does not have more than %2 array slices.")).arg(da).arg(dDesc.DepthOrArraySize), FATAL);
#endif

			SResourceStateTracker rst;
			GetResourceStateTracker(source, &rst);
			if (rst)
				rs()->EnterState(rst, { sm, 1u, sa, 1u, 0u, 0xFFFFFFFF }, D3D12_RESOURCE_STATE_RESOLVE_SOURCE); // Can throw! Unlocks resource immediately.

			GetResourceStateTracker(dest, &rst);
			if (rst)
				rs()->EnterState(rst, { dm, 1u, da, 1u, 0, 0xFFFFFFFF }, D3D12_RESOURCE_STATE_RESOLVE_DEST); // Can throw! Unlocks resource immediately.

			rs()->CommitResourceBarriers();
			rs()->ResolveSubresource(dest, dsr, source, ssr, sDesc.Format);
		}
		break;
		case OperatorType::COPY_RESOURCE:
		{
			ChildPtr<GraphicsResourceChip> ch0 = GetChild(0);
			CHECK_CHILD(ch0, 0);
			ch0->UpdateChip();
			ChildPtr<GraphicsResourceChip> ch1 = GetChild(1);
			CHECK_CHILD(ch0, 1);
			ch1->UpdateChip();
			ID3D12Resource* source = ch0->GetResource(); // Throws!
			ID3D12Resource* dest = ch1->GetResource(); // Throws!
			const auto sDesc = source->GetDesc();
			const auto dDesc = dest->GetDesc();
#ifdef DEVCHECKS
			if (source == dest)
				throw GraphicsException(String(MTEXT("Please supply different resources for the copy operation.")), FATAL);
			if (sDesc.Dimension != dDesc.Dimension)
				throw GraphicsException(String(MTEXT("Source and destination resources must be of the same type (Texture1D, Texture2D, Texture2DArray, etc).")), FATAL);
			if (sDesc.Width != dDesc.Width || sDesc.Height != dDesc.Height || sDesc.DepthOrArraySize != dDesc.DepthOrArraySize)
				throw GraphicsException(String(MTEXT("Source and destination resources must have the same dimensions (Width, height, depth or array size).")), FATAL);
			if (sDesc.MipLevels != dDesc.MipLevels)
				throw GraphicsException(String(MTEXT("Source and destination resources must have the same number of mip levels.")), FATAL);
			if (sDesc.SampleDesc.Count != dDesc.SampleDesc.Count || sDesc.SampleDesc.Quality != dDesc.SampleDesc.Quality)
				throw GraphicsException(String(MTEXT("Source and destination resources must have equal settings for multisampling.")), FATAL);
			if (dxgiformat::MakeTypeless(sDesc.Format) != dxgiformat::MakeTypeless(dDesc.Format))
				throw GraphicsException(strUtils::ConstructString(MTEXT("Source and destination formats are not compatible: %1 vs %2.")).arg(dxgiformat::ToString(sDesc.Format)).arg(dxgiformat::ToString(dDesc.Format)), FATAL);
#endif

			SResourceStateTracker rst;
			GetResourceStateTracker(source, &rst);
			if (rst)
				rs()->EnterState(rst, D3D12_RESOURCE_STATE_COPY_SOURCE); // Can throw! Unlocks resource immediately.

			GetResourceStateTracker(dest, &rst);
			if (rst)
				rs()->EnterState(rst, D3D12_RESOURCE_STATE_COPY_DEST); // Can throw! Unlocks resource immediately.

			rs()->CommitResourceBarriers();
			rs()->CopyResource(dest, source);
		}
		break;
		case OperatorType::COPY_SUBRESOURCE_REGION:
		{
			ChildPtr<GraphicsResourceChip> ch0 = GetChild(0);
			CHECK_CHILD(ch0, 0);
			ch0->UpdateChip();
			ChildPtr<GraphicsResourceChip> ch4 = GetChild(4);
			CHECK_CHILD(ch4, 4);
			ch4->UpdateChip();
			ID3D12Resource* source = ch0->GetResource(); // Throws!
			ID3D12Resource* dest = ch4->GetResource(); // Throws!
			const auto sDesc = source->GetDesc();
			const auto dDesc = dest->GetDesc();
			D3D12_BOX srcBox = { 0u, 0u, 0u, (UINT)sDesc.Width, sDesc.Height, std::max(1u, (sDesc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE3D ? (UINT)sDesc.DepthOrArraySize : 1u)) };
			ChildPtr<Value> ch1 = GetChild(1);
			ChildPtr<VectorChip> ch2 = GetChild(2);
			ChildPtr<VectorChip> ch3 = GetChild(3);
			ChildPtr<Value> ch5 = GetChild(5);
			ChildPtr<VectorChip> ch6 = GetChild(6);
			UINT srcSubresource = ch1 ? (UINT)ch1->GetValue() : 0;
			if (ch2) {
				XMFLOAT4 srcBoxA = ch2->GetVector();
				srcBox.left = (UINT)srcBoxA.x;
				srcBox.top = (UINT)srcBoxA.y;
				srcBox.front = (UINT)srcBoxA.z;
			}
			if (ch3) {
				XMFLOAT4 srcBoxB = ch3->GetVector();
				srcBox.right = srcBox.left + std::max(1u, (UINT)srcBoxB.x);
				srcBox.bottom = srcBox.top + std::max(1u, (UINT)srcBoxB.y);
				srcBox.back = srcBox.front + std::max(1u, (UINT)srcBoxB.z);
			}
			UINT dstSubresource = ch5 ? (UINT)ch5->GetValue() : 0;
			XMFLOAT4 fCoords = ch6 ? ch6->GetVector() : XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
			XMUINT4 dstCoords = XMUINT4((UINT)fCoords.x, (UINT)fCoords.y, (UINT)fCoords.z, 0);

			D3D12_TEXTURE_COPY_LOCATION srcLocation = CD3DX12_TEXTURE_COPY_LOCATION(source, srcSubresource);
			D3D12_TEXTURE_COPY_LOCATION dstLocation = CD3DX12_TEXTURE_COPY_LOCATION(dest, dstSubresource);

#ifdef DEVCHECKS
			// Check source
			{
				UINT m, a, p, w, h, d;
				UINT subresourceCount = sDesc.MipLevels * sDesc.DepthOrArraySize * (UINT)D3D12GetFormatPlaneCount(rs()->device(), sDesc.Format);
				D3D12DecomposeSubresource(srcSubresource, sDesc.MipLevels, sDesc.DepthOrArraySize, m, a, p);
				CalculateMipLevelSize(w, h, d, m, source);
				if (srcSubresource >= subresourceCount)
					throw GraphicsException(String(MTEXT("Invalid subresource specified for source resource.")), FATAL);
				if (srcBox.right > w || srcBox.bottom > h || srcBox.back > d)
					throw GraphicsException(String(MTEXT("Source does not fit the region being copied.")), FATAL);
			}
			// Check destination
			{
				UINT m, a, p, w, h, d;
				UINT subresourceCount = dDesc.MipLevels * dDesc.DepthOrArraySize * (UINT)D3D12GetFormatPlaneCount(rs()->device(), dDesc.Format);
				D3D12DecomposeSubresource(dstSubresource, dDesc.MipLevels, dDesc.DepthOrArraySize, m, a, p);
				CalculateMipLevelSize(w, h, d, m, dest);
				if (dstSubresource >= subresourceCount)
					throw GraphicsException(String(MTEXT("Invalid subresource specified for destination resource.")), FATAL);
				if (dstCoords.x >= w || dstCoords.y >= h || dstCoords.z >= d)
					throw GraphicsException(String(MTEXT("Destination Left/Upper/Front coordinates are not valid.")), FATAL);
				if (dstCoords.x + (srcBox.right - srcBox.left) > w || dstCoords.y + (srcBox.bottom - srcBox.top) > h || dstCoords.z + (srcBox.back - srcBox.front) > d)
					throw GraphicsException(String(MTEXT("Destination does not fit the region being copied.")), FATAL);
			}

			if (srcBox.right <= srcBox.left || srcBox.bottom <= srcBox.top || srcBox.back <= srcBox.front)
				throw GraphicsException(String(MTEXT("The region being specified is not valid.")), FATAL);
			if (sDesc.SampleDesc.Count != dDesc.SampleDesc.Count || sDesc.SampleDesc.Quality != dDesc.SampleDesc.Quality)
				throw GraphicsException(String(MTEXT("Source and destination resources must have equal settings for multisampling.")), FATAL);
			if (dxgiformat::MakeTypeless(sDesc.Format) != dxgiformat::MakeTypeless(dDesc.Format))
				throw GraphicsException(strUtils::ConstructString(MTEXT("Source and destination formats are not compatible: %1 vs %2.")).arg(dxgiformat::ToString(sDesc.Format)).arg(dxgiformat::ToString(dDesc.Format)), FATAL);
#endif

			SResourceStateTracker rst;
			GetResourceStateTracker(source, &rst);
			if (rst)
				rs()->EnterState(rst, D3D12_RESOURCE_STATE_COPY_SOURCE); // Can throw! Unlocks resource immediately.

			GetResourceStateTracker(dest, &rst);
			if (rst)
				rs()->EnterState(rst, D3D12_RESOURCE_STATE_COPY_DEST); // Can throw! Unlocks resource immediately.

			rs()->CommitResourceBarriers();
			rs()->CopyTextureRegion(&dstLocation, dstCoords.x, dstCoords.y, dstCoords.z, &srcLocation, &srcBox);
		}
		break;
		case OperatorType::DOWNLOAD_SUBRESOURCE_REGION:
		{
			ChildPtr<GraphicsResourceChip> ch0 = GetChild(0);
			CHECK_CHILD(ch0, 0);
			ch0->UpdateChip();
			ChildPtr<ReadbackBuffer> ch2 = GetChild(2);
			CHECK_CHILD(ch2, 2);
			ID3D12Resource* source = ch0->GetResource(); // Throws!
			const auto sDesc = source->GetDesc();
			ChildPtr<Value> ch1 = GetChild(1);
			UINT srcSubresource = ch1 ? (UINT)ch1->GetValue() : 0;

#ifdef DEVCHECKS
			// Check source
			{
				UINT m, a, p, w, h, d;
				UINT subresourceCount = sDesc.MipLevels * sDesc.DepthOrArraySize * (UINT)D3D12GetFormatPlaneCount(rs()->device(), sDesc.Format);
				D3D12DecomposeSubresource(srcSubresource, sDesc.MipLevels, sDesc.DepthOrArraySize, m, a, p);
				CalculateMipLevelSize(w, h, d, m, source);
				if (srcSubresource >= subresourceCount)
					throw GraphicsException(String(MTEXT("Invalid subresource specified for source resource.")), FATAL);
			}

			if (sDesc.SampleDesc.Count > 1)
				throw GraphicsException(String(MTEXT("Multisampling not supported for readback.")), FATAL);
			if (dxgiformat::IsTypeless(sDesc.Format))
				throw GraphicsException(String(MTEXT("Texture can not be typeless.")), FATAL);
#endif
			if (ch2->IsDownloading())
				throw GraphicsException(String(MTEXT("Readback buffer is busy downloading data.")), FATAL);

			UINT64 totalResourceSize = 0;
			UINT64 fpRowPitch = 0;
			UINT fpRowCount = 0;
			D3D12_PLACED_SUBRESOURCE_FOOTPRINT fp = {};
			rs()->device()->GetCopyableFootprints(&sDesc, srcSubresource, 1, 0, &fp, &fpRowCount, &fpRowPitch, &totalResourceSize);

			RID3D12Resource destRes;
			{
				D3D12_RESOURCE_DESC bufferDesc = {};
				bufferDesc.Alignment = sDesc.Alignment;
				bufferDesc.DepthOrArraySize = 1;
				bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
				bufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
				bufferDesc.Format = DXGI_FORMAT_UNKNOWN;
				bufferDesc.Height = 1;
				bufferDesc.Width = totalResourceSize;
				bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
				bufferDesc.MipLevels = 1;
				bufferDesc.SampleDesc.Count = 1;
				bufferDesc.SampleDesc.Quality = 0;

				CD3DX12_HEAP_PROPERTIES hp(D3D12_HEAP_TYPE_READBACK);
				HRESULT hr = rs()->device()->CreateCommittedResource(
					&hp,
					D3D12_HEAP_FLAG_NONE,
					&bufferDesc,
					D3D12_RESOURCE_STATE_COPY_DEST,
					nullptr,
					IID_PPV_ARGS(&destRes));
				if (FAILED(hr))
					throw GraphicsException(String(MTEXT("Failed to create readback heap.")));
			}

			CD3DX12_TEXTURE_COPY_LOCATION srcLocation(source, srcSubresource);
			CD3DX12_TEXTURE_COPY_LOCATION dstLocation(destRes, fp);

			SResourceStateTracker rst;
			GetResourceStateTracker(source, &rst);
			if (rst)
				rs()->EnterState(rst, D3D12_RESOURCE_STATE_COPY_SOURCE); // Can throw! Unlocks resource immediately.

			rs()->CommitResourceBarriers();
			rs()->GetCommandList1()->CopyTextureRegion(&dstLocation, 0, 0, 0, &srcLocation, nullptr);
			ch2->SetResource(destRes, fp);

		}
		break;
		case GraphicsCommand::OperatorType::DESTROY_RESOURCE:
		{
			for (uint32 i = 0, j = GetSubConnectionCount(0); i < j; i++) {
				ChildPtr<GraphicsResourceChip> ch0 = GetChild(0, i);
				if (ch0)
					ch0->ClearResource();
			}
		}
		break;
		case GraphicsCommand::OperatorType::SET_TEXTURE_PROPS:
		{
			ChildPtr<Texture> ch0 = GetChild(0);
			if (!ch0)
				throw MissingChildException(this, 0);
			auto d = ch0->GetInitDesc();
			{
				ChildPtr<Value> ch3 = GetChild(3);
				if (ch3)
					d.Depth = std::min((INT)ch3->GetValue(), D3D12_REQ_TEXTURE3D_U_V_OR_W_DIMENSION);
			}
			{
				ChildPtr<Value> ch2 = GetChild(2);
				if (ch2)
					d.Height = std::min((INT)ch2->GetValue(), d.Depth > 1 ? D3D12_REQ_TEXTURE3D_U_V_OR_W_DIMENSION : D3D12_REQ_TEXTURE2D_U_OR_V_DIMENSION);
			}
			{
				ChildPtr<Value> ch1 = GetChild(1);
				if (ch1)
					d.Width = std::max(1, std::min((INT)ch1->GetValue(), d.Depth > 1 ? D3D12_REQ_TEXTURE3D_U_V_OR_W_DIMENSION : (d.Height > 1 || d.SampleDesc.Count > 1 ? D3D12_REQ_TEXTURE2D_U_OR_V_DIMENSION : D3D12_REQ_TEXTURE1D_U_DIMENSION)));
			}
			{
				ChildPtr<Value> ch4 = GetChild(4);
				if (ch4)
					d.MipLevels = std::min((INT)ch4->GetValue(), 14);
			}
			{
				ChildPtr<Value> ch5 = GetChild(5);
				if (ch5)
					d.ArraySize = std::min((INT)ch5->GetValue(), d.Height > 1 || d.SampleDesc.Count ? D3D12_REQ_TEXTURE2D_ARRAY_AXIS_DIMENSION : D3D12_REQ_TEXTURE1D_ARRAY_AXIS_DIMENSION);
			}
			ch0->SetInitDesc(d);
		}
		break;
		/*	case OperatorType::MAP_RESOURCE_READ:
			case OperatorType::MAP_RESOURCE_WRITE:
			case OperatorType::MAP_RESOURCE_READ_WRITE:
			case OperatorType::MAP_RESOURCE_WRITE_DISCARD:
			{

				ChildPtr<D3DResource> ch0 = GetChild(0);
				ChildPtr<Value> ch1 = GetChild(1);
				ChildPtr<Value> ch2 = GetChild(2);
				ChildPtr<D3DMappedResource> ch3 = GetChild(3);
				if (ch0 && ch3) {
					if (ch3->IsMapped()) {} // fail!
					else {
						ID3D11Resource *res = ch0->GetResource();
						if (res) {
							UINT subresource = ch1 ? (UINT)ch1->GetValue() : 0;
							D3D11_MAP maptype;
							switch (_cmd) {
							case OperatorType::MAP_RESOURCE_READ: maptype = D3D11_MAP_READ; break;
							case OperatorType::MAP_RESOURCE_WRITE: maptype = D3D11_MAP_WRITE; break;
							case OperatorType::MAP_RESOURCE_READ_WRITE: maptype = D3D11_MAP_READ_WRITE; break;
							case OperatorType::MAP_RESOURCE_WRITE_DISCARD: maptype = D3D11_MAP_WRITE_DISCARD; break;
							}
							UINT flags = ch2 && ch2->GetValueAsBool() ? D3D11_MAP_FLAG_DO_NOT_WAIT : 0;
							ch3->Map(res, subresource, maptype, flags);
						}
					}
				}
			}
			break;
			case OperatorType::UNMAP_RESOURCE:
			{
				ChildPtr<D3DMappedResource> ch0 = GetChild(0);
				if (ch0)
					ch0->Unmap();
			}
			break;
			case OperatorType::LOAD_TEXTURE_FROM_FILE:
			{
				ChildPtr<Texture> ch0 = GetChild(0);
				ChildPtr<Text> ch1 = GetChild(1);
				if (ch0 && ch1) {
					Path p(ch1->GetText());
					if (p.IsFile()) {
						bool b = ch0->LoadImageDataFromFile(p);
					}
				}

			}
			break;
			case OperatorType::SET_GLOBAL_SHADER_RESOURCES:
			{
				ChildPtr<Text> ch0 = GetChild(0);
				if (!ch0)
					return;
				String t = ch0->GetText();
				String s = t;
				List<SID3D11ShaderResourceView> r;
				for (uint32 i = 0, j = GetSubConnectionCount(1); i < j; i++) {
					ChildPtr<D3DResource> ch1 = GetChild(1, i);
					r.push_back(ch1 ? ch1->GetShaderResourceView() : nullptr);
				}
				if (r.size())
					graphics()->rs()->SetShaderResources(s, r);
			}
			break;*/
		case OperatorType::SET_CAMERA_MATRIES:
		{
			ChildPtr<MatrixChip> ch0 = GetChild(0);
			ChildPtr<MatrixChip> ch1 = GetChild(1);
			ChildPtr<MatrixChip> ch2 = GetChild(2);
			ChildPtr<MatrixChip> ch3 = GetChild(3);
			XMFLOAT4X4 v, p, c, s;
			v = ch0 ? ch0->GetMatrix() : MatrixChip::IDENTITY;
			p = ch1 ? ch1->GetMatrix() : MatrixChip::IDENTITY;
			c = ch2 ? ch2->GetMatrix() : MatrixChip::IDENTITY; // <== This should probably be v*p by default
			s = ch3 ? ch3->GetMatrix() : MatrixChip::IDENTITY;
			rs()->SetViewMatrix(v);
			rs()->SetProjectionMatrix(p);
			rs()->SetCullingMatrix(c);
			rs()->SetShadowMatrix(s);
		}
		break;
		case OperatorType::DEBUG_DRAW:
		{
			dg()->CallChip();
		}
		break;
		case OperatorType::DEBUG_ADD_POINT:
		{
			ChildPtr<VectorChip> v0 = GetChild(0);
			ChildPtr<VectorChip> c0 = GetChild(1);
			CHECK_CHILD(v0, 0);
			XMFLOAT3 p0 = XMFLOAT3((float*)&v0->GetVector());
			XMUBYTE4 q0 = c0 ? XMUBYTE4((float*)&c0->GetVector()) : WHITE;
			dg()->AddPoint(DebugVertex(p0, q0));
		}
		break;
		case OperatorType::DEBUG_ADD_LINE:
		{
			ChildPtr<VectorChip> v0 = GetChild(0);
			ChildPtr<VectorChip> v1 = GetChild(1);
			ChildPtr<VectorChip> c0 = GetChild(2);
			ChildPtr<VectorChip> c1 = GetChild(3);
			CHECK_CHILD(v0, 0);
			CHECK_CHILD(v1, 1);
			XMFLOAT3 p0 = XMFLOAT3((float*)&v0->GetVector());
			XMUBYTE4 q0 = WHITE;
			if (c0) XMStoreUByte4(&q0, XMVectorScale(XMLoadFloat4(&c0->GetVector()), 255.0f));
			XMFLOAT3 p1 = XMFLOAT3((float*)&v1->GetVector());
			XMUBYTE4 q1 = WHITE;
			if (c1) XMStoreUByte4(&q1, XMVectorScale(XMLoadFloat4(&c1->GetVector()), 255.0f));
			dg()->AddLineSegment(DebugVertex(p0, q0), DebugVertex(p1, q1));
		}
		break;
		case OperatorType::DEBUG_ADD_TRIANGLE:
		{
			ChildPtr<VectorChip> v0 = GetChild(0);
			ChildPtr<VectorChip> v1 = GetChild(1);
			ChildPtr<VectorChip> v2 = GetChild(2);
			ChildPtr<VectorChip> c0 = GetChild(3);
			ChildPtr<VectorChip> c1 = GetChild(4);
			ChildPtr<VectorChip> c2 = GetChild(5);
			CHECK_CHILD(v0, 0);
			CHECK_CHILD(v1, 1);
			CHECK_CHILD(v2, 2);
			XMFLOAT3 p0 = XMFLOAT3((float*)&v0->GetVector());
			XMUBYTE4 q0 = WHITE;
			if (c0) XMStoreUByte4(&q0, XMVectorScale(XMLoadFloat4(&c0->GetVector()), 255.0f));
			XMFLOAT3 p1 = XMFLOAT3((float*)&v1->GetVector());
			XMUBYTE4 q1 = WHITE;
			if (c1) XMStoreUByte4(&q1, XMVectorScale(XMLoadFloat4(&c1->GetVector()), 255.0f));
			XMFLOAT3 p2 = XMFLOAT3((float*)&v2->GetVector());
			XMUBYTE4 q2 = WHITE;
			if (c2) XMStoreUByte4(&q2, XMVectorScale(XMLoadFloat4(&c2->GetVector()), 255.0f));
			dg()->AddTriangle(DebugVertex(p0, q0), DebugVertex(p1, q1), DebugVertex(p2, q2));
		}
		break;
		};
	}
	catch (const ChipException& exc)
	{
		AddException(exc);
	}
}
