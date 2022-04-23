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

#include "Exports.h"
#include "M3DEngine/Chip.h"
#include "M3DCore/Frustum.h"
#include "Geometry.h"


namespace m3d
{

#define STD1_VIEW (1 << 0)
#define STD1_PROJECTION (1 << 1)
#define STD1_VIEWINVERSE (1 << 2)
#define STD1_VIEWINVERSETRANSPOSE (1 << 3)
#define STD1_PROJECTIONINVERSE (1 << 4)
#define STD1_VIEWPROJECTION (1 << 5)
#define STD1_VIEWPROJECTIONINVERSE (1 << 6)
#define STD1_ELEMENTS (STD1_VIEW|STD1_PROJECTION|STD1_VIEWINVERSE|STD1_VIEWINVERSETRANSPOSE|STD1_PROJECTIONINVERSE|STD1_VIEWPROJECTION|STD1_VIEWPROJECTIONINVERSE)
#define STD1_ELEMENT_COUNT 7

#define STD2_WORLD (1 << 10)
#define STD2_WORLDINVERSE (1 << 11)
#define STD2_WORLDINVERSETRANSPOSE (1 << 12)
#define STD2_WORLDVIEW (1 << 13)
#define STD2_WORLDVIEWINVERSE (1 << 14)
#define STD2_WORLDVIEWINVERSETRANSPOSE (1 << 15)
#define STD2_WORLDVIEWPROJECTION (1 << 16)
#define STD2_WORLDVIEWPROJECTIONINVERSE (1 << 17)
#define STD2_SHADOW (1 << 18)
#define STD2_WORLDSHADOW (1 << 19)
#define STD2_ELEMENTS (STD2_WORLD|STD2_WORLDINVERSE|STD2_WORLDINVERSETRANSPOSE|STD2_WORLDVIEW|STD2_WORLDVIEWINVERSE|STD2_WORLDVIEWINVERSETRANSPOSE|STD2_WORLDVIEWPROJECTION|STD2_WORLDVIEWPROJECTIONINVERSE|STD2_SHADOW|STD2_WORLDSHADOW)
#define STD2_ELEMENT_COUNT 10

struct StdCB1
{
	XMFLOAT4X4 view;
	XMFLOAT4X4 projection;
	XMFLOAT4X4 viewInverse;
	XMFLOAT4X4 viewInverseTranspose;
	XMFLOAT4X4 projectionInverse;
	XMFLOAT4X4 viewProjection;
	XMFLOAT4X4 viewProjectionInverse;
	//	XMFLOAT4 dt;
	//	XMFLOAT4 bbsize;
	//	XMFLOAT4 rtsize;
};

struct StdCB2
{
	XMFLOAT4X4 world;
	XMFLOAT4X4 worldInverse;
	XMFLOAT4X4 worldInverseTranspose; // 3x3 aka normalMatrix in world space.
	XMFLOAT4X4 worldView;
	XMFLOAT4X4 worldViewInverse;
	XMFLOAT4X4 worldViewInverseTranspose; // 3x3 aka normalMatrix.
	XMFLOAT4X4 worldViewProjection;
	XMFLOAT4X4 worldViewProjectionInverse;
	XMFLOAT4X4 shadow;
	XMFLOAT4X4 worldShadow;
};

struct StdCB1Matrices
{
	XMMATRIX view;
	XMMATRIX projection;
	XMMATRIX viewInverse;
	XMMATRIX viewInverseTranspose;
	XMMATRIX projectionInverse;
	XMMATRIX viewProjection;
	XMMATRIX viewProjectionInverse;
};

struct StdCB2Matrices
{
	XMMATRIX world;
	XMMATRIX worldInverse;
	XMMATRIX worldInverseTranspose;
	XMMATRIX worldView;
	XMMATRIX worldViewInverse;
	XMMATRIX worldViewProjection;
	XMMATRIX worldViewProjectionInverse;
	XMMATRIX normal;
	XMMATRIX normalView;
	XMMATRIX shadow;
	XMMATRIX worldShadow;
};

static const Guid RENDERSTATE_GUID = { 0x6bb3fc86, 0xde16, 0x412b, { 0x81, 0x66, 0x37, 0xd9, 0xe2, 0x82, 0x19, 0x9d } };



class GRAPHICSCHIPS_API RenderState : public Chip
{
public:
	RenderState();
	~RenderState();

	const Frustum& GetFrustum(uint32 index);

	void SetWorldMatrix(const XMFLOAT4X4& worldMatrix);
	void SetViewMatrix(const XMFLOAT4X4& viewMatrix);
	void SetProjectionMatrix(const XMFLOAT4X4& projectionMatrix);
	void SetCullingMatrix(const XMFLOAT4X4& cullingMatrix);
	void SetShadowMatrix(const XMFLOAT4X4& shadowMatrix);

	const XMFLOAT4X4& GetWorldMatrix() const { return _world; }
	const XMFLOAT4X4& GetViewMatrix() const { return _view; }
	const XMFLOAT4X4& GetProjectionMatrix() const { return _projection; }
	const XMFLOAT4X4& GetCullingMatrix() const { return _culling; }
	const XMFLOAT4X4& GetShadowMatrix() const { return _shadow; }

	virtual void ClearGraphicsRootDescriptorTables() = 0;
	virtual void IASetPrimitiveTopology(M3D_PRIMITIVE_TOPOLOGY pt) = 0;
	virtual void PrepareDraw() = 0;
	virtual void CommitResourceBarriers() = 0;
	virtual void DrawIndexedInstanced(UINT IndexCountPerInstance, UINT InstanceCount, UINT StartIndexLocation, INT BaseVertexLocation, UINT StartInstanceLocation) = 0;
	virtual void DrawInstanced(UINT VertexCountPerInstance, UINT InstanceCount, UINT StartVertexLocation, UINT StartInstanceLocation) = 0;
	virtual void PushState() = 0;
	virtual void PopState() = 0;

protected:
	Frustum _frustum;

	// Bitmask for elements in std buffers that needs to be updated.
	UINT _stdElements;

	StdCB1 _std1Data;
	StdCB2 _std2Data;

	// For calculations!
	StdCB1Matrices* _std1Matrices;
	StdCB2Matrices* _std2Matrices;

	// Matrices as set by the Set...-functions.
	XMFLOAT4X4 _world;
	XMFLOAT4X4 _view;
	XMFLOAT4X4 _projection;
	XMFLOAT4X4 _culling;
	XMFLOAT4X4 _shadow;
};

}