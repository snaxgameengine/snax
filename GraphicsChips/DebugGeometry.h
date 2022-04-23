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
#include "M3DCore/MMath.h"
#include "Graphics.h"


namespace m3d
{


static const Guid DEBUGGEOMETRY_GUID = { 0xf4b394ae, 0x6b87, 0x4fd1, { 0x88, 0xcd, 0xd6, 0xa, 0x2f, 0x67, 0x40, 0xc1 } };


struct DebugVertex
{
	XMFLOAT3 position;
	FLOAT index; // 0-4. for points without gs.
	XMUBYTE4 color;
	DebugVertex(const XMFLOAT3& position, const XMUBYTE4& color) : position(position), index(0.0f), color(color) {}
	DebugVertex(const DebugVertex& dv, FLOAT index) : position(dv.position), index(index), color(dv.color) {}
};

typedef List<DebugVertex> DebugVertexList;
//template class GRAPHICSCHIPS_API List<DebugVertex>;


class GRAPHICSCHIPS_API DebugGeometry : public Chip, public GraphicsUsage
{
	CHIPDESC_DECL; 
public:
	// Called by engine every frame for hidden chips.
	void OnNewFrame() override;

	// Clears current geometry.
	virtual void ClearGeometry();

	// Adds geometry.
	virtual void AddPoint(const DebugVertex& p);
	virtual void AddLineSegment(const DebugVertex& a, const DebugVertex& b);
	virtual void AddTriangle(const DebugVertex& a, const DebugVertex& b, const DebugVertex& c);
	virtual void AddXZGrid(float32 scale = 10.0f);
	virtual void AddFrustum(CXMMATRIX vp, XMUBYTE4 color);
	virtual void AddBox(CXMMATRIX m, const XMFLOAT3& a, const XMFLOAT3& b, XMUBYTE4 color);

	// Renders the current geometry.
	void CallChip() override;

protected:
	struct CB
	{
		XMFLOAT4X4 vp;
		XMFLOAT2 gsScale;
		BYTE __padding[184]; // cb must be multiple of 256 bytes!
	};

	DebugVertexList _points;
	DebugVertexList _lineSegments;
	DebugVertexList _triangles;
};


}
