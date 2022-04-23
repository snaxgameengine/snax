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
#include "DebugGeometry.h"
#include "M3DEngine/Engine.h"
#include "M3DEngine/ChipManager.h"
#include "RenderSettings.h"
#include "DebugGeometry_hlsl.h"
#include "PipelineStatePool.h"
#include "D3DBlobUtil.h"
#include "RenderSettings.h"


using namespace m3d;


CHIPDESCV1_DEF_HIDDEN(DebugGeometry, MTEXT("Debug Geometry"), DEBUGGEOMETRY_GUID, CHIP_GUID);




void DebugGeometry::OnNewFrame() 
{ 
	ClearGeometry(); 
}

void DebugGeometry::ClearGeometry()
{
	_points.clear();
	_lineSegments.clear();
	_triangles.clear();
}

void DebugGeometry::AddPoint(const DebugVertex& p)
{
	_points.push_back(p);
}

void DebugGeometry::AddLineSegment(const DebugVertex& a, const DebugVertex& b)
{
	_lineSegments.push_back(a);
	_lineSegments.push_back(b);
}

void DebugGeometry::AddTriangle(const DebugVertex& a, const DebugVertex& b, const DebugVertex& c)
{
	_triangles.push_back(a);
	_triangles.push_back(b);
	_triangles.push_back(c);
}

void DebugGeometry::AddXZGrid(float32 scale)
{
	for (uint32 i = 1; i < 11; i++) {
		AddLineSegment(DebugVertex(XMFLOAT3(-scale, 0.0f, 0.1f * i * scale), GRAY), DebugVertex(XMFLOAT3(scale, 0.0f, 0.1f * i * scale), GRAY));
		AddLineSegment(DebugVertex(XMFLOAT3(-scale, 0.0f, -0.1f * i * scale), GRAY), DebugVertex(XMFLOAT3(scale, 0.0f, -0.1f * i * scale), GRAY));
		AddLineSegment(DebugVertex(XMFLOAT3(0.1f * i * scale, 0.0f, -scale), GRAY), DebugVertex(XMFLOAT3(0.1f * i * scale, 0.0f, scale), GRAY));
		AddLineSegment(DebugVertex(XMFLOAT3(-0.1f * i * scale, 0.0f, -scale), GRAY), DebugVertex(XMFLOAT3(-0.1f * i * scale, 0.0f, scale), GRAY));
	}
	AddLineSegment(DebugVertex(XMFLOAT3(-scale, 0.0f, 0.0f), GRAY), DebugVertex(XMFLOAT3(0.0f, 0.0f, 0.0f), GRAY));
	AddLineSegment(DebugVertex(XMFLOAT3(0.0f, 0.0f, -scale), GRAY), DebugVertex(XMFLOAT3(0.0f, 0.0f, 0.0f), GRAY));
	AddLineSegment(DebugVertex(XMFLOAT3(scale, 0.0f, 0.0f), RED), DebugVertex(XMFLOAT3(0.0f, 0.0f, 0.0f), RED));
	AddLineSegment(DebugVertex(XMFLOAT3(0.0f, 0.0f, scale), BLUE), DebugVertex(XMFLOAT3(0.0f, 0.0f, 0.0f), BLUE));
}

void DebugGeometry::AddFrustum(CXMMATRIX vp, XMUBYTE4 color)
{
	XMVECTOR det;
	XMMATRIX m = XMMatrixInverse(&det, vp);
	XMFLOAT4X4 fd;
	XMStoreFloat4x4(&fd, m);
	XMFLOAT3 corners[8] =
	{
		XMFLOAT3(-1.0f, -1.0f, 0.0f),
		XMFLOAT3(-1.0f, -1.0f, 1.0f),
		XMFLOAT3(-1.0f,  1.0f, 0.0f),
		XMFLOAT3(-1.0f,  1.0f, 1.0f),
		XMFLOAT3(1.0f, -1.0f, 0.0f),
		XMFLOAT3(1.0f, -1.0f, 1.0f),
		XMFLOAT3(1.0f,  1.0f, 0.0f),
		XMFLOAT3(1.0f,  1.0f, 1.0f)
	};

	static const UINT c[12][2] = { {0,1},{2,3},{4,5},{6,7},{0,2},{2,6},{6,4},{4,0},{1,3},{3,7},{7,5},{5,1} };

	XMVector3TransformCoordStream(corners, sizeof(XMFLOAT3), corners, sizeof(XMFLOAT3), 8, m);
	for (uint32 i = 0; i < 12; i++)
		AddLineSegment(DebugVertex(corners[c[i][0]], color), DebugVertex(corners[c[i][1]], color));

}

