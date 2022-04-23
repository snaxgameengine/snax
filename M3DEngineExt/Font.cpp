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
#include "Font.h"
#include "M3DEngine/Engine.h"
#include "M3DCore/DataBufferStream.h"
#include "TextWriter.hlsl.h"

using namespace m3d;


unsigned NextPowerOf2(unsigned n)
{
	n--;
	n |= n >> 1;
	n |= n >> 2;
	n |= n >> 4;
	n |= n >> 8;
	n |= n >> 16;
	n++; 
	return n;
}

inline unsigned AlignToPowerOf2(unsigned Value, unsigned Alignment)
{
    // to align to 2^N, add 2^N - 1 and AND with all but lowest N bits set
    return (Value + Alignment - 1) & (~(Alignment - 1));
}


Font::Font()
{
#ifdef WINDESKTOP
	_hDC = NULL;
	_hFont = NULL;
	_hbmBitmap = NULL;
#endif
	_size = 0;
	_height = 0;
	_dyBaseline = 0;
	_tx = _ty = 0;
	_tw = _th = 0;
	memset(_standardChars, 0xFFFFFFFF, sizeof(_standardChars));
	_lineGap = 0;
	_mipLevels = 0;
	_glyphsUpdateCount = 0;
	_glyphBufferCapacity = 256;
}

Font::~Font()
{
#ifdef WINDESKTOP
	if (_hbmBitmap)
		DeleteObject(_hbmBitmap);
	if (_hFont)
		DeleteObject(_hFont);
	if (_hDC)
		DeleteDC(_hDC);
#endif
}

bool Font::Init(String font, unsigned dpi, unsigned size, Weight weight, bool italic, bool underline, bool strikeout, unsigned mipLevels)
{
#ifdef WINDESKTOP
	_hDC = CreateCompatibleDC(NULL);
	if (_hDC == NULL)
		return false;

	int32 w;
	switch (weight) {
	case THIN: w = FW_THIN; break;
	case EXTRALIGHT: w = FW_EXTRALIGHT; break;
	case LIGHT: w = FW_LIGHT; break;
	case NORMAL: w = FW_NORMAL; break;
	case MEDIUM: w = FW_MEDIUM; break;
	case SEMIBOLD: w = FW_SEMIBOLD; break;
	case BOLD: w = FW_BOLD; break;
	case EXTRABOLD: w = FW_EXTRABOLD; break;
	case HEAVY: w = FW_HEAVY; break;
	default: w = FW_NORMAL; break;
	}

	_dpi = dpi;
	_size = size;

	int32 s = -MulDiv(int32(size), dpi, 72);
	_hFont = CreateFontA(s, 0, 0, 0, w, italic ? TRUE : FALSE, underline ? TRUE : FALSE, strikeout ? TRUE : FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, /*ANTIALIASED_QUALITY*/CLEARTYPE_QUALITY, DEFAULT_PITCH, font.c_str());
	if (_hFont == NULL)
		return false;

	SetMapMode(_hDC, MM_TEXT);

	SelectObject(_hDC, _hFont);
	SetTextColor(_hDC, RGB(255,255,255));
	SetBkColor(_hDC, RGB(0,0,0));
	SetTextAlign(_hDC, TA_TOP);

	OUTLINETEXTMETRIC metrics;
	memset(&metrics, 0, sizeof(OUTLINETEXTMETRIC));
	if (GetOutlineTextMetrics(_hDC, sizeof(OUTLINETEXTMETRIC), &metrics) == 0)
		return false;

	// Prepare to create a bitmap
	DWORD *pBitmapBits;
	BITMAPINFO bmi;
	ZeroMemory( &bmi.bmiHeader, sizeof(BITMAPINFOHEADER) );
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth = (LONG)metrics.otmTextMetrics.tmMaxCharWidth;
	bmi.bmiHeader.biHeight = (LONG)-metrics.otmTextMetrics.tmHeight;
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biCompression = BI_RGB;
	bmi.bmiHeader.biBitCount = 32;

	// Create a bitmap for the font
	_hbmBitmap = CreateDIBSection(_hDC, &bmi, DIB_RGB_COLORS, (void**)&pBitmapBits, NULL, 0);
	if (!_hbmBitmap)
		return false;

	SelectObject(_hDC, _hbmBitmap);

	_mipLevels = mipLevels;

	_height = metrics.otmTextMetrics.tmHeight;
	_dyBaseline = metrics.otmTextMetrics.tmDescent;
	_lineGap = metrics.otmLineGap;
	_tw = (unsigned)pow(2, ceil(logf(sqrtf(float32(metrics.otmTextMetrics.tmAveCharWidth*_height*250))) / logf(2.0f))); // trying to set a minimum texture size for 250 characters
	_th = (unsigned)pow(2, ceil(logf(float32(metrics.otmTextMetrics.tmAveCharWidth*_height*250/_tw)) / logf(2.0f)));
	_tw = std::min(2048u, _tw);
	_th = std::min(2028u, _th);

	// Add standard characters.
	for (unsigned i = 32; i < 256; i++) // exclude 0-31. Non printables!
		_addChar(i);

	return true;
#else
	return false; // Not supported!
#endif
}

