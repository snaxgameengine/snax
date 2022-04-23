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
#include "TexturePreviewWidget.h"
#include "M3DEngine/Engine.h"
#include "M3DEngine/RenderWindow.h"
#include "GraphicsChips/D3D12Formats.h"
#include "GraphicsChips/Graphics.h"
#include "GraphicsChips/D3D12RenderWindow.h"
#include "GraphicsChips/RenderSettings.h"
#include "GraphicsChips/D3DBlobUtil.h"
#include "GraphicsChips/ResourceStateTracker.h"
#include "TexturePreviewWidget.hlsl.h"

using namespace m3d;

struct CB
{
	XMFLOAT2 scale;
	XMFLOAT2 offset;
	XMFLOAT2 texSize;
	int arraySlice;
	int arraySize;
	int mipLevel;
	int pxTex;
	BOOL alpha;
	BYTE __padding[212]; // CB must be multiple of 256 bytes
};



TexturePreviewWidget::TexturePreviewWidget(QWidget *parent) : QWidget(parent) 
{
	ui.setupUi(this);

	HRESULT hr;

	Graphics *graphics = (Graphics*)engine->GetGraphics();

	{
		CD3DX12_DESCRIPTOR_RANGE ranges[2];
		ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
		ranges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 6, 0);

		CD3DX12_ROOT_PARAMETER rootParameters[2];
		rootParameters[0].InitAsDescriptorTable(1, ranges + 0, D3D12_SHADER_VISIBILITY_ALL);
		rootParameters[1].InitAsDescriptorTable(1, ranges + 1, D3D12_SHADER_VISIBILITY_PIXEL);

		CD3DX12_STATIC_SAMPLER_DESC sampler(0, D3D12_FILTER_MIN_MAG_MIP_LINEAR, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP);

		CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
		rootSignatureDesc.Init(_countof(rootParameters), rootParameters, 1, &sampler, D3D12_ROOT_SIGNATURE_FLAG_NONE);

		_rsID = graphics->GetPipelineStatePool()->RegisterRootSignatureDesc(rootSignatureDesc);

		// Describe and create the graphics pipeline state objects (PSO).
		PipelineStateDesc psDesc;
		psDesc.RasterizerState.MultisampleEnable = TRUE;
		psDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;/*D3D12_PIPELINE_STATE_FLAG_TOOL_DEBUG;*/
		psDesc.DepthStencilState.DepthEnable = FALSE;
		psDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;

		CreateD3DBlobFromCArray((const LPVOID)TEXTURE_PREVIEW_VS50_BYTECODE, sizeof(TEXTURE_PREVIEW_VS50_BYTECODE), &psDesc.VS);
		CreateD3DBlobFromCArray((const LPVOID)TEXTURE_PREVIEW_PS50_BYTECODE, sizeof(TEXTURE_PREVIEW_PS50_BYTECODE), &psDesc.PS);

		_psDescID = graphics->GetPipelineStatePool()->RegisterPipelineStateDesc(psDesc);

		PipelineInputStateDesc pisDesc;
		pisDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		_pisDescID = graphics->GetPipelineStatePool()->RegisterPipelineInputStateDesc(pisDesc);

		PipelineOutputStateDesc posDesc;
		posDesc.NumRenderTargets = 1;
		posDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
		_posDescID = graphics->GetPipelineStatePool()->RegisterPipelineOutputStateDesc(posDesc);

	}

	_texSize = XMFLOAT2(1.0f, 1.0f);
	_zoom = 0.0f;
	_pos = XMFLOAT2(0.0f, 0.0f);
	_texType = -1;
	_arraySize = 1;
	_fmt = DXGI_FORMAT_UNKNOWN;

	_firstResize = true;
}

TexturePreviewWidget::~TexturePreviewWidget()
{
}

