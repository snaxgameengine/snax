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

#include "stdafx.h"
#include "TextureUtil.h"
#include "M3DEngine/Err.h"
#include "M3DCore/Util.h"
#pragma warning(push)
#pragma warning(disable : 4005)
#include <wincodec.h> // <== This gives some c4005
#pragma warning(pop)

using namespace m3d;
using namespace DirectX;

DXGI_FORMAT _DefaultDecompress( _In_ DXGI_FORMAT format ) // Copied from DirectXTexCompress.cpp
{
    switch( format )
    {
    case DXGI_FORMAT_BC1_TYPELESS:
    case DXGI_FORMAT_BC1_UNORM:
    case DXGI_FORMAT_BC2_TYPELESS:
    case DXGI_FORMAT_BC2_UNORM:
    case DXGI_FORMAT_BC3_TYPELESS:
    case DXGI_FORMAT_BC3_UNORM:
    case DXGI_FORMAT_BC7_TYPELESS:
    case DXGI_FORMAT_BC7_UNORM:
        return DXGI_FORMAT_R8G8B8A8_UNORM;

    case DXGI_FORMAT_BC1_UNORM_SRGB:
    case DXGI_FORMAT_BC2_UNORM_SRGB:
    case DXGI_FORMAT_BC3_UNORM_SRGB:
    case DXGI_FORMAT_BC7_UNORM_SRGB:
        return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;

    case DXGI_FORMAT_BC4_TYPELESS:
    case DXGI_FORMAT_BC4_UNORM:
        return DXGI_FORMAT_R8_UNORM;

    case DXGI_FORMAT_BC4_SNORM:
        return DXGI_FORMAT_R8_SNORM;

    case DXGI_FORMAT_BC5_TYPELESS:
    case DXGI_FORMAT_BC5_UNORM:
        return DXGI_FORMAT_R8G8_UNORM;

    case DXGI_FORMAT_BC5_SNORM:
        return DXGI_FORMAT_R8G8_SNORM;

    case DXGI_FORMAT_BC6H_TYPELESS:
    case DXGI_FORMAT_BC6H_UF16:
    case DXGI_FORMAT_BC6H_SF16:
        // We could use DXGI_FORMAT_R32G32B32_FLOAT here since BC6H is always Alpha 1.0,
        // but this format is more supported by viewers
        return DXGI_FORMAT_R32G32B32A32_FLOAT;
    }
    return DXGI_FORMAT_UNKNOWN;
}

static size_t _CountMips( _In_ size_t width, _In_ size_t height ) // Copied from DirectXTexMipMaps.cpp
{
    size_t mipLevels = 1;

    while ( height > 1 || width > 1 )
    {
        if ( height > 1 )
            height >>= 1;

        if ( width > 1 )
            width >>= 1;

        ++mipLevels;
    }
    
    return mipLevels;
}

static size_t _CountMips3D( _In_ size_t width, _In_ size_t height, _In_ size_t depth ) // Copied from DirectXTexMipMaps.cpp
{
    size_t mipLevels = 1;

    while ( height > 1 || width > 1 || depth > 1 )
    {
        if ( height > 1 )
            height >>= 1;

        if ( width > 1 )
            width >>= 1;

        if ( depth > 1 )
            depth >>= 1;

        ++mipLevels;
    }
    
    return mipLevels;
}

HRESULT TextureUtil::Decompress(DirectX::ScratchImage *&image, DirectX::ScratchImage *&tmpImage, DXGI_FORMAT fmt)
{
	HRESULT hr;
	V_RETURN(DirectX::Decompress(image->GetImages(), image->GetImageCount(), image->GetMetadata(), fmt, *tmpImage)); // Use only 1st level. WHY!?!?!
//	V_RETURN(DirectX::Decompress(image->GetImages(), std::min(image->GetImageCount(), 1), image->GetMetadata(), fmt, *tmpImage)); // Use only 1st level.
	std::swap(image, tmpImage);
	return S_OK;
}

HRESULT TextureUtil::FlipRotate(DirectX::ScratchImage *&image, DirectX::ScratchImage *&tmpImage, DirectX::TEX_FR_FLAGS flags)
{
	HRESULT hr;
	if (IsCompressed(image->GetMetadata().format)) {
		V_RETURN(Decompress(image, tmpImage, DXGI_FORMAT_UNKNOWN));
	}
	V_RETURN(DirectX::FlipRotate(image->GetImages(), image->GetImageCount(), image->GetMetadata(), flags, *tmpImage));
	std::swap(image, tmpImage);
	return S_OK;
}

