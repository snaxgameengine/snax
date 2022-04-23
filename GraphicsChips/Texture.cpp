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
#include "Texture.h"
#include "M3DCore/Util.h"
#include "M3DEngine/DocumentSaveLoadUtil.h"
#include "Graphics.h"
#include "D3D12RenderWindow.h"
#include "D3D12RenderWindowManager.h"
#include "RenderSettings.h"

using namespace m3d;


CHIPDESCV1_DEF(Texture, MTEXT("Texture"), TEXTURE_GUID, GRAPHICSRESOURCECHIP_GUID);



ImageFileFormat m3d::GetImageFileFormat(Path file)
{
	String ext = strUtils::toLower(file.GetFileExtention());
	if (ext == MTEXT("dds")) return IFF_DDS;
	if (ext == MTEXT("bmp")) return IFF_BMP;
	if (ext == MTEXT("jpg")) return IFF_JPG;
	if (ext == MTEXT("jpeg")) return IFF_JPG;
	if (ext == MTEXT("png")) return IFF_PNG;
	if (ext == MTEXT("tif")) return IFF_TIFF;
	if (ext == MTEXT("tiff")) return IFF_TIFF;
	if (ext == MTEXT("gif")) return IFF_GIF;
	if (ext == MTEXT("hdp")) return IFF_WMP;
	if (ext == MTEXT("jxr")) return IFF_WMP;
	if (ext == MTEXT("wdp")) return IFF_WMP;
	if (ext == MTEXT("tga")) return IFF_TGA;
	if (ext == MTEXT("hdr")) return IFF_HDR;
	return IFF_UNKNOWN;
}

ImageCodecType m3d::GetImageCodecType(ImageFileFormat fmt)
{
	switch (fmt)
	{
	case IFF_DDS: return ICT_DDS;
	case IFF_BMP: return ICT_WIC;
	case IFF_JPG: return ICT_WIC;
	case IFF_PNG: return ICT_WIC;
	case IFF_TIFF: return ICT_WIC;
	case IFF_GIF: return ICT_WIC;
	case IFF_WMP: return ICT_WIC;
	case IFF_TGA: return ICT_TGA;
	case IFF_HDR: return ICT_HDR;
	}
	return ICT_UNKNOWN;
}


Texture::Texture()
{
	ClearConnections();
}

bool Texture::CopyChip(Chip* chip)
{
	Texture* c = dynamic_cast<Texture*>(chip);
	B_RETURN(GraphicsResourceChip::CopyChip(c));
	_initDesc = c->_initDesc;
	_imageData = c->_imageData;
	_imageFileFormat = c->_imageFileFormat;
	//ClearResource();
	return true;
}

bool Texture::LoadChip(DocumentLoader& loader)
{
	B_RETURN(GraphicsResourceChip::LoadChip(loader));
	LOADDEF("data", _imageData, DataBuffer());
	LOADDEF("fileFormat", _imageFileFormat, IFF_UNKNOWN);

	LOADDEF("width", _initDesc.Width, 0);
	LOADDEF("height", _initDesc.Height, 0);
	LOADDEF("depth", _initDesc.Depth, 0);
	LOADDEF("arraySize", _initDesc.ArraySize, 0);
	LOADDEF("mipLevels", _initDesc.MipLevels, 0);
	LOADDEF("format", _initDesc.Format, M3D_FORMAT_UNKNOWN);
	LOADDEF("msCount", _initDesc.SampleDesc.Count, 0);
	LOADDEF("msQuality", _initDesc.SampleDesc.Quality, 0);
	LOADDEF("flags", _initDesc.Flags, M3D_RESOURCE_FLAG_NONE);
	LOADDEF("flagsEx", _initDesc.FlagsEx, 0);

	return true;
}