bool Font::Init(DataBuffer &&db, size_t bufferSize)
{
	_serialized = std::move(db);
	DataBufferInputStream dbs(_serialized);

	unsigned recTexSize = 0;

	dbs >> recTexSize >> _size >> _height >> _dyBaseline >> _lineGap >> _mipLevels;
	if (dbs.IsError())
		return false;

	while (dbs.GetBufferPos() < bufferSize) {
		unsigned charCode;
		unsigned x;
		dbs >> charCode >> x;
		if (x == unsigned(-1)) {
			unsigned charCode2;
			dbs >> charCode2;
			if (dbs.IsError())
				break;
			auto n = _charCodeToGlyphIndex.find(charCode2);
			if (n == _charCodeToGlyphIndex.end())
				return false;
			if (!_charCodeToGlyphIndex.insert(std::make_pair(charCode, n->second)).second)
				return false;
			if (charCode < 256)
				_standardChars[charCode] = n->second;
		}
		else {
			int32 width = x, advance, movex;
			dbs >> advance >> movex;
			unsigned s = _height * width;
			const void *glyphTexData = dbs.GetDataBuffer().getConstBuffer() + dbs.GetBufferPos();
			dbs.MoveBufferPos(s);
			if (dbs.IsError())
				return false;
			Glyph g = { 0, 0, width, advance, movex, charCode, (unsigned)_glyphs.size(), glyphTexData };
			if (!_charCodeToGlyphIndex.insert(std::make_pair(charCode, (unsigned)_glyphs.size())).second)
				break;
			if (charCode < 256)
				_standardChars[charCode] = g.glyphIndex;
			_glyphs.push_back(g);
		}
	}
	if (dbs.IsError() || dbs.GetBufferPos() != bufferSize)
		return false;

	_tw = NextPowerOf2(recTexSize);
	_th = NextPowerOf2(recTexSize * recTexSize / _tw);
	_tw = std::min(2048u, _tw);
	_th = std::min(2028u, _th);

	return true;
}

//#include <iostream>
//#include <fstream>