void DebugGeometry::AddBox(CXMMATRIX m, const XMFLOAT3& a, const XMFLOAT3& b, XMUBYTE4 color)
{
	static const UINT c[12][2] = { {0,1},{2,3},{4,5},{6,7},{0,2},{2,6},{6,4},{4,0},{1,3},{3,7},{7,5},{5,1} };

	XMFLOAT3 corners[8] =
	{
		XMFLOAT3(a.x, a.y, a.z),
		XMFLOAT3(a.x, a.y, b.z),
		XMFLOAT3(a.x, b.y, a.z),
		XMFLOAT3(a.x, b.y, b.z),
		XMFLOAT3(b.x, a.y, a.z),
		XMFLOAT3(b.x, a.y, b.z),
		XMFLOAT3(b.x, b.y, a.z),
		XMFLOAT3(b.x, b.y, b.z)
	};

	XMVector3TransformCoordStream(corners, sizeof(XMFLOAT3), corners, sizeof(XMFLOAT3), 8, m);

	for (uint32 i = 0; i < 12; i++)
		AddLineSegment(DebugVertex(corners[c[i][0]], color), DebugVertex(corners[c[i][1]], color));
}

void DebugGeometry::CallChip()
{
	try {
		HRESULT hr;

		static RootSignatureID rsID = 0;
		static PipelineStateDescID psDescID = 0;
		static PipelineStateDescID psDescGSID = 0;
		static PipelineStateDescID pisDescPointsID = 0;
		static PipelineStateDescID pisDescLinesID = 0;
		static PipelineStateDescID pisDescTrianglesID = 0;

		if ((psDescID | psDescGSID) == 0)
		{
			CD3DX12_ROOT_PARAMETER rootParameters[1];
			rootParameters[0].InitAsConstantBufferView(0);

			CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
			rootSignatureDesc.Init(_countof(rootParameters), rootParameters, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

			rsID = graphics()->GetPipelineStatePool()->RegisterRootSignatureDesc(rootSignatureDesc);

			// Describe and create the graphics pipeline state objects (PSO).
			PipelineStateDesc psoDesc;
			psoDesc.RasterizerState.MultisampleEnable = TRUE;
			psoDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;//D3D12_PIPELINE_STATE_FLAG_TOOL_DEBUG;

			if (graphics()->GetFeatureLevel() < D3D_FEATURE_LEVEL_10_0) { // No geometry shaders?
				CreateD3DBlobFromCArray((const LPVOID)DEBUG_GEOMETRY_VS_40_91_BYTECODE, sizeof(DEBUG_GEOMETRY_VS_40_91_BYTECODE), &psoDesc.VS);
				CreateD3DBlobFromCArray((const LPVOID)DEBUG_GEOMETRY_PS_40_91_BYTECODE, sizeof(DEBUG_GEOMETRY_PS_40_91_BYTECODE), &psoDesc.PS);
			}
			else {
				CreateD3DBlobFromCArray((const LPVOID)DEBUG_GEOMETRY_VS_40_BYTECODE, sizeof(DEBUG_GEOMETRY_VS_40_BYTECODE), &psoDesc.VS);
				CreateD3DBlobFromCArray((const LPVOID)DEBUG_GEOMETRY_GS_40_BYTECODE, sizeof(DEBUG_GEOMETRY_GS_40_BYTECODE), &psoDesc.GS);
				CreateD3DBlobFromCArray((const LPVOID)DEBUG_GEOMETRY_PS_40_BYTECODE, sizeof(DEBUG_GEOMETRY_PS_40_BYTECODE), &psoDesc.PS);
				psDescGSID = graphics()->GetPipelineStatePool()->RegisterPipelineStateDesc(psoDesc);
				psoDesc.GS = nullptr;
			}

			psDescID = graphics()->GetPipelineStatePool()->RegisterPipelineStateDesc(psoDesc);
		}

		if ((pisDescPointsID | pisDescLinesID | pisDescTrianglesID) == 0)
		{
			static const D3D12_INPUT_ELEMENT_DESC ied[] = { {"POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
															{"COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0} };

			PipelineInputStateDesc psoInDesc;
			psoInDesc.InputLayout = D3D12_INPUT_LAYOUT_DESC{ ied, _countof(ied) };
			psoInDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
			pisDescPointsID = graphics()->GetPipelineStatePool()->RegisterPipelineInputStateDesc(psoInDesc);

			psoInDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
			pisDescLinesID = graphics()->GetPipelineStatePool()->RegisterPipelineInputStateDesc(psoInDesc);

			psoInDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
			pisDescTrianglesID = graphics()->GetPipelineStatePool()->RegisterPipelineInputStateDesc(psoInDesc);
		}

		UINT N = UINT(_points.size() + _lineSegments.size() + _triangles.size());

		if (N == 0)
			return; // Nothing to render!

		RenderSettings* rs = graphics()->rs();

		rs->SetGraphicsRootSignature(rsID);
		rs->SetPipelineStateDesc(psDescID);
		rs->CommitGraphicsRootSignature();

		// Create and set vertex buffer.
		{
			BYTE* vbData = nullptr;
			D3D12_VERTEX_BUFFER_VIEW vbView;
			vbView.SizeInBytes = sizeof(DebugVertex) * N;
			vbView.StrideInBytes = sizeof(DebugVertex);
			hr = graphics()->GetUploadHeap()->Allocate(vbView.SizeInBytes, 512, &vbData, &vbView.BufferLocation);
			if (FAILED(hr))
				throw GraphicsException(this, MTEXT("Failed to allocate memory from upload heap."));
			if (_points.size())
				std::memcpy(vbData, &_points.front(), sizeof(DebugVertex) * _points.size());
			if (_lineSegments.size())
				std::memcpy(vbData + sizeof(DebugVertex) * _points.size(), &_lineSegments.front(), sizeof(DebugVertex) * _lineSegments.size());
			if (_triangles.size())
				std::memcpy(vbData + sizeof(DebugVertex) * (_points.size() + _lineSegments.size()), &_triangles.front(), sizeof(DebugVertex) * _triangles.size());

			rs->IASetVertexBuffers(0, 1, &vbView);
		}

		// Create and set constant buffer.
		{
			CB* cb = nullptr;
			D3D12_GPU_VIRTUAL_ADDRESS vAddress;
			hr = graphics()->GetUploadHeap()->Allocate(sizeof(CB), D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT, (BYTE**)&cb, &vAddress);
			if (FAILED(hr))
				throw GraphicsException(this, MTEXT("Failed to allocate memory from upload heap."));

			XMStoreFloat4x4(&cb->vp, XMMatrixTranspose(XMLoadFloat4x4(&rs->GetViewMatrix()) * XMLoadFloat4x4(&rs->GetProjectionMatrix())));

			if (rs->GetNumViewportsAndScissorRects() > 0) {
				const D3D12_VIEWPORT* vp = rs->GetViewports();
				cb->gsScale = XMFLOAT2(5.0f / vp->Width, 5.0f / vp->Height);
			}

			rs->SetGraphicsRootConstantBufferView(0, vAddress);
		}


		if (_lineSegments.size()) {
			rs->SetPipelineInputStateDesc(pisDescLinesID);
			rs->IASetPrimitiveTopology((M3D_PRIMITIVE_TOPOLOGY)D3D_PRIMITIVE_TOPOLOGY_LINELIST);
			rs->CommitGraphicsRootSignatureAndPipelineState();
			rs->DrawInstanced((UINT)_lineSegments.size(), 1, (UINT)_points.size(), 0);
		}
		if (_triangles.size()) {
			rs->SetPipelineInputStateDesc(pisDescTrianglesID);
			rs->IASetPrimitiveTopology((M3D_PRIMITIVE_TOPOLOGY)D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			rs->CommitGraphicsRootSignatureAndPipelineState();
			rs->DrawInstanced((UINT)_triangles.size(), 1, UINT(_points.size() + _lineSegments.size()), 0);
		}
		if (_points.size()) { // _points should ALWAYS be empty when no _gs!
			rs->SetPipelineStateDesc(psDescGSID);
			rs->SetPipelineInputStateDesc(pisDescTrianglesID);
			rs->IASetPrimitiveTopology((M3D_PRIMITIVE_TOPOLOGY)D3D_PRIMITIVE_TOPOLOGY_POINTLIST);
			rs->CommitGraphicsRootSignatureAndPipelineState();
			rs->DrawInstanced((UINT)_points.size(), 1, 0, 0);
		}
	}
	catch (const ChipException& e) {
		AddException(e);
	}
}

