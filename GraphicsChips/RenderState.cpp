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
#include "RenderState.h"

using namespace m3d;


RenderState::RenderState()
{
	_stdElements = 0;

	_world = _view = _projection = _culling = _shadow = XMFLOAT4X4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1);

	_std1Matrices = mmnew StdCB1Matrices(); // Must be 16 byte aligned
	_std2Matrices = mmnew StdCB2Matrices(); // Must be 16 byte aligned
}

RenderState::~RenderState()
{
}

const Frustum& RenderState::GetFrustum(uint32 index)
{
	XMFLOAT4X4 vp;
	XMStoreFloat4x4(&vp, XMMatrixMultiply(_std1Matrices->view, _std1Matrices->projection));
	_frustum.Set(vp);
	return _frustum;
}

void RenderState::SetWorldMatrix(const XMFLOAT4X4& worldMatrix)
{
	_std2Matrices->world = XMLoadFloat4x4(&(_world = worldMatrix));
	_stdElements &= ~UINT(STD2_WORLD | STD2_WORLDINVERSE | STD2_WORLDINVERSETRANSPOSE | STD2_WORLDVIEW | STD2_WORLDVIEWINVERSE | STD2_WORLDVIEWINVERSETRANSPOSE | STD2_WORLDVIEWPROJECTION | STD2_WORLDVIEWPROJECTIONINVERSE | STD2_WORLDSHADOW);
}

void RenderState::SetViewMatrix(const XMFLOAT4X4& viewMatrix)
{
	_std1Matrices->view = XMLoadFloat4x4(&(_view = viewMatrix));
	_stdElements &= ~UINT(STD1_VIEW | STD1_VIEWINVERSE | STD1_VIEWINVERSETRANSPOSE | STD1_VIEWPROJECTION | STD1_VIEWPROJECTIONINVERSE | STD2_WORLDVIEW | STD2_WORLDVIEWINVERSE | STD2_WORLDVIEWINVERSETRANSPOSE | STD2_WORLDVIEWPROJECTION | STD2_WORLDVIEWPROJECTIONINVERSE);
}

void RenderState::SetProjectionMatrix(const XMFLOAT4X4& projectionMatrix)
{
	_std1Matrices->projection = XMLoadFloat4x4(&(_projection = projectionMatrix));
	_stdElements &= ~UINT(STD1_PROJECTION | STD1_PROJECTIONINVERSE | STD1_VIEWPROJECTION | STD1_VIEWPROJECTIONINVERSE | STD2_WORLDVIEWPROJECTION | STD2_WORLDVIEWPROJECTIONINVERSE);
}


void RenderState::SetCullingMatrix(const XMFLOAT4X4& cullingMatrix)
{
	_culling = cullingMatrix;
}

void RenderState::SetShadowMatrix(const XMFLOAT4X4& shadowMatrix)
{
	_std2Matrices->shadow = XMLoadFloat4x4(&(_shadow = shadowMatrix));
	_stdElements &= ~UINT(STD2_SHADOW | STD2_WORLDSHADOW);
}
