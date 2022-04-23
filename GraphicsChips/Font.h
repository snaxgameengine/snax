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
#include "M3DEngine/GlobalDef.h"
#include "D3D12Include.h"
#include "M3DCore/MString.h"
#include "M3DCore/Containers.h"
#include "M3DCore/DataBuffer.h"
#include "DescriptorHeapManager.h"


namespace m3d
{

class Graphics;


class GRAPHICSCHIPS_API Font
{
public:
	enum class Weight { THIN, EXTRALIGHT, LIGHT, NORMAL, MEDIUM, SEMIBOLD, BOLD, EXTRABOLD, HEAVY };

	struct Glyph
	{
		// x texcoord 
		int32 tx;
		// y texcoord
		int32 ty;
		// width of character
		int32 w;
		// pixels to advance to next character (not needed on GPU)
		int32 advance;
		// pixels to move current x pos before drawing Char (not needed on GPU)
		int32 movex;
		// The character code
		uint32 charCode;
		// Our index in the array
		uint32 glyphIndex;
		// texture data when initialized from serialized font.
		const void *texData;
	};

	Font();
	virtual ~Font();

	// Initializes the font. Only for win desktop!
	virtual bool Init(String font = MTEXT("Arial"), uint32 size = 16, Weight weight = Weight::NORMAL, bool italic = false, bool underline = false, bool strikeout = false, uint32 mipLevels = 0);
	// Initialized from serialized data. No new glyphs allowed!
	virtual bool Init(DataBuffer &&db, size_t bufferSize);
	// Serialize this font. Only for win desktop!
	virtual bool SerializeFont(DataBuffer &db, size_t &bufferSize, uint32 firstChar = 32, uint32 lastChar = 255, uint32 maxTextureSize = 8192, uint32 maxCharacters = 4096) const;
	// Gets the maximal width a line in the given text block. 
	virtual int32 GetWidth(String text);
	//
	virtual String FormatText(String text, int32 &width, int32 &lines);
	// Returns the glyph for a given unicode character.
	virtual const Glyph *GetGlyph(uint32 charCode);
	//
	virtual uint32 GetTextureHeight() const { return _th; }
	// Get the height of the glyphs.
	virtual uint32 GetGlyphHeight() const { return _height; }
	// Get the size of the font as given in the constructor.
	virtual uint32 GetSize() const { return _size; }

	// Updates buffer and texture. Call before each rendering!
	virtual HRESULT Update(Graphics *graphics);
	// Call when device is destroyed!
	virtual void OnDestroyDevice();
	// Glyph buffer maps a glyph index to texture coordinates
	virtual ID3D12Resource *GetGlyphBuffer() { return _glyphBuffer; }
	// Get SRV for glyph buffer (0) and texture (1).
	virtual const DescriptorTable *GetSRV() { return _srv; }

protected:
	bool _addChar(uint32 charCode);
	uint32 _getGlyphIndex(uint32 charCode);
	uint32 _increment(uint32 dp) const;
	HRESULT _updateGlyphBuffer(Graphics *graphics, uint32 fromGlyphIndex);

#ifdef WINDESKTOP
	// In memory context
	HDC _hDC;
	// Current font
	HFONT _hFont;
	// Bitmap for rendering
	HBITMAP _hbmBitmap;
#endif
	// Serialized data
	DataBuffer _serialized;
	// Specified size of font
	uint32 _size;
	// Height of a character in pixels
	uint32 _height;
	// Number of pixels from lower left corner of character to baseline.
	int32 _dyBaseline;
	// x Texcoord of next character to add
	uint32 _tx;
	// y Texcoord of next character to add
	uint32 _ty;
	// Width of texture
	uint32 _tw;
	// Height of texture
	uint32 _th;
	// Recommended line gap
	uint32 _lineGap;
	// Mapping of charIndex to GlyphArrayIndex
	Map<uint32, uint32> _charIndexToGlyphIndex;
	// Mapping of charCode to GlyphArrayIndex
	Map<uint32 ,uint32> _charCodeToGlyphIndex;
	// Mapping of charCode to GlyphArrayIndex of standard 0-255 characters (for speed, no map!).
	uint32 _standardChars[256];
	// GlyphArray: To be used on GPU.
	List<Glyph> _glyphs;
	// Number of mipLevels for the font texture.
	uint32 _mipLevels;
	// Number of glyphs updated
	uint32 _glyphsUpdateCount;
	// Capacity of glyph buffer
	uint32 _glyphBufferCapacity;

	RID3D12Resource _texture;
	RID3D12Resource _glyphBuffer;
	SDescriptorTable _srv; // For both glyph buffer and texture!
};

}