void TexturePreviewWidget::SetTexture(ID3D12Resource *texture, DXGI_FORMAT fmtHint)
{
	HRESULT hr;

	_texture = nullptr;
	_texDesc = nullptr;
	_texType = -1;

	ui.comboBox_arraySlice->clear();
	ui.comboBox_mipSlice->clear();

	if (!texture)
		return;

	D3D12_RESOURCE_DESC desc = texture->GetDesc();

	if (desc.Flags & D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE)
		return;

	Graphics* graphics = (Graphics*)engine->GetGraphics();
	ID3D12Device* device = graphics->GetDevice();
	if (!device)
		return;

	auto fmtChk = [=](DXGI_FORMAT fmt) ->bool
	{
		D3D12_FEATURE_DATA_FORMAT_SUPPORT support{ fmt, D3D12_FORMAT_SUPPORT1_NONE, D3D12_FORMAT_SUPPORT2_NONE };
		HRESULT hr = graphics->GetDevice()->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT, &support, sizeof(support));
		if (FAILED(hr))
			return false;
		if (desc.SampleDesc.Count > 1) {
			if ((support.Support1 & D3D12_FORMAT_SUPPORT1_MULTISAMPLE_LOAD) == 0)
				return false;
		}
		else {
			if ((support.Support1 & D3D12_FORMAT_SUPPORT1_SHADER_SAMPLE) == 0)
				return false;
		}
		return true;
	};

	_fmt = desc.Format;
	_cm = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	if (desc.Flags & D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL)
		_cm = D3D12_ENCODE_SHADER_4_COMPONENT_MAPPING(D3D12_SHADER_COMPONENT_MAPPING_FROM_MEMORY_COMPONENT_0, D3D12_SHADER_COMPONENT_MAPPING_FROM_MEMORY_COMPONENT_0, D3D12_SHADER_COMPONENT_MAPPING_FROM_MEMORY_COMPONENT_0, D3D12_SHADER_COMPONENT_MAPPING_FROM_MEMORY_COMPONENT_3);

	if (!fmtChk(_fmt)) {
		if (fmtHint != DXGI_FORMAT_UNKNOWN && fmtChk(fmtHint) && dxgiformat::MakeTypeless(_fmt) == dxgiformat::MakeTypeless(fmtHint)) {
			_fmt = fmtHint;
		}
		else {
			if (fmtChk(dxgiformat::MakeTypelessUNORM(_fmt)))
				_fmt = dxgiformat::MakeTypelessUNORM(_fmt);
			else if (fmtChk(dxgiformat::MakeTypelessFLOAT(_fmt)))
				_fmt = dxgiformat::MakeTypelessFLOAT(_fmt);
			else {
				switch (_fmt)
				{
				case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
					_fmt = DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS;
					break;
				case DXGI_FORMAT_D32_FLOAT:
					_fmt = DXGI_FORMAT_R32_FLOAT;
					break;
				case DXGI_FORMAT_D24_UNORM_S8_UINT:
					_fmt = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
					break;
				case DXGI_FORMAT_D16_UNORM:
					_fmt = DXGI_FORMAT_R16_UNORM;
					break;
				default:
					return;
				}
			}
		}
	}

	_texture = texture;

	_texSize = XMFLOAT2(desc.Width, desc.Height);
	_arraySize = desc.DepthOrArraySize;

	static const Char *AS[6] = {"%1 (%2 +x)", "%1 (%2 -x)", "%1 (%2 +y)", "%1 (%2 -y)", "%1 (%2 +z)", "%1 (%2 -z)"};

	if (desc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE3D) {
		for (UINT i = 0; i < _arraySize; i++)
			ui.comboBox_arraySlice->addItem(QString::number(i));
	}
	else {
		if (_arraySize % 6 == 0)
			for (UINT i = 0; i < _arraySize; i++)
				ui.comboBox_arraySlice->addItem(QString(AS[i % 6]).arg(i).arg(i / 6));
		else
			for (UINT i = 0; i < _arraySize; i++)
				ui.comboBox_arraySlice->addItem(QString::number(i));
	}

	ui.comboBox_mipSlice->addItem("Auto");
	for (UINT i = 0; i < desc.MipLevels; i++)
		ui.comboBox_mipSlice->addItem(QString::number(i));

	// TODO: get srgb right!
