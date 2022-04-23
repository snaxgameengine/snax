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

using namespace m3d;


TextWriter::TextWriter(bool tech2) : _tech2(tech2)
{
	_vertexBufferCapacity = 0;

	_featureLevel = D3D_FEATURE_LEVEL_11_0;

	_font = nullptr;

	_color = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	_offset = XMFLOAT2(0.0f, 0.0f);
	_transform = XMFLOAT4X4(1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1);
}

TextWriter::~TextWriter()
{
}

HRESULT TextWriter::Init(ID3D11Device *device, Font *font)
{
	SetFont(font);

	_vertexBufferCapacity = 512;

	HRESULT hr;

	_featureLevel = device->GetFeatureLevel();

	if (_featureLevel < D3D_FEATURE_LEVEL_10_0) { // No support for geometry shaders?
		static const D3D11_INPUT_ELEMENT_DESC d[3] = {
			{"POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"TEXCOORD", 0, DXGI_FORMAT_R32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0}};

		if (_tech2) {
			V_RETURN(device->CreateVertexShader(TEXTWRITER_VS2_40_91_BYTECODE, sizeof(TEXTWRITER_VS2_40_91_BYTECODE), nullptr, &_vs));
			V_RETURN(device->CreateInputLayout(d, 2, TEXTWRITER_VS2_40_91_BYTECODE, sizeof(TEXTWRITER_VS2_40_91_BYTECODE), &_il));
		}
		else {
			V_RETURN(device->CreateVertexShader(TEXTWRITER_VS1_40_91_BYTECODE, sizeof(TEXTWRITER_VS1_40_91_BYTECODE), nullptr, &_vs));
			V_RETURN(device->CreateInputLayout(d, 2, TEXTWRITER_VS1_40_91_BYTECODE, sizeof(TEXTWRITER_VS1_40_91_BYTECODE), &_il));
		}
		V_RETURN(device->CreatePixelShader(TEXTWRITER_PS_40_91_BYTECODE, sizeof(TEXTWRITER_PS_40_91_BYTECODE), nullptr, &_ps));
	}
	else {
		static const D3D11_INPUT_ELEMENT_DESC d[3] = {
			{"POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"TEXCOORD", 0, DXGI_FORMAT_R16_UINT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"TEXCOORD", 1, DXGI_FORMAT_R16_UINT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0}};

		V_RETURN(device->CreateVertexShader(TEXTWRITER_VS_40_BYTECODE, sizeof(TEXTWRITER_VS_40_BYTECODE), nullptr, &_vs));
		V_RETURN(device->CreateInputLayout(d, 2, TEXTWRITER_VS_40_BYTECODE, sizeof(TEXTWRITER_VS_40_BYTECODE), &_il));

		if (_tech2) {
			V_RETURN(device->CreateGeometryShader(TEXTWRITER_GS2_40_BYTECODE, sizeof(TEXTWRITER_GS2_40_BYTECODE), nullptr, &_gs));
		}
		else {
			V_RETURN(device->CreateGeometryShader(TEXTWRITER_GS1_40_BYTECODE, sizeof(TEXTWRITER_GS1_40_BYTECODE), nullptr, &_gs));
		}

		V_RETURN(device->CreatePixelShader(TEXTWRITER_PS_40_BYTECODE, sizeof(TEXTWRITER_PS_40_BYTECODE), nullptr, &_ps));
	}

	D3D11_BUFFER_DESC bd = {sizeof(CB), D3D11_USAGE_DYNAMIC, D3D11_BIND_CONSTANT_BUFFER, D3D11_CPU_ACCESS_WRITE, 0, 0};
	V_RETURN(device->CreateBuffer(&bd, nullptr, &_cb));

	D3D11_BLEND_DESC bDesc = CD3D11_BLEND_DESC(CD3D11_DEFAULT());
	bDesc.RenderTarget[0].BlendEnable = TRUE;
	bDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	bDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	V_RETURN(device->CreateBlendState(&bDesc, &_bs));

	D3D11_DEPTH_STENCIL_DESC dsDesc = CD3D11_DEPTH_STENCIL_DESC(CD3D11_DEFAULT());
	dsDesc.DepthEnable = FALSE;
	V_RETURN(device->CreateDepthStencilState(&dsDesc, &_dss));

	D3D11_SAMPLER_DESC sDesc = CD3D11_SAMPLER_DESC(CD3D11_DEFAULT());
	V_RETURN(device->CreateSamplerState(&sDesc, &_ss));

	return S_OK;
}

void TextWriter::OnDestroyDevice()
{
	_vertexBufferCapacity = 0;
	_font = nullptr;
	_vertices.clear();
	_vertexBuffer = nullptr;
	_il = nullptr;
	_vs = nullptr;
	_gs = nullptr;
	_ps = nullptr;
	_bs = nullptr;
	_dss = nullptr;
	_ss = nullptr;
	_cb = nullptr;
}


void TextWriter::SetFont(Font *font)
{
	if (_font != font) {
		_font = font;
		_vertices.clear();
	}
}

void TextWriter::Write(XMFLOAT2 position, Alignment alignment, bool multiline, XMFLOAT4 color, String text)
{
	_color = color;
	return Write(position, alignment, multiline, text);
}

void TextWriter::Write(XMFLOAT2 position, Alignment alignment, bool multiline, String text)
{
	XMFLOAT2 p = position;

	// When using tech2 (freely transformed) we scale everything by glyph height.
	float32 S = _tech2 ? (1.0f / _font->GetGlyphHeight()) : 1.0f;

	for (unsigned i = 0; i < text.length(); i++) {
		if (multiline && text[i] == '\n') {
			p.x = position.x;
			p.y += S * _font->GetGlyphHeight();
			continue;
		}
		const Font::Glyph *g = _font->GetGlyph(text[i]);
		if (g == nullptr)
			continue;
		if (_featureLevel < D3D_FEATURE_LEVEL_10_0) { // For level 9.1 we use a vertex shader and have to set up 6 vertices (2 triangles pr letter)!
			Vertex v = {p.x + S * g->movex + _offset.x, p.y + _offset.y, 0};
			v.glyphAndIndex = (float32)g->glyphIndex * 4.0f + 0.0f;
			_vertices.push_back(v);
			v.glyphAndIndex = (float32)g->glyphIndex * 4.0f + 1.0f;
			_vertices.push_back(v);
			v.glyphAndIndex = (float32)g->glyphIndex * 4.0f + 2.0f;
			_vertices.push_back(v);
			v.glyphAndIndex = (float32)g->glyphIndex * 4.0f + 1.0f;
			_vertices.push_back(v);
			v.glyphAndIndex = (float32)g->glyphIndex * 4.0f + 3.0f;
			_vertices.push_back(v);
			v.glyphAndIndex = (float32)g->glyphIndex * 4.0f + 2.0f;
			_vertices.push_back(v);
		}
		else {
			Vertex v = {p.x + S * g->movex + _offset.x, p.y + _offset.y, (uint16)g->glyphIndex};
			_vertices.push_back(v);
		}

		p.x += S * g->advance;
	}
}

HRESULT TextWriter::Write(ID3D11DeviceContext *context, XMFLOAT2 position, Alignment alignment, bool multiline, String text)
{
	Write(position, alignment, multiline, text);
	return Flush(context);
}

HRESULT TextWriter::Write(ID3D11DeviceContext *context, XMFLOAT2 position, Alignment alignment, bool multiline, XMFLOAT4 color, String text)
{
	Write(position, alignment, multiline, color, text);
	return Flush(context);
}

HRESULT TextWriter::Flush(ID3D11DeviceContext *context)
{
	if (_vertices.size() == 0)
		return S_OK;
	HRESULT hr;

	V_RETURN(_font->Update(context));

	if (_vertices.size() > _vertexBufferCapacity) {
		_vertexBufferCapacity = std::max(_vertexBufferCapacity * 2, (unsigned)_vertices.size());
		_vertexBuffer = nullptr;
	}
	if (!_vertexBuffer) {
		D3D11_BUFFER_DESC bDesc;
		bDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bDesc.ByteWidth = sizeof(Vertex) * _vertexBufferCapacity;
		bDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		bDesc.MiscFlags = 0;
		bDesc.StructureByteStride = 0;
		bDesc.Usage = D3D11_USAGE_DYNAMIC;
		SID3D11Device device;
		context->GetDevice(&device);
		V_RETURN(device->CreateBuffer(&bDesc, nullptr, &_vertexBuffer));
	}

	D3D11_MAPPED_SUBRESOURCE r;
	V_RETURN(context->Map(_vertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &r));
	std::memcpy(r.pData, &_vertices.front(), sizeof(Vertex) * _vertices.size());
	context->Unmap(_vertexBuffer, 0);

	UINT numVP = 1;
	D3D11_VIEWPORT vp;
	context->RSGetViewports(&numVP, &vp);
	if (numVP == 0)
		return E_FAIL;

	CB cb;
	cb.fontColor = _color;
	cb.props = XMFLOAT4(2.0f / vp.Width, -2.0f / vp.Height, float32(_font->GetGlyphHeight()) / _font->GetTextureHeight(), float32(_font->GetGlyphHeight()));
	if (_tech2) {
		XMStoreFloat4x4(&cb.wvp, XMMatrixTranspose(XMLoadFloat4x4(&_transform)));
	}
	D3D11_MAPPED_SUBRESOURCE mapped;
	V_RETURN(context->Map(_cb, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped));
	std::memcpy(mapped.pData, &cb, sizeof(CB));
	context->Unmap(_cb, 0);

	context->IASetInputLayout(_il);
	UINT strides = sizeof(Vertex);
	UINT offset = 0;
	context->IASetVertexBuffers(0, 1, &(const SID3D11Buffer&)_vertexBuffer, &strides, &offset);
	context->VSSetShader(_vs, nullptr, 0);
	if (_featureLevel < D3D_FEATURE_LEVEL_10_0) {
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		context->GSSetShader(nullptr, nullptr, 0);
		ID3D11Buffer *vsCB[2] = {_cb, _font->GetGlyphBuffer()};
		context->VSSetConstantBuffers(0, 2, vsCB);
	}
	else {
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
		context->GSSetShader(_gs, nullptr, 0);
		context->GSSetShaderResources(0, 1, &(const SID3D11ShaderResourceView&)_font->GetGlyphBufferSRV());
		context->GSSetConstantBuffers(0, 1, &(const SID3D11Buffer&)_cb);
	}
	context->HSSetShader(nullptr, nullptr, 0);
	context->DSSetShader(nullptr, nullptr, 0);
	context->PSSetShader(_ps, nullptr, 0);
	context->PSSetShaderResources(0, 1, &(const SID3D11ShaderResourceView&)_font->GetGlyphTexture());
	context->PSSetSamplers(0, 1, &(const SID3D11SamplerState&)_ss);
	context->PSSetConstantBuffers(0, 1, &(const SID3D11Buffer&)_cb);
	context->RSSetState(nullptr);
	context->OMSetDepthStencilState(_dss, 0);
	const XMFLOAT4 z(0, 0, 0, 0);
	context->OMSetBlendState(_bs, (const FLOAT*)&z, 0xFFFFFFFF);

	context->Draw((unsigned)_vertices.size() , 0);

	_vertices.clear();
	return S_OK;
}