bool Font::SerializeFont(DataBuffer &db, size_t &bufferSize, unsigned firstChar, unsigned lastChar, unsigned maxTextureSize, unsigned maxCharacters) const
{
#ifdef WINDESKTOP
	DataBufferOutputStream dbs(db);

	unsigned count = 0;
	unsigned tx = 0;
	unsigned ty = 0;

	dbs << unsigned(0) << _size << _height << _dyBaseline << _lineGap << _mipLevels;

	Map<unsigned, unsigned> charIndexToCharCode;
	for (unsigned charCode = firstChar; charCode <= lastChar; charCode++) {
		if (count > maxCharacters)
			return false; // Not enough glyph buffer space.

		Char txt[3] = { Char(charCode), MCHAR('\0')};
		WORD charIndex = 0;
		if (GetGlyphIndicesA(_hDC, txt, 1, &charIndex, GGI_MARK_NONEXISTING_GLYPHS) != 1 || charIndex == WORD(-1))
			continue; // This character does not have a glyph!
		auto e = charIndexToCharCode.insert(std::make_pair(charIndex, charCode));
		if (!e.second) { // Glyph already present! Reuse!
			dbs << charCode << unsigned(-1) << e.first->second;
			count++;
			continue;
		}
		ABC abc;
		if (!GetCharABCWidths(_hDC, charCode, charCode, &abc))
			continue;

		int32 width = std::max(abc.abcA, 0) + abc.abcB + std::max(abc.abcC, 0);
		int32 advance = abc.abcA + abc.abcB + abc.abcC;
		int32 movex = std::min(abc.abcA, 0);
		unsigned size = unsigned(width * _height);

		tx += _increment(width);
		if (tx > maxTextureSize) {
			tx = 0;
			ty += _increment(_height);
			if (ty + _increment(_height) > maxTextureSize)
				return false; // Not enough texture space!
		}

		if (ExtTextOutA(_hDC, -movex, 0, ETO_OPAQUE, NULL, txt, 1, NULL) == FALSE)
			continue;

		BITMAP bitmap;
		GetObject(_hbmBitmap, sizeof(BITMAP), &bitmap);

		uint8 *data = (uint8*)mmalloc(size);

		for (unsigned q = 0; q < _height; q++) {
			for (unsigned p = 0; p < (unsigned)width; p++) {
				uint8 *gl = (uint8*)bitmap.bmBits + q * bitmap.bmWidthBytes + p * 4;
				data[width * q + p] = uint8((float32(gl[0]) + float32(gl[1]) + float32(gl[2])) / 3.0f + 0.5f);
			}
		}

		dbs << charCode << width << advance << movex;
		dbs.Write(data, size);

		mmfree(data);

		count++;
	}
	bufferSize = dbs.GetBufferPos();
	unsigned minTexSize = unsigned(sqrt(float32(tx * _increment(_height) + maxTextureSize * _increment(ty))) + 0.5f);
	dbs.SetBufferPos(0);
	dbs << minTexSize;
/*
	{
		String s = String(MTEXT("const uint8 FONT[%1] =\n{\n\t")).arg(String::fromNum(bufferSize));
		String t;
		const uint8 *C = (const uint8*)db.getConstBuffer();
		for (size_t i = 0, j = bufferSize; i < j;) {
			for (size_t k = std::min(i + 20, j); i < k; i++) {
				s += String(MTEXT("0x%1, ")).arg(String::fromNum((unsigned)C[i], MTEXT("%02X")));
			}
			if (i < j - 1)
				s += MTEXT("\n\t");
		}
		s.erase(s.size() - 2, 2);
		s += MTEXT("\n};");

		std::wofstream myfile;
		myfile.open ("c:\\temp\\example.txt");
		myfile << s.c_str();
		myfile.close();
	}
*/
	return dbs.IsError() == false;
#else
	return false;
#endif
}

int32 Font::GetWidth(String text)
{
	int32 lineCount = 1;
	int32 maxWidth = 0;
	int32 w = 0;
	int32 t = 0;
	for (unsigned i = 0; i < text.length(); i++) {
		if (text[i] == L'\n') {
			lineCount++;
			maxWidth = std::max(maxWidth, w + t);
			w = t = 0;
		}
		else {
			unsigned glyphIndex = _getGlyphIndex(text[i]);
			if (glyphIndex == -1)
				continue;
			const Glyph &g = _glyphs[glyphIndex];
			w += g.advance;
			t = g.w;
		}
	}
	maxWidth = std::max(maxWidth, w + t);
	return maxWidth;
}

