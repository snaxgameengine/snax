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

#include "stdafx.h"
#include "Geometry3DView.h"
#include "M3DEngine/Engine.h"
#include "GraphicsChips/D3D12RenderWindow.h"
#include "Geometry3DView.hlsl.h"
#include "GraphicsChips/RenderSettings.h"
//#include "InputLayouts.h"
#include <qevent.h>
#include "GraphicsChips/Utils.h"
#include "GraphicsChips/GraphicsException.h"
#include "GraphicsChips/D3DBlobUtil.h"
#include "GraphicsChips/Geometry.h"

using namespace m3d;




Geometry3DView::Geometry3DView(QWidget * parent, Qt::WindowFlags f) : D3DWidget(parent, f)
{
	_geometry = nullptr;
	connect(this, &Geometry3DView::render, this, &Geometry3DView::_render);
	connect(this, &Geometry3DView::resize, this, &Geometry3DView::_resize);
	_zoom = 1.3f;
	_yaw = 0.785f;
	_pitch = 0.5f;
	_pos = XMFLOAT3(0.0f, 0.0f, 0.0f);
	_sampleDesc.Count = 4;
	_sampleDesc.Quality = 0;
	_backFaceCulling = true;
	_scaleWholeObject = true;
	_dsv = nullptr;
}

Geometry3DView::~Geometry3DView()
{
int gg = 5;
}

void Geometry3DView::init(Geometry*d)
{
	_geometry = d;

}

void Geometry3DView::wheelEvent(QWheelEvent *event)
{
	if (event->angleDelta().y() != 0) {
		_zoom -= (float32)event->angleDelta().y() / 120.0f / 20.0f * _zoom;
		_zoom = std::min(std::max(_zoom, 0.1f), 10.0f);
	}
} 

void Geometry3DView::mousePressEvent(QMouseEvent *event)
{
	_x = event->x();
	_y = event->y();
}

void Geometry3DView::mouseMoveEvent(QMouseEvent *event)
{
	if (event->buttons().testFlag(Qt::MouseButton::MiddleButton)) {
		float32 dx = 0.005f*_zoom*(event->x() - _x);
		float32 dy = -0.005f*_zoom*(event->y() - _y);
		XMFLOAT3 t;
		XMStoreFloat3(&t, XMVector3Transform(XMVectorSet(dx, dy, 0.0f, 0.0f), XMMatrixRotationRollPitchYaw(_pitch, _yaw, 0.0f)));
		_pos.x += t.x;
		_pos.y += t.y;
		_pos.z += t.z;
	}
	if (event->buttons() & Qt::RightButton) {
		float32 dx = 0.01f*(event->x() - _x);
		float32 dy = 0.01f*(event->y() - _y);
		_yaw += dx;
		_pitch += dy;
		_pitch = std::min(std::max(_pitch, -1.57f), 1.57f);
	}
	_x = event->x();
	_y = event->y();
}

void Geometry3DView::_render()
{
	_render3D();
}

