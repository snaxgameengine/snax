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

#include "M3DCore/ComPtr.h"
#include "D3DInclude.h"
#include "GraphicsDefines.h"
#include "M3DCore/MString.h"
#include "M3DCore/Containers.h"

namespace m3d
{

class ClassView;
class Chip;

class Font;
class TextWriter;

struct Folder;


class ClassRenderer2
{
public:
	ClassRenderer2();
	~ClassRenderer2();

	// Destroy me on detroy device!
	HRESULT Init(ID3D11Device *device);
	void OnDestroyDevice();

	HRESULT Render(ID3D11DeviceContext *context, ClassView &view, int32 devicePixelRatio, const Vector4 *templateExtent, bool hasFocus);

	void FormatTextItem(String &str, Vector2 &s, float32 textSize);

protected:
	ColorScheme _colorScheme;

	bool _level10;

	Font *_font;
	Map<int32, Font> _fonts;
	TextWriter *_textWriter;

	Font *_font2;
	TextWriter *_textWriter2;

	SID3D11ShaderResourceView _srvChip;
	SID3D11ShaderResourceView _srvChipHalf;
	SID3D11ShaderResourceView _srvChipBorder;
	SID3D11ShaderResourceView _srvPadlock;
	SID3D11ShaderResourceView _srvShield;
	SID3D11ShaderResourceView _srvNote;
	SID3D11ShaderResourceView _srvShortcut;
	SID3D11ShaderResourceView _srvMeteor;
	SID3D11ShaderResourceView _srvFolder;
	SID3D11ShaderResourceView _srvStart;
	SID3D11ShaderResourceView _srvMove;
	SID3D11ShaderResourceView _srvScale;
	SID3D11ShaderResourceView _srvChDir;
	SID3D11ShaderResourceView _srvRefresh;
	SID3D11ShaderResourceView _srvOnce;
	SID3D11ShaderResourceView _srvFrame;
	SID3D11ShaderResourceView _srvInfoBox;
	SID3D11ShaderResourceView _srvInfoCornerCh;
	SID3D11ShaderResourceView _srvInfoCornerCo;
	SID3D11ShaderResourceView _srvConstant;
	SID3D11ShaderResourceView _srvInfo;
	SID3D11ShaderResourceView _srvNotice;
	SID3D11ShaderResourceView _srvWarning;
	SID3D11ShaderResourceView _srvError;
	SID3D11ShaderResourceView _srvConnectors;
	SID3D11ShaderResourceView _srvPause;
	SID3D11ShaderResourceView _srvLibrary;

	SID3D11BlendState _bsEnable;
	SID3D11BlendState _bsScreen;
	SID3D11RasterizerState _rs;

	SID3D11SamplerState _sampler;
	SID3D11SamplerState _wrapSampler;

	SID3D11InputLayout _il;
	SID3D11InputLayout _ilPlain;
	SID3D11InputLayout _ilConnectors;
	SID3D11Buffer _vb;
	SID3D11Buffer _vbStatic;
	UINT _vbSize;

	SID3D11VertexShader _vsSquare;
	SID3D11VertexShader _vsPlain;
	SID3D11VertexShader _vs2DLines;
	SID3D11VertexShader _vs2DLinesBox;
	SID3D11VertexShader _vs2DQuads;
	SID3D11VertexShader _vsChip;
	SID3D11VertexShader _vsConnectors;
	SID3D11PixelShader _psColored;
	SID3D11PixelShader _psTextured;
	SID3D11PixelShader _psTexturedWithVertexColor;
	SID3D11PixelShader _psChDir;
	SID3D11PixelShader _psChip;
	SID3D11PixelShader _psVertexColor;
	SID3D11PixelShader _psDataParticles;

	SID3D11Buffer _cb;

	struct CB
	{
		Vector2 p;
		Vector2 s;
		Vector4 vp;
		Vector4 scaling;
		Color color1;
		Color color2;
		Color color3;
		FLOAT time;
		Vector3 __padding2;
	};

	CB _cbData;

	int32 _time;

	HRESULT _updateCB(ID3D11DeviceContext *context);
	HRESULT _prepareVertexBuffer(ID3D11Device *device, UINT size);
	HRESULT _renderChip(ID3D11Device *device, ID3D11DeviceContext *context, ClassView &view, Chip *cv);
	HRESULT _renderFolder(ID3D11Device *device, ID3D11DeviceContext *context, ClassView &view, const Folder &folder);
	HRESULT _renderInfoBox(ID3D11DeviceContext *context, ClassView &view);

	// Functions for level9 device.
	HRESULT _init9(ID3D11Device *device);
	HRESULT _render9(ID3D11DeviceContext *context, ClassView &view, const Vector4 *templateExtent);
	HRESULT _renderChip9(ID3D11Device *device, ID3D11DeviceContext *context, ClassView &view, Chip *cv);
	HRESULT _renderFolder9(ID3D11Device *device, ID3D11DeviceContext *context, ClassView &view, const Folder &folder);
	HRESULT _renderInfoBox9(ID3D11DeviceContext *context, ClassView &view);

};


}
