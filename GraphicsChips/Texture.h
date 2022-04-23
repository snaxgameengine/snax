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
#include "GraphicsResourceChip.h"
#include "M3DCore/DataBuffer.h"
#include "GraphicsDefines.h"

namespace m3d
{


static const Guid TEXTURE_GUID = { 0x3b32ee42, 0x3ae8, 0x459f, { 0xb2, 0xb1, 0x39, 0xd9, 0xb9, 0x6e, 0xb5, 0x1a } };


enum ImageFileFormat { IFF_UNKNOWN, IFF_DDS, IFF_BMP, IFF_JPG, IFF_PNG, IFF_TIFF, IFF_GIF, IFF_WMP, IFF_TGA, IFF_HDR };
enum ImageCodecType { ICT_UNKNOWN, ICT_DDS, ICT_WIC, ICT_TGA, ICT_HDR };

ImageFileFormat GRAPHICSCHIPS_API GetImageFileFormat(Path file);
ImageCodecType GRAPHICSCHIPS_API GetImageCodecType(ImageFileFormat fmt);


enum TEXTURE_DESC_FLAGS
{
	TEXTURE_USE_BACKBUFFER_SIZE_BY_PERCENT = 1,
	TEXTURE_USE_BACKBUFFER_FORMAT = 2,
	TEXTURE_unused = 4,
	TEXTURE_FORCE_SRGB = 8,
	TEXTURE_GEN_MIPMAPS = 16
};


struct TextureDesc
{
	TextureDesc() : Width(256), Height(256), Depth(0), ArraySize(1), MipLevels(0), Format(M3D_FORMAT_R8G8B8A8_UNORM), Flags(M3D_RESOURCE_FLAG_NONE), FlagsEx(0), _failed(false) { SampleDesc.Count = 1; SampleDesc.Quality = 0; }

	bool operator==(const TextureDesc& rhs) { return std::memcmp(this, &rhs, sizeof(TextureDesc) - sizeof(BOOL)) == 0; }
	bool operator!=(const TextureDesc& rhs) { return !(*this == rhs); }

	UINT64 Width;
	UINT Height;
	UINT16 Depth;
	UINT16 ArraySize;
	UINT16 MipLevels;
	M3D_FORMAT Format;
	M3D_SAMPLE_DESC SampleDesc;
	M3D_RESOURCE_FLAGS Flags;
	UINT FlagsEx;
	BOOL _failed;
};

class GRAPHICSCHIPS_API Texture : public GraphicsResourceChip
{
	CHIPDESC_DECL; 
public:
	Texture();

	bool CopyChip(Chip* chip) override;
	bool LoadChip(DocumentLoader& loader) override;
	bool SaveChip(DocumentSaver& saver) const override;

	// Returns the descriptor used for creating the texture. Note: If we have image data, the desc is not valid until texture is loaded successfully.
	virtual const TextureDesc& GetInitDesc() const { return _initDesc; }
	// Sets the descriptor used for creating texture. If we have image data, only flags are updated.
	virtual void SetInitDesc(const TextureDesc& desc);
	// true if we have image data.
	virtual bool HasImageData() const { return _imageData.getBufferSize() > 0; }
	// Returns the image data. This is an in-memory image file (eg dds).
	virtual const DataBuffer& GetImageData() const { return _imageData; }
	// The file format of the sotred image.
	virtual ImageFileFormat GetImageDataFileFormat() const { return _imageFileFormat; }

	void OnReleasingBackBuffer(RenderWindow* rw) override;

	// Creates the texture if it does not exist!
	void UpdateChip(BufferLayoutID layoutID = InvalidBufferLayoutID) override;

	// Load image data from file and clear current texture. Recreates on request.
	virtual bool LoadImageDataFromFile(Path path);
	// Sets the image data, and clears current texture. Data is moved, not copied!
	virtual bool SetImageData(DataBuffer&& db, ImageFileFormat fmt, TextureDesc* descHint = nullptr);
	// Saves current texture to image data with given file format.
	virtual bool SaveTextureToImageData(ImageFileFormat fmt = IFF_DDS);
	// Saves current texture (not image data!) to file.
	virtual bool SaveTextureToFile(Path filename, ImageFileFormat fmt = IFF_UNKNOWN);
	// Saves image data to given file.
	virtual bool SaveImageDataToFile(Path filename);
	// Removes the image data. Texture is not cleared.
	virtual void ClearImageData();


protected:
	// Initial descriptor for creating an empty texture.
	TextureDesc _initDesc;
	// Image can be loaded and stored in the chip.
	DataBuffer _imageData;
	// The file format of the image data.
	ImageFileFormat _imageFileFormat = IFF_UNKNOWN;

private:
	void _createTexture();
	void _loadTextureFromImageData();
	bool _saveTextureToImageData(ID3D12Resource* res, ImageFileFormat fmt = IFF_UNKNOWN);
	void _createStateTracker(D3D12_RESOURCE_STATES initStates);
};

}
