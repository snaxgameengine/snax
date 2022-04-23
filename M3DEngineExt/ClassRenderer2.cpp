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
#include "ClassRenderer2.h"
#include "ClassRenderer2.hlsl.h"
#include "ClassRenderer2_9.hlsl.h"
#include "ClassExt.h"
#include "ClassView.h"
#include "Images.h"
#include "TextWriter.h"
#include "ClassDiagramView.h"
#include "EditorState.h"
#include "M3DEngine/Err.h"
#include "M3DEngine/Chip.h"
#include "M3DEngine/Engine.h"
#include "M3DEngine/ChipManager.h"
#include "M3DEngine/Function.h"
#include "StdChips/Shortcut.h"
#include "StdChips/FunctionCall.h"
#include "StdChips/Parameter.h"
#include "StdChips/ClassDiagramChip.h"
#include "M3DEngine/Document.h"
#include "M3DEngine/Environment.h"

using namespace m3d;

struct VLink
{
	XMFLOAT2 a; // Point A
	XMFLOAT2 b; // Point B
	XMUBYTEN4 c; // Link color
	INT8 d; // direction -1/0/1
	UINT8 e; // Link activeness
};

struct VConnectors
{
	XMFLOAT2 pos; // Position
	XMFLOAT2 scale; // Scale (even though all connectors have the same size. It saves us from updating the cb.)
	XMUBYTEN4 col; // color
	UINT8 tc0; // Texture offset.
};


ClassRenderer2::ClassRenderer2()
{
	_colorScheme = StdColors;
	_vbSize = 0;
	_time = 0;
	_font = nullptr;
	_textWriter = mmnew TextWriter();
	_font2 = mmnew Font();
	_textWriter2 = mmnew TextWriter(true);
}

ClassRenderer2::~ClassRenderer2()
{
	mmdelete(_textWriter);
	mmdelete(_font2);
	mmdelete(_textWriter2);
}


