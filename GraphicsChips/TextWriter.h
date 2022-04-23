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

#include "Font.h"
#include "Graphics.h"

namespace m3d
{


class GRAPHICSCHIPS_API TextWriter
{
public:
	TextWriter();
	virtual ~TextWriter();

	enum Alignment { LEFT_TOP, CENTER_TOP, RIGHT_TOP, LEFT_CENTER, CENTER, RIGHT_CENTER, LEFT_BOTTOM, CENTER_BOTTOM, RIGHT_BOTTOM };
	enum HAlign { LEFT, HMID, RIGHT };
	enum VAlign { TOP, VMID, BOTTOM };

	// Initializes the text writer.
	virtual HRESULT Init(Graphics *graphics, Font *font);
	// Release (Optional)
	virtual void OnDestroyDevice();
	// Write the text to buffer, but do not flush!
	virtual void Write(XMFLOAT2 position, HAlign hAlign, VAlign vAlign, bool multiline, String text);
	// Write the text to buffer, but do not flush! Also updates current color.
	virtual void Write(XMFLOAT2 position, HAlign hAlign, VAlign vAlign, bool multiline, XMFLOAT4 color, String text);
	// Write the text, and flush to render target.
	virtual HRESULT Write(Graphics *graphics, XMFLOAT2 position, HAlign hAlign, VAlign vAlign, bool multiline, String text);
	// Write the text, and flush to render target. Also updates current color.
	virtual HRESULT Write(Graphics *graphics, XMFLOAT2 position, HAlign hAlign, VAlign vAlign, bool multiline, XMFLOAT4 color, String text);
	// Flush text to render target.
	virtual HRESULT Flush(Graphics *graphics);
	// Sets the current render color.
	virtual void SetColor(XMFLOAT4 color) { _color = color; }
	// Set a global offset to be applied to all coordinates.
	virtual void SetOffset(XMFLOAT2 offset) { _offset = offset; }
	// Returns current font.
	virtual Font *GetFont() const { return _font; }
	// Sets the font to be used.
	virtual void SetFont(Font *font);
	// Transform used for tech 2.
	virtual void SetTransform(const XMFLOAT4X4 &t) { _transform = t; }
	//
	virtual void SetUseTransform(bool b) { _useTransform = b; }

protected:
	// Font used for rendering
	Font *_font;
	// Capacity of vertex buffer
	uint32 _vertexBufferCapacity;
	// Color of text
	XMFLOAT4 _color;
	// Global offset added to coordinates
	XMFLOAT2 _offset;
	// Transform
	XMFLOAT4X4 _transform;
	//
	bool _useTransform;

	struct Vertex
	{
		// Pixel x coordinate of character
		float x;
		// Pixel y coordinate of character
		float y;
		// Index used to index glyph buffer on GPU. For level >=10.
		uint16 glyphIndex;
	};

	// Array with vertices to be rendered.
	List<Vertex> _vertices;

	struct CB
	{
		XMFLOAT4 fontColor;
		XMFLOAT4 props;
		XMFLOAT4X4 wvp;
		BYTE __padding[160]; // cb must be multiple of 256 bytes!
	};

	static RootSignatureID _rsID;
	static PipelineStateDescID _psDesc1ID;
	static PipelineStateDescID _psDesc2ID;
	static PipelineStateDescID _pisDescID;
};

}