HRESULT Geometry3DView::_render3D()
{
	HRESULT hr = S_OK;
	
	Graphics *graphics = _geometry->graphics();
	RenderSettings *rs = graphics->rs();
	ID3D12Device *device = graphics->GetDevice();

	static RootSignatureID rsID = 0;
	static PipelineStateDescID psDescID = 0;
	static PipelineStateDescID psDescGSID = 0;

	if ((psDescID | psDescGSID) == 0)
	{
		CD3DX12_ROOT_PARAMETER rootParameters[1];
		rootParameters[0].InitAsConstantBufferView(0);

		CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
		rootSignatureDesc.Init(_countof(rootParameters), rootParameters, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

		rsID = graphics->GetPipelineStatePool()->RegisterRootSignatureDesc(rootSignatureDesc);

		// Describe and create the graphics pipeline state objects (PSO).
		PipelineStateDesc psDesc;
		psDesc.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;
		psDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
		psDesc.RasterizerState.MultisampleEnable = TRUE;
		psDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;//D3D12_PIPELINE_STATE_FLAG_TOOL_DEBUG;

		CreateD3DBlobFromCArray((const LPVOID)DRAWABLE3DVIEW_vs0, sizeof(DRAWABLE3DVIEW_vs0), &psDesc.VS);
		CreateD3DBlobFromCArray((const LPVOID)DRAWABLE3DVIEW_ps0, sizeof(DRAWABLE3DVIEW_ps0), &psDesc.PS);
		psDescID = graphics->GetPipelineStatePool()->RegisterPipelineStateDesc(psDesc);
	}

	const Descriptor &rtvDesc = GetRenderWindow()->GetDescriptor();

	UINT w, h;
	CalculateRenderTargetSize(w, h, rtvDesc.resource, rtvDesc.rtv);

//	context->ClearState();

	float32 clear[4] = {0.3f, 0.3f, 0.3f, 1.0f};
	rs->ClearRenderTargetView(rtvDesc.GetCPUHandle(), clear, 0, nullptr);

	if (!_geometry)
		return hr;

	// rs->SetPipelineInputStateDesc(...);
	// rs->IASetVertexBuffers(...);
	// rs->IASetIndexBuffer(...);
	_geometry->Prepare();

	const PipelineInputStateDesc *descs = graphics->GetPipelineStatePool()->GetPipelineInputStateDesc(_geometry->GetPipelineInputStateDescID());
	
	bool hasPosition = descs->InputLayout.HasElement("POSITION", 0);
	bool hasNormal = descs->InputLayout.HasElement("NORMAL", 0);
	bool hasTangent = descs->InputLayout.HasElement("TANGENT", 0);
	bool hasBinormal = descs->InputLayout.HasElement("BINORMAL", 0);
	bool hasTexcoord[4] = { descs->InputLayout.HasElement("TEXCOORD", 0), descs->InputLayout.HasElement("TEXCOORD", 1), descs->InputLayout.HasElement("TEXCOORD", 2), descs->InputLayout.HasElement("TEXCOORD", 3) };

	if (!hasPosition)
		return hr;

	if (!_dsv) {
		D3D12_RESOURCE_DESC tDesc = 
		{
			D3D12_RESOURCE_DIMENSION_TEXTURE2D, 
			0, 
			w,
			h,
			1,
			1,
			DXGI_FORMAT_D24_UNORM_S8_UINT,
			{1, 0},
			D3D12_TEXTURE_LAYOUT_UNKNOWN,
			D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL
		};

		D3D12_CLEAR_VALUE optimizedClearValue = { tDesc.Format, {1.0f, 0xFF } };

		SID3D12Resource resource;
		CD3DX12_HEAP_PROPERTIES hp(D3D12_HEAP_TYPE_DEFAULT);
		if (FAILED(device->CreateCommittedResource(&hp, D3D12_HEAP_FLAG_NONE, &tDesc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &optimizedClearValue, IID_PPV_ARGS(&resource))))
			throw GraphicsException(_geometry, MTEXT(""));

		rs->GetDSVHeapManager()->GetDescriptorTable(1, &_dsv);
		Descriptor &d = _dsv->InitDescriptor(0, DescriptorType::DSV, FALSE);
		d.resource = resource;

		_dsv->CreateDescriptors();
	}

	D3D12_VIEWPORT vp = { 0.0f, 0.0f, (FLOAT)w, (FLOAT)h, 0.0f, 1.0f };
	D3D12_RECT sr = { 0, 0, (LONG)w, (LONG)h };

	rs->ClearDepthStencilView(_dsv->GetCPUHandle(), D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
	D3D12_CPU_DESCRIPTOR_HANDLE rh = rtvDesc.GetCPUHandle();
	D3D12_CPU_DESCRIPTOR_HANDLE dh = _dsv->GetCPUHandle();
	rs->OMSetRenderTargets(1, &rh, &dh);
	rs->RSSetViewportsAndScissorRects(1, &vp, &sr);
	rs->SetGraphicsRootSignature(rsID);
	rs->SetPipelineStateDesc(psDescID);
	rs->CommitGraphicsRootSignature();

	bool _wireframe = false;
	bool _cullBack = true;

	/*
	if (_wireframe) {
		if (_cullBack) {
			if (!_rsWireCull) {
				D3D11_RASTERIZER_DESC rDesc = CD3D11_RASTERIZER_DESC(CD3D11_DEFAULT());
				rDesc.FillMode = D3D11_FILL_WIREFRAME;
				rDesc.CullMode = D3D11_CULL_BACK;
				V_RETURN(device->CreateRasterizerState(&rDesc, &_rsWireCull));
			}
			context->RSSetState(_rsWireCull);
		}
		else {
			if (!_rsWireCull) {
				D3D11_RASTERIZER_DESC rDesc = CD3D11_RASTERIZER_DESC(CD3D11_DEFAULT());
				rDesc.FillMode = D3D11_FILL_WIREFRAME;
				V_RETURN(device->CreateRasterizerState(&rDesc, &_rsWire));
			}
			context->RSSetState(_rsWire);
		}
	}
	else {
		if (_cullBack) {
			if (!_rsWireCull) {
				D3D11_RASTERIZER_DESC rDesc = CD3D11_RASTERIZER_DESC(CD3D11_DEFAULT());
				rDesc.CullMode = D3D11_CULL_BACK;
				V_RETURN(device->CreateRasterizerState(&rDesc, &_rsCull));
			}
			context->RSSetState(_rsCull);
		}
		else {
			context->RSSetState(nullptr);
		}
	}

	if (hasNormal) {
		if (!_vs1) {
			V_RETURN(device->CreateVertexShader(DRAWABLE3DVIEW_vs1, ARRAYSIZE(DRAWABLE3DVIEW_vs1), nullptr, &_vs1));
			_is1 = g->il().RegisterShaderSignature(InputSignature(DRAWABLE3DVIEW_vs1, ARRAYSIZE(DRAWABLE3DVIEW_vs1)));
		}
		if (!_ps1) {
			V_RETURN(device->CreatePixelShader(DRAWABLE3DVIEW_ps1, ARRAYSIZE(DRAWABLE3DVIEW_ps1), nullptr, &_ps1));
		}
		context->VSSetShader(_vs1, nullptr, 0);
		context->PSSetShader(_ps1, nullptr, 0);
		SID3D11InputLayout il;
		V_RETURN(g->il().GetInputLayout(_drawable->GetInputElementDescsID(), _is1, 0, &il));
		context->IASetInputLayout(il);
	}
	else {
		if (!_vs0) {
			V_RETURN(device->CreateVertexShader(DRAWABLE3DVIEW_vs0, ARRAYSIZE(DRAWABLE3DVIEW_vs0), nullptr, &_vs0));
			_is0 = g->il().RegisterShaderSignature(InputSignature(DRAWABLE3DVIEW_vs0, ARRAYSIZE(DRAWABLE3DVIEW_vs0)));
		}
		if (!_ps0) {
			V_RETURN(device->CreatePixelShader(DRAWABLE3DVIEW_ps0, ARRAYSIZE(DRAWABLE3DVIEW_ps0), nullptr, &_ps0));
		}
		context->VSSetShader(_vs0, nullptr, 0);
		context->PSSetShader(_ps0, nullptr, 0);
		SID3D11InputLayout il;
		V_RETURN(g->il().GetInputLayout(_drawable->GetInputElementDescsID(), _is0, 0, &il));
		context->IASetInputLayout(il);
	}
	*/




	const GeometrySubsetList &ssl = _geometry->GetSubsets();

	AxisAlignedBox aabb;
	aabb.SetNull();
	if (_scaleWholeObject) {
		for (size_t i = 0; i < ssl.size(); i++)
			aabb += ssl[i].boundingBox;
	}
	else {
		for (const auto &i : _subsets) {
			if (i >= ssl.size())
				break;
			aabb += ssl[i].boundingBox;
		}
	}

	XMFLOAT3 sc(aabb.GetMax().x-aabb.GetMin().x, aabb.GetMax().y-aabb.GetMin().y, aabb.GetMax().z-aabb.GetMin().z);
	float32 s = std::max(std::max(sc.x, sc.y), sc.z);

	XMMATRIX scale = XMMatrixScaling(1.0f/s, 1.0f/s, 1.0f/s);
	XMMATRIX translate = XMMatrixTranslation(-(aabb.GetMax().x+aabb.GetMin().x)*0.5f,-(aabb.GetMax().y+aabb.GetMin().y)*0.5f,-(aabb.GetMax().z+aabb.GetMin().z)*0.5f);
	XMMATRIX world = translate * scale;
	XMMATRIX view =  XMMatrixInverse(nullptr, XMMatrixTranslation(0.0f, 0.0f, -_zoom) * XMMatrixRotationRollPitchYaw(_pitch, _yaw, 0.0f) * XMMatrixTranslation(_pos.x, _pos.y, _pos.z));
	XMMATRIX projection = XMMatrixPerspectiveFovLH(1.0f, (float32)w/h, 0.01f, 100.0f);

	// Create and set constant buffer.
	{
		struct CB
		{
			XMFLOAT4X4 wvp;
			XMFLOAT4X4 wv;
			XMFLOAT4 color;
		};

		CB *cb = nullptr;
		D3D12_GPU_VIRTUAL_ADDRESS vAddress;
		hr = graphics->GetUploadHeap()->Allocate(sizeof(CB), D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT, (BYTE**)&cb, &vAddress);
		if (FAILED(hr))
			throw GraphicsException(_geometry, MTEXT("Failed to allocate memory from upload heap."));

		XMStoreFloat4x4(&cb->wvp, XMMatrixTranspose(world * view * projection));
		XMStoreFloat4x4(&cb->wv, XMMatrixTranspose(world * view));
		cb->color = XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f);

		rs->SetGraphicsRootConstantBufferView(0, vAddress);
	}

//	rs->SetPipelineInputStateDesc(pisDescTrianglesID);
//	rs->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
//	rs->CommitGraphicsRootSignatureAndPipelineState();

	for (const auto &i : _subsets) {
		if (i >= ssl.size())
			break;
		const GeometrySubset &ss = ssl[i];
		rs->IASetPrimitiveTopology(ss.pt);
		rs->CommitGraphicsRootSignatureAndPipelineState();
		if (_geometry->GetAPI() == DRAW)
			rs->DrawInstanced(ss.count, 1, ss.startLocation + ss.baseVertexLocation, 0);
		else
			rs->DrawIndexedInstanced(ss.count, 1, ss.startLocation, ss.baseVertexLocation, 0);
	}

	
	return S_OK;
}

void Geometry3DView::_resize()
{
	_dsv = nullptr;
}

void Geometry3DView::DestroyDeviceObject()
{
	_dsv = nullptr;
}

void Geometry3DView::_draw(const GeometrySubsetList &ssl/*, ID3D11DeviceContext *context*/)
{
/*	for (Set<uint32>::cNode n = _subsets.front(); n.valid(); n++) {
		uint32 i = n.get();
		if (i >= ssl.size())
			break;
		context->IASetPrimitiveTopology(ssl[i].pt);
		if (_drawable->GetAPI() == DRAW)
			context->Draw(ssl[i].count, ssl[i].startLocation + ssl[i].baseVertexLocation);
		else
			context->DrawIndexed(ssl[i].count, ssl[i].startLocation, ssl[i].baseVertexLocation);
	}*/
}



/*	bool _wireOverlay = true;

	if (_wireOverlay) {
		cb.color = XMFLOAT4(0.5,0.5,0,1);
		context->UpdateSubresource(_cb, 0, nullptr, &cb, 0, 0);
		context->RSSetState(_rsWire);
	//	context->OMSetDepthStencilState(_dssWireOverlay, 0);		

		_draw(ssl, context);
	}

	if (!_rsFilled) {
		D3D11_RASTERIZER_DESC rDesc = CD3D11_RASTERIZER_DESC(CD3D11_DEFAULT());
		V_RETURN(device->CreateRasterizerState(&rDesc, &_rsFilled));
	}
	if (!_rsWire) {
		D3D11_RASTERIZER_DESC rDesc = CD3D11_RASTERIZER_DESC(CD3D11_DEFAULT());
		rDesc.AntialiasedLineEnable = FALSE;
		rDesc.FillMode = D3D11_FILL_WIREFRAME;
		rDesc.DepthBias = -100;
		rDesc.SlopeScaledDepthBias = -0.1f;
		V_RETURN(device->CreateRasterizerState(&rDesc, &_rsWire));
	}
	if (!_rsWireNoCull) {
		D3D11_RASTERIZER_DESC rDesc = CD3D11_RASTERIZER_DESC(CD3D11_DEFAULT());
		rDesc.AntialiasedLineEnable = FALSE;
		rDesc.FillMode = D3D11_FILL_WIREFRAME;
		rDesc.CullMode = D3D11_CULL_NONE;
		V_RETURN(device->CreateRasterizerState(&rDesc, &_rsWireNoCull));
	}
	if (!_dssWireOverlay) {
		D3D11_DEPTH_STENCIL_DESC dssDesc = CD3D11_DEPTH_STENCIL_DESC(CD3D11_DEFAULT());
		dssDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
		V_RETURN(device->CreateDepthStencilState(&dssDesc, &_dssWireOverlay));
	}
HRESULT _setRasterizerState(uint32 n, ID3D11DeviceContext *context)
{
	HRESULT hr;
	SID3D11RasterizerState _rs[4];
	if (!_rs[n]) {
		D3D11_RASTERIZER_DESC rDesc = CD3D11_RASTERIZER_DESC(CD3D11_DEFAULT());
		switch (n)
		{
		case 0:
			rDesc.FillMode = D3D11_FILL_SOLID;
			rDesc.CullMode = D3D11_CULL_BACK;
			break;
		case 1:
			rDesc.FillMode = D3D11_FILL_WIREFRAME;
			rDesc.CullMode = D3D11_CULL_BACK;
			break;
		case 2:
			rDesc.FillMode = D3D11_FILL_SOLID;
			rDesc.CullMode = D3D11_CULL_NONE;
			break;
		case 3:
			rDesc.FillMode = D3D11_FILL_WIREFRAME;
			rDesc.CullMode = D3D11_CULL_NONE;
			break;
		}
		SID3D11Device device;
		context->GetDevice(&device);
		V_RETURN(device->CreateRasterizerState(&rDesc, &_rs[n]));
	}
	context->RSSetState(_rs[n]);
	return S_OK;
}

HRESULT _setShaders(uint32 n, D3DGraphics *g, ID3D11DeviceContext *context, ID3D11Device *device)
{
	HRESULT hr;
	SID3D11VertexShader _vs[2];
	SID3D11PixelShader _ps[2];
	InputSignatureID _isID[2];
	static const uint8 *VS[2] = {DRAWABLE3DVIEW_vs0, DRAWABLE3DVIEW_vs1};
	static const uint8 *PS[2] = {DRAWABLE3DVIEW_ps0, DRAWABLE3DVIEW_ps1};
	static const SIZE_T VSS[2] = {ARRAYSIZE(DRAWABLE3DVIEW_vs0), ARRAYSIZE(DRAWABLE3DVIEW_vs1)};
	static const SIZE_T PSS[2] = {ARRAYSIZE(DRAWABLE3DVIEW_ps0), ARRAYSIZE(DRAWABLE3DVIEW_ps1)};
	if (!_vs[n]) {
		V_RETURN(device->CreateVertexShader(VS[n], VSS[n], nullptr, &_vs[n]));
		_isID[n] = g->il().RegisterShaderSignature(InputSignature(VS[n], VSS[n]));
	}
	if (!_ps[n]) {
		V_RETURN(device->CreatePixelShader(PS[n], PSS[n], nullptr, &_ps[n]));
	}
	SID3D11InputLayout il;
	V_RETURN(g->il().GetInputLayout(_drawable->GetInputElementDescsID(), _isID[n], 0, &il));
	context->VSSetShader(_vs[n], nullptr, 0);
	context->PSSetShader(_ps[n], nullptr, 0);
	context->IASetInputLayout(il);
	return S_OK;
}

HRESULT Drawable3DView::_renderFlat(D3DGraphics *g, ID3D11DeviceContext *context, ID3D11Device *device)
{
	HRESULT hr;

	SID3D11VertexShader _vsFlat;
	SID3D11PixelShader _psFlat;
	InputSignatureID _isFlatID;
	SID3D11RasterizerState _rs;

	if (!_vsFlat) {
		V_RETURN(device->CreateVertexShader(DRAWABLE3DVIEW_vs0, ARRAYSIZE(DRAWABLE3DVIEW_vs0), nullptr, &_vsFlat));
		_isFlatID = g->il().RegisterShaderSignature(InputSignature(DRAWABLE3DVIEW_vs0, ARRAYSIZE(DRAWABLE3DVIEW_vs0)));
	}
	if (!_psFlat) {
		V_RETURN(device->CreatePixelShader(DRAWABLE3DVIEW_ps0, ARRAYSIZE(DRAWABLE3DVIEW_ps0), nullptr, &_psFlat));
	}

	SID3D11InputLayout il;
	V_RETURN(g->il().GetInputLayout(_drawable->GetInputElementDescsID(), _isFlatID, 0, &il));

	context->VSSetShader(_vsFlat, nullptr, 0);
	context->PSSetShader(_psFlat, nullptr, 0);
	context->IASetInputLayout(il);


}

*/