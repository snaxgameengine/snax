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

#include "M3DEngine/GlobalDef.h"
#include "D3DInclude.h"
#include "M3DCore/MString.h"
#include "M3DCore/Containers.h"
#include "M3DCore/DataBuffer.h"


namespace m3d
{


class Font
{
public:
	enum Weight { THIN, EXTRALIGHT, LIGHT, NORMAL, MEDIUM, SEMIBOLD, BOLD, EXTRABOLD, HEAVY };

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
		// pixels to move current x pos before drawing char (not needed on GPU)
		int32 movex;
		// The character code
		unsigned charCode;
		// Our index in the array
		unsigned glyphIndex;
		// texture data when initialized from serialized font.
		const void *texData;
	};

	Font();
	virtual ~Font();

	// Initializes the font. Only for win desktop!
	virtual bool Init(String font = MTEXT("Arial"), unsigned dpi = 96, unsigned size = 16, Weight weight = NORMAL, bool italic = false, bool underline = false, bool strikeout = false, unsigned mipLevels = 0);
	// Initialized from serialized data. No new glyphs allowed!
	virtual bool Init(DataBuffer &&db, size_t bufferSize);
	// Serialize this font. Only for win desktop!
	virtual bool SerializeFont(DataBuffer &db, size_t &bufferSize, unsigned firstChar = 32, unsigned lastChar = 255, unsigned maxTextureSize = 8192, unsigned maxCharacters = 4096) const;
	// Gets the maximal width a line in the given text block. 
	virtual int32 GetWidth(String text);
	//
	virtual String FormatText(String text, int32 &width, int32 &lines);
	// Returns the glyph for a given unicode character.
	virtual const Glyph *GetGlyph(unsigned charCode);
	//
	virtual unsigned GetTextureHeight() const { return _th; }
	// Get the height of the glyphs.
	virtual unsigned GetGlyphHeight() const { return _height; }
	// Get the size of the font as given in the constructor.
	virtual unsigned GetSize() const { return _size; }
	// Gets the logical dpi used for creating this font.
	virtual unsigned GetDPI() const { return _dpi; }

	// Updates buffer and texture. Call before each rendering!
	virtual HRESULT Update(ID3D11DeviceContext *context);
	// Call when device is destroyed!
	virtual void OnDestroyDevice();
	// Glyph buffer maps a glyph index to texture coordinates
	virtual SID3D11Buffer GetGlyphBuffer() { return _glyphBuffer; }
	// Glyph buffer as a SRV. Not available for feature level < 10.
	virtual SID3D11ShaderResourceView GetGlyphBufferSRV() { return _glyphBufferSRV; }
	// Glyph texture contains the glyphs.
	virtual SID3D11ShaderResourceView GetGlyphTexture() { return _textureSRV; }

protected:
	bool _addChar(unsigned charCode);
	unsigned _getGlyphIndex(unsigned charCode);
	unsigned _increment(unsigned dp) const;
	HRESULT _updateGlyphBuffer(ID3D11DeviceContext *context, unsigned fromGlyphIndex, D3D_FEATURE_LEVEL featureLevel);

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
	// Logical dpi.
	unsigned _dpi;
	// Specified size of font
	unsigned _size;
	// Height of a character in pixels
	unsigned _height;
	// Number of pixels from lower left corner of character to baseline.
	int32 _dyBaseline;
	// x Texcoord of next character to add
	unsigned _tx;
	// y Texcoord of next character to add
	unsigned _ty;
	// Width of texture
	unsigned _tw;
	// Height of texture
	unsigned _th;
	// Recommended line gap
	unsigned _lineGap;
	// Mapping of charIndex to GlyphArrayIndex
	Map<unsigned, unsigned> _charIndexToGlyphIndex;
	// Mapping of charCode to GlyphArrayIndex
	Map<unsigned ,unsigned> _charCodeToGlyphIndex;
	// Mapping of charCode to GlyphArrayIndex of standard 0-255 characters (for speed, no map!).
	unsigned _standardChars[256];
	// GlyphArray: To be used on GPU.
	List<Glyph> _glyphs;
	// Number of mipLevels for the font texture.
	unsigned _mipLevels;
	// Number of glyphs updated
	unsigned _glyphsUpdateCount;
	// Capacity of glyph buffer
	unsigned _glyphBufferCapacity;

	SID3D11Texture2D _texture;
	SID3D11Buffer _glyphBuffer;
	SID3D11ShaderResourceView _glyphBufferSRV; // Not supported for level 9.1. glyph buffer is constant buffer.
	SID3D11ShaderResourceView _textureSRV;
};

}