String Font::FormatText(String text, int32 &width, int32 &lines)
{
	String result;
	result.reserve(text.length());
	int32 lineCount = 1;
	int32 lineWidth = 0;
	int32 V = 0, W = 0;

	bool firstWord = true;
	String currentWord;
	int32 wordWidth = 0;

	const Glyph &h = _glyphs[_getGlyphIndex(L'-')];

	unsigned i = 0;
	for (; i < text.length(); i++) { // Iterate letters
		if (text[i] == L'\n') { // New line?
			result += currentWord;
			currentWord.clear();
			firstWord = true;
			V = W;
			W = std::max(W, lineWidth + wordWidth);
			wordWidth = 0;
			lineWidth = 0;
			if (lineCount >= lines)
				break;
			result += MTEXT("\n");
			lineCount++;
		}
		else {
			unsigned glyphIndex = _getGlyphIndex(text[i]);
			if (glyphIndex == -1)
				continue; // nonprintable...
			const Glyph &g = _glyphs[glyphIndex];

			if (text[i] == L' ') { // Space character?
				if (lineWidth + wordWidth + g.w > width) { // Line exeeds max width?
					result += currentWord;
					currentWord.clear();
					firstWord = true;
					V = W;
					W = std::max(W, lineWidth + wordWidth);
					lineWidth = 0;
					wordWidth = 0;
					if (lineCount >= lines)
						break;
					result += MTEXT("\n");
					lineCount++;
				}
				else {
					result += currentWord + MTEXT(" ");
					currentWord.clear();
					firstWord = false;
					lineWidth += wordWidth + g.advance;
					wordWidth = 0;
				}
			}
			else { // Ordinary character
				if (lineWidth + wordWidth + g.w > width) { // Line exeeds max width?
					if (firstWord) { // First word on line?
						// Split word!
						unsigned j = 0;
						for (; j < currentWord.length(); j++) { // Iterate current word
							const Glyph &g = _glyphs[_getGlyphIndex(currentWord[j])];
							if (lineWidth + g.advance + h.w > width) { // Put in as much chars as possible at line. 
								if (i == 0) {
									wordWidth -= g.advance;
									i++;
								}
								break;
							}
							wordWidth -= g.advance;
							lineWidth += g.advance;
						}
						result += currentWord.substr(0, j) + MTEXT("-");
						currentWord = currentWord.substr(j);
						lineWidth += h.w;
					}
					firstWord = true;
					V = W;
					W = std::max(W, lineWidth);
					lineWidth = 0;
					if (lineCount >= lines)
						break;
					result += MTEXT("\n");
					lineCount++;
				}
				currentWord += text.substr(i, 1);
				wordWidth += g.advance;
			}
		}
	}
	if (i < text.length()) { // Text to long?
		if (result.length() > 3)
			result = result.substr(0, result.length() - 3) + MTEXT("...");
	}
	else {
		result += currentWord;
		lineWidth += wordWidth;
	}

	width = std::max(W, lineWidth);
	lines = lineCount;

	return result;
}

const Font::Glyph *Font::GetGlyph(unsigned charCode)
{
	unsigned i = _getGlyphIndex(charCode);
	if (i == -1)
		return nullptr;
	return &_glyphs[i];
}

unsigned Font::_getGlyphIndex(unsigned charCode)
{
	unsigned idx = -1;
	if (charCode < 256 && _standardChars[charCode] != -1)
		idx = _standardChars[charCode];
	else {
		auto itr = _charCodeToGlyphIndex.find(charCode);
		if (itr != _charCodeToGlyphIndex.end())
			idx = itr->second;
		else {
			_addChar(charCode); // try to add character glyph (may fail).
			itr = _charCodeToGlyphIndex.find(charCode);
			if (itr != _charCodeToGlyphIndex.end())
				idx = itr->second;
		}
	}
	return idx;
}

unsigned Font::_increment(unsigned dp) const
{
	return AlignToPowerOf2(dp, 1 << _mipLevels); // How much space does a glyph need to generate _mipLevels extra mip-levels...
}

