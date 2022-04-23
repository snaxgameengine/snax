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
#include "TextWriter.hlsl.h"
#include "M3DCore/DataBufferStream.h"
#include "Graphics.h"
#include "RenderSettings.h"
#include "ResourceStateTracker.h"

using namespace m3d;


uint32 NextPowerOf2(uint32 n)
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

SIZE_T __align(SIZE_T location, UINT alignment)
{
	return (location + alignment - 1) & ~UINT64(alignment - 1);
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

bool Font::Init(String font, uint32 size, Weight weight, bool italic, bool underline, bool strikeout, uint32 mipLevels)
{
#ifdef WINDESKTOP
	_hDC = CreateCompatibleDC(NULL);
	if (_hDC == NULL)
		return false;

	int32 w;
	switch (weight) {
	case Weight::THIN: w = FW_THIN; break;
	case Weight::EXTRALIGHT: w = FW_EXTRALIGHT; break;
	case Weight::LIGHT: w = FW_LIGHT; break;
	case Weight::NORMAL: w = FW_NORMAL; break;
	case Weight::MEDIUM: w = FW_MEDIUM; break;
	case Weight::SEMIBOLD: w = FW_SEMIBOLD; break;
	case Weight::BOLD: w = FW_BOLD; break;
	case Weight::EXTRABOLD: w = FW_EXTRABOLD; break;
	case Weight::HEAVY: w = FW_HEAVY; break;
	default: w = FW_NORMAL; break;
	}

	_size = size;

	int32 s = -MulDiv(int32(size), GetDeviceCaps(_hDC, LOGPIXELSY), 72);
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
	_tw = (uint32)pow(2, ceil(logf(sqrtf(float(metrics.otmTextMetrics.tmAveCharWidth*_height*250))) / logf(2.0f))); // trying to set a minimum texture size for 250 characters
	_th = (uint32)pow(2, ceil(logf(float(metrics.otmTextMetrics.tmAveCharWidth*_height*250/_tw)) / logf(2.0f)));
	_tw = std::min(2048u, _tw);
	_th = std::min(2028u, _th);

	// Add standard characters.
	for (uint32 i = 32; i < 256; i++) // exclude 0-31. Non printables!
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

	uint32 recTexSize = 0;

	dbs >> recTexSize >> _size >> _height >> _dyBaseline >> _lineGap >> _mipLevels;
	if (dbs.IsError())
		return false;

	while (dbs.GetBufferPos() < bufferSize) {
		uint32 charCode;
		uint32 x;
		dbs >> charCode >> x;
		if (x == uint32(-1)) {
			uint32 charCode2;
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
			uint32 s = _height * width;
			const void *glyphTexData = dbs.GetDataBuffer().getConstBuffer() + dbs.GetBufferPos();
			dbs.MoveBufferPos(s);
			if (dbs.IsError())
				return false;
			Glyph g = { 0, 0, width, advance, movex, charCode, (uint32)_glyphs.size(), glyphTexData };
			if (!_charCodeToGlyphIndex.insert(std::make_pair(charCode, (uint32)_glyphs.size())).second)
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


bool Font::SerializeFont(DataBuffer &db, size_t &bufferSize, uint32 firstChar, uint32 lastChar, uint32 maxTextureSize, uint32 maxCharacters) const
{
#ifdef WINDESKTOP
	DataBufferOutputStream dbs(db);

	uint32 count = 0;
	uint32 tx = 0;
	uint32 ty = 0;

	dbs << uint32(0) << _size << _height << _dyBaseline << _lineGap << _mipLevels;

	Map<uint32, uint32> charIndexToCharCode;
	for (uint32 charCode = firstChar; charCode <= lastChar; charCode++) {
		if (count > maxCharacters)
			return false; // Not enough glyph buffer space.

		Char txt[3] = { Char(charCode), MCHAR('\0')};
		WORD charIndex = 0;
		if (GetGlyphIndicesA(_hDC, txt, 1, &charIndex, GGI_MARK_NONEXISTING_GLYPHS) != 1 || charIndex == WORD(-1))
			continue; // This character does not have a glyph!
		auto e = charIndexToCharCode.insert(std::make_pair(charIndex, charCode));
		if (!e.second) { // Glyph already present! Reuse!
			dbs << charCode << uint32(-1) << e.first->second;
			count++;
			continue;
		}
		ABC abc;
		if (!GetCharABCWidths(_hDC, charCode, charCode, &abc))
			continue;

		int32 width = std::max(abc.abcA, 0) + abc.abcB + std::max(abc.abcC, 0);
		int32 advance = abc.abcA + abc.abcB + abc.abcC;
		int32 movex = std::min(abc.abcA, 0);
		uint32 size = uint32(width * _height);

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

		for (uint32 q = 0; q < _height; q++) {
			for (uint32 p = 0; p < (uint32)width; p++) {
				uint8 *gl = (uint8*)bitmap.bmBits + q * bitmap.bmWidthBytes + p * 4;
				data[width * q + p] = uint8((float(gl[0]) + float(gl[1]) + float(gl[2])) / 3.0f + 0.5f);
			}
		}

		dbs << charCode << width << advance << movex;
		dbs.Write(data, size);

		mmfree(data);

		count++;
	}
	bufferSize = dbs.GetBufferPos();
	uint32 minTexSize = uint32(sqrt(float(tx * _increment(_height) + maxTextureSize * _increment(ty))) + 0.5f);
	dbs.SetBufferPos(0);
	dbs << minTexSize;
/*
	{
		String s = String(MTEXT("const uint8 FONT[%1] =\n{\n\t")).arg(String::fromNum(bufferSize));
		String t;
		const uint8 *C = (const uint8*)db.getConstBuffer();
		for (size_t i = 0, j = bufferSize; i < j;) {
			for (size_t k = std::min(i + 20, j); i < k; i++) {
				s += String(MTEXT("0x%1, ")).arg(String::fromNum((uint32)C[i], MTEXT("%02X")));
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
	for (uint32 i = 0; i < text.length(); i++) {
		if (text[i] == MCHAR('\n')) {
			lineCount++;
			maxWidth = std::max(maxWidth, w + t);
			w = t = 0;
		}
		else {
			uint32 glyphIndex = _getGlyphIndex(text[i]);
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

	const Glyph &h = _glyphs[_getGlyphIndex(MCHAR('-'))];

	uint32 i = 0;
	for (; i < text.length(); i++) { // Iterate letters
		if (text[i] == MCHAR('\n')) { // New line?
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
			uint32 glyphIndex = _getGlyphIndex(text[i]);
			if (glyphIndex == -1)
				continue; // nonprintable...
			const Glyph &g = _glyphs[glyphIndex];

			if (text[i] == MCHAR(' ')) { // Space character?
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
						uint32 j = 0;
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

const Font::Glyph *Font::GetGlyph(uint32 charCode)
{
	uint32 i = _getGlyphIndex(charCode);
	if (i == -1)
		return nullptr;
	return &_glyphs[i];
}

uint32 Font::_getGlyphIndex(uint32 charCode)
{
	uint32 idx = -1;
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

uint32 Font::_increment(uint32 dp) const
{
	return (uint32)__align(dp, 1 << _mipLevels); // How much space does a glyph need to generate _mipLevels extra mip-levels...
}

bool Font::_addChar(uint32 charCode)
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

	Glyph g = { (int32)_tx, (int32)_ty, width, advance, std::min(abc.abcA, 0), charCode, (uint32)_glyphs.size(), nullptr };

	_tx += _increment(width);

	_charCodeToGlyphIndex.insert(std::make_pair(charCode, (uint32)_glyphs.size()));
	if (charCode < 256)
		_standardChars[charCode] = (uint32)_glyphs.size();
	_charIndexToGlyphIndex.insert(std::make_pair(charIndex, (uint32)_glyphs.size()));
	_glyphs.push_back(g);

	return true;
#else
	return false; // Not supported!
#endif
}

HRESULT Font::Update(Graphics *graphics)
{
	HRESULT hr;

	// Update glyphs
	if (_glyphsUpdateCount < _glyphs.size()) {
		// Assign texture coordinates to new glyphs!
		uint32 H = _increment(_height);

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
						_srv = nullptr;
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

		if (_texture) {
			CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(_texture, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COPY_DEST);
			graphics->rs()->ResourceBarrier(1, &barrier);
			graphics->rs()->CommitResourceBarriers();
		}
		else {
			D3D12_RESOURCE_DESC tDesc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R8_UNORM, _tw, _th, 1, 1 + _mipLevels);

			try {
				CD3DX12_HEAP_PROPERTIES hp(D3D12_HEAP_TYPE_DEFAULT);
				V_RETURN(graphics->GetDevice()->CreateCommittedResource(&hp, D3D12_HEAP_FLAG_NONE, &tDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&_texture)));
			}
			catch (const GraphicsException &) {
				return E_FAIL;
			}
		}
		
		// Iterate and render glyphs to texture.
		for (uint32 i = _glyphsUpdateCount; i < _glyphs.size(); i++) {
			const Glyph &g = _glyphs[i];

			uint32 w = _increment(g.w), h = _increment(_height);
			BYTE *data = nullptr;
			
			D3D12_PLACED_SUBRESOURCE_FOOTPRINT placedTexture2D = { };
			placedTexture2D.Footprint = CD3DX12_SUBRESOURCE_FOOTPRINT(DXGI_FORMAT_R8_UNORM, w, h, 1, (UINT)__align(w, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT));
			graphics->GetUploadHeap()->Allocate(h * placedTexture2D.Footprint.RowPitch, D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT, &data, nullptr, &placedTexture2D.Offset);

#ifdef WINDESKTOP
			if (g.texData == nullptr) {
				Char txt[3] = { Char(g.charCode), MCHAR('\0')};

				if (ExtTextOutA(_hDC, -g.movex, 0, ETO_OPAQUE, NULL, txt, 1, NULL) == FALSE)
					return E_FAIL;

				BITMAP bitmap;
				GetObject(_hbmBitmap, sizeof(BITMAP), &bitmap);

				if (w > (uint32)g.w || h > _height)
					memset(data, 0, sizeof(uint8) * w * h);

				for (int32 q = 0; q < (int32)_height; q++) {
					for (int32 p = 0; p < g.w; p++) {
						uint8 *gl = (uint8*)bitmap.bmBits + q * bitmap.bmWidthBytes + p * 4;
						data[placedTexture2D.Footprint.RowPitch * q + p] = uint8((float(gl[0]) + float(gl[1]) + float(gl[2])) / 3.0f + 0.5f);
					}
				}
			}
			else 
#endif
			{
				if (w > (uint32)g.w || h > _height)
					memset(data, 0, sizeof(uint8) * w * h);

				for (int32 q = 0; q < (int32)_height; q++) {
					for (int32 p = 0; p < g.w; p++) {
						data[placedTexture2D.Footprint.RowPitch * q + p] = ((const uint8*)g.texData)[g.w * q + p];
					}
				}
			}

			CD3DX12_TEXTURE_COPY_LOCATION l0(_texture, 0);
			CD3DX12_TEXTURE_COPY_LOCATION l1(graphics->GetUploadHeap()->GetBufferResource(), placedTexture2D);
			CD3DX12_BOX box(0, 0, 0, w, h, 1);
			graphics->rs()->CopyTextureRegion(&l0, g.tx, g.ty, 0, &l1, &box);
			
			/* TODO: Not currently correct, but not used neighter! Fix it when you have to!! :)
			// Generate mip chain for the glyph!
			for (uint32 m = 0, px = g.tx / 2, py = g.ty / 2; m < _mipLevels; m++, w /= 2, h /= 2, px /= 2, py /= 2) {
				px = std::max(px, 1); py = std::max(py, 1); w = std::max(w, 1); h = std::max(h, 1);
				BYTE *data0 = data;
				graphics->GetRingBuffer()->Allocate(h * placedTexture2D.Footprint.RowPitch, D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT, &data, nullptr, &placedTexture2D.Offset);
				for (uint32 b = 0; b < h - 1; b += 2) {
					for (uint32 a = 0; a < w - 1; a += 2) {
						data[b / 2 * placedTexture2D.Footprint.RowPitch + a / 2] = uint8((float(data0[b * w + a]) + float(data0[b * w + a + 1]) + float(data0[(b + 1) * w + a]) + float(data0[(b + 1) * w + a + 1])) * 0.25f + 0.5f);
					}
				}

				graphics->rs()->GetCommandList()->GetCommandList()->CopyTextureRegion(&CD3DX12_TEXTURE_COPY_LOCATION(_texture, 1 + m), px, py, 0, &CD3DX12_TEXTURE_COPY_LOCATION(graphics->GetRingBuffer()->GetBufferResource(), placedTexture2D), &CD3DX12_BOX(0, 0, 0, w / 2, h / 2, 1));
			}*/
		}

		CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(_texture, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		graphics->rs()->ResourceBarrier(1, &barrier);

		V_RETURN(_updateGlyphBuffer(graphics, _glyphsUpdateCount));

		_glyphsUpdateCount = (uint32)_glyphs.size();

		if (!_srv) {
			graphics->GetHeapManager()->GetDescriptorTable(2, &_srv);

			{
				Descriptor &d = _srv->InitDescriptor(0, DescriptorType::SRV);
				d.resource = _glyphBuffer;
				d.srv->Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
				d.srv->ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
				d.srv->Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
				d.srv->Buffer.FirstElement = 0;
				d.srv->Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
				d.srv->Buffer.NumElements = _glyphBufferCapacity;
				d.srv->Buffer.StructureByteStride = 0;
			}
			{
				Descriptor &d = _srv->InitDescriptor(1, DescriptorType::SRV);
				d.resource = _texture;
				d.srv->Format = DXGI_FORMAT_R8_UNORM;
				d.srv->ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
				d.srv->Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
				d.srv->Texture2D.MipLevels = -1;
				d.srv->Texture2D.MostDetailedMip = 0;
				d.srv->Texture2D.PlaneSlice = 0;
				d.srv->Texture2D.ResourceMinLODClamp = 0;
			}
			_srv->CreateDescriptors();
		}
	}
	return S_OK;
}

void Font::OnDestroyDevice()
{
	_srv = nullptr;
	_texture = nullptr;
	_glyphBuffer = nullptr;
	_glyphsUpdateCount = 0;
}

HRESULT Font::_updateGlyphBuffer(Graphics *graphics, uint32 fromGlyphIndex)
{
	HRESULT hr;

	// TODO: For tech without gs we could use several constant buffers to fit all glyphs...?
	// Then we could just check the glyphIndex to see which cb to use in the textwriter.
	// eg 0 <= index < VERTEX_GLYPH_BUFFER_SIZE uses cb 0 and so on...

	if (_glyphs.size() > _glyphBufferCapacity) {
		_glyphBufferCapacity = std::max(_glyphBufferCapacity * 2, (uint32)_glyphs.size());
		_srv = nullptr;
		_glyphBuffer = nullptr;
	}

	if (_glyphBuffer) {
		CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(_glyphBuffer, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, D3D12_RESOURCE_STATE_COPY_DEST);
		graphics->rs()->ResourceBarrier(1, &barrier);
		graphics->rs()->CommitResourceBarriers();
	}
	else {
		D3D12_RESOURCE_DESC bDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(XMFLOAT4) * _glyphBufferCapacity);
		
		CD3DX12_HEAP_PROPERTIES hp(D3D12_HEAP_TYPE_DEFAULT);
		V_RETURN(graphics->GetDevice()->CreateCommittedResource(&hp, D3D12_HEAP_FLAG_NONE, &bDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&_glyphBuffer)));

		fromGlyphIndex = 0; // update entire buffer!
	}

	List<XMFLOAT4> g;
	for (size_t i = fromGlyphIndex, j = std::min(_glyphs.size(), (size_t)_glyphBufferCapacity); i < j; i++)
		g.push_back(XMFLOAT4(float(_glyphs[i].tx) / _tw, float(_glyphs[i].ty) / _th, float(_glyphs[i].w) / _tw, float(_glyphs[i].w)));

	XMFLOAT4 *buffer = nullptr;
	UINT64 offset = 0;
	V_RETURN(graphics->GetUploadHeap()->Allocate(sizeof(XMFLOAT4) * g.size(), 512, (BYTE**)&buffer, nullptr, &offset)); // Is 512 bytes alignment neccessary?

	std::memcpy(buffer, &g.front(), sizeof(XMFLOAT4) * g.size());

	graphics->rs()->CopyBufferRegion(_glyphBuffer, sizeof(XMFLOAT4) * fromGlyphIndex, graphics->GetUploadHeap()->GetBufferResource(), offset, sizeof(XMFLOAT4) * g.size());
	CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(_glyphBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	graphics->rs()->ResourceBarrier(1, &barrier);

	return S_OK;
}