bool Texture::SaveChip(DocumentSaver& saver) const
{
	B_RETURN(GraphicsResourceChip::SaveChip(saver));
	SAVEDEF("data", _imageData, DataBuffer());
	SAVEDEF("fileFormat", _imageFileFormat, IFF_UNKNOWN);
	if (_imageData.getBufferSize() == 0) {
		SAVEDEF("width", _initDesc.Width, 0);
		SAVEDEF("height", _initDesc.Height, 0);
		SAVEDEF("depth", _initDesc.Depth, 0);
		SAVEDEF("arraySize", _initDesc.ArraySize, 0);
		SAVEDEF("mipLevels", _initDesc.MipLevels, 0);
		SAVEDEF("format", _initDesc.Format, M3D_FORMAT_UNKNOWN);
		SAVEDEF("msCount", _initDesc.SampleDesc.Count, 0);
		SAVEDEF("msQuality", _initDesc.SampleDesc.Quality, 0);
	}
	SAVEDEF("flags", _initDesc.Flags, M3D_RESOURCE_FLAG_NONE);
	SAVEDEF("flagsEx", _initDesc.FlagsEx, 0);
	return true;
}

void Texture::SetInitDesc(const TextureDesc& desc)
{
	if (_initDesc == desc)
		return; // No changes
	if (_imageData.getBufferSize() > 0) { // Only the flags should be copied when we have image data!
		_initDesc.Flags = desc.Flags;
		_initDesc.FlagsEx = desc.FlagsEx;
	}
	else
		_initDesc = desc;
	_initDesc._failed = false;
	ClearResource(); // Remove current d3d resource.
}

void Texture::OnReleasingBackBuffer(RenderWindow* rw)
{
	GraphicsResourceChip::OnReleasingBackBuffer(rw);
	if (_initDesc.FlagsEx & (TEXTURE_USE_BACKBUFFER_SIZE_BY_PERCENT | TEXTURE_USE_BACKBUFFER_FORMAT))
		ClearResource(); // Clear because size, format or multisampling is dependent on the back buffer!
}

void Texture::UpdateChip(BufferLayoutID /*layoutID*/)
{
	if (_res)
		return;

	if (_initDesc._failed)
		throw GraphicsException(this, MTEXT("This chip has earlier failed to create a texture."));

	D3D_DEBUG_REPORTER_BLOCK

		if (_imageData.getBufferSize()) { // Do we have image data?
			_loadTextureFromImageData(); // Throws!
		}
		else {
			_createTexture(); // Throws!
		}

	String s = GetName() + MTEXT(" (") + GuidToString(GetGlobalID()) + MTEXT(")");
	_res->SetPrivateData(WKPDID_D3DDebugObjectName, (UINT)s.size(), s.c_str());
}

bool Texture::LoadImageDataFromFile(Path path)
{
	DataBuffer db;
	if (!LoadDataBuffer(path, db)) {
		return false;
	}
	return SetImageData(std::move(db), GetImageFileFormat(path));
}

bool Texture::SetImageData(DataBuffer&& db, ImageFileFormat fmt, TextureDesc* descHint)
{
	if (fmt == IFF_UNKNOWN)
		return false;

	if (fmt == IFF_TGA)
	{
		msg(FATAL, MTEXT("TGA-images are only supported through Texture dialog import."));
		return false;
	}

	if (descHint)
		_initDesc = *descHint;
	else {
		// NOTE: _desc is not valid (except the flags) when we have image data, but not loaded the texture yet.
		TextureDesc d = _initDesc;
		_initDesc = TextureDesc();
		_initDesc.Flags = d.Flags;
		_initDesc.FlagsEx = d.FlagsEx;
	}

	_imageData = std::move(db);
	_imageFileFormat = fmt;
	ClearResource(); // Clear texture and views!

	return true;
}

bool Texture::SaveTextureToImageData(ImageFileFormat fmt)
{
	return _saveTextureToImageData(GetResource(), fmt);
}