HRESULT ClassRenderer2::Init(ID3D11Device *device)
{
	HRESULT hr;

	_level10 = device->GetFeatureLevel() > D3D_FEATURE_LEVEL_10_0; // Do we have access to level 10.0?

	V_RETURN(CreateDDSTextureFromMemory(device, IMG_CHIP_DDS, ARRAYSIZE(IMG_CHIP_DDS), nullptr, &_srvChip));
	V_RETURN(CreateDDSTextureFromMemory(device, IMG_CHIPBORDER_DDS, ARRAYSIZE(IMG_CHIPBORDER_DDS), nullptr, &_srvChipBorder));
	V_RETURN(CreateDDSTextureFromMemory(device, IMG_CHIPHALF_DDS, ARRAYSIZE(IMG_CHIPHALF_DDS), nullptr, &_srvChipHalf));
	V_RETURN(CreateDDSTextureFromMemory(device, IMG_PADLOCK_DDS, ARRAYSIZE(IMG_PADLOCK_DDS), nullptr, &_srvPadlock));
	V_RETURN(CreateDDSTextureFromMemory(device, IMG_SHIELD_DDS, ARRAYSIZE(IMG_SHIELD_DDS), nullptr, &_srvShield));
	V_RETURN(CreateDDSTextureFromMemory(device, IMG_NOTE_DDS, ARRAYSIZE(IMG_NOTE_DDS), nullptr, &_srvNote));
	V_RETURN(CreateDDSTextureFromMemory(device, IMG_SHORTCUT_DDS, ARRAYSIZE(IMG_SHORTCUT_DDS), nullptr, &_srvShortcut));
	V_RETURN(CreateDDSTextureFromMemory(device, IMG_METEOR_DDS, ARRAYSIZE(IMG_METEOR_DDS), nullptr, &_srvMeteor));
	V_RETURN(CreateDDSTextureFromMemory(device, IMG_FOLDER_DDS, ARRAYSIZE(IMG_FOLDER_DDS), nullptr, &_srvFolder));
	V_RETURN(CreateDDSTextureFromMemory(device, IMG_STARTARROW_DDS, ARRAYSIZE(IMG_STARTARROW_DDS), nullptr, &_srvStart));
	V_RETURN(CreateDDSTextureFromMemory(device, IMG_MOVE_DDS, ARRAYSIZE(IMG_MOVE_DDS), nullptr, &_srvMove));
	V_RETURN(CreateDDSTextureFromMemory(device, IMG_SCALE_DDS, ARRAYSIZE(IMG_SCALE_DDS), nullptr, &_srvScale));
	V_RETURN(CreateDDSTextureFromMemory(device, IMG_CHDIR_DDS, ARRAYSIZE(IMG_CHDIR_DDS), nullptr, &_srvChDir));
	V_RETURN(CreateDDSTextureFromMemory(device, IMG_REFRESH_DDS, ARRAYSIZE(IMG_REFRESH_DDS), nullptr, &_srvRefresh));
	V_RETURN(CreateDDSTextureFromMemory(device, IMG_FRAME_DDS, ARRAYSIZE(IMG_FRAME_DDS), nullptr, &_srvFrame));
	V_RETURN(CreateDDSTextureFromMemory(device, IMG_ONCE_DDS, ARRAYSIZE(IMG_ONCE_DDS), nullptr, &_srvOnce));
	V_RETURN(CreateDDSTextureFromMemory(device, IMG_INFOBOX_DDS, ARRAYSIZE(IMG_INFOBOX_DDS), nullptr, &_srvInfoBox));
	V_RETURN(CreateDDSTextureFromMemory(device, IMG_INFOCORNERCH_DDS, ARRAYSIZE(IMG_INFOCORNERCH_DDS), nullptr, &_srvInfoCornerCh));
	V_RETURN(CreateDDSTextureFromMemory(device, IMG_INFOCORNERCO_DDS, ARRAYSIZE(IMG_INFOCORNERCO_DDS), nullptr, &_srvInfoCornerCo));
	V_RETURN(CreateDDSTextureFromMemory(device, IMG_CONSTANT_DDS, ARRAYSIZE(IMG_CONSTANT_DDS), nullptr, &_srvConstant));
	V_RETURN(CreateDDSTextureFromMemory(device, IMG_INFO_DDS, ARRAYSIZE(IMG_INFO_DDS), nullptr, &_srvInfo));
	V_RETURN(CreateDDSTextureFromMemory(device, IMG_NOTICE_DDS, ARRAYSIZE(IMG_NOTICE_DDS), nullptr, &_srvNotice));
	V_RETURN(CreateDDSTextureFromMemory(device, IMG_WARNING_DDS, ARRAYSIZE(IMG_WARNING_DDS), nullptr, &_srvWarning));
	V_RETURN(CreateDDSTextureFromMemory(device, IMG_ERROR_DDS, ARRAYSIZE(IMG_ERROR_DDS), nullptr, &_srvError));
	V_RETURN(CreateDDSTextureFromMemory(device, IMG_CONNECTORS_DDS, ARRAYSIZE(IMG_CONNECTORS_DDS), nullptr, &_srvConnectors));
	V_RETURN(CreateDDSTextureFromMemory(device, IMG_PAUSE_DDS, ARRAYSIZE(IMG_PAUSE_DDS), nullptr, &_srvPause));
	V_RETURN(CreateDDSTextureFromMemory(device, IMAGE_LIBRARY_DDS, ARRAYSIZE(IMAGE_LIBRARY_DDS), nullptr, &_srvLibrary));

	D3D11_BUFFER_DESC cbDesc = {sizeof(CB), D3D11_USAGE_DYNAMIC, D3D11_BIND_CONSTANT_BUFFER, D3D11_CPU_ACCESS_WRITE, 0, 0};
	V_RETURN(device->CreateBuffer(&cbDesc, nullptr, &_cb));
	
	D3D11_BLEND_DESC bDesc = CD3D11_BLEND_DESC(CD3D11_DEFAULT());
	bDesc.RenderTarget[0].BlendEnable = true;
	bDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	bDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	bDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	V_RETURN(device->CreateBlendState(&bDesc, &_bsEnable));

	bDesc.RenderTarget[0].BlendEnable = true;
	bDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	bDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_INV_DEST_COLOR;
	bDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
	V_RETURN(device->CreateBlendState(&bDesc, &_bsScreen));

	D3D11_SAMPLER_DESC sDesc = CD3D11_SAMPLER_DESC(CD3D11_DEFAULT());
	V_RETURN(device->CreateSamplerState(&sDesc, &_sampler));

	sDesc = CD3D11_SAMPLER_DESC(CD3D11_DEFAULT());
	sDesc.AddressU = sDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	V_RETURN(device->CreateSamplerState(&sDesc, &_wrapSampler));

	D3D11_RASTERIZER_DESC rDesc = CD3D11_RASTERIZER_DESC(CD3D11_DEFAULT());
	rDesc.MultisampleEnable = TRUE;
	V_RETURN(device->CreateRasterizerState(&rDesc, &_rs));
	
	V_RETURN(_textWriter->Init(device, nullptr));

	if (!_font2->Init(MTEXT("Segoe UI"), 96, 50, Font::NORMAL, false, false, false, 4))
		return E_FAIL;
	V_RETURN(_textWriter2->Init(device, _font2));
	_textWriter2->SetOffset(XMFLOAT2(0.0f, -0.2f));

	if (!_level10)
		return _init9(device);

	V_RETURN(device->CreateVertexShader(HLSL_BYTECODE_VS_SQUARE, ARRAYSIZE(HLSL_BYTECODE_VS_SQUARE), nullptr, &_vsSquare));
	V_RETURN(device->CreateVertexShader(HLSL_BYTECODE_VS_PLAIN, ARRAYSIZE(HLSL_BYTECODE_VS_PLAIN), nullptr, &_vsPlain));
	V_RETURN(device->CreateVertexShader(HLSL_BYTECODE_VS_2DLINES, ARRAYSIZE(HLSL_BYTECODE_VS_2DLINES), nullptr, &_vs2DLines));
	V_RETURN(device->CreateVertexShader(HLSL_BYTECODE_VS_2DLINESBOX, ARRAYSIZE(HLSL_BYTECODE_VS_2DLINESBOX), nullptr, &_vs2DLinesBox));
	V_RETURN(device->CreateVertexShader(HLSL_BYTECODE_VS_2DQUADS, ARRAYSIZE(HLSL_BYTECODE_VS_2DQUADS), nullptr, &_vs2DQuads));
	V_RETURN(device->CreateVertexShader(HLSL_BYTECODE_VS_CHIP, ARRAYSIZE(HLSL_BYTECODE_VS_CHIP), nullptr, &_vsChip));
	V_RETURN(device->CreateVertexShader(HLSL_BYTECODE_VS_CONNECTORS, ARRAYSIZE(HLSL_BYTECODE_VS_CONNECTORS), nullptr, &_vsConnectors));
	V_RETURN(device->CreatePixelShader(HLSL_BYTECODE_PS_COLORED, ARRAYSIZE(HLSL_BYTECODE_PS_COLORED), nullptr, &_psColored));
	V_RETURN(device->CreatePixelShader(HLSL_BYTECODE_PS_TEXTURED, ARRAYSIZE(HLSL_BYTECODE_PS_TEXTURED), nullptr, &_psTextured));
	V_RETURN(device->CreatePixelShader(HLSL_BYTECODE_PS_TEXTURED_WITH_VERTEXCOLOR, ARRAYSIZE(HLSL_BYTECODE_PS_TEXTURED_WITH_VERTEXCOLOR), nullptr, &_psTexturedWithVertexColor));
	V_RETURN(device->CreatePixelShader(HLSL_BYTECODE_PS_CHDIR, ARRAYSIZE(HLSL_BYTECODE_PS_CHDIR), nullptr, &_psChDir));
	V_RETURN(device->CreatePixelShader(HLSL_BYTECODE_PS_CHIP, ARRAYSIZE(HLSL_BYTECODE_PS_CHIP), nullptr, &_psChip));
	V_RETURN(device->CreatePixelShader(HLSL_BYTECODE_PS_VERTEXCOLOR, ARRAYSIZE(HLSL_BYTECODE_PS_VERTEXCOLOR), nullptr, &_psVertexColor));
	V_RETURN(device->CreatePixelShader(HLSL_BYTECODE_PS_DATAPARTICLES, ARRAYSIZE(HLSL_BYTECODE_PS_DATAPARTICLES), nullptr, &_psDataParticles));


	static const D3D11_INPUT_ELEMENT_DESC ieDesc[4] = {
		{"POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1},
		{"COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
		{"DIR", 0, DXGI_FORMAT_R8_SINT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
		{"POW", 0, DXGI_FORMAT_R8_UNORM, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1}};

	V_RETURN(device->CreateInputLayout(ieDesc, 4, HLSL_BYTECODE_VS_2DLINES, ARRAYSIZE(HLSL_BYTECODE_VS_2DLINES), &_il));

	static const D3D11_INPUT_ELEMENT_DESC ieDescPlain[1] = {{"POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0}};

	V_RETURN(device->CreateInputLayout(ieDescPlain, 1, HLSL_BYTECODE_VS_PLAIN, ARRAYSIZE(HLSL_BYTECODE_VS_PLAIN), &_ilPlain));

	static const D3D11_INPUT_ELEMENT_DESC ieDescConnectors[3] = {
		{"POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1},
		{"COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
		{"TEXCOORD", 0, DXGI_FORMAT_R8_UNORM, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1}};

	V_RETURN(device->CreateInputLayout(ieDescConnectors, 3, HLSL_BYTECODE_VS_CONNECTORS, ARRAYSIZE(HLSL_BYTECODE_VS_CONNECTORS), &_ilConnectors));

	const float32 VERTICES[52] = 
	{
		-0.5f,-0.5f,0.0f,0.0f, // Rectangle (lines)
		0.5f,-0.5f,0.0f,0.0f,
		0.5f,0.5f,0.0f,0.0f,
		-0.5f,0.5f,0.0f,0.0f,
		-0.5f,-0.5f,0.0f,0.0f,
		-0.5f,0.0f,0.0f,0.0f, // + symbol (lines)
		0.5f,0.0f,0.0f,0.0f,
		0.0f,-0.5f,0.0f,0.0f,
		0.0f,0.5f,0.0f,0.0f,
		-0.5f,-0.5f,0.0f,1.0f, // square (tripstrip)
		-0.5f,0.5f,0.0f,0.0f,
		0.5f,-0.5f,1.0f,1.0f,
		0.5f,0.5f,1.0f,0.0f
	};

	D3D11_BUFFER_DESC bd  = {sizeof(VERTICES), D3D11_USAGE_IMMUTABLE, D3D11_BIND_VERTEX_BUFFER, 0, 0, 0};
	D3D11_SUBRESOURCE_DATA dd = {VERTICES, 0, 0};
	V_RETURN(device->CreateBuffer(&bd, &dd, &_vbStatic));

	return S_OK;
}

void ClassRenderer2::OnDestroyDevice()
{
	_fonts.clear();
	_font = nullptr;
	_textWriter->OnDestroyDevice();

	_font2->OnDestroyDevice();
	_textWriter2->OnDestroyDevice();

	_srvChip = nullptr;
	_srvChipHalf = nullptr;
	_srvChipBorder = nullptr;
	_srvPadlock = nullptr;
	_srvShield = nullptr;
	_srvNote = nullptr;
	_srvShortcut = nullptr;
	_srvMeteor = nullptr;
	_srvFolder = nullptr;
	_srvStart = nullptr;
	_srvMove = nullptr;
	_srvScale = nullptr;
	_srvChDir = nullptr;
	_srvRefresh = nullptr;
	_srvOnce = nullptr;
	_srvFrame = nullptr;
	_srvInfoBox = nullptr;
	_srvInfoCornerCh = nullptr;
	_srvInfoCornerCo = nullptr;
	_srvConstant = nullptr;
	_srvInfo = nullptr;
	_srvNotice = nullptr;
	_srvWarning = nullptr;
	_srvError = nullptr;
	_srvPause = nullptr;

	_bsEnable = nullptr;
	_bsScreen = nullptr;
	_rs = nullptr;

	_sampler = nullptr;
	_wrapSampler = nullptr;

	_il = nullptr;
	_ilPlain = nullptr;
	_ilConnectors = nullptr;
	_vb = nullptr;
	_vbStatic = nullptr;
	_vbSize = 0;

	_vsSquare = nullptr;
	_vsPlain = nullptr;
	_vs2DLines = nullptr;
	_vs2DLinesBox = nullptr;
	_vs2DQuads = nullptr;
	_vsChip = nullptr;
	_vsConnectors = nullptr;
	_psColored = nullptr;
	_psTextured= nullptr;
	_psTexturedWithVertexColor = nullptr;
	_psChDir = nullptr;
	_psChip = nullptr;
	_psVertexColor = nullptr;
	_psDataParticles = nullptr;

	_cb = nullptr;
}

HRESULT ClassRenderer2::_updateCB(ID3D11DeviceContext *context)
{
	HRESULT hr;
	D3D11_MAPPED_SUBRESOURCE mapped;
	V_RETURN(context->Map(_cb, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped));
	std::memcpy(mapped.pData, &_cbData, sizeof(CB));
	context->Unmap(_cb, 0);
	return S_OK;
}

HRESULT ClassRenderer2::_prepareVertexBuffer(ID3D11Device *device, UINT size)
{
	HRESULT hr;

	if (_vbSize >= size)
		return S_OK; // buffer is large enough!

	_vb = nullptr;
	_vbSize = 0;

	if (size > 0) {
		D3D11_BUFFER_DESC bd = {size, D3D11_USAGE_DYNAMIC, D3D11_BIND_VERTEX_BUFFER, D3D11_CPU_ACCESS_WRITE, 0, 0};
		V_RETURN(device->CreateBuffer(&bd, 0, &_vb));
		_vbSize = size;
	}

	return S_OK;
}

void ClassRenderer2::FormatTextItem(String &str, Vector2 &s, float32 textSize)
{
	float32 scale = textSize * _font2->GetGlyphHeight() / _font2->GetSize();
	int32 w = int32(s.x * _font2->GetGlyphHeight() / scale);
	int32 l = int32(s.y / scale);
	str = _font2->FormatText(str, w, l);
	s.x = float32(w) * scale / _font2->GetGlyphHeight();
	s.y = float32(l) * scale;
}

HRESULT ClassRenderer2::Render(ID3D11DeviceContext *context, ClassView &view, int32 devicePixelRatio, const Vector4 *templateExtent, bool hasFocus)
{
	HRESULT hr;

	{
		auto a = _fonts.find(devicePixelRatio);
		if (a == _fonts.end()) {
			_fonts[devicePixelRatio].Init(MTEXT("Segoe UI"), 96 * devicePixelRatio, 10);
			a = _fonts.find(devicePixelRatio);
		}

		_font = &a->second;
		_textWriter->SetFont(_font);
		_textWriter->SetOffset(XMFLOAT2(0.0f, -2.0f * devicePixelRatio));
	}

	_time = engine->GetClockTime();

	_cbData.vp = Vector4(-view.camPos.x, -view.camPos.y, 2.0f / (view.zoom * view.ar), 2.0f / view.zoom);
	_cbData.time = (float32)_time * DATAPARTICLE_SPEED / CLOCKS_PER_SEC; // time is only used by particles...

	bool isLibrary = false;

	// Clear Screen
	SimpleMath::Color clearColor;
	{
		SID3D11RenderTargetView rtv;
		context->OMGetRenderTargets(1, &rtv, 0);
		if (dynamic_cast<ClassDiagramView*>(&view)) 
			clearColor = _colorScheme.backgroundClassDiagram;
		else {
			Document *doc = view.GetClass()->GetDocument();
			clearColor = _colorScheme.background;
			isLibrary = doc && doc->GetFileName().IsFile() && !engine->GetEnvironment()->IsPathInsideProjectRootFolder(doc->GetFileName()) && engine->GetEnvironment()->IsPathInsideLibraryFolder(doc->GetFileName());
			if (isLibrary)
				clearColor = _colorScheme.backgroundLibrary;
		}
		context->ClearRenderTargetView(rtv, clearColor);
	}

	if (!_level10)
		return _render9(context, view, templateExtent);

	// Set some common states for the first items!
	UINT strides = sizeof(Vector4);
	UINT offset = 0;
	context->IASetVertexBuffers(0, 1, &(const SID3D11Buffer&)_vbStatic, &strides, &offset);
	context->IASetInputLayout(_ilPlain);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
	context->VSSetShader(_vsPlain, nullptr, 0);
	context->VSSetConstantBuffers(0, 1, &(const SID3D11Buffer&)_cb);
	context->GSSetShader(nullptr, nullptr, 0);
	context->PSSetShader(_psColored, nullptr, 0);
	context->PSSetConstantBuffers(0, 1, &(const SID3D11Buffer&)_cb);
	context->PSSetSamplers(0, 1, &(const SID3D11SamplerState&)_sampler);
	context->RSSetState(_rs);
	const Vector4 z(0, 0, 0, 0);
	context->OMSetBlendState(_bsEnable, (const FLOAT*)&z, 0xFFFFFFFF);

	SID3D11Device device;
	context->GetDevice(&device);

	// Render background +
	{
		_cbData.p = Vector2(0.0f, 0.0f);
		_cbData.s = Vector2(10000.0f, 10000.0f);
		_cbData.color1 = _colorScheme.lines;
		V_RETURN(_updateCB(context));
		context->Draw(4, 5);
	}

	// Render background items (pictures, texts, rects)
	for (size_t i = 0; i < view.vBackgroundItems.size(); i++) {
		const CGBackgroundItem &itm = view.vBackgroundItems[i]->second;
		switch (itm.type)
		{
		case CGBackgroundItem::RECT: // All of this rendered first!
			{
				_cbData.p = itm.pos;
				_cbData.s = itm.size;
				_cbData.color1 = (Color)itm.color;
				V_RETURN(_updateCB(context));

				context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
				context->PSSetShader(_psColored, nullptr, 0);
				context->Draw(4, 9);
			}
			break;
		case CGBackgroundItem::IMAGE: // All of these rendered second!
			{
				if (!itm.imageSRV) {
					// Load image from image data!
					SID3D11Device device;
					context->GetDevice(&device);
					V_RETURN(CreateWICTextureFromMemory(device, context, itm.imageData.getConstBuffer(), itm.imageData.getBufferSize(), nullptr, &itm.imageSRV));
				}
				if (itm.imageSRV) {
					_cbData.p = itm.pos;
					_cbData.s = itm.size;
					_cbData.color1 = Color(1.0f, 1.0f, 1.0f, 1.0f);
					V_RETURN(_updateCB(context));
					context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
					context->PSSetShader(_psTextured, nullptr, 0);
					context->PSSetShaderResources(0, 1, &(const SID3D11ShaderResourceView&) itm.imageSRV);
					context->Draw(4, 9);
				}
			}
			break;
		case CGBackgroundItem::TEXT: // Texts are all rendered last of the background items!
			{
				String str = itm.text;
				Vector2 size = itm.size;
				FormatTextItem(str, size, itm.textSize);

				Vector2 pos = Vector2(itm.pos.x, -itm.pos.y) + Vector2(-itm.size.x, -itm.size.y) * 0.5f;

				Matrix vp = Matrix::CreateScale(itm.textSize * ((float32)_font2->GetGlyphHeight() / _font2->GetSize())) * Matrix::CreateTranslation(pos.x, pos.y, 0.0f) *
								Matrix::CreateTranslation(-view.camPos.x, view.camPos.y, 0.0f) * 
								Matrix::CreateOrthographic(view.zoom * view.ar, -view.zoom, -1.0f, 1.0f);
				_textWriter2->SetTransform((Matrix&)vp);
				_textWriter2->Write(context, Vector2(0.0f, 0.0f), TextWriter::LEFT_TOP, true, itm.color, str);

				// Reset some common states after text rendering!
				context->IASetInputLayout(_ilPlain);
				context->IASetVertexBuffers(0, 1, &(const SID3D11Buffer&)_vbStatic, &strides, &offset);
				context->VSSetShader(_vsPlain, nullptr, 0);
				context->VSSetConstantBuffers(0, 1, &(const SID3D11Buffer&)_cb);
				context->GSSetShader(nullptr, nullptr, 0);
				context->PSSetConstantBuffers(0, 1, &(const SID3D11Buffer&)_cb);
				context->PSSetSamplers(0, 1, &(const SID3D11SamplerState&)_sampler);
				context->RSSetState(_rs);
				const Vector4 z(0, 0, 0, 0);
				context->OMSetBlendState(_bsEnable, (const FLOAT*)&z, 0xFFFFFFFF);
			}
			break;
		}

		// Render icons to move/resize background items
		if (itm.showHandles) {
			_cbData.p = itm.pos + (Vector2(-itm.size.x, itm.size.y) + Vector2(BACKGROUNDITEM_HANDLE_SIZE, -BACKGROUNDITEM_HANDLE_SIZE)) * 0.5f;
			_cbData.s = Vector2(BACKGROUNDITEM_HANDLE_SIZE, BACKGROUNDITEM_HANDLE_SIZE);
			_cbData.color1 = Color(1.0f, 1.0f, 1.0f, 1.0f); // HANDLE COLORS?!
			if (itm.hover)
				_cbData.color1 *= _colorScheme.hoverFactor;
			V_RETURN(_updateCB(context));
			
			// Draw move icon
			context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
			context->PSSetShader(_psTextured, nullptr, 0);
			context->PSSetShaderResources(0, 1, &(const SID3D11ShaderResourceView&)_srvMove);
			context->Draw(4, 9);

			// Draw scale icon
			_cbData.p = itm.pos + (Vector2(itm.size.x, -itm.size.y) - Vector2(BACKGROUNDITEM_HANDLE_SIZE, -BACKGROUNDITEM_HANDLE_SIZE)) * 0.5f;
			V_RETURN(_updateCB(context));
			context->PSSetShaderResources(0, 1, &(const SID3D11ShaderResourceView&)_srvScale);
			context->Draw(4, 9);
		}

		// Render rectangle to indicate selected item
		if (itm.selected) {
			_cbData.p = itm.pos;
			_cbData.s = itm.size + Vector2(0.1f);
			_cbData.color1 = _colorScheme.selected;
			V_RETURN(_updateCB(context));

			context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);
			context->PSSetShader(_psColored, nullptr, 0);
			context->Draw(5, 0);
		}
	}
	
	// Render links
	if (view.vLinks.size() + view.vILinks.size() > 0) {
		_cbData.p = Vector2(0.0f);
		_cbData.s = Vector2(LINK_ARROW_SIZE);
		_cbData.color1 = Color(1.0f, 1.0f, 1.0f, 1.0f);
		_cbData.scaling = Vector4(VERTICES_PR_LINK - 1, 0, 0, 0);
		V_RETURN(_updateCB(context));

		V_RETURN(_prepareVertexBuffer(device, (UINT)((view.vLinks.size() * 2 + view.vILinks.size()) * sizeof(VLink))));

		D3D11_MAPPED_SUBRESOURCE vbMapped;
		V_RETURN(context->Map(_vb, 0, D3D11_MAP_WRITE_DISCARD, 0, &vbMapped));
		VLink *vb = (VLink*)vbMapped.pData;
		unsigned mCount = 0;
		unsigned aCount = unsigned(view.vLinks.size() + view.vILinks.size());

		for (unsigned i = 0; i < view.vLinks.size(); i++, mCount++) {
			Link &l = view.vLinks[i];

			float32 tDiff = (float32)(_time - l.lastHit) * 0.001f;// (float32)CLOCKS_PER_SEC;
			tDiff = std::min(std::max(tDiff, 0.0f), 1.0f);

			Color c = _colorScheme.links;

			if (l.selected)
				c = _colorScheme.selected + _colorScheme.active * (1.0f - tDiff);
			else
				c = Color::Lerp(_colorScheme.active, c, tDiff);

			if (l.hover) // Hover?
				c *= _colorScheme.hoverFactor;

			vb[mCount].a = l.a;
			vb[mCount].b = l.b;
			vb[mCount].c = c.RGBA();
			vb[mCount].d = INT8(l.dir);
			vb[mCount].e = UINT8(255.0f * (1.0f - tDiff));
			
			if (tDiff < 0.999f)
				vb[aCount++] = vb[mCount]; // Add glowing particle!
		}
		for (unsigned i = 0; i < view.vILinks.size(); i++, mCount++) {
			ILink &l = view.vILinks[i];
			vb[mCount].a = l.a;
			vb[mCount].b = l.b;
			vb[mCount].c = _colorScheme.iLinks.RGBA();
			vb[mCount].d = INT8(l.dir);
			vb[mCount].e = 0;
		}
		context->Unmap(_vb, 0);

		UINT strides = sizeof(VLink);
		UINT offset = 0;

		// Draw symbol at midlines.
		context->IASetVertexBuffers(0, 1, &(const SID3D11Buffer&)_vb, &strides, &offset);
		context->IASetInputLayout(_il);
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
		context->VSSetShader(_vs2DLinesBox, nullptr, 0);
		context->PSSetShader(_psChDir, nullptr, 0);
		context->PSSetShaderResources(0, 1, &(const SID3D11ShaderResourceView&)_srvChDir);
		context->PSSetSamplers(0, 1, &(const SID3D11SamplerState&)_sampler);
		context->DrawInstanced(4, mCount, 0, 0);

		// Draw lines
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);
		context->VSSetShader(_vs2DLines, nullptr, 0);
		context->PSSetShader(_psVertexColor, nullptr, 0);
		context->DrawInstanced(VERTICES_PR_LINK, mCount, 0, 0);

		// Draw glow around active lines
		if (aCount > mCount) {
			_cbData.s = Vector2(DATA_PARTICLE_WIDTH, DATA_PARTICLE_LENGTH);
			_cbData.color1 = Color(1.0f, 1.0f, 1.0f, 1.0f); // CENTER GLOW COLOR
			_cbData.color2 = _colorScheme.active; // PERIFER COLOR
			V_RETURN(_updateCB(context));

			context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
			context->VSSetShader(_vs2DQuads, nullptr, 0);
			context->PSSetShader(_psDataParticles, nullptr, 0);
			context->PSSetShaderResources(0, 1, &(const SID3D11ShaderResourceView&)_srvMeteor);
			context->PSSetSamplers(0, 1, &(const SID3D11SamplerState&)_wrapSampler);
			const Vector4 z(0, 0, 0, 0);
			context->OMSetBlendState(_bsScreen, (const FLOAT*)&z, 0xFFFFFFFF);
			context->DrawInstanced(VERTICES_PR_LINK * 2 /*4*/, aCount - mCount, 0, mCount);
		}
	}

	// Set some common state
	{
		context->IASetInputLayout(nullptr);
		context->PSSetSamplers(0, 1, &(const SID3D11SamplerState&)_sampler);
		context->RSSetState(_rs);
		const Vector4 z(0, 0, 0, 0);
		context->OMSetBlendState(_bsEnable, (const FLOAT*)&z, 0xFFFFFFFF);
	}

	// Render chips
	for (unsigned i = 0; i < view.vChips.size(); i++) {
		V_RETURN(_renderChip(device, context, view, view.vChips[i]));
	}

	ClassExt *cge = view.clazz;

	// Render folders
	for (unsigned i = 0; i < view.vFolders.size(); i++) {
		V_RETURN(_renderFolder(device, context, view, cge->GetFolderList()[view.vFolders[i]]));
	}

	// Render Start arrow
	{
		Class *startClass = engine->GetClassManager()->GetStartClass();
		Chip *chStart = cge->GetStartChip();
		if (chStart) {
			for (unsigned i = 0; i < view.vChips.size(); i++) {
				Chip *cv = view.vChips[i];
				if (cv == chStart) {
					_cbData.p = Vector2(cv->GetChipEditorData()->pos.x, cv->GetChipEditorData()->pos.y + CHIP_HEIGHT);
					_cbData.s = Vector2(CHIP_HEIGHT * 1.2f, CHIP_HEIGHT * 1.2f);
					_cbData.color1 = Color(1.0f, 1.0f, 1.0f, cge == startClass ? 1.0f : 0.25f); // Start arrow color
					V_RETURN(_updateCB(context));
					context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
					context->VSSetShader(_vsSquare, nullptr, 0);
					context->PSSetShader(_psTextured, nullptr, 0);
					context->PSSetShaderResources(0, 1, &(const SID3D11ShaderResourceView&)_srvStart);
					context->Draw(4, 0);
					break;
				}
			}
		}
	}

	Vector2 vpPos = Vector2(-view.camPos.x / (view.zoom * view.ar) + 0.5f, view.camPos.y / (view.zoom) + 0.5f) * Vector2(view.w, view.h);
	Vector2 vpScale = Vector2(1.0f / (view.zoom * view.ar), -1.0f / view.zoom) * Vector2(view.w, view.h);

	// Render Item Text
	if (view.h / view.zoom > CHIPTEXT_CUTOFF) {
		Vector2 cd;

		struct L 
		{
			bool operator() (const XMFLOAT2 &x, const XMFLOAT2 &y) const {return x.x < y.x || x.x == y.x && x.y < y.y; }
		};

		// All text string sorted left to right, top to bottom. This is for the decluttering algorithm!
		Map<XMFLOAT2, String, L> m;

		const float32 h = 0.6f * _font->GetGlyphHeight();

		//bool noAutohideValues = -(CHIP_HEIGHT * 0.5f * vpScale.y + 3.0f) > h;

		// Render Chip Text
		for (unsigned i = 0; i < view.vChips.size(); i++) {
			Vector2 position = view.vChips[i]->GetChipEditorData()->pos;
			float32 width = view.vChips[i]->GetChipEditorData()->width;
			// Render name
			cd = Vector2((-width * 0.5f + position.x + 0.12f), (CHIP_HEIGHT * 0.5f + position.y - 0.1f)) * vpScale + vpPos;
			String name = view.vChips[i]->GetName();
			int32 NUMBER_OF_CHAR = int32(view.h / view.zoom);
			if (name.length() > NUMBER_OF_CHAR)
				name = name.substr(0, NUMBER_OF_CHAR - 3) + MTEXT("...");
			m.insert(std::make_pair(Vector2(cd.x, cd.y), name));
			
			// Render value
//			if (noAutohideValues) { // Check is declutter will remove the value anyway!
				String valueStr = view.vChips[i]->GetValueAsString();
				if (valueStr.length()) {
					cd = Vector2((-width * 0.5f + position.x + 0.12f), (position.y - 0.1f)) * vpScale + vpPos;
					m.insert(std::make_pair(Vector2(cd.x, cd.y - 3.0f), valueStr));
				}
//			}			
		}

		// Render Folder Text
		for (unsigned i = 0; i < view.vFolders.size(); i++) {
			Vector2 position = view.vFolders[i] == view.folder ? cge->GetFolderList()[view.vFolders[i]].inPos : cge->GetFolderList()[view.vFolders[i]].pos;
			// Render name
			cd = Vector2((-CHIP_HEIGHT * 0.5f + position.x + 0.12f), (CHIP_HEIGHT * 0.5f + position.y - 0.1f)) * vpScale + vpPos;
			m.insert(std::make_pair(Vector2(cd.x, cd.y), cge->GetFolderList()[view.vFolders[i]].name.c_str()));
		}

		List<XMFLOAT3> l;

		List<std::pair<XMFLOAT2, String>> txtList, decTxtList;

		// Declutter and render!
		for (const auto &n : m) {
			bool b = true;
			XMFLOAT3 t;
			for (size_t i = l.size(); i > 0 && (t = l[i - 1]).z > n.first.x && (b = !(t.x > n.first.x && t.y > (n.first.y - h) && (t.y - h) < n.first.y)); i--);
			if (b) {
				float32 textRight = n.first.x + _font->GetWidth(n.second);
				l.push_back(XMFLOAT3(textRight, n.first.y, l.size() > 0 ? std::max(textRight, l.back().z) : textRight));
				txtList.push_back(std::make_pair(XMFLOAT2(floor(n.first.x), floor(n.first.y)), n.second.c_str()));
			} else {
				decTxtList.push_back(std::make_pair(XMFLOAT2(floor(n.first.x), floor(n.first.y)), n.second.c_str()));
			}
		}

		// First, write text that is decluttered.
		XMFLOAT4 c = _colorScheme.foreground;
		c.w *= 0.2f;
		_textWriter->SetColor(c);
		for (size_t i = 0; i < decTxtList.size(); i++)
			_textWriter->Write(decTxtList[i].first, TextWriter::LEFT_TOP, false, decTxtList[i].second);
		V_RETURN(_textWriter->Flush(context));
		// Then, write the uncluttered text.
		_textWriter->SetColor(_colorScheme.foreground);
		for (size_t i = 0; i < txtList.size(); i++) 
			_textWriter->Write(txtList[i].first, TextWriter::LEFT_TOP, false, txtList[i].second);


		// THIS IS THE OLD CODE WITHOUT DECLUTTERING:
		/*
		// Render Chip Text
		for (unsigned i = 0; i < view.vChips.size(); i++) {
			Vector2 position = view.vChips[i]->GetChipEditorData()->pos;
			float32 width = view.vChips[i]->GetChipEditorData()->width;
			// Render name
			cd = Vector2((-width * 0.5f + position.x + 0.12f), (CHIP_HEIGHT * 0.5f + position.y - 0.1f)) * vpScale + vpPos;
			String name = view.vChips[i]->GetName();
			int32 NUMBER_OF_CHAR = int32(view.h / view.zoom);
			if (name.length() > NUMBER_OF_CHAR)
				name = name.substr(0, NUMBER_OF_CHAR - 3) + MTEXT("...");
			_textWriter->Write(Vector2(floor(cd.x), floor(cd.y)), TextWriter::LEFT_TOP, false, name.c_str());
			// Render value
			String valueStr = view.vChips[i]->GetValueAsString();
			if (valueStr.length()) {
				cd = Vector2((-width * 0.5f + position.x + 0.12f), (position.y - 0.1f)) * vpScale + vpPos;
				_textWriter->Write(Vector2(floor(cd.x), floor(cd.y) - 3.0f), TextWriter::LEFT_TOP, false, valueStr.c_str());
			}
		}

		// Render Folder Text
		for (unsigned i = 0; i < view.vFolders.size(); i++) {
			Vector2 position = view.vFolders[i] == view.folder ? cge->GetFolderList()[view.vFolders[i]].inPos : cge->GetFolderList()[view.vFolders[i]].pos;
			// Render name
			cd = Vector2((-CHIP_HEIGHT * 0.5f + position.x + 0.12f), (CHIP_HEIGHT * 0.5f + position.y - 0.1f)) * vpScale + vpPos;
			_textWriter->Write(Vector2(floor(cd.x), floor(cd.y)), TextWriter::LEFT_TOP, false, cge->GetFolderList()[view.vFolders[i]].name.c_str());

		}*/
	}

	// Flush texts
	V_RETURN(_textWriter->Flush(context));
	
	// Reset common state after drawing text
	context->IASetVertexBuffers(0, 1, &(const SID3D11Buffer&)_vbStatic, &strides, &offset);
	context->IASetInputLayout(_ilPlain);
	context->VSSetConstantBuffers(0, 1, &(const SID3D11Buffer&)_cb);
	context->PSSetConstantBuffers(0, 1, &(const SID3D11Buffer&)_cb);
	context->GSSetShader(nullptr, nullptr, 0);
	context->RSSetState(_rs);
	context->OMSetBlendState(_bsEnable, (const FLOAT*)&z, 0xFFFFFFFF);

	// Render selection box
	if (view.dm == DM_SELECT) {
		_cbData.p = (view.selectionStart + view.mousePos) * 0.5f;
		_cbData.s = view.mousePos - view.selectionStart;
		_cbData.color1 = _colorScheme.lines;
		V_RETURN(_updateCB(context));

		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);
		context->VSSetShader(_vsPlain, nullptr, 0);
		context->PSSetShader(_psColored, nullptr, 0);
		context->Draw(5, 0);
		
		unsigned n = unsigned(view.sChips.size() + view.sFolders.size() + view.sLinks.size() + view.sBackgroundItems.size());
		if (n > 0 && view.selectionStart != view.mousePos) {
			Vector2 cd = view.mousePos * vpScale + vpPos + Vector2(1, -14);
			_textWriter->Write(context, cd, TextWriter::LEFT_TOP, false, _colorScheme.lines, strUtils::fromNum(n) + (n == 1 ? MTEXT(" item selected") : MTEXT(" items selected")) );
		}
	}

	// Reset common state after drawing text
	context->VSSetConstantBuffers(0, 1, &(const SID3D11Buffer&)_cb);
	context->PSSetConstantBuffers(0, 1, &(const SID3D11Buffer&)_cb);
	context->GSSetShader(nullptr, nullptr, 0);
	context->RSSetState(_rs);
	context->OMSetBlendState(_bsEnable, (const FLOAT*)&z, 0xFFFFFFFF);

	// Render dragging links
	if (view.dragStarts.size()) {
		V_RETURN(_prepareVertexBuffer(device, (UINT)(view.dragStarts.size() * sizeof(VLink))));

		D3D11_MAPPED_SUBRESOURCE vbMapped;
		V_RETURN(context->Map(_vb, 0, D3D11_MAP_WRITE_DISCARD, 0, &vbMapped));
		VLink *vb = (VLink*)vbMapped.pData;
		unsigned mCount = 0;
		for (unsigned i = 0; i < view.dragStarts.size(); i++, mCount++) {
			if (view.GetDragMode() == DM_TOPCONN) {
				vb[mCount].a = view.mousePos;
				vb[mCount].b = view.dragStarts[i];
			}
			else {
				vb[mCount].a = view.dragStarts[i];
				vb[mCount].b = view.mousePos;
			}
			vb[mCount].c = _colorScheme.lines.RGBA();
			vb[mCount].d = 0;
			vb[mCount].e = 0;
		}
		context->Unmap(_vb, 0);

		_cbData.scaling = Vector4(VERTICES_PR_LINK - 1, 0, 0, 0);
		V_RETURN(_updateCB(context));

		UINT strides = sizeof(VLink);
		UINT offset = 0;
		context->IASetVertexBuffers(0, 1, &(const SID3D11Buffer&)_vb, &strides, &offset);
		context->IASetInputLayout(_il);
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);
		context->VSSetShader(_vs2DLines, nullptr, 0);
		context->PSSetShader(_psVertexColor, nullptr, 0);
		context->PSSetShaderResources(0, 1, &(const SID3D11ShaderResourceView&)_srvMeteor);
		context->PSSetSamplers(0, 1, &(const SID3D11SamplerState&)_wrapSampler);
		context->DrawInstanced(VERTICES_PR_LINK, mCount, 0, 0);
	}

	context->IASetVertexBuffers(0, 1, &(const SID3D11Buffer&)_vbStatic, &strides, &offset);
	context->IASetInputLayout(nullptr);

	// Render semi-transparent rectangle when dragging in a template 
	if (templateExtent) {
		Vector2 a(templateExtent->x, templateExtent->y), b(templateExtent->z, templateExtent->w);
		_cbData.p = view.mousePos + (a + b) * 0.5f;
		_cbData.s =  b - a;
		_cbData.color1 = _colorScheme.templateExtent;
		V_RETURN(_updateCB(context));

		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
		context->VSSetShader(_vsSquare, nullptr, 0);
		context->PSSetShader(_psColored, nullptr, 0);
		context->Draw(4, 0);
	}

	if (EditorState::state == EditorState::PAUSE || EditorState::state == EditorState::BREAKPOINT) {
		_cbData.vp = Vector4(-view.w * 0.5f, -view.h * 0.5f, 2.0f / view.w, 2.0f / view.h); // Origo is down-left.
		_cbData.p = Vector2(80.0f, view.h - 90.0f);
		_cbData.s = Vector2(128.0f, 128.0f);
		_cbData.color1 = Color(1.0f, 1.0f, 1.0f, 0.3f);
		V_RETURN(_updateCB(context));

		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
		context->VSSetShader(_vsSquare, nullptr, 0);
		context->PSSetShader(_psTextured, nullptr, 0);
		context->PSSetShaderResources(0, 1, &(const SID3D11ShaderResourceView&)_srvPause);
		context->Draw(4, 0);
	}

	if (isLibrary) {
		_cbData.vp = Vector4(-view.w * 0.5f, -view.h * 0.5f, 2.0f / view.w, 2.0f / view.h); // Origo is down-left.
		_cbData.p = Vector2(view.w * 0.5f, 90.0f);
		_cbData.s = Vector2(1024.0f, 128.0f);
		_cbData.color1 = Color(1.0f, 1.0f, 1.0f, 0.3f);
		V_RETURN(_updateCB(context));

		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
		context->VSSetShader(_vsSquare, nullptr, 0);
		context->PSSetShader(_psTextured, nullptr, 0);
		context->PSSetShaderResources(0, 1, &(const SID3D11ShaderResourceView&)_srvLibrary);
		context->Draw(4, 0);
	}

	if (hasFocus) {
		_cbData.vp = Vector4(-view.w * 0.5f, -view.h * 0.5f, 2.0f / view.w, 2.0f / view.h); // Origo is down-left.
		_cbData.color1 = Color(clearColor.R() * 2.0f, clearColor.G() * 2.0f, clearColor.B() * 2.0f, 0.8f);

		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
		context->VSSetShader(_vsSquare, nullptr, 0);
		context->PSSetShader(_psColored, nullptr, 0);
		context->PSSetShaderResources(0, 1, &(const SID3D11ShaderResourceView&)_srvLibrary);

		_cbData.p = Vector2(0.0f, 1.0f);
		_cbData.s = Vector2(view.w * 2.0f, 2.0f);
		V_RETURN(_updateCB(context));
		context->Draw(4, 0);

		_cbData.p = Vector2(0.0f, view.h - 1.0);
		V_RETURN(_updateCB(context));
		context->Draw(4, 0);

		_cbData.p = Vector2(1.0f, 0.0f);
		_cbData.s = Vector2(2.0f, view.h * 2.0f);
		V_RETURN(_updateCB(context));
		context->Draw(4, 0);

		_cbData.p = Vector2(view.w - 1.0f, 0.0f);
		V_RETURN(_updateCB(context));
		context->Draw(4, 0);
	}

	// Render info box
	V_RETURN(_renderInfoBox(context, view));

	return S_OK;
}

HRESULT ClassRenderer2::_renderChip(ID3D11Device *device, ID3D11DeviceContext *context, ClassView &view, Chip *chip)
{
	HRESULT hr;

	ChipEditorData *cv = chip->GetChipEditorData();

	// The original chip if shortcut. if not, it is equal to chip.
	Chip *orgChip = chip->AsShortcut() && chip->AsShortcut()->GetOriginal() ? chip->AsShortcut()->GetOriginal() : chip;
	ChipEditorData *orgCV = orgChip->GetChipEditorData();

	Color color1, color2, color3(0.0f, 0.0f, 0.0f, 0.0f);

	// Get fill color
	if (orgChip->GetFunction()) {
		switch (orgChip->GetFunction()->GetType())
		{
		case Function::Type::Static:
			color1 = _colorScheme.staticFunc;
			break;
		case Function::Type::NonVirtual:
			color1 = _colorScheme.nonvirtualFunc;
			break;
		case Function::Type::Virtual:
			color1 = _colorScheme.virtualFunc;
			break;
		};
	}
	else {
		color1 = ((Color*)&_colorScheme)[orgCV->colorIndex];
		// Check for library chip in relation graph:
		if (orgCV->colorIndex == ColorScheme::CI_RG_CHIP) {
			ClassDiagramChip *classDiagramChip = dynamic_cast<ClassDiagramChip*>(orgChip);
			if (classDiagramChip && classDiagramChip->GetCG()) {
				Document* doc = classDiagramChip->GetCG()->GetDocument();
				bool isLibrary = doc && doc->GetFileName().IsFile() && !engine->GetEnvironment()->IsPathInsideProjectRootFolder(doc->GetFileName()) && engine->GetEnvironment()->IsPathInsideLibraryFolder(doc->GetFileName());
				if (isLibrary)
					color1 = ((Color*)&_colorScheme)[ColorScheme::CI_BACKGROUND + 2]; // background color for libraries..
			}
		}
	}

	// Do the chip have two background colors? 
	if (orgChip->GetFunction() && orgCV->colorIndex != ColorScheme::CI_CHIP)
		color2 = ((Color*)&_colorScheme)[cv->colorIndex]; // Set the second (Lower left) background color
	else
		color2 = color1;

	// Calculate 'activeness' [0-1]
	float32 tDiff = (float32)(_time - chip->GetLastHitTime()) * 0.001f; //  / (float32)CLOCKS_PER_SEC;
	tDiff = std::min(std::max(tDiff, 0.0f), 1.0f);

	// Calculate border color
	if (tDiff < 0.999f || cv->selected) {
		color3 = Color(_colorScheme.active.x, _colorScheme.active.y, _colorScheme.active.z, 1.0f - tDiff);
		if (cv->selected)
			color3 = _colorScheme.selected + Color(color3.x, color3.y, color3.z, 0.0f) * (1.0f - tDiff);
	}

	if (chip->AsShortcut()) { // shortcut?
		color1 *= _colorScheme.chipShortcutFactor;
		color2 *= _colorScheme.chipShortcutFactor;
	}
	if (cv->hm == HM_CHIP) { // Hover?
		color1 *= _colorScheme.hoverFactor;
		color2 *= _colorScheme.hoverFactor;
		color3 *= _colorScheme.hoverFactor;
	}

	// Update CB
	_cbData.p = cv->pos;
	_cbData.s = Vector2(cv->width, CHIP_HEIGHT);
	_cbData.scaling = Vector4(0.25f);
	_cbData.color1 = color1;
	_cbData.color2 = color2;
	_cbData.color3 = color3;
	V_RETURN(_updateCB(context));

	// Render chip
	ID3D11ShaderResourceView *textures[3] = {_srvChip, _srvChipHalf, _srvChipBorder};
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	context->VSSetShader(_vsChip, nullptr, 0);
	context->PSSetShader(_psChip, nullptr, 0);
	context->PSSetShaderResources(0, 3, textures);
	context->Draw(54, 0);

	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	context->VSSetShader(_vsSquare, nullptr, 0);
	context->PSSetShader(_psTextured, nullptr, 0);

	// Render Refresh Icon
	if (orgChip->GetRefreshManager().GetRefreshMode() != RefreshManager::RefreshMode::OncePerFunctionCall) {
		_cbData.s = Vector2(CHIP_HEIGHT * 0.7f, CHIP_HEIGHT * 0.7f);
		switch (orgChip->GetRefreshManager().GetRefreshMode()) 
		{
		case RefreshManager::RefreshMode::Always:
			_cbData.color1 = Color(0.2f, 0.2f, 1.0f, 0.7f); // SYMBOL COLOR
			context->PSSetShaderResources(0, 1, &(const SID3D11ShaderResourceView&)_srvRefresh);
			break;
		case RefreshManager::RefreshMode::OncePerFrame:
			_cbData.color1 = Color(0.2f, 0.2f, 1.0f, 0.7f); // SYMBOL COLOR
			context->PSSetShaderResources(0, 1, &(const SID3D11ShaderResourceView&)_srvFrame);
			break;
		case RefreshManager::RefreshMode::Once:
			_cbData.color1 = Color(0.2f, 0.2f, 1.0f, 0.7f); // SYMBOL COLOR
			context->PSSetShaderResources(0, 1, &(const SID3D11ShaderResourceView&)_srvOnce);
			break;
		case RefreshManager::RefreshMode::Never:
			_cbData.color1 = Color(0.2f, 0.2f, 1.0f, 0.7f); // SYMBOL COLOR
			context->PSSetShaderResources(0, 1, &(const SID3D11ShaderResourceView&)_srvConstant);
		default:
			assert(false);
		}
		V_RETURN(_updateCB(context));
		context->Draw(4, 0);
	}

	// Render function access icon
	if (orgChip->GetFunction() && orgChip->GetFunction()->GetAccess() != Function::Access::Public) {
		_cbData.s = Vector2(CHIP_HEIGHT * 0.7f, CHIP_HEIGHT * 0.7f);
		_cbData.color1 = Color(1.0f, 1.0f, 1.0f, 0.3f); // SYMBOL COLOR
		V_RETURN(_updateCB(context));
		if (orgChip->GetFunction()->GetAccess() == Function::Access::Private)
			context->PSSetShaderResources(0, 1, &(const SID3D11ShaderResourceView&)_srvShield);
		else
			context->PSSetShaderResources(0, 1, &(const SID3D11ShaderResourceView&)_srvPadlock);
		context->Draw(4, 0);
	}

	// Render message icon
	if (orgChip->HasMessages()) {
		_cbData.p = Vector2(cv->pos.x + cv->width * 0.5f - 0.3f, cv->pos.y - CHIP_HEIGHT * 0.5f + 0.3f);
		_cbData.s = Vector2(CHIP_HEIGHT * 0.35f, CHIP_HEIGHT * 0.35f);
		_cbData.color1 = Color(1.0f, 1.0f, 1.0f, 1.0f);
		V_RETURN(_updateCB(context));
		const ChipMessageList *l = orgChip->GetMessages();
		MessageSeverity s = DINFO;
		for (size_t i = 0; i < l->size(); i++) {
			s = std::max(l->at(i).severity, s);
		}
		switch (s) 
		{
		case DINFO:
		case INFO:
			context->PSSetShaderResources(0, 1, &(const SID3D11ShaderResourceView&)_srvInfo);
			break;
		case NOTICE:
			context->PSSetShaderResources(0, 1, &(const SID3D11ShaderResourceView&)_srvNotice);
			break;
		case WARN:
			context->PSSetShaderResources(0, 1, &(const SID3D11ShaderResourceView&)_srvWarning);
			break;
		case FATAL:
		default:
			context->PSSetShaderResources(0, 1, &(const SID3D11ShaderResourceView&)_srvError);
			break;
		}
		context->Draw(4, 0);
	}

	// Render Comment icon
	if (orgChip->GetChipEditorData()->comment.size()) {
		_cbData.p = Vector2(cv->pos.x - cv->width * 0.5f + 0.3f, cv->pos.y - CHIP_HEIGHT * 0.5f + 0.3f);
		_cbData.s = Vector2(CHIP_HEIGHT * 0.35f, CHIP_HEIGHT * 0.35f);
		_cbData.color1 = Color(0.0f, 0.0f, 0.0f, 0.7f);
		V_RETURN(_updateCB(context));
		context->PSSetShaderResources(0, 1, &(const SID3D11ShaderResourceView&)_srvNote);
		context->Draw(4, 0);
	}

	// Render shortcut icon
	if (chip->AsShortcut()) {
		_cbData.p = Vector2(cv->pos.x - cv->width * 0.5f + 0.3f, cv->pos.y - CHIP_HEIGHT * 0.5f + 0.3f);
		_cbData.s = Vector2(CHIP_HEIGHT * 0.35f, CHIP_HEIGHT * 0.35f);
		_cbData.color1 = Color(1.0f, 1.0f, 1.0f, 1.0f);
		V_RETURN(_updateCB(context));
		context->PSSetShaderResources(0, 1, &(const SID3D11ShaderResourceView&)_srvShortcut);
		context->Draw(4, 0);
	}

	// Calculate number of connectors!
	UINT cCount = 1;
	for (unsigned i = 0; i < chip->GetChildren().size(); i++) {
		ChildConnection *cc = chip->GetChildren()[i];
		if (cc)
			cCount += cc->desc.connType == ChildConnectionDesc::GROWING ? ((UINT)cc->connections.size() + 1) : 1;
	}

	// Map vertex buffer
	V_RETURN(_prepareVertexBuffer(device, (UINT)(cCount * sizeof(VConnectors))));
	D3D11_MAPPED_SUBRESOURCE vbMapped;
	V_RETURN(context->Map(_vb, 0, D3D11_MAP_WRITE_DISCARD, 0, &vbMapped));
	VConnectors *vb = (VConnectors*)vbMapped.pData;
	UINT vCount = 1;
	
	Vector2 scale =  Vector2(CONNECTION_WIDTH - CONNECTION_SPACE, CONNECTION_HEIGHT + CONNECTION_YPAD * 2.0f);

	// Add top connector
	{
		if (view.dm == DM_TOPCONN || view.dm == DM_CHILDCONN) {
			if (view.dm == DM_CHILDCONN && engine->GetChipManager()->IsChipTypeSupported(view.dragGuid, chip->GetChipTypeIndex()) && orgChip->GetID() != view.connDragChip)
				color1 = _colorScheme.canLink;
			else 
				color1 = _colorScheme.cantLink;
		}
		else {
			color1 = _colorScheme.connectors;
			if (tDiff < 0.999f)
				color1 = Color::Lerp(_colorScheme.active, color1, tDiff);
		}

		if (cv->hm == HM_TOPCONN) // Hover?
			color1 *= _colorScheme.hoverFactor;

		vb[0].pos = Vector2(cv->pos.x, cv->pos.y + CHIP_HALF_HEIGHT + CONNECTION_HEIGHT * 0.5f - CONNECTION_YPAD);
		vb[0].scale = scale;
		vb[0].col = color1.RGBA();
		vb[0].tc0 = 0; // <= Offset for the top connection in the texture (0.0)
	}

	// Add sub connectors
	float32 pos = cv->pos.x - cv->width * 0.5f + CHIP_HALF_HEIGHT + CONNECTION_WIDTH * 0.5f;
	float32 yPos = cv->pos.y - CHIP_HALF_HEIGHT - CONNECTION_HEIGHT * 0.5f + CONNECTION_YPAD;
	for (unsigned i = 0, k = 0; i < chip->GetChildren().size(); i++, k++, pos += CONNECTION_WIDTH) {
		ChildConnection *cc = chip->GetChildren()[i];
		if (!cc)
			continue;

		bool overrideColor = false;
		if (overrideColor = (view.dm == DM_TOPCONN || view.dm == DM_CHILDCONN)) { // Dragging from a connector?
			// TODO: When dragging from topconnection of a shortcut, child connections of the main chip can be green! Need a fix!
			if (view.dm == DM_TOPCONN && engine->GetChipManager()->IsChipTypeSupported(cc->chipTypeIndex, view.dragGuid) && chip->GetID() != view.connDragChip)
				color1 = _colorScheme.canLink;
			else
				color1 = _colorScheme.cantLink;
		}
		else {
			if (cc->desc.connType == ChildConnectionDesc::SINGLE)
				color1 = _colorScheme.connectors;
			else if (cc->desc.connType == ChildConnectionDesc::GROWING)
				color1 = _colorScheme.growingConnectors;
			else
				color1 = _colorScheme.multiConnector;
		}

		if (cc->desc.connType == ChildConnectionDesc::GROWING) {
			for (unsigned j = 0; j < cc->connections.size(); j++, k++, pos += CONNECTION_WIDTH) {
				Color c = color1;
				if (!overrideColor) {
					float32 tDiff = (float32)(_time - cc->connections[j].lastHit) * 0.001f; // (float32)CLOCKS_PER_SEC;
					tDiff = std::min(std::max(tDiff, 0.0f), 1.0f);
					c = Color::Lerp(_colorScheme.active, c, tDiff);
				}
				if (cv->hm == HM_CHILDCONN && view.hoverConn == k)
					c *= _colorScheme.hoverFactor;

				vb[vCount].pos = Vector2(pos, yPos);
				vb[vCount].scale = scale;
				vb[vCount].col = c.RGBA();
				vb[vCount].tc0 = 64; // <= Offset for the child connection in the texture (0.25)
				vCount++;
			}

			// Add final ... connector!
			vb[vCount].pos = Vector2(pos, yPos);
			vb[vCount].scale = scale;
			vb[vCount].col = color1.RGBA();
			vb[vCount].tc0 = 128; // <= Offset for the growing child connection in the texture (0.5)
			vCount++;
		}
		else { // Nongrowing
			if (!overrideColor) {
				float32 tDiff = (float32)(_time - (cc->connections.size() ? cc->connections[0].lastHit : 0)) * 0.001f; // (float32)CLOCKS_PER_SEC;
				tDiff = std::min(std::max(tDiff, 0.0f), 1.0f);
				color1 = Color::Lerp(_colorScheme.active, color1, tDiff);
			}
			if (cv->hm == HM_CHILDCONN && view.hoverConn == k)
				color1 *= _colorScheme.hoverFactor;

			vb[vCount].pos = Vector2(pos, yPos);
			vb[vCount].scale = scale;
			vb[vCount].col = color1.RGBA();
			vb[vCount].tc0 = 64; // <= Offset for the child connection in the texture (0.25)
			vCount++;
		}
	}
	
	context->Unmap(_vb, 0);

	// Render all connectors at once!
	UINT strides = sizeof(VConnectors);
	UINT offset = 0;
	context->IASetVertexBuffers(0, 1, &(const SID3D11Buffer&)_vb, &strides, &offset);
	context->IASetInputLayout(_ilConnectors);
	context->VSSetShader(_vsConnectors, nullptr, 0);
	context->PSSetShader(_psTexturedWithVertexColor, nullptr, 0);
	context->PSSetShaderResources(0, 1, &(const SID3D11ShaderResourceView&)_srvConnectors);
	context->DrawInstanced(4, vCount, 0, 0);
	
	context->IASetInputLayout(nullptr);

	return S_OK;
}

HRESULT ClassRenderer2::_renderFolder(ID3D11Device *device, ID3D11DeviceContext *context, ClassView &view, const Folder &folder)
{
	HRESULT hr;

	Vector2 position = view.folder == folder.parentFolder ? folder.pos : folder.inPos;

	_cbData.p = position;
	_cbData.s = Vector2(CHIP_HEIGHT, CHIP_HEIGHT);
	_cbData.scaling = Vector4(0.25f);
	_cbData.color1 =  _colorScheme.folder;
	_cbData.color2 =  _colorScheme.folder;
	_cbData.color3 = folder.selected ? _colorScheme.selected : Color(0.0f, 0.0f, 0.0f, 0.0f);
	if (folder.hover) {
		_cbData.color1 *= _colorScheme.hoverFactor;
		_cbData.color2 *= _colorScheme.hoverFactor;
		_cbData.color3 *= _colorScheme.hoverFactor;
	}
	V_RETURN(_updateCB(context));
	
	ID3D11ShaderResourceView *textures[3] = {_srvChip, _srvChipHalf, _srvChipBorder};
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	context->IASetInputLayout(nullptr);
	context->VSSetShader(_vsChip, nullptr, 0);
	context->PSSetShader(_psChip, nullptr, 0);
	context->PSSetShaderResources(0, 3, textures);
	context->Draw(54, 0);

	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	context->VSSetShader(_vsSquare, nullptr, 0);
	context->PSSetShader(_psTextured, nullptr, 0);

	// Render icon
	{
		_cbData.s = Vector2(CHIP_HEIGHT * 0.7f, CHIP_HEIGHT * 0.7f);
		_cbData.color1 *= Color(0.4f, 0.4f, 0.4f, 1.0f);
		V_RETURN(_updateCB(context));
		context->PSSetShaderResources(0, 1, &(const SID3D11ShaderResourceView&)_srvFolder);
		context->Draw(4,0);
	}

	// Map vertex buffer
	V_RETURN(_prepareVertexBuffer(device, (UINT)(2 * sizeof(VConnectors))));
	D3D11_MAPPED_SUBRESOURCE vbMapped;
	V_RETURN(context->Map(_vb, 0, D3D11_MAP_WRITE_DISCARD, 0, &vbMapped));
	VConnectors *vb = (VConnectors*)vbMapped.pData;

	Vector2 scale =  Vector2(CONNECTION_WIDTH - CONNECTION_SPACE, CONNECTION_HEIGHT);

	// Add top connection
	vb[0].pos = Vector2(position.x, position.y + CHIP_HALF_HEIGHT + CONNECTION_HEIGHT * 0.5f);
	vb[0].scale = scale;
	vb[0].col = _colorScheme.connectors.RGBA();
	vb[0].tc0 = 0; // <= Offset for the top connection in the texture (0.0)

	// Add sub connection
	vb[1].pos = Vector2(position.x, position.y - CHIP_HALF_HEIGHT - CONNECTION_HEIGHT * 0.5f);
	vb[1].scale = scale;
	vb[1].col = _colorScheme.connectors.RGBA();
	vb[1].tc0 = 64; // <= Offset for the child connection in the texture (0.25)

	context->Unmap(_vb, 0);

	// Render both connectors at once!
	UINT strides = sizeof(VConnectors);
	UINT offset = 0;
	context->IASetVertexBuffers(0, 1, &(const SID3D11Buffer&)_vb, &strides, &offset);
	context->IASetInputLayout(_ilConnectors);
	context->VSSetShader(_vsConnectors, nullptr, 0);
	context->PSSetShader(_psTexturedWithVertexColor, nullptr, 0);
	context->PSSetShaderResources(0, 1, &(const SID3D11ShaderResourceView&)_srvConnectors);
	context->DrawInstanced(4, 2, 0, 0);

	context->IASetInputLayout(nullptr);

	return S_OK;
}

HRESULT ClassRenderer2::_renderInfoBox(ID3D11DeviceContext *context, ClassView &view)
{
	HRESULT hr;
	if (!view.infoBox.show)
		return S_OK;

	const float32 SCALE = (float32)_font->GetDPI() / 96.0f;

	const float32 AM = 4.0f * SCALE; // Left column
	const float32 BM = 90.0f * SCALE; // Right column
	const float32 CM = 150.0f * SCALE; // Rightest column
	const float32 PERF_EXTRA = 87.0f * SCALE; // Vertical size of perf section.
	const XMFLOAT4 texcolor1(0.0f, 0.0f ,0.0f ,1.0f);
	const XMFLOAT4 texcolor2(0.2f, 0.2f ,0.2f ,1.0f);
	const XMFLOAT4 texcolor3(1.0f, 1.0f, 1.0f ,1.0f);
	const XMFLOAT4 texcolor4(1.0f, 0.0f ,0.0f ,1.0f);

	float32 TOP_SECTION_HEIGHT = 20.0f * SCALE;

	Chip *chip = view.clazz->GetChip(view.infoBox.hoverCh);
	if (!chip)
		return S_OK;

	ChipEditorData *cv = chip->GetChipEditorData();

	if (chip->AsShortcut()) {
		Chip *c = chip->AsShortcut()->GetOriginal();
		if (c) {
			chip = c;
			cv = c->GetChipEditorData();
		}
	}

	assert(cv);

	ChipTypeIndex type;
	String name;
	List<String> typeNames;
	if (view.infoBox.hm == HM_CHILDCONN) { // Mouse is over child connection?
		ConnectionID cid = cv->GetConnectionID(view.infoBox.hoverConn);
		assert(cid != InvalidConnectionID);
		if (cid.first >= chip->GetChildren().size())
			return S_OK;
		ChildConnection *cc = chip->GetChildren()[cid.first];
		if (!cc)
			return S_OK;
		name = cc->desc.name;
		type = cc->chipTypeIndex;
		
		ChildConnectionDesc::DataDirection dir = cc->desc.dataDirection;
		if (dir == ChildConnectionDesc::UP)
			name = MTEXT("[IN] ") + name;
		else if (dir == ChildConnectionDesc::DOWN)
			name = MTEXT("[OUT] ") + name;
	}
	else { // Mouse is over top connection or chip itself!
		name = chip->GetName();
		type = chip->GetChipTypeIndex();
	}

	if (name.length() > MAX_NAME_LENGTH)
		name = name.substr(0, MAX_NAME_LENGTH - 3) + MTEXT("..."); // Crop length of name

	// Build list of type names!
	const ChipInfo *nfo = engine->GetChipManager()->GetChipInfo(type);
	while (true) {
		typeNames.push_back(nfo->chipDesc.name);
		if (nfo->chipDesc.basetype == nfo->chipDesc.type || view.infoBox.hm == HM_CHILDCONN) // for child connections we only show the first type...
			break;
		nfo = engine->GetChipManager()->GetChipInfo(nfo->chipDesc.basetype);
	}

	if (typeNames.size() > 1)
		typeNames.pop_back(); // remove the 'obvious' type "Chip"

	// Are the chip a proxy type?
	if (view.infoBox.hm != HM_CHILDCONN) {
		if (engine->GetChipManager()->GetChipTypeIndex(chip->GetChipDesc().type) != type) {
			typeNames[0] += MTEXT("*");
			typeNames.push_back(MTEXT("(") + String(chip->GetChipDesc().name) + MTEXT(")"));
		}
	}

	float32 boxWidth = 0.0f;

	// Find the width neccessary for name and types!
	{
		int32 w = 0;
		for (unsigned i = 0; i < typeNames.size(); i++) {
			int32 tmp = _font->GetWidth(typeNames[i]);
			w = std::max(w, tmp);
		}
		int32 tmp = _font->GetWidth(name);
		boxWidth = std::max(BM + w, TOP_SECTION_HEIGHT + tmp);
	}

	// p0: Hit count
	// p1/q1: Call time (div by frames)
	// p2/q2: Fraction view
	// p3/p3: Call time excluding sub calls (div by frames)
	// p4/q4: Fraction view
	String p0, p1, p2, p3, p4, q1, q2, q3, q4;
	Function *function = chip->GetFunction();
	bool perf = functionStack.GetPrefMon() != FunctionStack::PerfMon::PERF_NONE && view.infoBox.hm == HM_CHIP && function; // Performance monitoring?
	if (perf) {
		if (function->GetLastFrame() == functionStack.GetPerfFrame() && function->GetHitCount() > 0 && functionStack.GetPerfTime() > 0 && functionStack.GetPerfFrameCount() > 0) {
			p0 = strUtils::fromNum(function->GetHitCount());
			int32 time1US = (int32)(function->GetCallTime() * 1000000.0 / functionStack.GetQFreq());
			int32 time2US = time1US / (int32)function->GetHitCount();
			time1US /= functionStack.GetPerfFrameCount();
			q1 = strUtils::fromNum(time1US > 1000 ? (time1US / 1000) : time1US) + (time1US > 1000 ? MTEXT(" ms") : MTEXT(" µs"));
			p1 = strUtils::fromNum(time2US > 1000 ? (time2US / 1000) : time2US) + (time2US > 1000 ? MTEXT(" ms") : MTEXT(" µs"));
			int32 time1Frac = (int32)(function->GetCallTime() * 1000 / functionStack.GetPerfTime());
			int32 time2Frac = time1Frac * functionStack.GetPerfFrameCount() / (int32)function->GetHitCount();
			q2 = strUtils::fromNum(time1Frac > 10 ? (time1Frac / 10) : time1Frac)  + (time1Frac > 10 ? MTEXT("%") : MTEXT("‰"));
			p2 = strUtils::fromNum(time2Frac > 10 ? (time2Frac / 10) : time2Frac)  + (time2Frac > 10 ? MTEXT("%") : MTEXT("‰"));
			int32 time3US = (int32)(function->GetCallTimeExclSubFunc() * 1000000.0 / functionStack.GetQFreq());
			int32 time4US = time3US / (int32)function->GetHitCount();
			time3US /= functionStack.GetPerfFrameCount();
			q3 = strUtils::fromNum(time3US > 1000 ? (time3US / 1000) : time3US) + (time3US > 1000 ? MTEXT(" ms") : MTEXT(" µs"));
			p3 = strUtils::fromNum(time4US > 1000 ? (time4US / 1000) : time4US) + (time4US > 1000 ? MTEXT(" ms") : MTEXT(" µs"));
			int32 time3Frac = (int32)(function->GetCallTimeExclSubFunc() * 1000 / functionStack.GetPerfTime());
			int32 time4Frac = time3Frac * functionStack.GetPerfFrameCount() / (int32)function->GetHitCount();
			p4 = strUtils::fromNum(time4Frac > 10 ? (time4Frac / 10) : time4Frac)  + (time4Frac > 10 ? MTEXT("%") : MTEXT("‰"));
			q4 = strUtils::fromNum(time3Frac > 10 ? (time3Frac / 10) : time3Frac)  + (time3Frac > 10 ? MTEXT("%") : MTEXT("‰"));
		}
		else {
			p0 = String(MTEXT("0")); // No hits!
		}
		int32 n1 = _font->GetWidth(q1);
		int32 n2 = _font->GetWidth(q2);
		int32 n3 = _font->GetWidth(q3);
		float32 n = std::max((float32)std::max(n1, std::max(n2, n3)), 7.0f * SCALE);
		boxWidth = std::max(boxWidth, CM + n); // Calculate the new box with based on the "rightest" column.
	}

#define SECTION_SPACE (8.0f * SCALE) // Vertical distance between text sections
#define TEXT_SECTION_MAX_WIDTH (500.0f * SCALE) // Max width of text blocks

	List<std::pair<int32, String>> errorTxts;
	int32 ERROR_EXTRA = 0; // Vertical size of error messages

	String commentTxt;
	int32 COMMENT_EXTRA = 0; // Vertical size of comment section

	if (view.infoBox.hm == HM_CHILDCONN) { // Mouse over child connection?
		if (chip->AsFunctionCall()) { // Is the chip a function calls?
			Function *f = chip->AsFunctionCall()->GetFunction();
			if (f) {
				ConnectionID cid = cv->GetConnectionID(view.infoBox.hoverConn);
				if (cid.first > 0 && cid.second == 0) {
					for (const auto &n : f->GetParameters()) {
						if (n.c == cid.first - 1) {
							if (n.p)
								commentTxt = n.p->GetChipEditorData()->comment;
							break;
						}
					}
				}
			}
		}
	}
	else { // No. mouse is over chip or top connection.
		if (chip->HasMessages()) {
			const ChipMessageList *msgs = chip->GetMessages();
			static const String SEV[5] = {MTEXT("Info: "), MTEXT("Info: "), MTEXT("Notice: "), MTEXT("Warning: "), MTEXT("Error: ")};
			for (size_t i = 0; i < std::min(msgs->size(), 3ull); i++) { // Iterate all messages
				int32 w = (int32)TEXT_SECTION_MAX_WIDTH;
				int32 errorTxtLines = 4; // <= Max error lines!
				const ChipMessage &m = msgs->at(i);
				String s = _font->FormatText(SEV[std::min(m.severity, (MessageSeverity)4)] + m.msg, w, errorTxtLines); // This will crop the text if too long!
				errorTxts.push_back(std::make_pair(ERROR_EXTRA + (int32)SECTION_SPACE, s));
				boxWidth = std::max(boxWidth, AM + w);
				ERROR_EXTRA += int32(SECTION_SPACE + _font->GetGlyphHeight() * errorTxtLines);
			}
			if  (msgs->size() > 3) { // Too many messages?
				int32 w = (int32)TEXT_SECTION_MAX_WIDTH;
				int32 errorTxtLines = 4; // <= Max error lines!
				String s = _font->FormatText(strUtils::ConstructString(msgs->size() == 4 ? MTEXT("+ 1 more message...") : MTEXT("+ %1 more messages...")).arg(strUtils::fromNum((unsigned)msgs->size() - 3)), w, errorTxtLines); // This will crop the text if too long!
				errorTxts.push_back(std::make_pair(ERROR_EXTRA + (int32)SECTION_SPACE, s));
				boxWidth = std::max(boxWidth, AM + w);
				ERROR_EXTRA += int32(SECTION_SPACE + _font->GetGlyphHeight() * errorTxtLines);
			}
		}
		
		commentTxt = chip->GetChipEditorData()->comment; // Chip comment.
		if (chip->AsFunctionCall()) { // If the chip is a function call, we include the comment of the function!
			Function *f = chip->AsFunctionCall()->GetFunction();
			if (f) {
				Chip *c = f->GetChip();
				if (c) {
					String s = c->GetChipEditorData()->comment;
					if (s.size()) {
						if (commentTxt.size())
							commentTxt += MTEXT("\n\n");
						commentTxt += MTEXT("Function Comment:\n") + s;
					}
				}
			}
		}
	}

	// Calculate comment measures.
	if (!commentTxt.empty()) {
		int32 w = (int32)TEXT_SECTION_MAX_WIDTH;
		int32 commentTxtLines = 10; // <= Max comment lines!
		commentTxt = _font->FormatText(commentTxt, w, commentTxtLines);
		boxWidth = std::max(boxWidth, AM + w);
		COMMENT_EXTRA = int32(SECTION_SPACE + _font->GetGlyphHeight() * commentTxtLines); // Calculate vertical size of comment section.
	}

	const float32 EXTRA_SPACE = 4.0f * SCALE;
	const float32 TYPE_LINE_HEIGHT = 16.0f * SCALE;
	const float32 BOX_Y_OFFSET = -40.0f;

	// Calculate size of box.
	Vector2 s(boxWidth + EXTRA_SPACE, TOP_SECTION_HEIGHT + EXTRA_SPACE + TYPE_LINE_HEIGHT * typeNames.size() + (perf ? PERF_EXTRA : 0.0f) + ERROR_EXTRA + COMMENT_EXTRA);

	Vector2 a(std::min(std::max(view.infoBox.mousePosVPInfo.x, 20.0f), view.w - s.x - 1.0f), std::min(std::max(view.h - view.infoBox.mousePosVPInfo.y - s.y + BOX_Y_OFFSET, 1.0f), view.h - s.y - 1.0f));
	Vector2 b = a + s;

	_cbData.vp = Vector4(-view.w * 0.5f, -view.h * 0.5f, 2.0f / view.w, 2.0f / view.h); // Origo is down-left.
	_cbData.p = (a + b) * 0.5f;
	_cbData.s = s + Vector2(32.0f); // 32 is the whitespace around the box in the texture.
	_cbData.scaling = Vector4(32.0f, 32.0f, 0.25f, 0.25f);
	_cbData.color1 =  _colorScheme.infoBox;
	V_RETURN(_updateCB(context));

	// Render box
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	context->VSSetShader(_vsChip, nullptr, 0);
	context->PSSetShader(_psTextured, nullptr, 0);
	context->PSSetShaderResources(0, 1, &(const SID3D11ShaderResourceView&)_srvInfoBox);
	context->Draw(54, 0);

	// Render top section
	_cbData.p = Vector2((a.x + b.x) * 0.5f, b.y - TOP_SECTION_HEIGHT * 0.5f);
	_cbData.s = Vector2(s.x, TOP_SECTION_HEIGHT);
	_cbData.color1 = Color(0.0f, 0.0f, 0.0f, 1.0f);
	V_RETURN(_updateCB(context));
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	context->VSSetShader(_vsSquare, nullptr, 0);
	context->PSSetShader(_psColored, nullptr, 0);
	context->Draw(4, 0);

	// Render icon
	_cbData.p = Vector2(a.x + TOP_SECTION_HEIGHT * 0.5f, b.y - TOP_SECTION_HEIGHT * 0.5f);
	_cbData.s = Vector2(16.0f) * SCALE;
	_cbData.color1 = Color(1.0f, 1.0f, 1.0f, 1.0f);
	V_RETURN(_updateCB(context));
	context->PSSetShader(_psTextured, nullptr, 0);
	context->PSSetShaderResources(0, 1, &(const SID3D11ShaderResourceView&)(view.infoBox.hm == HM_CHILDCONN ? _srvInfoCornerCo : _srvInfoCornerCh));
	context->Draw(4, 0);

	//V_RETURN(_textWriter->Write(context, XMFLOAT2(a.x + AM, view.h - b.y + 4.0f), TextWriter::LEFT_TOP, false, texcolor2, MTEXT("Name:")));
	V_RETURN(_textWriter->Write(context, XMFLOAT2(a.x + TOP_SECTION_HEIGHT, view.h - b.y + 2 * SCALE), TextWriter::LEFT_TOP, false, texcolor3, name));
	V_RETURN(_textWriter->Write(context, XMFLOAT2(a.x + AM, view.h - b.y + TOP_SECTION_HEIGHT + 2 * SCALE), TextWriter::LEFT_TOP, false, texcolor2, typeNames.size() > 1 ? MTEXT("Types:") : MTEXT("Type:")));
	for (unsigned i = 0; i < typeNames.size(); i++) {
		V_RETURN(_textWriter->Write(context, XMFLOAT2(a.x + BM, view.h - b.y + TOP_SECTION_HEIGHT + TYPE_LINE_HEIGHT * i + 2 * SCALE), TextWriter::LEFT_TOP, false, texcolor1, typeNames[i]));
	}

	float32 y = view.h - b.y + TOP_SECTION_HEIGHT + TYPE_LINE_HEIGHT * typeNames.size() + 2 * SCALE;

	if (perf) {
		V_RETURN(_textWriter->Write(context, XMFLOAT2(a.x + AM, y +  2 * SCALE), TextWriter::LEFT_TOP, false, texcolor2,  MTEXT("Hit Count:")));
		V_RETURN(_textWriter->Write(context, XMFLOAT2(a.x + AM, y + 19 * SCALE), TextWriter::LEFT_TOP, false, texcolor2, MTEXT("Sum Time:")));
		V_RETURN(_textWriter->Write(context, XMFLOAT2(a.x + AM, y + 53 * SCALE), TextWriter::LEFT_TOP, false, texcolor2, MTEXT("÷Fx-Calls:")));
		V_RETURN(_textWriter->Write(context, XMFLOAT2(a.x + BM, y +  2 * SCALE), TextWriter::LEFT_TOP, false, texcolor2, p0));
		V_RETURN(_textWriter->Write(context, XMFLOAT2(a.x + BM, y + 19 * SCALE), TextWriter::LEFT_TOP, false, texcolor2, p1));
		V_RETURN(_textWriter->Write(context, XMFLOAT2(a.x + BM, y + 36 * SCALE), TextWriter::LEFT_TOP, false, texcolor2, p2));
		V_RETURN(_textWriter->Write(context, XMFLOAT2(a.x + BM, y + 53 * SCALE), TextWriter::LEFT_TOP, false, texcolor2, p3));
		V_RETURN(_textWriter->Write(context, XMFLOAT2(a.x + BM, y + 70 * SCALE), TextWriter::LEFT_TOP, false, texcolor2, p4));
		V_RETURN(_textWriter->Write(context, XMFLOAT2(a.x + CM, y + 19 * SCALE), TextWriter::LEFT_TOP, false, texcolor2, q1));
		V_RETURN(_textWriter->Write(context, XMFLOAT2(a.x + CM, y + 36 * SCALE), TextWriter::LEFT_TOP, false, texcolor2, q2));
		V_RETURN(_textWriter->Write(context, XMFLOAT2(a.x + CM, y + 53 * SCALE), TextWriter::LEFT_TOP, false, texcolor2, q3));
		V_RETURN(_textWriter->Write(context, XMFLOAT2(a.x + CM, y + 70 * SCALE), TextWriter::LEFT_TOP, false, texcolor2, q4));
		y += PERF_EXTRA;
	}

	if (!errorTxts.empty()) {
		for (size_t i = 0; i < errorTxts.size(); i++) {
			V_RETURN(_textWriter->Write(context, XMFLOAT2(a.x + AM, y + errorTxts[i].first), TextWriter::LEFT_TOP, true, texcolor4, errorTxts[i].second));
		}
		y += ERROR_EXTRA;
	}

	if (!commentTxt.empty()) {
		V_RETURN(_textWriter->Write(context, XMFLOAT2(a.x + AM, y + SECTION_SPACE), TextWriter::LEFT_TOP, true, texcolor2, commentTxt));
		y += COMMENT_EXTRA;
	}

	return S_OK;
}