bool Font::_addChar(unsigned charCode)
{
#ifdef WINDESKTOP
	if (_hDC == NULL)
		return false; // font is not initialized for this operation!
	if (charCode < 32)
		return false; // Don't care about these nonprintable chars!
	Char txt[3] = { Char(charCode), MCHAR('\0')};
	WORD charIndex = 0;
	if (GetGlyphIndicesA(_hDC, txt, 1, &charIndex, GGI_MARK_NONEXISTING_GLYPHS) != 1)
		return false;

	auto itr = _charIndexToGlyphIndex.find(charIndex);
	if (itr != _charIndexToGlyphIndex.end()) {
		_charCodeToGlyphIndex.insert(std::make_pair(charCode, itr->second));
		if (charCode < 256)
			_standardChars[charCode] = itr->second;
		return true;
	}

	if (charIndex == WORD(-1)) {
		// Map everything to -1. Glyph does not exist!
		_charCodeToGlyphIndex.insert(std::make_pair(charCode, -1));
		if (charCode < 256)
			_standardChars[charCode] = -1;
		_charIndexToGlyphIndex.insert(std::make_pair(-1, -1));
		return true;
	}

	ABC abc;
	if (!GetCharABCWidths(_hDC, charCode, charCode, &abc))
		return false;

	int32 width = std::max(abc.abcA, 0) + abc.abcB + std::max(abc.abcC, 0);
	int32 advance = abc.abcA + abc.abcB + abc.abcC;
	/*
	if (_texture) { // Only assign texture coordinate if we have a texture!
//		bool recoord = false;

		unsigned H  = _increment(_height);

		if (_tx + width > _tw) {
			_tx = 0;
			if (_ty + H * 2 > _th) {
				_ty = 0;
				// Needs to increase texture size! TODO: check for max texture size based on _featureLevel!
				if (_th < _tw)
					_th *= 2;
				else
					_tw *= 2;

				_clearTexture();
//				recoord = true;
			}
			else
				_ty += H;
		}
	}*/

	Glyph g = { (int32)_tx, (int32)_ty, width, advance, std::min(abc.abcA, 0), charCode, (unsigned)_glyphs.size(), nullptr };

	_tx += _increment(width);

	_charCodeToGlyphIndex.insert(std::make_pair(charCode, (unsigned)_glyphs.size()));
	if (charCode < 256)
		_standardChars[charCode] = (unsigned)_glyphs.size();
	_charIndexToGlyphIndex.insert(std::make_pair(charIndex, (unsigned)_glyphs.size()));
	_glyphs.push_back(g);

	/*// After changing texture size, we need to recreate texture coordinates for all glyphs!
	while (recoord) {
		recoord = false;
		_tx = _ty = 0;
		for (size_t i = 0; i < _glyphs.size(); i++) {
			Glyph &g = _glyphs[i];
			if (_tx + g.w > _tw) {
				_tx = 0;
				if (_ty + H * 2 > _th) {
					_ty = 0;
					// Needs to increase texture size (AGAIN)!
					if (_th < _tw)
						_th *= 2;
					else
						_tw *= 2;
					recoord = true;
					break;
				}
				else
					_ty += H;
			}
			g.tx = _tx;
			g.ty = _ty;
			_tx += _increment(g.w);
		}
	}*/

	return true;
#else
	return false; // Not supported!
#endif
}