bool Texture::SaveTextureToFile(Path filename, ImageFileFormat fmt)
{
	try {
		ID3D12Resource* res = GetResource();

		ImageCodecType codec = GetImageCodecType(_imageFileFormat);

		HRESULT hr = E_NOTIMPL;

		return SUCCEEDED(hr);
	}
	catch (const ChipException& e)
	{
		AddException(e);
	}
	return false;
}

bool Texture::SaveImageDataToFile(Path filename)
{
	return SaveDataBuffer(filename, _imageData);
}

void Texture::ClearImageData()
{
	_imageData.clear();
	_imageFileFormat = IFF_UNKNOWN;
}


void Texture::_createTexture()
{
	D3D12_RESOURCE_DESC desc;

	_initDesc._failed = true; // Assume this goes wrong!

	if (_initDesc.Depth > 1)
		desc = CD3DX12_RESOURCE_DESC::Tex3D((DXGI_FORMAT)_initDesc.Format, _initDesc.Width, _initDesc.Height, _initDesc.Depth, _initDesc.MipLevels, (D3D12_RESOURCE_FLAGS)_initDesc.Flags);
	else if (_initDesc.Height > 1 || _initDesc.SampleDesc.Count > 1)
		desc = CD3DX12_RESOURCE_DESC::Tex2D((DXGI_FORMAT)_initDesc.Format, _initDesc.Width, _initDesc.Height, _initDesc.ArraySize, _initDesc.MipLevels, _initDesc.SampleDesc.Count, _initDesc.SampleDesc.Quality, (D3D12_RESOURCE_FLAGS)_initDesc.Flags);
	else
		desc = CD3DX12_RESOURCE_DESC::Tex1D((DXGI_FORMAT)_initDesc.Format, _initDesc.Width, _initDesc.ArraySize, _initDesc.MipLevels, (D3D12_RESOURCE_FLAGS)_initDesc.Flags);

	//desc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R32G32B32A32_FLOAT, 64, 16, 1);

	if (_initDesc.FlagsEx & (TEXTURE_USE_BACKBUFFER_SIZE_BY_PERCENT | TEXTURE_USE_BACKBUFFER_FORMAT)) {
		D3D12RenderWindow* rw = (D3D12RenderWindow*)graphics()->GetRenderWindowManager()->GetRenderWindow();
		if (!rw)
			throw GraphicsException(this, MTEXT("Failed to create a back buffer dependent texture because we found no render window!"));
		ID3D12Resource* bb = rw->GetBackBuffer();
		if (!bb)
			throw GraphicsException(this, MTEXT("Failed to create a back buffer dependent texture because the back buffer is not available!"));
		D3D12_RESOURCE_DESC bbDesc = bb->GetDesc();
		if (_initDesc.FlagsEx & TEXTURE_USE_BACKBUFFER_SIZE_BY_PERCENT) {
			desc.Width = bbDesc.Width * _initDesc.Width / 100;
			if (desc.Dimension != D3D12_RESOURCE_DIMENSION_TEXTURE1D)
				desc.Height = bbDesc.Height * _initDesc.Height / 100;
		}
		if (_initDesc.FlagsEx & TEXTURE_USE_BACKBUFFER_FORMAT) {
			if ((desc.Flags & D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL) != 0) {
				UINT size = dxgiformat::BitsPerPixel(bbDesc.Format);
				switch (size) {
				case 16:
					desc.Format = DXGI_FORMAT_D16_UNORM;
					break;
				case 32:
					desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT; // or DXGI_FORMAT_D32_FLOAT
					break;
				case 64:
					desc.Format = DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
					break;
				default:
					desc.Format = bbDesc.Format; // could not find any better format...
				};
			}
			else
				desc.Format = bbDesc.Format;
		}
	}

	D3D12_CLEAR_VALUE optimizedClearValue;
	optimizedClearValue.Format = (desc.Flags & (D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL)) ? desc.Format : DXGI_FORMAT_UNKNOWN;

	if (dxgiformat::IsTypeless(desc.Format)) {
		if (desc.Flags & D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET)
			optimizedClearValue.Format = (DXGI_FORMAT)GetRTVDesc().format; // Can be unknown
		else if (desc.Flags & D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL)
			optimizedClearValue.Format = (DXGI_FORMAT)GetDSVDesc().format; // Can be unknown
		else
			optimizedClearValue.Format = DXGI_FORMAT_UNKNOWN;
	}

	if (m3d::dxgiformat::IsDepthStencil(optimizedClearValue.Format)) {
		optimizedClearValue.DepthStencil.Depth = 1.0f;
		optimizedClearValue.DepthStencil.Stencil = 0xFF;
	}
	else
		((XMFLOAT4&)optimizedClearValue.Color) = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);

	D3D12_RESOURCE_STATES initState = D3D12_RESOURCE_STATE_COMMON;

	if (desc.Flags & (D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL | D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE))
		initState = D3D12_RESOURCE_STATE_DEPTH_WRITE;

	CD3DX12_HEAP_PROPERTIES hp(D3D12_HEAP_TYPE_DEFAULT);
	HRESULT hr = device()->CreateCommittedResource(&hp, D3D12_HEAP_FLAG_NONE, &desc, initState, optimizedClearValue.Format != DXGI_FORMAT_UNKNOWN ? &optimizedClearValue : nullptr, IID_PPV_ARGS(&_res));

	if (FAILED(hr)) {
		throw GraphicsException(this, MTEXT("Failed to create texture!"));
	}

	SetUpdateStamp();

	/*Char *data = new Char[64*16*4*4];

	FILE * filp;
	fopen_s(&filp, "c:\\temp\\irradiance_tex.bin", "rb");
	int32 bytes_read = fread(data, sizeof(Char), 64 * 16 * 4 * 4, filp);


	D3D12_SUBRESOURCE_DATA textureData = {};
	textureData.pData = data;
	textureData.RowPitch = 64 * 4 * 4;
	textureData.SlicePitch = textureData.RowPitch * 16;
	D3D12_GPU_VIRTUAL_ADDRESS vAddr;
	UINT64 offset;
	rs()->GetUploadHeap()->Allocate(64 * 16 * 4 * 4, 1, nullptr, &vAddr, &offset);

	rs()->GetCommandList1()->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(_res, initState, D3D12_RESOURCE_STATE_COPY_DEST));
	UpdateSubresources(rs()->GetCommandList1(), _res, rs()->GetUploadHeap()->GetBufferResource(), offset, 0, 1, &textureData);
	rs()->GetCommandList1()->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(_res, D3D12_RESOURCE_STATE_COPY_DEST, initState));
	*/

	_createStateTracker(initState);

	_initDesc._failed = false;
}