HRESULT TextureUtil::Resize(DirectX::ScratchImage *&image, DirectX::ScratchImage *&tmpImage, size_t width, size_t height, DirectX::TEX_FILTER_FLAGS flags)
{
	HRESULT hr;
	if (IsCompressed(image->GetMetadata().format)) {
		V_RETURN(Decompress(image, tmpImage, DXGI_FORMAT_UNKNOWN));
	}
	V_RETURN(DirectX::Resize(image->GetImages(), image->GetImageCount(), image->GetMetadata(), width, height, flags, *tmpImage));
	std::swap(image, tmpImage);
	return S_OK;
}

HRESULT TextureUtil::Convert(
	DirectX::ScratchImage *&image, 
	DirectX::ScratchImage *&tmpImage, 
	DXGI_FORMAT format, 
	DirectX::TEX_FILTER_FLAGS filter,
	DirectX::TEX_COMPRESS_FLAGS compress, 
	float32 threshold
#if defined(__d3d11_h__) || defined(__d3d11_x_h__)
	, ID3D11Device *device
#endif
	)
{
	HRESULT hr;
	if (IsCompressed(image->GetMetadata().format)) {
		V_RETURN(Decompress(image, tmpImage, DXGI_FORMAT_UNKNOWN));
	}
	if (IsCompressed(format)) {
//		format = _DefaultDecompress(format);
//		if (image->GetMetadata().width < 4 || image->GetMetadata().height < 4)
//			compress &= ~TEX_COMPRESS_PARALLEL;
#if defined(__d3d11_h__) || defined(__d3d11_x_h__)
		if (device && (format == DXGI_FORMAT_BC6H_UF16 || format == DXGI_FORMAT_BC6H_SF16 || format == DXGI_FORMAT_BC7_UNORM || format == DXGI_FORMAT_BC7_UNORM_SRGB)) 
		{
			V_RETURN(DirectX::Compress(device, image->GetImages(), image->GetImageCount(), image->GetMetadata(), format, compress, 1.0f, *tmpImage));
		}
		else 
#endif
		{
			V_RETURN(DirectX::Compress(image->GetImages(), image->GetImageCount(), image->GetMetadata(), format, compress, threshold, *tmpImage));
		}
	}
	else if (image->GetMetadata().format != format) {
		V_RETURN(DirectX::Convert(image->GetImages(), image->GetImageCount(), image->GetMetadata(), format, filter, threshold, *tmpImage));
	}
	std::swap(image, tmpImage);
	return S_OK;
}

HRESULT TextureUtil::GenerateMipMaps(DirectX::ScratchImage *&image, DirectX::ScratchImage *&tmpImage, DirectX::TEX_FILTER_FLAGS filter, size_t levels)
{
	HRESULT hr;
	if (IsCompressed(image->GetMetadata().format)) {
		V_RETURN(Decompress(image, tmpImage, DXGI_FORMAT_UNKNOWN));
	}
	if (image->GetMetadata().dimension == TEX_DIMENSION_TEXTURE3D) {
		V_RETURN(DirectX::GenerateMipMaps3D(image->GetImages(), image->GetImageCount(), image->GetMetadata(), filter, levels, *tmpImage));
	}
	else {
		V_RETURN(DirectX::GenerateMipMaps(image->GetImages(), image->GetImageCount(), image->GetMetadata(), filter, levels, *tmpImage));
	}
	std::swap(image, tmpImage);
	return S_OK;
}

HRESULT TextureUtil::ComputeNormalMap(DirectX::ScratchImage *&image, DirectX::ScratchImage *&tmpImage, DirectX::CNMAP_FLAGS flags, float32 amplitude, DXGI_FORMAT format)
{
	HRESULT hr;
	if (IsCompressed(image->GetMetadata().format)) {
		V_RETURN(Decompress(image, tmpImage, DXGI_FORMAT_UNKNOWN));
	}
	if (IsCompressed(format))
		format = _DefaultDecompress(format);
	V_RETURN(DirectX::ComputeNormalMap(image->GetImages(), image->GetImageCount(), image->GetMetadata(), flags, amplitude, format, *tmpImage));
	std::swap(image, tmpImage);
	return S_OK;
}

