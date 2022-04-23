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
#include "TextWriter.h"
#include "TextWriter.hlsl.h"
#include "D3DBlobUtil.h"
#include "RenderSettings.h"

using namespace m3d;


RootSignatureID TextWriter::_rsID = 0;
PipelineStateDescID TextWriter::_psDesc1ID = 0;
PipelineStateDescID TextWriter::_psDesc2ID = 0;
PipelineStateDescID TextWriter::_pisDescID = 0;


TextWriter::TextWriter()
{
	_vertexBufferCapacity = 0;

	_font = nullptr;

	_color = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	_offset = XMFLOAT2(0.0f, 0.0f);
	_transform = XMFLOAT4X4(1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1);
	_useTransform = false;
}

TextWriter::~TextWriter()
{
}

HRESULT TextWriter::Init(Graphics *graphics, Font *font)
{
	SetFont(font);

	_vertexBufferCapacity = 512;

	if (_rsID == 0)
	{
		CD3DX12_DESCRIPTOR_RANGE ranges[1];
		ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 2, 0);

		CD3DX12_ROOT_PARAMETER rootParameters[2];

		rootParameters[0].InitAsConstantBufferView(0);
		rootParameters[1].InitAsDescriptorTable(1, ranges);

		CD3DX12_STATIC_SAMPLER_DESC samplerDesc(0);

		CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
		rootSignatureDesc.Init(_countof(rootParameters), rootParameters, 1, &samplerDesc, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

		_rsID = graphics->GetPipelineStatePool()->RegisterRootSignatureDesc(rootSignatureDesc);
	}

	if (_psDesc1ID == 0 || _psDesc2ID == 0)
	{
		PipelineStateDesc psoDesc;
		psoDesc.RasterizerState.MultisampleEnable = FALSE;
		psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
		psoDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;//D3D12_PIPELINE_STATE_FLAG_TOOL_DEBUG;
		psoDesc.DepthStencilState.DepthEnable = FALSE;
		psoDesc.BlendState.RenderTarget[0].BlendEnable = TRUE;
		psoDesc.BlendState.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
		psoDesc.BlendState.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;

		CreateD3DBlobFromCArray((const LPVOID)TEXTWRITER_VS_40_BYTECODE, sizeof(TEXTWRITER_VS_40_BYTECODE), &psoDesc.VS);
		CreateD3DBlobFromCArray((const LPVOID)TEXTWRITER_PS_40_BYTECODE, sizeof(TEXTWRITER_PS_40_BYTECODE), &psoDesc.PS);

		CreateD3DBlobFromCArray((const LPVOID)TEXTWRITER_GS1_40_BYTECODE, sizeof(TEXTWRITER_GS1_40_BYTECODE), &psoDesc.GS);
		_psDesc1ID = graphics->GetPipelineStatePool()->RegisterPipelineStateDesc(psoDesc);

		CreateD3DBlobFromCArray((const LPVOID)TEXTWRITER_GS2_40_BYTECODE, sizeof(TEXTWRITER_GS2_40_BYTECODE), &psoDesc.GS);
		_psDesc2ID = graphics->GetPipelineStatePool()->RegisterPipelineStateDesc(psoDesc);
	}

	if (_pisDescID == 0)
	{
		static const D3D12_INPUT_ELEMENT_DESC d[2] = {
			{"POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
			{"TEXCOORD", 0, DXGI_FORMAT_R16_UINT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}};

		PipelineInputStateDesc psoInDesc;
		psoInDesc.InputLayout = D3D12_INPUT_LAYOUT_DESC { d, _countof(d) };
		psoInDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
		_pisDescID = graphics->GetPipelineStatePool()->RegisterPipelineInputStateDesc(psoInDesc);
	}

	return S_OK;
}

void TextWriter::OnDestroyDevice()
{
	_vertexBufferCapacity = 0;
	_font = nullptr;
	_vertices.clear();
}

void TextWriter::SetFont(Font *font)
{
	_font = font;
	_vertices.clear();
}

void TextWriter::Write(XMFLOAT2 position, HAlign hAlign, VAlign vAlign, bool multiline, XMFLOAT4 color, String text)
{
	_color = color;
	return Write(position, hAlign, vAlign, multiline, text);
}

void TextWriter::Write(XMFLOAT2 position, HAlign hAlign, VAlign vAlign, bool multiline, String text)
{
	XMFLOAT2 p = position;

	// When using tech2 (freely transformed) we scale everything by glyph height.
	float S = 1.0;//_tech2 ? (/*1.0f */ _font->GetGlyphHeight()) : 1.0f;

	uint32 nLines = 1;
	
	size_t first = _vertices.size(), firstInLine = _vertices.size();

	auto hAlignFunc = [&]() 
	{
		float delta = 0.0f;
		if (hAlign == HMID)
			delta = floor((p.x - position.x) * 0.5f + 0.5f);
		else if (hAlign == RIGHT)
			delta = floor((p.x - position.x) + 0.5f);
		if (delta != 0.0f)
			while (firstInLine < _vertices.size())
				_vertices[firstInLine++].x -= delta;
	};

	for (uint32 i = 0; i < text.length(); i++) {
		if (multiline && text[i] == '\n') {
			hAlignFunc();
			p.x = position.x;
			p.y += S * _font->GetGlyphHeight();
			nLines++;
			continue;
		}
		const Font::Glyph *g = _font->GetGlyph(text[i]);
		if (g == nullptr)
			continue;

		Vertex v = {p.x + S * g->movex + _offset.x, p.y + _offset.y, (uint16)g->glyphIndex};
		_vertices.push_back(v);

		p.x += S * g->advance;
	}

	hAlignFunc();

	float delta = 0.0f;
	if (vAlign == VMID)
		delta = floor((_font->GetGlyphHeight() + p.y - position.y) * 0.5f + 0.5f);
	else if (vAlign == BOTTOM)
		delta = floor(_font->GetGlyphHeight() + p.y - position.y + 0.5f);
	if (delta != 0.0f)
		while (first < _vertices.size())
			_vertices[first++].y -= delta;
}

HRESULT TextWriter::Write(Graphics *graphics, XMFLOAT2 position, HAlign hAlign, VAlign vAlign, bool multiline, String text)
{
	Write(position, hAlign, vAlign, multiline, text);
	return Flush(graphics);
}

HRESULT TextWriter::Write(Graphics *graphics, XMFLOAT2 position, HAlign hAlign, VAlign vAlign, bool multiline, XMFLOAT4 color, String text)
{
	Write(position, hAlign, vAlign, multiline, color, text);
	return Flush(graphics);
}

HRESULT TextWriter::Flush(Graphics *graphics)
{
	if (_vertices.size() == 0)
		return S_OK;

	HRESULT hr;

	V_RETURN(_font->Update(graphics));

	RenderSettings *rs = graphics->rs();

	rs->SetGraphicsRootSignature(_rsID);
	rs->SetPipelineStateDesc(_psDesc2ID);// : _psDesc1ID);
	rs->SetPipelineInputStateDesc(_pisDescID);
	rs->CommitGraphicsRootSignatureAndPipelineState();

	// Create and set vertex buffer.
	{
		BYTE *vbData = nullptr;
		D3D12_VERTEX_BUFFER_VIEW vbView;
		vbView.SizeInBytes = UINT(sizeof(Vertex) * _vertices.size());
		vbView.StrideInBytes = sizeof(Vertex);
		V_RETURN(graphics->GetUploadHeap()->Allocate(vbView.SizeInBytes, 512, &vbData, &vbView.BufferLocation));
		std::memcpy(vbData, &_vertices.front(), vbView.SizeInBytes);
		rs->IASetVertexBuffers(0, 1, &vbView);
	}

	UINT numVP = rs->GetNumViewportsAndScissorRects();
	const D3D12_VIEWPORT *vp = rs->GetViewports();
	if (numVP == 0)
		return E_FAIL;

	// Create and set constant buffer.
	{
		CB *cb = nullptr;
		D3D12_GPU_VIRTUAL_ADDRESS vAddress;
		V_RETURN(graphics->GetUploadHeap()->Allocate(sizeof(CB), D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT, (BYTE**)&cb, &vAddress));

		cb->fontColor = _color;
		cb->props = XMFLOAT4(/*2.0f / vp->Width, -2.0f / vp->Height*/0.0f, 0.0f, float(_font->GetGlyphHeight()) / _font->GetTextureHeight(), float(_font->GetGlyphHeight()));
		if (_useTransform) {
			XMStoreFloat4x4(&cb->wvp, XMMatrixTranspose(XMLoadFloat4x4(&_transform)));
		}
		else {
			XMFLOAT4X4 m(2.0f / vp->Width, 0.0f, 0.0f, 0.0f, 0.0f, -2.0f / vp->Height, 0.0, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, -1.0f, 1.0f, 0.0f, 1.0f);
			XMStoreFloat4x4(&cb->wvp, XMMatrixTranspose(XMLoadFloat4x4(&m)));
		}

		rs->SetGraphicsRootConstantBufferView(0, vAddress);
	}
	
	rs->SetGraphicsRootDescriptorTable(1, _font->GetSRV());

	rs->OMSetBlendFactor(XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f));
	rs->OMSetStencilRef(0xFFFFFFFF);
	rs->IASetPrimitiveTopology((M3D_PRIMITIVE_TOPOLOGY)D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
	rs->CommitResourceBarriers();
	rs->DrawInstanced((UINT)_vertices.size(), 1, 0, 0);

	_vertices.clear();

	return S_OK;
}

