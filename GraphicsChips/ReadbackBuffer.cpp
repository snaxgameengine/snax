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
#include "ReadbackBuffer.h"


using namespace m3d;

CHIPDESCV1_DEF(ReadbackBuffer, MTEXT("Readback Buffer"), READBACKBUFFER_GUID, CHIP_GUID);


ReadbackBuffer::ReadbackBuffer()
{
}

ReadbackBuffer::~ReadbackBuffer()
{
	SAFE_DELETE(_data);
}

void ReadbackBuffer::OnDestroyDevice()
{
	_res = nullptr;
}

void ReadbackBuffer::SetResource(ID3D12Resource* resource, D3D12_PLACED_SUBRESOURCE_FOOTPRINT fp)
{
	_res = resource;
	_fp = fp;
	_readbackStamp = graphics()->GetCurrentFrameIndex();
	_bpp = dxgiformat::BitsPerPixel(_fp.Footprint.Format);
	SAFE_DELETE(_data);
}

bool ReadbackBuffer::IsDownloading() const
{
	return _res && !IsDownloadComplete();
}

bool ReadbackBuffer::IsDownloadComplete() const
{
	if (_data)
		return true;
	if (!_res)
		return false;
	UINT64 lastCompletedFrame = graphics()->GetLastCompletedFrameIndex();
	if (lastCompletedFrame < _readbackStamp)
		return false;
	return true;
}