HRESULT TextureUtil::Convert(
	DirectX::ScratchImage *&image,
	DirectX::ScratchImage *&tmpImage, 
	const TextureConvertInfo &nfo, 
	bool *converted
#if defined(__d3d11_h__) || defined(__d3d11_x_h__)	
	, ID3D11Device *device
#endif
	)
{
	HRESULT hr;

	if (converted)
		*converted = false;

	if (nfo.flipRotate != TEX_FR_ROTATE0) {
		V_RETURN(FlipRotate(image, tmpImage, nfo.flipRotate));
		if (converted)
			*converted = true;
	}

	if (nfo.width != image->GetMetadata().width && nfo.width != 0 || nfo.height != image->GetMetadata().height && nfo.height != 0) {
		V_RETURN(Resize(image, tmpImage, nfo.width ? nfo.width : image->GetMetadata().width, nfo.height ? nfo.height : image->GetMetadata().height, nfo.filter));
		if (converted)
			*converted = true;
	}

	if (nfo.nmConvert) {
		V_RETURN(ComputeNormalMap(image, tmpImage, nfo.nmFlags, nfo.nmAmplitude, nfo.format));
		if (converted)
			*converted = true;
	}
	
	
	// TODO: I have to improve this! Should be able to remove miplevels without generating them. Also, the current code does not handle setting mip-levels to 1...
	size_t mipLevels = nfo.mipLevels == 0 ? CalculateMipLevels(image->GetMetadata()) : std::min(nfo.mipLevels, CalculateMipLevels(image->GetMetadata()));

	if (mipLevels != image->GetMetadata().mipLevels) {
		if (mipLevels > 1) {
			V_RETURN(GenerateMipMaps(image, tmpImage, nfo.mipFilter, mipLevels));
			if (converted)
				*converted = true;
		}
		else {
		}
	}

	// NOTE: I belive there is a bug in the compress function of DirectXTex. It doesn't handle non-multiples of 4 mip-levels correctly.
	// In fact, I think it read out of the data array at the lowest scanline.
	// Also, I think there is a similar bug in the Decompress function. Miplevels of non-multiple of 4 wont decompress.
	// It does not seem to handle cases where you specify a smaller number than the actual number of images as image-count.
	if (nfo.format != image->GetMetadata().format) {
#if defined(__d3d11_h__) || defined(__d3d11_x_h__)	
		V_RETURN(Convert(image, tmpImage, nfo.format, nfo.filter, nfo.compress, 0, device)); // TEX_COMPRESS_PARALLEL
#else
		V_RETURN(Convert(image, tmpImage, nfo.format, nfo.filter, nfo.compress, 0)); // TEX_COMPRESS_PARALLEL
#endif
		if (converted)
			*converted = true;
	}

	return S_OK;
}

HRESULT TextureUtil::Convert(
	DataBuffer &db, 
	ImageFileFormat &iff, 
	const TextureConvertInfo &nfo
#if defined(__d3d11_h__) || defined(__d3d11_x_h__)	
	, ID3D11Device *device
#endif
	)
{
	HRESULT hr;

	ScratchImage a, b;
	ScratchImage *image = &a, *tmpImage = &b;

	ImageCodecType ict = GetImageCodecType(iff);

	if (ict == ICT_DDS) {
		V_RETURN(LoadFromDDSMemory(db.getConstBuffer(), db.getBufferSize(), DirectX::DDS_FLAGS_NONE, nullptr, *image));
	}
	else if (ict == ICT_WIC) {
		V_RETURN(LoadFromWICMemory(db.getConstBuffer(), db.getBufferSize(), DirectX::WIC_FLAGS_NONE, nullptr, *image));
	}
	else if (ict == ICT_TGA) {
		V_RETURN(LoadFromTGAMemory(db.getConstBuffer(), db.getBufferSize(), nullptr, *image));
	}
	else if (ict == ICT_HDR) {
		V_RETURN(LoadFromHDRMemory(db.getConstBuffer(), db.getBufferSize(), nullptr, *image));
	}
	else
		return E_FAIL;

	bool converted = false;
	
#if defined(__d3d11_h__) || defined(__d3d11_x_h__)	
	V_RETURN(TextureUtil::Convert(image, tmpImage, nfo, &converted, device));
#else
	V_RETURN(TextureUtil::Convert(image, tmpImage, nfo, &converted));
#endif

	if (ict == ICT_TGA || ict == ICT_HDR || converted || ict != ICT_DDS && nfo.forceDDSFormat) {
		Blob b;
		V_RETURN(SaveToDDSMemory(image->GetImages(), image->GetImageCount(), image->GetMetadata(), DirectX::DDS_FLAGS_NONE, b));
		db.setBufferData((const uint8*)b.GetBufferPointer(), b.GetBufferSize());
		ict = ICT_DDS;
		iff = IFF_DDS;
	}

	return S_OK;
}