HRESULT Font::Update(ID3D11DeviceContext *context)
{
	HRESULT hr;

	SID3D11Device device;
	context->GetDevice(&device);
	D3D_FEATURE_LEVEL featureLevel = device->GetFeatureLevel();

	// Update glyphs
	if (_glyphsUpdateCount < _glyphs.size()) {
		// Assign texture coordinates to new glyphs!
		unsigned H = _increment(_height);

		for (bool recoord = true; recoord;) {
			recoord = false;
					
			for (size_t i = _glyphsUpdateCount; i < _glyphs.size(); i++) {
				Glyph &g = _glyphs[i];
				if (_tx + g.w > _tw) {
					_tx = 0;
					if (_ty + H * 2 > _th) {
						_ty = 0;
						// Needs to increase texture size!
						if (_th < _tw)
							_th *= 2;
						else
							_tw *= 2;
						recoord = true;
						_tx = _ty = 0;
						_textureSRV = nullptr;
						_texture = nullptr;
						_glyphsUpdateCount = 0;
						break;
					}
					else
						_ty += H;
				}
				g.tx = _tx;
				g.ty = _ty;
				_tx += _increment(g.w);
			}
		}

		if (!_texture) {
			D3D11_TEXTURE2D_DESC tDesc;
			tDesc.ArraySize = 1;
			tDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
			tDesc.CPUAccessFlags = 0;
			tDesc.Format = DXGI_FORMAT_R8_UNORM;
			tDesc.Height = _th;
			tDesc.MipLevels = 1 + _mipLevels;
			tDesc.MiscFlags = 0;
			tDesc.SampleDesc.Count = 1;
			tDesc.SampleDesc.Quality = 0;
			tDesc.Usage = D3D11_USAGE_DEFAULT;
			tDesc.Width = _tw;
			V_RETURN(device->CreateTexture2D(&tDesc, nullptr, &_texture));

			D3D11_SHADER_RESOURCE_VIEW_DESC d;
			d.Format = tDesc.Format;
			d.ViewDimension = D3D_SRV_DIMENSION_TEXTURE2D;
			d.Texture2D.MipLevels = tDesc.MipLevels;
			d.Texture2D.MostDetailedMip = 0;
			V_RETURN(device->CreateShaderResourceView(_texture, &d, &_textureSRV));
		}

		// Iterate and render glyphs to texture.
		for (unsigned i = _glyphsUpdateCount; i < _glyphs.size(); i++) {
			const Glyph &g = _glyphs[i];

			unsigned w = _increment(g.w), h = _increment(_height);
			uint8 *data = nullptr;
			
#ifdef WINDESKTOP
			if (g.texData == nullptr) {
				Char txt[3] = {Char(g.charCode), MCHAR('\0')};

				if (ExtTextOutA(_hDC, -g.movex, 0, ETO_OPAQUE, NULL, txt, 1, NULL) == FALSE)
					return E_FAIL;

				BITMAP bitmap;
				GetObject(_hbmBitmap, sizeof(BITMAP), &bitmap);

				data = (uint8*)mmalloc(w * h);
				if (w > (unsigned)g.w || h > _height)
					memset(data, 0, sizeof(uint8) * w * h);

				for (int32 q = 0; q < (int32)_height; q++) {
					for (int32 p = 0; p < g.w; p++) {
						uint8 *gl = (uint8*)bitmap.bmBits + q * bitmap.bmWidthBytes + p * 4;
						data[w * q + p] = uint8((float32(gl[0]) + float32(gl[1]) + float32(gl[2])) / 3.0f + 0.5f);
					}
				}
			}
			else 
#endif
			{
				data = (uint8*)mmalloc(w * h);
				if (w > (unsigned)g.w || h > _height)
					memset(data, 0, sizeof(uint8) * w * h);

				for (int32 q = 0; q < (int32)_height; q++) {
					for (int32 p = 0; p < g.w; p++) {
						data[w * q + p] = ((const uint8*)g.texData)[g.w * q + p];
					}
				}
			}

			D3D11_BOX box = { (UINT)g.tx, (UINT)g.ty, 0u, (UINT)(g.tx + g.w), (UINT)(g.ty + _height), 1u };
			context->UpdateSubresource(_texture, 0, &box, data, w, 0);

			// Generate mip chain for the glyph. We could use the context->GenerateMips(...) funtion, but it requires feature level 10 for given format! (AND it must generate for the entire texture!)
			for (unsigned m = 0, px = g.tx, py = g.ty; m < _mipLevels; m++, w /= 2, h /= 2, px /= 2, py /= 2) {
				for (unsigned b = 0; b < h - 1; b += 2) {
					for (unsigned a = 0; a < w - 1; a += 2) {
						data[b / 2 * w / 2 + a / 2] = uint8((float32(data[b * w + a]) + float32(data[b * w + a + 1]) + float32(data[(b + 1) * w + a]) + float32(data[(b + 1) * w + a + 1])) * 0.25f + 0.5f);
					}
				}
				D3D11_BOX box = { px / 2, py / 2, 0, (px + w) / 2, (py + h) / 2, 1 };
				context->UpdateSubresource(_texture, m + 1, &box, data, w / 2, 0);
			}

			mmfree(data);
		}
		V_RETURN(_updateGlyphBuffer(context, _glyphsUpdateCount, featureLevel));

		_glyphsUpdateCount = (unsigned)_glyphs.size();
	}
	return S_OK;
}

