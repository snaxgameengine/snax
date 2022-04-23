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

#include "GraphicsChips/Texture.h"
#include <d3d12.h>
#include <DirectXTex.h>


namespace m3d
{


struct TextureConvertInfo
{
	size_t width;
	size_t height;
	size_t mipLevels;
	DXGI_FORMAT format;
	DirectX::TEX_FILTER_FLAGS filter;
	DirectX::TEX_FILTER_FLAGS mipFilter;
	DirectX::TEX_FR_FLAGS flipRotate;
	DirectX::TEX_COMPRESS_FLAGS compress;
	bool nmConvert;
	DirectX::CNMAP_FLAGS nmFlags;
	float32 nmAmplitude;
	bool forceDDSFormat;
	TextureConvertInfo() : width(0), height(0), mipLevels(0), format(DXGI_FORMAT_UNKNOWN), filter(TEX_FILTER_DEFAULT), mipFilter(TEX_FILTER_DEFAULT), flipRotate(TEX_FR_ROTATE0), compress(TEX_COMPRESS_DEFAULT), nmConvert(false), nmFlags(CNMAP_DEFAULT), nmAmplitude(1.0f), forceDDSFormat(false) {}
};

struct TextureUtil
{
	static HRESULT Decompress(DirectX::ScratchImage *&image, DirectX::ScratchImage *&tmpImage, DXGI_FORMAT fmt);
	static HRESULT FlipRotate(DirectX::ScratchImage *&image, DirectX::ScratchImage *&tmpImage, DirectX::TEX_FR_FLAGS flags);
	static HRESULT Resize(DirectX::ScratchImage *&image, DirectX::ScratchImage *&tmpImage, size_t width, size_t height, DirectX::TEX_FILTER_FLAGS flags);
	static HRESULT GenerateMipMaps(DirectX::ScratchImage *&image, DirectX::ScratchImage *&tmpImage, DirectX::TEX_FILTER_FLAGS filter, size_t levels);
	static HRESULT ComputeNormalMap(DirectX::ScratchImage *&image, DirectX::ScratchImage *&tmpImage, DirectX::CNMAP_FLAGS flags, float32 amplitude, DXGI_FORMAT format);
#if defined(__d3d11_h__) || defined(__d3d11_x_h__)
	static HRESULT Convert(DirectX::ScratchImage *&image, DirectX::ScratchImage *&tmpImage, DXGI_FORMAT format, DirectX::TEX_FILTER_FLAGS filter, DirectX::TEX_COMPRESS_FLAGS compress, float32 threshold, ID3D11Device *device);
	static HRESULT Convert(DirectX::ScratchImage *&image, DirectX::ScratchImage *&tmpImage, const TextureConvertInfo &nfo, bool *converted = nullptr, ID3D11Device *device);
	static HRESULT Convert(DataBuffer &db, ImageFileFormat &iff, const TextureConvertInfo &nfo, ID3D11Device *device);
#else
	static HRESULT Convert(DirectX::ScratchImage *&image, DirectX::ScratchImage *&tmpImage, DXGI_FORMAT format, DirectX::TEX_FILTER_FLAGS filter, DirectX::TEX_COMPRESS_FLAGS compress, float32 threshold);
	static HRESULT Convert(DirectX::ScratchImage *&image, DirectX::ScratchImage *&tmpImage, const TextureConvertInfo &nfo, bool *converted = nullptr);
	static HRESULT Convert(DataBuffer &db, ImageFileFormat &iff, const TextureConvertInfo &nfo);
#endif
	static HRESULT GetMetainfo(const DataBuffer &db, ImageFileFormat iff, DirectX::TexMetadata &meta);
	static size_t CalculateMipLevels(const DirectX::TexMetadata &meta);

	static HRESULT LoadFromDataBuffer(const DataBuffer &db, ImageFileFormat iff, ScratchImage &image);
	static HRESULT SaveToFile(const DataBuffer &db, ImageFileFormat iff, Path filename);
	static HRESULT SaveToFile(const ScratchImage &image, Path filename);
	static HRESULT SaveToMemory(const ScratchImage &image, DirectX::Blob &b, ImageFileFormat iff);
};


}