HRESULT TextureUtil::GetMetainfo(const DataBuffer &db, ImageFileFormat iff, DirectX::TexMetadata &meta)
{
	ImageCodecType ict = GetImageCodecType(iff);

	HRESULT hr;

	if (ict == ICT_DDS) {
		V_RETURN(DirectX::GetMetadataFromDDSMemory(db.getConstBuffer(), db.getBufferSize(), DirectX::DDS_FLAGS_NONE, meta));
	}
	else if (ict == ICT_WIC) {
		V_RETURN(DirectX::GetMetadataFromWICMemory(db.getConstBuffer(), db.getBufferSize(), DirectX::WIC_FLAGS_NONE, meta));
	}
	else if (ict == ICT_TGA) {
		V_RETURN(DirectX::GetMetadataFromTGAMemory(db.getConstBuffer(), db.getBufferSize(), meta));
	}
	else if (ict == ICT_HDR) {
		V_RETURN(DirectX::GetMetadataFromHDRMemory(db.getConstBuffer(), db.getBufferSize(), meta));
	}
	else
		return E_INVALIDARG;

	return S_OK;
}

size_t TextureUtil::CalculateMipLevels(const DirectX::TexMetadata &meta)
{
	if (meta.dimension == TEX_DIMENSION_TEXTURE3D)
		return _CountMips3D(meta.width, meta.height, meta.depth);
	return _CountMips(meta.width, meta.height);
}

HRESULT TextureUtil::LoadFromDataBuffer(const DataBuffer &db, ImageFileFormat iff, ScratchImage &image)
{
	HRESULT hr;

	ImageCodecType ict = GetImageCodecType(iff);

	if (ict == ICT_DDS) {
		V_RETURN(LoadFromDDSMemory(db.getConstBuffer(), db.getBufferSize(), DDS_FLAGS_NONE, nullptr, image));
	}
	else if (ict == ICT_WIC) {
		V_RETURN(LoadFromWICMemory(db.getConstBuffer(), db.getBufferSize(), WIC_FLAGS_NONE, nullptr, image));
	}
	else if (ict == ICT_TGA) {
		V_RETURN(LoadFromTGAMemory(db.getConstBuffer(), db.getBufferSize(), nullptr, image));
	}
	else if (ict == ICT_HDR) {
		V_RETURN(LoadFromHDRMemory(db.getConstBuffer(), db.getBufferSize(), nullptr, image));
	}
	else
		return E_FAIL;

	return S_OK;
}

HRESULT TextureUtil::SaveToFile(const DataBuffer &db, ImageFileFormat iff, Path filename)
{
	HRESULT hr;

	ScratchImage image;

	V_RETURN(LoadFromDataBuffer(db, iff, image));

	return SaveToFile(image, filename);
}

HRESULT TextureUtil::SaveToFile(const ScratchImage &image, Path filename)
{
	HRESULT hr;

	DirectX::Blob b;

	ImageFileFormat iffTarget = GetImageFileFormat(filename);

	V_RETURN(SaveToMemory(image, b, iffTarget));

	DataBuffer db;
	db.setBufferData((const uint8*)b.GetBufferPointer(), b.GetBufferSize(), nullptr);

	if (!SaveDataBuffer(filename, db))
		return E_FAIL;

	return S_OK;
}

HRESULT TextureUtil::SaveToMemory(const ScratchImage &image, DirectX::Blob &b, ImageFileFormat iff)
{
	HRESULT hr;

	ImageCodecType ictTarget = GetImageCodecType(iff);	

	if (ictTarget == ICT_DDS) {
		V_RETURN(SaveToDDSMemory(image.GetImages(), image.GetImageCount(), image.GetMetadata(), DDS_FLAGS_NONE, b));
	}
	else if (ictTarget == ICT_WIC) {
		GUID containerFmt;
		switch (iff)
		{
		case IFF_BMP: containerFmt = GUID_ContainerFormatBmp; break;
		case IFF_JPG: containerFmt = GUID_ContainerFormatJpeg; break;
		case IFF_PNG: containerFmt = GUID_ContainerFormatPng; break;
		case IFF_TIFF: containerFmt = GUID_ContainerFormatTiff; break;
		case IFF_GIF: containerFmt = GUID_ContainerFormatGif; break;
		case IFF_WMP: containerFmt = GUID_ContainerFormatWmp; break;
		default: containerFmt = { 0 }; break;
		}
		if (containerFmt == GUID({ 0 })) {
			return E_FAIL;
		}

		V_RETURN(SaveToWICMemory(*image.GetImage(0, 0, 0), WIC_FLAGS_NONE, containerFmt, b));
	}
	else if (ictTarget == ICT_TGA) {
		V_RETURN(SaveToTGAMemory(*image.GetImage(0, 0, 0), b));
	}
	else if (ictTarget == ICT_HDR) {
		V_RETURN(SaveToHDRMemory(*image.GetImage(0, 0, 0), b));
	}
	else
		return E_FAIL;

	return S_OK;
}