void Font::OnDestroyDevice()
{
	_textureSRV = nullptr;
	_glyphBufferSRV = nullptr;
	_texture = nullptr;
	_glyphBuffer = nullptr;
	_glyphsUpdateCount = 0;
}

HRESULT Font::_updateGlyphBuffer(ID3D11DeviceContext *context, unsigned fromGlyphIndex, D3D_FEATURE_LEVEL featureLevel)
{
	HRESULT hr;

	// TODO: For tech without gs we could use several constant buffers to fit all glyphs...?
	// Then we could just check the glyphIndex to see which cb to use in the textwriter.
	// eg 0 <= index < VERTEX_GLYPH_BUFFER_SIZE uses cb 0 and so on...

	if (featureLevel < D3D_FEATURE_LEVEL_10_0) { // No support for geometry shaders?
		fromGlyphIndex = 0; // partial update of constant buffer is not allowed.
		if (_glyphBufferCapacity != VERTEX_GLYPH_BUFFER_SIZE) {
			_glyphBufferCapacity = VERTEX_GLYPH_BUFFER_SIZE;
			_glyphBuffer = nullptr;
			_glyphBufferSRV = nullptr;			
		}
	}
	else if (_glyphs.size() > _glyphBufferCapacity) {
		_glyphBufferCapacity = std::max(_glyphBufferCapacity * 2, (unsigned)_glyphs.size());
		_glyphBuffer = nullptr;
		_glyphBufferSRV = nullptr;
	}

	if (!_glyphBuffer) {
		D3D11_BUFFER_DESC bDesc;
		if (featureLevel < D3D_FEATURE_LEVEL_10_0)
			bDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		else
			bDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

		bDesc.ByteWidth = sizeof(XMFLOAT4) * _glyphBufferCapacity;
		bDesc.CPUAccessFlags = 0;
		bDesc.Usage = D3D11_USAGE_DEFAULT;
		bDesc.MiscFlags = 0;
		bDesc.StructureByteStride = 0;
		SID3D11Device device;
		context->GetDevice(&device);
		V_RETURN(device->CreateBuffer(&bDesc, nullptr, &_glyphBuffer));

		if (featureLevel >= D3D_FEATURE_LEVEL_10_0) {
			D3D11_SHADER_RESOURCE_VIEW_DESC d;
			d.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
			d.ViewDimension = D3D_SRV_DIMENSION_BUFFER;
			d.Buffer.ElementOffset = 0;
			d.Buffer.ElementWidth = sizeof(XMFLOAT4);
			d.Buffer.FirstElement = 0;
			d.Buffer.NumElements = _glyphBufferCapacity;
			V_RETURN(device->CreateShaderResourceView(_glyphBuffer, &d, &_glyphBufferSRV));
		}
		fromGlyphIndex = 0; // update entire buffer!
	}

	List<XMFLOAT4> g;
	for (size_t i = fromGlyphIndex, j = std::min(_glyphs.size(), (size_t)_glyphBufferCapacity); i < j; i++)
		g.push_back(XMFLOAT4(float32(_glyphs[i].tx) / _tw, float32(_glyphs[i].ty) / _th, float32(_glyphs[i].w) / _tw, float32(_glyphs[i].w)));

	if (featureLevel < D3D_FEATURE_LEVEL_10_0) {
		for (size_t i = _glyphs.size(); i < _glyphBufferCapacity; i++)
			g.push_back(XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)); // Have to fill up entire buffer!
		context->UpdateSubresource(_glyphBuffer, 0, nullptr, &g.front(), 0, 0); // No partial update of cb!
	}
	else {
		D3D11_BOX box = { unsigned(sizeof(XMFLOAT4) * fromGlyphIndex), 0, 0, unsigned(sizeof(XMFLOAT4) * _glyphs.size()), 1, 1 };
		context->UpdateSubresource(_glyphBuffer, 0, &box, &g.front(), 0, 0); 
	}

	return S_OK;
}





