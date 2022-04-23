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


#include <QWidget>
#include "GraphicsChips/D3D12Include.h"
#include "ui_TexturePreviewWidget.h"
#include "GraphicsChips/PipelineStatePool.h"
#include "GraphicsChips/DescriptorHeapManager.h"

namespace m3d
{


class TexturePreviewWidget : public QWidget
{
	Q_OBJECT
public:
	TexturePreviewWidget(QWidget *parent = 0);
	~TexturePreviewWidget();

	void SetTexture(ID3D12Resource *texture, DXGI_FORMAT fmtHint);
	ID3D12Resource *GetTexture() { return _texture; }

	void update();

	void DestroyDeviceObject();

protected:
	Ui::TexturePreviewWidget ui;

	PipelineStateDescID _psDescID;
	PipelineStateDescID _pisDescID;
	PipelineStateDescID _posDescID;
	RootSignatureID _rsID;


	RID3D12Resource _texture;
	DXGI_FORMAT _fmt;
	UINT _cm;

	SDescriptorTable _texDesc;
	XMFLOAT2 _texSize;
	float32 _zoom;
	XMFLOAT2 _pos;
	int _texType;
	uint32 _arraySize;

	bool _firstResize;

	float32 _calcFitZoom();

private slots:
	void renderPreview();
	void onPreviewZoom(int d, int x, int y);
	void onPreviewMove(int dx, int dy);
	void onPreviewResize();
	void zoomToFit();
	void zoomToActual();

};


}