//	ui.dxWidget->SetFormat(dxgiformat::IsSRGB(desc.Format) ? DXGI_FORMAT_R8G8B8A8_UNORM_SRGB : DXGI_FORMAT_R8G8B8A8_UNORM);

	_zoom = _calcFitZoom();
}

void TexturePreviewWidget::renderPreview()
{
	Graphics *graphics = (Graphics*)engine->GetGraphics();
	RenderSettings *rs = graphics->rs();

	D3D12_RESOURCE_DESC bbDesc = ui.dxWidget->GetRenderWindow()->GetBackBuffer()->GetDesc();

	D3D12_CPU_DESCRIPTOR_HANDLE rtv = ui.dxWidget->GetRenderWindow()->GetDescriptor(true).GetCPUHandle();

	HRESULT hr;

	rs->ClearRenderTargetView(rtv, &(const FLOAT&)XMFLOAT4(0.3f,0.3f,0.3f,1.0f), 0, nullptr);

	if (!_texture)
		return;

	rs->SetPipelineInputStateDesc(_pisDescID);
	rs->SetPipelineOutputStateDesc(_posDescID);
	rs->SetPipelineStateDesc(_psDescID);
	rs->SetGraphicsRootSignature(_rsID);
	rs->CommitGraphicsRootSignatureAndPipelineState();

	D3D12_RESOURCE_DESC desc = _texture->GetDesc();

	if (_texDesc == nullptr)
	{
		graphics->GetHeapManager()->GetDescriptorTable(6, &_texDesc);

		if (desc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE1D) {
			Descriptor& d = _texDesc->InitDescriptor(0, DescriptorType::SRV, TRUE);
			*d.srv = CD3DX12_SHADER_RESOURCE_VIEW_DESC(desc.DepthOrArraySize > 1 ? D3D12_SRV_DIMENSION_TEXTURE1DARRAY : D3D12_SRV_DIMENSION_TEXTURE1D, _fmt, _cm);
			if (desc.DepthOrArraySize > 1)
				d.srv->Texture1DArray.ArraySize = desc.DepthOrArraySize;
		}
		else { // Default SRV
			Descriptor &d = _texDesc->InitDescriptor(0, DescriptorType::SRV, TRUE);
			*d.srv = CD3DX12_SHADER_RESOURCE_VIEW_DESC(D3D12_SRV_DIMENSION_TEXTURE1D, DXGI_FORMAT_R8G8B8A8_UNORM);
		}

		if (desc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D && desc.SampleDesc.Count == 1) {
			Descriptor &d = _texDesc->InitDescriptor(1, DescriptorType::SRV, TRUE);
			d.resource = _texture;
			*d.srv = CD3DX12_SHADER_RESOURCE_VIEW_DESC(desc.DepthOrArraySize > 1 ? D3D12_SRV_DIMENSION_TEXTURE2DARRAY : D3D12_SRV_DIMENSION_TEXTURE2D, _fmt, _cm);
			if (desc.DepthOrArraySize > 1)
				d.srv->Texture2DArray.ArraySize = desc.DepthOrArraySize;
			_texType = 0;
		}
		else { // Default SRV
			Descriptor &d = _texDesc->InitDescriptor(1, DescriptorType::SRV, TRUE);
			*d.srv = CD3DX12_SHADER_RESOURCE_VIEW_DESC(D3D12_SRV_DIMENSION_TEXTURE2DARRAY, DXGI_FORMAT_R8G8B8A8_UNORM);
		}

		if (desc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D && desc.SampleDesc.Count == 2) {
			Descriptor &d = _texDesc->InitDescriptor(2, DescriptorType::SRV, TRUE);
			d.resource = _texture;
			*d.srv = CD3DX12_SHADER_RESOURCE_VIEW_DESC(desc.DepthOrArraySize > 1 ? D3D12_SRV_DIMENSION_TEXTURE2DMSARRAY : D3D12_SRV_DIMENSION_TEXTURE2DMS, _fmt, _cm);
			if (desc.DepthOrArraySize > 1)
				d.srv->Texture2DMSArray.ArraySize = desc.DepthOrArraySize;
			_texType = 2;
		}
		else { // Default SRV
			Descriptor &d = _texDesc->InitDescriptor(2, DescriptorType::SRV, TRUE);
			*d.srv = CD3DX12_SHADER_RESOURCE_VIEW_DESC(D3D12_SRV_DIMENSION_TEXTURE2DMS, DXGI_FORMAT_R8G8B8A8_UNORM);
		}

		if (desc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D && desc.SampleDesc.Count == 4) {
			Descriptor &d = _texDesc->InitDescriptor(3, DescriptorType::SRV, TRUE);
			d.resource = _texture;
			*d.srv = CD3DX12_SHADER_RESOURCE_VIEW_DESC(desc.DepthOrArraySize > 1 ? D3D12_SRV_DIMENSION_TEXTURE2DMSARRAY : D3D12_SRV_DIMENSION_TEXTURE2DMS, _fmt, _cm);
			if (desc.DepthOrArraySize > 1)
				d.srv->Texture2DMSArray.ArraySize = desc.DepthOrArraySize;
			_texType = 3;
		}
		else { // Default SRV
			Descriptor &d = _texDesc->InitDescriptor(3, DescriptorType::SRV, TRUE);
			*d.srv = CD3DX12_SHADER_RESOURCE_VIEW_DESC(D3D12_SRV_DIMENSION_TEXTURE2DMS, DXGI_FORMAT_R8G8B8A8_UNORM);
		}

		if (desc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D && desc.SampleDesc.Count == 8) {
			Descriptor &d = _texDesc->InitDescriptor(4, DescriptorType::SRV, TRUE);
			d.resource = _texture;
			*d.srv = CD3DX12_SHADER_RESOURCE_VIEW_DESC(desc.DepthOrArraySize > 1 ? D3D12_SRV_DIMENSION_TEXTURE2DMSARRAY : D3D12_SRV_DIMENSION_TEXTURE2DMS, _fmt, _cm);
			if (desc.DepthOrArraySize > 1)
				d.srv->Texture2DMSArray.ArraySize = desc.DepthOrArraySize;
			_texType = 4;
		}
		else { // Default SRV
			Descriptor &d = _texDesc->InitDescriptor(4, DescriptorType::SRV, TRUE);
			*d.srv = CD3DX12_SHADER_RESOURCE_VIEW_DESC(D3D12_SRV_DIMENSION_TEXTURE2DMS, DXGI_FORMAT_R8G8B8A8_UNORM);
		}

		if (desc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE3D) {
			Descriptor &d = _texDesc->InitDescriptor(5, DescriptorType::SRV, TRUE);
			d.resource = _texture;
			*d.srv = CD3DX12_SHADER_RESOURCE_VIEW_DESC(D3D12_SRV_DIMENSION_TEXTURE3D, _fmt, _cm);
			_texType = 1;
		}
		else { // Default SRV
			Descriptor &d = _texDesc->InitDescriptor(5, DescriptorType::SRV, TRUE);
			*d.srv = CD3DX12_SHADER_RESOURCE_VIEW_DESC(D3D12_SRV_DIMENSION_TEXTURE3D, DXGI_FORMAT_R8G8B8A8_UNORM);
		}

		_texDesc->CreateDescriptors();
	}

	// Create and set constant buffer.
	SDescriptorTable r = nullptr;
	{
		CB *cb = nullptr;
		graphics->GetHeapManager()->GetDescriptorTable(1, &r);
		Descriptor &d = r->InitDescriptor(0, DescriptorType::CBV);
		d.cbv->SizeInBytes = sizeof(CB);
		hr = graphics->GetUploadHeap()->Allocate(d.cbv->SizeInBytes, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT, (BYTE**)&cb, &d.cbv->BufferLocation);

		cb->scale = XMFLOAT2(_texSize.x * _zoom / bbDesc.Width, _texSize.y * _zoom / bbDesc.Height);
		cb->offset = _pos;
		cb->texSize = _texSize;
		cb->arraySize = _arraySize;
		cb->arraySlice = ui.comboBox_arraySlice->currentIndex();
		cb->mipLevel = ui.comboBox_mipSlice->currentIndex();
		cb->pxTex = _texType;
		cb->alpha = ui.checkBox_alphaBlend->isChecked() ? ~0 : 0;

		r->CreateDescriptors();
		rs->SetGraphicsRootDescriptorTable(0, r);
	}

	D3D12_VIEWPORT vp = {0.0f, 0.0f, (FLOAT)bbDesc.Width, (FLOAT)bbDesc.Height, 0.0f, 1.0f};
	D3D12_RECT sr = {0, 0, (LONG)bbDesc.Width, (LONG)bbDesc.Height};
	
	rs->RSSetViewportsAndScissorRects(1, &vp, &sr);
	rs->SetGraphicsRootDescriptorTable(1, _texDesc);
	rs->OMSetRenderTargets(1, &rtv, nullptr);
	rs->CommitResourceBarriers();
	rs->IASetPrimitiveTopology(M3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	rs->DrawInstanced(4, 1, 0, 0);
}

void TexturePreviewWidget::onPreviewZoom(int delta, int x, int y)
{
	QSize r = ui.dxWidget->size();
	float32 dx = (float32)(x - r.width() * 0.5) / r.width();
	float32 dy = (float32)(y - r.height() * 0.5) / r.height();
	float32 zoom = _zoom;
	_zoom += (float32)delta/120.0f/20.0f*_zoom;
	float32 fitZoom = _calcFitZoom();
	_zoom = std::min(std::max(_zoom, std::min(fitZoom, 0.01f)), std::max(fitZoom, 20.0f)); // zoom range: [0.01x-20x]
	_pos.x += (_pos.x - dx*2.0f) * (_zoom - zoom)/zoom;
	_pos.y += (_pos.y + dy*2.0f) * (_zoom - zoom)/zoom;
	onPreviewMove(0, 0); // Limit move
}

float32 TexturePreviewWidget::_calcFitZoom()
{
	QSize r = ui.dxWidget->size();
	return std::min(std::min((float32)r.width() / _texSize.x, (float32)r.height() / _texSize.y), 1.0f); // Shrinks texture, but not magnifies it.
}

void TexturePreviewWidget::onPreviewMove(int dx,int dy)
{
	QSize r = ui.dxWidget->size();

	XMFLOAT2 scale(_texSize.x * _zoom / r.width(), _texSize.y * _zoom / r.height());

	_pos.x += (float32)dx / r.width() * 2.0f;
	_pos.y -= (float32)dy / r.height() * 2.0f;

	_pos.x = std::max(1.0f - std::max(1.0f, scale.x), std::min(std::max(scale.x, 1.0f) - 1.0f, _pos.x));
	_pos.y = std::max(1.0f - std::max(1.0f, scale.y), std::min(std::max(scale.y, 1.0f) - 1.0f, _pos.y));
}

void TexturePreviewWidget::zoomToFit()
{
	_zoom = _calcFitZoom();
	onPreviewZoom(0,0,0);
}

void TexturePreviewWidget::zoomToActual()
{
	_zoom = 1.0f;
	onPreviewZoom(0,0,0);
}

void TexturePreviewWidget::onPreviewResize()
{
	if (_firstResize)
		_zoom = _calcFitZoom();
	_firstResize = false;
	onPreviewZoom(0,0,0);
}

void TexturePreviewWidget::update()
{
	ui.dxWidget->invalidateGraphics();
}

void TexturePreviewWidget::DestroyDeviceObject()
{
	_texture = nullptr;
	_texDesc = nullptr;
	ui.dxWidget->DestroyDeviceObject();
}