/*
	_tw = NextPowerOf2(recTexSize);
	_th = NextPowerOf2(recTexSize * recTexSize / _tw);
	unsigned H = _increment(_height);

	for (bool recoord = true; recoord;) {
		recoord = false;
		_tx = _ty = 0;
		for (size_t i = 0; i < _glyphs.size(); i++) {
			Glyph &g = _glyphs[i];
			if (_tx + g.w > _tw) {
				_tx = 0;
				if (_ty + H * 2 > _th) {
					_ty = 0;
					// Needs to increase texture size (AGAIN)!
					if (_th < _tw)
						_th *= 2;
					else
						_tw *= 2;
					recoord = true;
					break;
				}
				else
					_ty += H;
			}
			g.tx = _tx;
			g.ty = _ty;
			_tx += _increment(g.w);
		}
	}
	//unsigned minTexSize = unsigned(sqrt(float32(_tx * _increment(_height) + _tw * _increment(_ty))) + 0.5f);

	_featureLevel = D3D_FEATURE_LEVEL_9_1;
*/

/*
struct FontDesc
{
	unsigned size; // Size of the font.
	unsigned height; // Height of a character in pixels
	int32 dyBaseline; // Number of pixels from lower left corner of character to baseline.
	unsigned textureWidth; // Width of texture
	unsigned textureHeight; // Height of texture
	unsigned lineGap; // Recommended line gap
	unsigned mipLevels; // Number of mip levels.
	List<Font::Glyph> glyphs; // Glyph descriptions.
	DataBuffer textureData; // Binary data for the font texture.
};

HRESULT Font::GetFont(ID3D11Device *device)
{
	HRESULT hr;

	{

	}

	FontDesc fd;
	
	SID3D11DeviceContext context;
	device->GetImmediateContext(&context);
	
	D3D11_TEXTURE2D_DESC tDesc;
	_texture->GetDesc(&tDesc);
	tDesc.BindFlags = 0;
	tDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	tDesc.Usage = D3D11_USAGE_STAGING;
	tDesc.MiscFlags = 0;
	tDesc.MipLevels = 1;
	SID3D11Texture2D cpuTex;
	V_RETURN(device->CreateTexture2D(&tDesc, nullptr, &cpuTex));

	context->CopySubresourceRegion(cpuTex, 0, 0, 0, 0, _texture, 0, nullptr);
	
	D3D11_MAPPED_SUBRESOURCE mapped;
	V_RETURN(context->Map(cpuTex, 0, D3D11_MAP_READ, 0, &mapped));

	fd.textureData = DataBuffer(sizeof(uint8) * tDesc.Width * tDesc.Height);
	uint8* imgData = (uint8*)fd.textureData.getBuffer();
	
	for (UINT j = 0; j < tDesc.Height; j++)
		for (UINT i = 0; i < tDesc.Width; i++)
			imgData[j * tDesc.Width + i] = *((BYTE*)mapped.pData + j * mapped.RowPitch + i);

	context->Unmap(cpuTex, 0);

	fd.size = _size;
	fd.height = _height;
	fd.dyBaseline = _dyBaseline;
	fd.textureWidth = _tw;
	fd.textureHeight = _th;
	fd.lineGap = _lineGap;
	fd.mipLevels = _mipLevels;
	fd.glyphs = _glyphs;

	return S_OK;
}*/