void Texture::_createStateTracker(D3D12_RESOURCE_STATES initStates)
{
	const auto desc = _res->GetDesc();

	// For now: Only create a state tracker if this object can be used as RTV or UAV or DSV. (For DSV when can be used in shaders as well).
	if (desc.Flags & (D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS) || (desc.Flags & D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL) && !(desc.Flags & D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE)) {
		SResourceStateTracker t;
		HRESULT hr = CreateResourceStateTracker(_res, initStates, t.fill());
	}
}

void Texture::_loadTextureFromImageData()
{
	_initDesc._failed = true; // Assume this goes wrong!

	if (_imageData.getBufferSize() == 0) {
		throw GraphicsException(this, MTEXT("No image data to create texture from!"));
	}

	RID3D12Resource uh;
	SID3D12Device d = device();

	// texture should be in D3D12_RESOURCE_STATE_COMMON. Should we change it for our usage?
	D3D12_RESOURCE_FLAGS resFlags = (D3D12_RESOURCE_FLAGS)_initDesc.Flags;
	DDS_LOADER_FLAGS loadFlags = DDS_LOADER_DEFAULT; // Can be DDS_LOADER_FORCE_SRGB, DDS_LOADER_MIP_AUTOGEN, and DDS_LOADER_MIP_RESERVE
	WIC_LOADER_FLAGS loadFlagsWIC = WIC_LOADER_DEFAULT;
	if (_initDesc.FlagsEx & TEXTURE_FORCE_SRGB) {
		loadFlags |= DDS_LOADER_FORCE_SRGB;
		loadFlagsWIC |= WIC_LOADER_FORCE_SRGB;
	}
	if (_initDesc.FlagsEx & TEXTURE_GEN_MIPMAPS) {
		loadFlags |= DDS_LOADER_MIP_AUTOGEN;
		loadFlagsWIC |= WIC_LOADER_MIP_AUTOGEN;
	}
	DDS_ALPHA_MODE alphaMode;
	bool isCubeMap;

	HRESULT hr;

	ResourceUploadBatch* rub = graphics()->GetResourceUploadBatch();

	try
	{
		struct RUB
		{
			Graphics* g;
			ResourceUploadBatch* rub;
			RUB(Graphics* g, ResourceUploadBatch* rub) : g(g), rub(rub) { rub->Begin(); }
			~RUB()
			{
				auto uploadResourcesFinished = rub->End(g->GetCommandQueue());

				// Wait for the upload thread to terminate
				uploadResourcesFinished.wait();
			}
		} Rub(graphics(), rub); // We need this to ensure End() is always called!

		switch (GetImageCodecType(_imageFileFormat))
		{
		case ICT_DDS:
			hr = CreateDDSTextureFromMemoryEx(device(), *rub, _imageData.getConstBuffer(), _imageData.getBufferSize(), 0, resFlags, loadFlags, &_res, &alphaMode, &isCubeMap);
			break;
		case ICT_WIC:
			hr = CreateWICTextureFromMemoryEx(device(), *rub, _imageData.getConstBuffer(), _imageData.getBufferSize(), 0, resFlags, loadFlagsWIC, &_res);
			break;
		case ICT_TGA:
		case ICT_HDR:
			throw GraphicsException(this, MTEXT("TGA and HDR files are not supported. Please convert to DDS using the Texture dialog!"));
			break;
		default:
			break;
		}

		if (FAILED(hr)) {
			throw GraphicsException(this, MTEXT("Failed to create texture from image data!"));
		}
	}
	catch (const std::exception& e)
	{
		throw GraphicsException(this, MTEXT("Failed to create texture from image data: ") + String(e.what()) + MTEXT("."));
	}

	D3D12_RESOURCE_DESC rd = _res->GetDesc();
	_initDesc.Width = rd.Width;
	_initDesc.Height = rd.Height;
	_initDesc.Depth = rd.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE3D ? rd.DepthOrArraySize : 1;
	_initDesc.ArraySize = rd.Dimension != D3D12_RESOURCE_DIMENSION_TEXTURE3D ? rd.DepthOrArraySize : 1;
	_initDesc.MipLevels = rd.MipLevels;
	_initDesc.Format = (M3D_FORMAT)rd.Format;
	_initDesc.SampleDesc = (const M3D_SAMPLE_DESC&)rd.SampleDesc;
	_initDesc._failed = false;

	// Resource starts in the pixel resource state. Move to common state!
	// There is no need for a resource barrier as long as the resource is not RTV/DSV/UTV. The resource is promoted from common to shader resource state!
	// ref: https://docs.microsoft.com/nb-no/windows/win32/direct3d12/using-resource-barriers-to-synchronize-resource-states-in-direct3d-12.
	// It can even be used as copy src/dest without the need of barriers.
	CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(_res, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COMMON);
	graphics()->rs()->ResourceBarrier(1, &barrier);

	_createStateTracker(D3D12_RESOURCE_STATE_COMMON);
}

bool Texture::_saveTextureToImageData(ID3D12Resource* res, ImageFileFormat fmt)
{
	if (!res)
		return false;

	// We probably have to save texture to file, then load it to image data because DirectXTK does not have such a function...

	return false;
}