bool ReadbackBuffer::GetPixel(XMFLOAT4& p, UINT x, UINT y, UINT z)
{
	if (!IsDownloadComplete())
		return false;

	if (!_data) {
		UINT64 size = _res->GetDesc().Width;

		D3D12_RANGE readRange = { 0, size };
		D3D12_RANGE writeRange = { 0, 0 };
		void* data = nullptr;
		_data = new BYTE[size];
		_res->Map(0, &readRange, &data);
		std::memcpy(_data, data, readRange.End);
		_res->Unmap(0, &writeRange);
		_res = nullptr;

		_convertFunc = nullptr;

		switch (_fp.Footprint.Format)
		{
		case DXGI_FORMAT_R32G32B32A32_TYPELESS: break;
		case DXGI_FORMAT_R32G32B32A32_FLOAT: _convertFunc = [](XMFLOAT4& v, const BYTE* pixel) { v = *(XMFLOAT4*)(pixel); }; break;
		case DXGI_FORMAT_R32G32B32A32_UINT: _convertFunc = [](XMFLOAT4& v, const BYTE* pixel) { const UINT* p = (const UINT*)pixel; v = XMFLOAT4((FLOAT)p[0], (FLOAT)p[1], (FLOAT)p[2], (FLOAT)p[3]); }; break;
		case DXGI_FORMAT_R32G32B32A32_SINT: _convertFunc = [](XMFLOAT4& v, const BYTE* pixel) { const INT* p = (const INT*)pixel; v = XMFLOAT4((FLOAT)p[0], (FLOAT)p[1], (FLOAT)p[2], (FLOAT)p[3]); }; break;
		case DXGI_FORMAT_R32G32B32_TYPELESS: break;
		case DXGI_FORMAT_R32G32B32_FLOAT: _convertFunc = [](XMFLOAT4& v, const BYTE* pixel) { const FLOAT* p = (const FLOAT*)pixel; v = XMFLOAT4((FLOAT)p[0], (FLOAT)p[1], (FLOAT)p[2], 1.0f); }; break;
		case DXGI_FORMAT_R32G32B32_UINT: _convertFunc = [](XMFLOAT4& v, const BYTE* pixel) { const UINT* p = (const UINT*)pixel; v = XMFLOAT4((FLOAT)p[0], (FLOAT)p[1], (FLOAT)p[2], 1.0f); }; break;
		case DXGI_FORMAT_R32G32B32_SINT: _convertFunc = [](XMFLOAT4& v, const BYTE* pixel) { const INT* p = (const INT*)pixel; v = XMFLOAT4((FLOAT)p[0], (FLOAT)p[1], (FLOAT)p[2], 1.0f); }; break;
		case DXGI_FORMAT_R16G16B16A16_TYPELESS: break;
		case DXGI_FORMAT_R16G16B16A16_FLOAT: _convertFunc = [](XMFLOAT4& v, const BYTE* pixel) { const HALF* p = (const HALF*)pixel; v = XMFLOAT4(XMConvertHalfToFloat(p[0]), XMConvertHalfToFloat(p[1]), XMConvertHalfToFloat(p[2]), XMConvertHalfToFloat(p[3])); }; break;
		case DXGI_FORMAT_R16G16B16A16_UNORM: break;
		case DXGI_FORMAT_R16G16B16A16_UINT: break;
		case DXGI_FORMAT_R16G16B16A16_SNORM: break;
		case DXGI_FORMAT_R16G16B16A16_SINT: break;
		case DXGI_FORMAT_R32G32_TYPELESS: break;
		case DXGI_FORMAT_R32G32_FLOAT: _convertFunc = [](XMFLOAT4& v, const BYTE* pixel) { const FLOAT* p = (const FLOAT*)pixel; v = XMFLOAT4((FLOAT)p[0], (FLOAT)p[1], 0.0f, 1.0f); }; break;
		case DXGI_FORMAT_R32G32_UINT: _convertFunc = [](XMFLOAT4& v, const BYTE* pixel) { const UINT* p = (const UINT*)pixel; v = XMFLOAT4((FLOAT)p[0], (FLOAT)p[1], 0.0f, 1.0f); }; break;
		case DXGI_FORMAT_R32G32_SINT: _convertFunc = [](XMFLOAT4& v, const BYTE* pixel) { const INT* p = (const INT*)pixel; v = XMFLOAT4((FLOAT)p[0], (FLOAT)p[1], 0.0f, 1.0f); }; break;
		case DXGI_FORMAT_R32G8X24_TYPELESS: break;
		case DXGI_FORMAT_D32_FLOAT_S8X24_UINT: break;
		case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS: break;
		case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT: break;
		case DXGI_FORMAT_R10G10B10A2_TYPELESS: break;
		case DXGI_FORMAT_R10G10B10A2_UNORM: break;
		case DXGI_FORMAT_R10G10B10A2_UINT: break;
		case DXGI_FORMAT_R11G11B10_FLOAT: /*convertAndAssign = [&]() { *(XMFLOAT3PK*)(pixel) = XMFLOAT3PK(v.x, v.y, v.z); }; */break;
		case DXGI_FORMAT_R8G8B8A8_TYPELESS: break;
		case DXGI_FORMAT_R8G8B8A8_UNORM:
		case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB: _convertFunc = [](XMFLOAT4& v, const BYTE* pixel) { v = XMFLOAT4((FLOAT)pixel[0] / 255.0f, (FLOAT)pixel[1] / 255.0f, (FLOAT)pixel[2] / 255.0f, (FLOAT)pixel[3] / 255.0f); }; break;
		case DXGI_FORMAT_R8G8B8A8_UINT: _convertFunc = [](XMFLOAT4& v, const BYTE* pixel) { v = XMFLOAT4((FLOAT)pixel[0], (FLOAT)pixel[1], (FLOAT)pixel[2], (FLOAT)pixel[3]); }; break;
		case DXGI_FORMAT_R8G8B8A8_SNORM: _convertFunc = [](XMFLOAT4& v, const BYTE* pixel) { const CHAR* p = (const CHAR*)pixel; v = XMFLOAT4((FLOAT)p[0] / 255.0f, (FLOAT)p[1] / 255.0f, (FLOAT)p[2] / 255.0f, (FLOAT)p[3] / 255.0f); }; break;
		case DXGI_FORMAT_R8G8B8A8_SINT: break;
		case DXGI_FORMAT_R16G16_TYPELESS: break;
		case DXGI_FORMAT_R16G16_FLOAT: _convertFunc = [](XMFLOAT4& v, const BYTE* pixel) { const HALF* p = (const HALF*)pixel; v = XMFLOAT4(XMConvertHalfToFloat(p[0]), XMConvertHalfToFloat(p[1]), 0.0f, 1.0f); }; break;
		case DXGI_FORMAT_R16G16_UNORM: break;
		case DXGI_FORMAT_R16G16_UINT: break;
		case DXGI_FORMAT_R16G16_SNORM: break;
		case DXGI_FORMAT_R16G16_SINT: break;
		case DXGI_FORMAT_R32_TYPELESS: break;
		case DXGI_FORMAT_D32_FLOAT:
		case DXGI_FORMAT_R32_FLOAT: _convertFunc = [](XMFLOAT4& v, const BYTE* pixel) { v = XMFLOAT4(*((const FLOAT*)pixel), 0.0f, 0.0f, 1.0f); }; break;
		case DXGI_FORMAT_R32_UINT: _convertFunc = [](XMFLOAT4& v, const BYTE* pixel) { v = XMFLOAT4((FLOAT) * ((const UINT*)pixel), 0.0f, 0.0f, 1.0f); }; break;
		case DXGI_FORMAT_R32_SINT: _convertFunc = [](XMFLOAT4& v, const BYTE* pixel) { v = XMFLOAT4((FLOAT) * ((const INT*)pixel), 0.0f, 0.0f, 1.0f); }; break;
		case DXGI_FORMAT_R24G8_TYPELESS: break;
		case DXGI_FORMAT_D24_UNORM_S8_UINT: break;
		case DXGI_FORMAT_R24_UNORM_X8_TYPELESS: break;
		case DXGI_FORMAT_X24_TYPELESS_G8_UINT: break;
		case DXGI_FORMAT_R8G8_TYPELESS: break;
		case DXGI_FORMAT_R8G8_UNORM: break;
		case DXGI_FORMAT_R8G8_UINT: break;
		case DXGI_FORMAT_R8G8_SNORM: break;
		case DXGI_FORMAT_R8G8_SINT: break;
		case DXGI_FORMAT_R16_TYPELESS: break;
		case DXGI_FORMAT_R16_FLOAT: break;
		case DXGI_FORMAT_D16_UNORM: break;
		case DXGI_FORMAT_R16_UNORM: break;
		case DXGI_FORMAT_R16_UINT: break;
		case DXGI_FORMAT_R16_SNORM: break;
		case DXGI_FORMAT_R16_SINT: break;
		case DXGI_FORMAT_R8_TYPELESS: break;
		case DXGI_FORMAT_R8_UNORM: break;
		case DXGI_FORMAT_R8_UINT: break;
		case DXGI_FORMAT_R8_SNORM: break;
		case DXGI_FORMAT_R8_SINT: break;
		case DXGI_FORMAT_A8_UNORM: break;
		default: break;
		};
	}

	if (x >= _fp.Footprint.Width || y >= _fp.Footprint.Width || z >= _fp.Footprint.Depth)
		return false;

	if (!_convertFunc)
		return false;

	const BYTE* pixel = _data + (_fp.Footprint.RowPitch * (_fp.Footprint.Height * z + y)) + x * _bpp / 8;

	_convertFunc(p, pixel);

	return true;
}