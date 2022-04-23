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

#include "D3DWidget.h"
#include "GraphicsChips/Geometry.h"

namespace m3d
{

class Geometry3DView : public D3DWidget
{
	Q_OBJECT
public:
	Geometry3DView(QWidget * parent = 0, Qt::WindowFlags f = Qt::WindowFlags());
	~Geometry3DView();

	void init(Geometry*d);

	virtual void wheelEvent(QWheelEvent *event);
	virtual void mousePressEvent(QMouseEvent *event);
	virtual void mouseMoveEvent(QMouseEvent *event);
	virtual void mouseReleaseEvent(QMouseEvent *event) {}



	enum Tech { TECH_NONE, TECH_PLAIN, TECH_WF };

	Geometry*_geometry;
	Set<uint32> _subsets;
	Tech _tech;
	bool _backFaceCulling;
	bool _scaleWholeObject;


protected:
	float32 _zoom;
	float32 _yaw;
	float32 _pitch;
	XMFLOAT3 _pos;

	int _mx;
	int _my;

	SDescriptorTable _dsv;
	/*
	SID3D11DepthStencilView _dsv;
	SID3D11VertexShader _vs;
	SID3D11PixelShader _ps;
	SID3D11Buffer _cb;
	SID3D11RasterizerState _rsFilled;
	SID3D11RasterizerState _rsWireNoCull;
	SID3D11DepthStencilState _dssWireOverlay;
	InputSignatureID _visID;

	SID3D11RasterizerState _rsWireCull;
	SID3D11RasterizerState _rsWire;
	SID3D11RasterizerState _rsCull;
	SID3D11VertexShader _vs0;
	SID3D11PixelShader _ps0;
	InputSignatureID _is0;
	SID3D11VertexShader _vs1;
	SID3D11PixelShader _ps1;
	InputSignatureID _is1;
*/

	HRESULT _render3D();
	void _draw(const GeometrySubsetList &ssl/*, ID3D11DeviceContext *context*/);
	HRESULT _renderFlat();

	void DestroyDeviceObject() override;

private:
	void _render();
	void _resize();
};


}