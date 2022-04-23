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
#include "D3D12Formats.h"

using namespace m3d;

// We don't care about these exotic formats as we don't use them anyway...
#ifdef DXGI_1_2_FORMATS
#undef DXGI_1_2_FORMATS
#endif

// Copied from the DirectXTex project

//-------------------------------------------------------------------------------------
// Returns bits-per-pixel for a given DXGI format, or 0 on failure
//-------------------------------------------------------------------------------------
uint32 m3d::dxgiformat::BitsPerPixel( DXGI_FORMAT fmt )
{
    switch( fmt )
    {
    case DXGI_FORMAT_R32G32B32A32_TYPELESS:
    case DXGI_FORMAT_R32G32B32A32_FLOAT:
    case DXGI_FORMAT_R32G32B32A32_UINT:
    case DXGI_FORMAT_R32G32B32A32_SINT:
        return 128;

    case DXGI_FORMAT_R32G32B32_TYPELESS:
    case DXGI_FORMAT_R32G32B32_FLOAT:
    case DXGI_FORMAT_R32G32B32_UINT:
    case DXGI_FORMAT_R32G32B32_SINT:
        return 96;

    case DXGI_FORMAT_R16G16B16A16_TYPELESS:
    case DXGI_FORMAT_R16G16B16A16_FLOAT:
    case DXGI_FORMAT_R16G16B16A16_UNORM:
    case DXGI_FORMAT_R16G16B16A16_UINT:
    case DXGI_FORMAT_R16G16B16A16_SNORM:
    case DXGI_FORMAT_R16G16B16A16_SINT:
    case DXGI_FORMAT_R32G32_TYPELESS:
    case DXGI_FORMAT_R32G32_FLOAT:
    case DXGI_FORMAT_R32G32_UINT:
    case DXGI_FORMAT_R32G32_SINT:
    case DXGI_FORMAT_R32G8X24_TYPELESS:
    case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
    case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
    case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
        return 64;

    case DXGI_FORMAT_R10G10B10A2_TYPELESS:
    case DXGI_FORMAT_R10G10B10A2_UNORM:
    case DXGI_FORMAT_R10G10B10A2_UINT:
    case DXGI_FORMAT_R11G11B10_FLOAT:
    case DXGI_FORMAT_R8G8B8A8_TYPELESS:
    case DXGI_FORMAT_R8G8B8A8_UNORM:
    case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
    case DXGI_FORMAT_R8G8B8A8_UINT:
    case DXGI_FORMAT_R8G8B8A8_SNORM:
    case DXGI_FORMAT_R8G8B8A8_SINT:
    case DXGI_FORMAT_R16G16_TYPELESS:
    case DXGI_FORMAT_R16G16_FLOAT:
    case DXGI_FORMAT_R16G16_UNORM:
    case DXGI_FORMAT_R16G16_UINT:
    case DXGI_FORMAT_R16G16_SNORM:
    case DXGI_FORMAT_R16G16_SINT:
    case DXGI_FORMAT_R32_TYPELESS:
    case DXGI_FORMAT_D32_FLOAT:
    case DXGI_FORMAT_R32_FLOAT:
    case DXGI_FORMAT_R32_UINT:
    case DXGI_FORMAT_R32_SINT:
    case DXGI_FORMAT_R24G8_TYPELESS:
    case DXGI_FORMAT_D24_UNORM_S8_UINT:
    case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
    case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
    case DXGI_FORMAT_R9G9B9E5_SHAREDEXP:
    case DXGI_FORMAT_R8G8_B8G8_UNORM:
    case DXGI_FORMAT_G8R8_G8B8_UNORM:
    case DXGI_FORMAT_B8G8R8A8_UNORM:
    case DXGI_FORMAT_B8G8R8X8_UNORM:
    case DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM:
    case DXGI_FORMAT_B8G8R8A8_TYPELESS:
    case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
    case DXGI_FORMAT_B8G8R8X8_TYPELESS:
    case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
        return 32;

    case DXGI_FORMAT_R8G8_TYPELESS:
    case DXGI_FORMAT_R8G8_UNORM:
    case DXGI_FORMAT_R8G8_UINT:
    case DXGI_FORMAT_R8G8_SNORM:
    case DXGI_FORMAT_R8G8_SINT:
    case DXGI_FORMAT_R16_TYPELESS:
    case DXGI_FORMAT_R16_FLOAT:
    case DXGI_FORMAT_D16_UNORM:
    case DXGI_FORMAT_R16_UNORM:
    case DXGI_FORMAT_R16_UINT:
    case DXGI_FORMAT_R16_SNORM:
    case DXGI_FORMAT_R16_SINT:
    case DXGI_FORMAT_B5G6R5_UNORM:
    case DXGI_FORMAT_B5G5R5A1_UNORM:
        return 16;

    case DXGI_FORMAT_R8_TYPELESS:
    case DXGI_FORMAT_R8_UNORM:
    case DXGI_FORMAT_R8_UINT:
    case DXGI_FORMAT_R8_SNORM:
    case DXGI_FORMAT_R8_SINT:
    case DXGI_FORMAT_A8_UNORM:
        return 8;

    case DXGI_FORMAT_R1_UNORM:
        return 1;

    case DXGI_FORMAT_BC1_TYPELESS:
    case DXGI_FORMAT_BC1_UNORM:
    case DXGI_FORMAT_BC1_UNORM_SRGB:
    case DXGI_FORMAT_BC4_TYPELESS:
    case DXGI_FORMAT_BC4_UNORM:
    case DXGI_FORMAT_BC4_SNORM:
        return 4;

    case DXGI_FORMAT_BC2_TYPELESS:
    case DXGI_FORMAT_BC2_UNORM:
    case DXGI_FORMAT_BC2_UNORM_SRGB:
    case DXGI_FORMAT_BC3_TYPELESS:
    case DXGI_FORMAT_BC3_UNORM:
    case DXGI_FORMAT_BC3_UNORM_SRGB:
    case DXGI_FORMAT_BC5_TYPELESS:
    case DXGI_FORMAT_BC5_UNORM:
    case DXGI_FORMAT_BC5_SNORM:
    case DXGI_FORMAT_BC6H_TYPELESS:
    case DXGI_FORMAT_BC6H_UF16:
    case DXGI_FORMAT_BC6H_SF16:
    case DXGI_FORMAT_BC7_TYPELESS:
    case DXGI_FORMAT_BC7_UNORM:
    case DXGI_FORMAT_BC7_UNORM_SRGB:
        return 8;

#ifdef DXGI_1_2_FORMATS
    case DXGI_FORMAT_B4G4R4A4_UNORM:
        return 16;

    // We don't support the video formats ( see IsVideo function )
#endif // DXGI_1_2_FORMATS

    default:
        return 0;
    }
}

bool m3d::dxgiformat::IsValid( DXGI_FORMAT fmt )
{
#ifdef DXGI_1_2_FORMATS
    return ( static_cast<size_t>(fmt) >= 1 && static_cast<size_t>(fmt) <= 115 );
#else
    return ( static_cast<size_t>(fmt) >= 1 && static_cast<size_t>(fmt) <= 99 );
#endif
}

DXGI_FORMAT m3d::dxgiformat::PrevFormat(DXGI_FORMAT fmt)
{
#ifdef DXGI_1_2_FORMATS
	if (fmt == DXGI_FORMAT_UNKNOWN)
		return DXGI_FORMAT(115);
#else
	if (fmt == DXGI_FORMAT_UNKNOWN)
		return DXGI_FORMAT(99);
#endif
	return DXGI_FORMAT(fmt - 1);
}

DXGI_FORMAT m3d::dxgiformat::NextFormat(DXGI_FORMAT fmt)
{
#ifdef DXGI_1_2_FORMATS
	if (static_cast<size_t>(fmt) < 115)
		return DXGI_FORMAT(fmt + 1);
#else
	if (static_cast<size_t>(fmt) < 99)
		return DXGI_FORMAT(fmt + 1);
#endif
	return DXGI_FORMAT_UNKNOWN;
}

bool m3d::dxgiformat::IsCompressed( DXGI_FORMAT fmt )
{
    switch ( fmt )
    {
    case DXGI_FORMAT_BC1_TYPELESS:
    case DXGI_FORMAT_BC1_UNORM:
    case DXGI_FORMAT_BC1_UNORM_SRGB:
    case DXGI_FORMAT_BC2_TYPELESS:
    case DXGI_FORMAT_BC2_UNORM:
    case DXGI_FORMAT_BC2_UNORM_SRGB:
    case DXGI_FORMAT_BC3_TYPELESS:
    case DXGI_FORMAT_BC3_UNORM:
    case DXGI_FORMAT_BC3_UNORM_SRGB:
    case DXGI_FORMAT_BC4_TYPELESS:
    case DXGI_FORMAT_BC4_UNORM:
    case DXGI_FORMAT_BC4_SNORM:
    case DXGI_FORMAT_BC5_TYPELESS:
    case DXGI_FORMAT_BC5_UNORM:
    case DXGI_FORMAT_BC5_SNORM:
    case DXGI_FORMAT_BC6H_TYPELESS:
    case DXGI_FORMAT_BC6H_UF16:
    case DXGI_FORMAT_BC6H_SF16:
    case DXGI_FORMAT_BC7_TYPELESS:
    case DXGI_FORMAT_BC7_UNORM:
    case DXGI_FORMAT_BC7_UNORM_SRGB:
        return true;

    default:
        return false;
    }
}

bool m3d::dxgiformat::IsPacked( DXGI_FORMAT fmt )
{
    return ( (fmt == DXGI_FORMAT_R8G8_B8G8_UNORM) || (fmt == DXGI_FORMAT_G8R8_G8B8_UNORM) );
}

bool m3d::dxgiformat::IsVideo( DXGI_FORMAT fmt )
{
#ifdef DXGI_1_2_FORMATS
    switch ( fmt )
    {
    case DXGI_FORMAT_AYUV:
    case DXGI_FORMAT_Y410:
    case DXGI_FORMAT_Y416:
    case DXGI_FORMAT_NV12:
    case DXGI_FORMAT_P010:
    case DXGI_FORMAT_P016:
    case DXGI_FORMAT_YUY2:
    case DXGI_FORMAT_Y210:
    case DXGI_FORMAT_Y216:
    case DXGI_FORMAT_NV11:
        // These video formats can be used with the 3D pipeline through special view mappings
        return true;

    case DXGI_FORMAT_420_OPAQUE:
    case DXGI_FORMAT_AI44:
    case DXGI_FORMAT_IA44:
    case DXGI_FORMAT_P8:
    case DXGI_FORMAT_A8P8:
        // These are limited use video formats not usable in any way by the 3D pipeline
        return true;

    default:
        return false;
    }
#else // !DXGI_1_2_FORMATS
    UNREFERENCED_PARAMETER(fmt);
    return false;
#endif
}

bool m3d::dxgiformat::IsSRGB( DXGI_FORMAT fmt )
{
    switch( fmt )
    {
    case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
    case DXGI_FORMAT_BC1_UNORM_SRGB:
    case DXGI_FORMAT_BC2_UNORM_SRGB:
    case DXGI_FORMAT_BC3_UNORM_SRGB:
    case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
    case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
    case DXGI_FORMAT_BC7_UNORM_SRGB:
        return true;

    default:
        return false;
    }
}

bool m3d::dxgiformat::IsTypeless( DXGI_FORMAT fmt, bool partialTypeless)
{
	switch (fmt)
	{
	case DXGI_FORMAT_R32G32B32A32_TYPELESS:
	case DXGI_FORMAT_R32G32B32_TYPELESS:
	case DXGI_FORMAT_R16G16B16A16_TYPELESS:
	case DXGI_FORMAT_R32G32_TYPELESS:
	case DXGI_FORMAT_R32G8X24_TYPELESS:
	case DXGI_FORMAT_R10G10B10A2_TYPELESS:
	case DXGI_FORMAT_R8G8B8A8_TYPELESS:
	case DXGI_FORMAT_R16G16_TYPELESS:
	case DXGI_FORMAT_R32_TYPELESS:
	case DXGI_FORMAT_R24G8_TYPELESS:
	case DXGI_FORMAT_R8G8_TYPELESS:
	case DXGI_FORMAT_R16_TYPELESS:
	case DXGI_FORMAT_R8_TYPELESS:
	case DXGI_FORMAT_BC1_TYPELESS:
	case DXGI_FORMAT_BC2_TYPELESS:
	case DXGI_FORMAT_BC3_TYPELESS:
	case DXGI_FORMAT_BC4_TYPELESS:
	case DXGI_FORMAT_BC5_TYPELESS:
	case DXGI_FORMAT_B8G8R8A8_TYPELESS:
	case DXGI_FORMAT_B8G8R8X8_TYPELESS:
	case DXGI_FORMAT_BC6H_TYPELESS:
	case DXGI_FORMAT_BC7_TYPELESS:
		return true;

	case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
	case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
	case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
	case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
		return partialTypeless;

	default:
		return false;
	}
}

bool m3d::dxgiformat::HasAlpha( DXGI_FORMAT fmt )
{
    switch( fmt )
    {
    case DXGI_FORMAT_R32G32B32A32_TYPELESS:
    case DXGI_FORMAT_R32G32B32A32_FLOAT:
    case DXGI_FORMAT_R32G32B32A32_UINT:
    case DXGI_FORMAT_R32G32B32A32_SINT:
    case DXGI_FORMAT_R16G16B16A16_TYPELESS:
    case DXGI_FORMAT_R16G16B16A16_FLOAT:
    case DXGI_FORMAT_R16G16B16A16_UNORM:
    case DXGI_FORMAT_R16G16B16A16_UINT:
    case DXGI_FORMAT_R16G16B16A16_SNORM:
    case DXGI_FORMAT_R16G16B16A16_SINT:
    case DXGI_FORMAT_R10G10B10A2_TYPELESS:
    case DXGI_FORMAT_R10G10B10A2_UNORM:
    case DXGI_FORMAT_R10G10B10A2_UINT:
    case DXGI_FORMAT_R8G8B8A8_TYPELESS:
    case DXGI_FORMAT_R8G8B8A8_UNORM:
    case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
    case DXGI_FORMAT_R8G8B8A8_UINT:
    case DXGI_FORMAT_R8G8B8A8_SNORM:
    case DXGI_FORMAT_R8G8B8A8_SINT:
    case DXGI_FORMAT_A8_UNORM:
    case DXGI_FORMAT_BC2_TYPELESS:
    case DXGI_FORMAT_BC2_UNORM:
    case DXGI_FORMAT_BC2_UNORM_SRGB:
    case DXGI_FORMAT_BC3_TYPELESS:
    case DXGI_FORMAT_BC3_UNORM:
    case DXGI_FORMAT_BC3_UNORM_SRGB:
    case DXGI_FORMAT_B5G5R5A1_UNORM:
    case DXGI_FORMAT_B8G8R8A8_UNORM:
    case DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM:
    case DXGI_FORMAT_B8G8R8A8_TYPELESS:
    case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
    case DXGI_FORMAT_BC7_TYPELESS:
    case DXGI_FORMAT_BC7_UNORM:
    case DXGI_FORMAT_BC7_UNORM_SRGB:
#ifdef DXGI_1_2_FORMATS
    case DXGI_FORMAT_B4G4R4A4_UNORM:
#endif
        return true;

    default:
        return false;
    }
}


bool m3d::dxgiformat::IsDepthStencil(DXGI_FORMAT fmt)
{
	switch (fmt)
	{
	case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
	case DXGI_FORMAT_D32_FLOAT:
	case DXGI_FORMAT_D24_UNORM_S8_UINT:
	case DXGI_FORMAT_D16_UNORM:
		return true;
	}
	return false;
}

//-------------------------------------------------------------------------------------
// Converts to an SRGB equivalent type if available
//-------------------------------------------------------------------------------------
DXGI_FORMAT m3d::dxgiformat::MakeSRGB( DXGI_FORMAT fmt )
{
    switch( fmt )
    {
    case DXGI_FORMAT_R8G8B8A8_UNORM:
        return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;

    case DXGI_FORMAT_BC1_UNORM:
        return DXGI_FORMAT_BC1_UNORM_SRGB;

    case DXGI_FORMAT_BC2_UNORM:
        return DXGI_FORMAT_BC2_UNORM_SRGB;

    case DXGI_FORMAT_BC3_UNORM:
        return DXGI_FORMAT_BC3_UNORM_SRGB;

    case DXGI_FORMAT_B8G8R8A8_UNORM:
        return DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;

    case DXGI_FORMAT_B8G8R8X8_UNORM:
        return DXGI_FORMAT_B8G8R8X8_UNORM_SRGB;

    case DXGI_FORMAT_BC7_UNORM:
        return DXGI_FORMAT_BC7_UNORM_SRGB;

    default:
        return fmt;
    }
}


//-------------------------------------------------------------------------------------
// Converts to a format to an equivalent TYPELESS format if available
//-------------------------------------------------------------------------------------
DXGI_FORMAT m3d::dxgiformat::MakeTypeless( DXGI_FORMAT fmt )
{
    switch( fmt )
    {
    case DXGI_FORMAT_R32G32B32A32_FLOAT:
    case DXGI_FORMAT_R32G32B32A32_UINT:
    case DXGI_FORMAT_R32G32B32A32_SINT:
        return DXGI_FORMAT_R32G32B32A32_TYPELESS;

    case DXGI_FORMAT_R32G32B32_FLOAT:
    case DXGI_FORMAT_R32G32B32_UINT:
    case DXGI_FORMAT_R32G32B32_SINT:
        return DXGI_FORMAT_R32G32B32_TYPELESS;

    case DXGI_FORMAT_R16G16B16A16_FLOAT:
    case DXGI_FORMAT_R16G16B16A16_UNORM:
    case DXGI_FORMAT_R16G16B16A16_UINT:
    case DXGI_FORMAT_R16G16B16A16_SNORM:
    case DXGI_FORMAT_R16G16B16A16_SINT:
        return DXGI_FORMAT_R16G16B16A16_TYPELESS;

    case DXGI_FORMAT_R32G32_FLOAT:
    case DXGI_FORMAT_R32G32_UINT:
    case DXGI_FORMAT_R32G32_SINT:
        return DXGI_FORMAT_R32G32_TYPELESS;

    case DXGI_FORMAT_R10G10B10A2_UNORM:
    case DXGI_FORMAT_R10G10B10A2_UINT:
        return DXGI_FORMAT_R10G10B10A2_TYPELESS;

    case DXGI_FORMAT_R8G8B8A8_UNORM:
    case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
    case DXGI_FORMAT_R8G8B8A8_UINT:
    case DXGI_FORMAT_R8G8B8A8_SNORM:
    case DXGI_FORMAT_R8G8B8A8_SINT:
        return DXGI_FORMAT_R8G8B8A8_TYPELESS;

    case DXGI_FORMAT_R16G16_FLOAT:
    case DXGI_FORMAT_R16G16_UNORM:
    case DXGI_FORMAT_R16G16_UINT:
    case DXGI_FORMAT_R16G16_SNORM:
    case DXGI_FORMAT_R16G16_SINT:
        return DXGI_FORMAT_R16G16_TYPELESS;

    case DXGI_FORMAT_D32_FLOAT:
    case DXGI_FORMAT_R32_FLOAT:
    case DXGI_FORMAT_R32_UINT:
    case DXGI_FORMAT_R32_SINT:
        return DXGI_FORMAT_R32_TYPELESS;

    case DXGI_FORMAT_R8G8_UNORM:
    case DXGI_FORMAT_R8G8_UINT:
    case DXGI_FORMAT_R8G8_SNORM:
    case DXGI_FORMAT_R8G8_SINT:
        return DXGI_FORMAT_R8G8_TYPELESS;

    case DXGI_FORMAT_R16_FLOAT:
    case DXGI_FORMAT_D16_UNORM:
    case DXGI_FORMAT_R16_UNORM:
    case DXGI_FORMAT_R16_UINT:
    case DXGI_FORMAT_R16_SNORM:
    case DXGI_FORMAT_R16_SINT:
        return DXGI_FORMAT_R16_TYPELESS;

    case DXGI_FORMAT_R8_UNORM:
    case DXGI_FORMAT_R8_UINT:
    case DXGI_FORMAT_R8_SNORM:
    case DXGI_FORMAT_R8_SINT:
    case DXGI_FORMAT_A8_UNORM:
        return DXGI_FORMAT_R8_TYPELESS;

    case DXGI_FORMAT_BC1_UNORM:
    case DXGI_FORMAT_BC1_UNORM_SRGB:
        return DXGI_FORMAT_BC1_TYPELESS;

    case DXGI_FORMAT_BC2_UNORM:
    case DXGI_FORMAT_BC2_UNORM_SRGB:
        return DXGI_FORMAT_BC2_TYPELESS;

    case DXGI_FORMAT_BC3_UNORM:
    case DXGI_FORMAT_BC3_UNORM_SRGB:
        return DXGI_FORMAT_BC3_TYPELESS;

    case DXGI_FORMAT_BC4_UNORM:
    case DXGI_FORMAT_BC4_SNORM:
        return DXGI_FORMAT_BC4_TYPELESS;

    case DXGI_FORMAT_BC5_UNORM:
    case DXGI_FORMAT_BC5_SNORM:
        return DXGI_FORMAT_BC5_TYPELESS;

    case DXGI_FORMAT_B8G8R8A8_UNORM:
    case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
        return DXGI_FORMAT_B8G8R8A8_TYPELESS;

    case DXGI_FORMAT_B8G8R8X8_UNORM:
    case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
        return DXGI_FORMAT_B8G8R8X8_TYPELESS;

    case DXGI_FORMAT_BC6H_UF16:
    case DXGI_FORMAT_BC6H_SF16:
        return DXGI_FORMAT_BC6H_TYPELESS;

    case DXGI_FORMAT_BC7_UNORM:
    case DXGI_FORMAT_BC7_UNORM_SRGB:
        return DXGI_FORMAT_BC7_TYPELESS;

    default:
        return fmt;
    }
}

//-------------------------------------------------------------------------------------
// Converts to a TYPELESS format to an equivalent UNORM format if available
//-------------------------------------------------------------------------------------
DXGI_FORMAT m3d::dxgiformat::MakeTypelessUNORM( DXGI_FORMAT fmt )
{
    switch( fmt )
    {
    case DXGI_FORMAT_R16G16B16A16_TYPELESS:
        return DXGI_FORMAT_R16G16B16A16_UNORM;

    case DXGI_FORMAT_R10G10B10A2_TYPELESS:
        return DXGI_FORMAT_R10G10B10A2_UNORM;

    case DXGI_FORMAT_R8G8B8A8_TYPELESS:
        return DXGI_FORMAT_R8G8B8A8_UNORM;

    case DXGI_FORMAT_R16G16_TYPELESS:
        return DXGI_FORMAT_R16G16_UNORM;

    case DXGI_FORMAT_R8G8_TYPELESS:
        return DXGI_FORMAT_R8G8_UNORM;

    case DXGI_FORMAT_R16_TYPELESS:
        return DXGI_FORMAT_R16_UNORM;

    case DXGI_FORMAT_R8_TYPELESS:
        return DXGI_FORMAT_R8_UNORM;

    case DXGI_FORMAT_BC1_TYPELESS:
        return DXGI_FORMAT_BC1_UNORM;

    case DXGI_FORMAT_BC2_TYPELESS:
        return DXGI_FORMAT_BC2_UNORM;

    case DXGI_FORMAT_BC3_TYPELESS:
        return DXGI_FORMAT_BC3_UNORM;

    case DXGI_FORMAT_BC4_TYPELESS:
        return DXGI_FORMAT_BC4_UNORM;

    case DXGI_FORMAT_BC5_TYPELESS:
        return DXGI_FORMAT_BC5_UNORM;

    case DXGI_FORMAT_B8G8R8A8_TYPELESS:
        return DXGI_FORMAT_B8G8R8A8_UNORM;

    case DXGI_FORMAT_B8G8R8X8_TYPELESS:
        return DXGI_FORMAT_B8G8R8X8_UNORM;

    case DXGI_FORMAT_BC7_TYPELESS:
        return DXGI_FORMAT_BC7_UNORM;

    default:
        return fmt;
    }
}


//-------------------------------------------------------------------------------------
// Converts to a TYPELESS format to an equivalent FLOAT format if available
//-------------------------------------------------------------------------------------
DXGI_FORMAT m3d::dxgiformat::MakeTypelessFLOAT( DXGI_FORMAT fmt )
{
    switch( fmt )
    {
    case DXGI_FORMAT_R32G32B32A32_TYPELESS:
        return DXGI_FORMAT_R32G32B32A32_FLOAT;

    case DXGI_FORMAT_R32G32B32_TYPELESS:
        return DXGI_FORMAT_R32G32B32_FLOAT;

    case DXGI_FORMAT_R16G16B16A16_TYPELESS:
        return DXGI_FORMAT_R16G16B16A16_FLOAT;

    case DXGI_FORMAT_R32G32_TYPELESS:
        return DXGI_FORMAT_R32G32_FLOAT;

    case DXGI_FORMAT_R16G16_TYPELESS:
        return DXGI_FORMAT_R16G16_FLOAT;

    case DXGI_FORMAT_R32_TYPELESS:
        return DXGI_FORMAT_R32_FLOAT;

    case DXGI_FORMAT_R16_TYPELESS:
        return DXGI_FORMAT_R16_FLOAT;

    default:
        return fmt;
    }
}


static const Char *_DXGI_FORMAT_STR[] = 
{
"UNKNOWN", 
"R32G32B32A32_TYPELESS", 
"R32G32B32A32_FLOAT",
"R32G32B32A32_UINT",
"R32G32B32A32_SINT",
"R32G32B32_TYPELESS",
"R32G32B32_FLOAT",
"R32G32B32_UINT",
"R32G32B32_SINT",
"R16G16B16A16_TYPELESS",
"R16G16B16A16_FLOAT",
"R16G16B16A16_UNORM",
"R16G16B16A16_UINT",
"R16G16B16A16_SNORM",
"R16G16B16A16_SINT",
"R32G32_TYPELESS",
"R32G32_FLOAT",
"R32G32_UINT",
"R32G32_SINT",
"R32G8X24_TYPELESS",
"D32_FLOAT_S8X24_UINT",
"R32_FLOAT_X8X24_TYPELESS",
"X32_TYPELESS_G8X24_UINT",
"R10G10B10A2_TYPELESS",
"R10G10B10A2_UNORM",
"R10G10B10A2_UINT",
"R11G11B10_FLOAT",
"R8G8B8A8_TYPELESS",
"R8G8B8A8_UNORM",
"R8G8B8A8_UNORM_SRGB",
"R8G8B8A8_UINT",
"R8G8B8A8_SNORM",
"R8G8B8A8_SINT",
"R16G16_TYPELESS",
"R16G16_FLOAT",
"R16G16_UNORM",
"R16G16_UINT",
"R16G16_SNORM",
"R16G16_SINT",
"R32_TYPELESS",
"D32_FLOAT",
"R32_FLOAT",
"R32_UINT",
"R32_SINT",
"R24G8_TYPELESS",
"D24_UNORM_S8_UINT",
"R24_UNORM_X8_TYPELESS",
"X24_TYPELESS_G8_UINT",
"R8G8_TYPELESS",
"R8G8_UNORM",
"R8G8_UINT",
"R8G8_SNORM",
"R8G8_SINT",
"R16_TYPELESS",
"R16_FLOAT",
"D16_UNORM",
"R16_UNORM",
"R16_UINT",
"R16_SNORM",
"R16_SINT",
"R8_TYPELESS",
"R8_UNORM",
"R8_UINT",
"R8_SNORM",
"R8_SINT",
"A8_UNORM",
"R1_UNORM",
"R9G9B9E5_SHAREDEXP",
"R8G8_B8G8_UNORM",
"G8R8_G8B8_UNORM",
"BC1_TYPELESS",
"BC1_UNORM",
"BC1_UNORM_SRGB",
"BC2_TYPELESS",
"BC2_UNORM",
"BC2_UNORM_SRGB",
"BC3_TYPELESS",
"BC3_UNORM",
"BC3_UNORM_SRGB",
"BC4_TYPELESS",
"BC4_UNORM",
"BC4_SNORM",
"BC5_TYPELESS",
"BC5_UNORM",
"BC5_SNORM",
"B5G6R5_UNORM",
"B5G5R5A1_UNORM",
"B8G8R8A8_UNORM",
"B8G8R8X8_UNORM",
"R10G10B10_XR_BIAS_A2_UNORM",
"B8G8R8A8_TYPELESS",
"B8G8R8A8_UNORM_SRGB",
"B8G8R8X8_TYPELESS",
"B8G8R8X8_UNORM_SRGB",
"BC6H_TYPELESS",
"BC6H_UF16",
"BC6H_SF16",
"BC7_TYPELESS",
"BC7_UNORM",
"BC7_UNORM_SRGB",
"AYUV",
"Y4102",
"Y416",
"NV12",
"P010",
"P016",
"420_OPAQUE",
"YUY2",
"Y210",
"Y216",
"NV11",
"AI44",
"IA44",
"P8",
"A8P8",
"B4G4R4A4_UNORM",
0,0,0,0,0,0,0,0,0,0,0,0,0,0,
"P208",
"V208",
"V408"
};

const Char *m3d::dxgiformat::ToString( DXGI_FORMAT fmt )
{
	if (fmt < sizeof(_DXGI_FORMAT_STR) / sizeof(const Char*))
		return _DXGI_FORMAT_STR[fmt];
	return 0;
}

static const Char* _DXGI_FORMAT_COMMENT_STR[] =
{
"The format is not known.",
"A four-component, 128-bit typeless format that supports 32 bits per channel including alpha.",
"A four-component, 128-bit floating-point format that supports 32 bits per channel including alpha.",
"A four-component, 128-bit unsigned-integer format that supports 32 bits per channel including alpha.",
"A four-component, 128-bit signed-integer format that supports 32 bits per channel including alpha.",
"A three-component, 96-bit typeless format that supports 32 bits per color channel.",
"A three-component, 96-bit floating-point format that supports 32 bits per color channel.",
"A three-component, 96-bit unsigned-integer format that supports 32 bits per color channel.",
"A three-component, 96-bit signed-integer format that supports 32 bits per color channel.",
"A four-component, 64-bit typeless format that supports 16 bits per channel including alpha.",
"A four-component, 64-bit floating-point format that supports 16 bits per channel including alpha.",
"A four-component, 64-bit unsigned-normalized - integer format that supports 16 bits per channel including alpha.",
"A four-component, 64-bit unsigned-integer format that supports 16 bits per channel including alpha.",
"A four-component, 64-bit signed-normalized-integer format that supports 16 bits per channel including alpha.",
"A four-component, 64-bit signed-integer format that supports 16 bits per channel including alpha.",
"A two-component, 64-bit typeless format that supports 32 bits for the red channel and 32 bits for the green channel.",
"A two-component, 64-bit floating-point format that supports 32 bits for the red channel and 32 bits for the green channel.",
"A two-component, 64-bit unsigned-integer format that supports 32 bits for the red channel and 32 bits for the green channel.",
"A two-component, 64-bit signed-integer format that supports 32 bits for the red channel and 32 bits for the green channel.",
"A two-component, 64-bit typeless format that supports 32 bits for the red channel, 8 bits for the green channel, and 24 bits are unused.",
"A 32-bit floating-point component, and two unsigned-integer components (with an additional 32 bits). This format supports 32-bit depth, 8-bit stencil, and 24 bits are unused.",
"A 32-bit floating-point component, and two typeless components (with an additional 32 bits). This format supports 32-bit red channel, 8 bits are unused, and 24 bits are unused.",
"A 32-bit typeless component,and two unsigned - integer components(with an additional 32 bits). This format has 32 bits unused, 8 bits for green channel, and 24 bits are unused.",
"A four-component, 32-bit typeless format that supports 10 bits for each color and 2 bits for alpha.",
"A four-component, 32-bit unsigned-normalized-integer format that supports 10 bits for each color and 2 bits for alpha.",
"A four-component, 32-bit unsigned-integer format that supports 10 bits for each color and 2 bits for alpha.",
"Three partial-precision floating-point numbers encoded into a single 32-bit value (a variant of s10e5, which is sign bit, 10-bit mantissa, and 5-bit biased (15) exponent). There are no sign bits, and there is a 5-bit biased (15) exponent for each channel, 6-bit mantissa for R and G, and a 5-bit mantissa for B.",
"A four-component, 32-bit typeless format that supports 8 bits per channel including alpha.",
"A four-component, 32-bit unsigned-normalized-integer format that supports 8 bits per channel including alpha.",
"A four-component, 32-bit unsigned-normalized integer sRGB format that supports 8 bits per channel including alpha.",
"A four-component, 32-bit unsigned-integer format that supports 8 bits per channel including alpha.",
"A four-component, 32-bit signed-normalized-integer format that supports 8 bits per channel including alpha.",
"A four-component, 32-bit signed-integer format that supports 8 bits per channel including alpha.",
"A two-component, 32-bit typeless format that supports 16 bits for the red channel and 16 bits for the green channel.",
"A two-component, 32-bit floating-point format that supports 16 bits for the red channel and 16 bits for the green channel.",
"A two-component, 32-bit unsigned-normalized-integer format that supports 16 bits each for the greenand red channels.",
"A two-component, 32-bit unsigned-integer format that supports 16 bits for the red channel and 16 bits for the green channel.",
"A two-component, 32-bit signed-normalized-integer format that supports 16 bits for the red channel and 16 bits for the green channel.",
"A two-component, 32-bit signed-integer format that supports 16 bits for the red channel and 16 bits for the green channel.",
"A single-component, 32-bit typeless format that supports 32 bits for the red channel.",
"A single-component, 32-bit floating-point format that supports 32 bits for depth.",
"A single-component, 32-bit floating-point format that supports 32 bits for the red channel.",
"A single-component, 32-bit unsigned-integer format that supports 32 bits for the red channel.",
"A single-component, 32-bit signed-integer format that supports 32 bits for the red channel.",
"A two-component, 32-bit typeless format that supports 24 bits for the red channel and 8 bits for the green channel.",
"A 32-bit z-buffer format that supports 24 bits for depth and 8 bits for stencil.",
"A 32-bit format, that contains a 24 bit, single-component, unsigned-normalized integer, with an additional typeless 8 bits. This format has 24 bits red channel and 8 bits unused.",
"A 32-bit format, that contains a 24 bit, single-component, typeless format, with an additional 8 bit unsigned integer component. This format has 24 bits unused and 8 bits green channel.",
"A two-component, 16-bit typeless format that supports 8 bits for the red channel and 8 bits for the green channel.",
"A two-component, 16-bit unsigned-normalized-integer format that supports 8 bits for the red channel and 8 bits for the green channel.",
"A two-component, 16-bit unsigned-integer format that supports 8 bits for the red channel and 8 bits for the green channel.",
"A two-component, 16-bit signed-normalized-integer format that supports 8 bits for the red channel and 8 bits for the green channel.",
"A two-component, 16-bit signed-integer format that supports 8 bits for the red channel and 8 bits for the green channel.",
"A single-component, 16-bit typeless format that supports 16 bits for the red channel.",
"A single-component, 16-bit floating-point format that supports 16 bits for the red channel.",
"A single-component, 16-bit unsigned-normalized-integer format that supports 16 bits for depth.",
"A single-component, 16-bit unsigned-normalized-integer format that supports 16 bits for the red channel.",
"A single-component, 16-bit unsigned-integer format that supports 16 bits for the red channel.",
"A single-component, 16-bit signed-normalized-integer format that supports 16 bits for the red channel.",
"A single-component, 16-bit signed-integer format that supports 16 bits for the red channel.",
"A single-component, 8-bit typeless format that supports 8 bits for the red channel.",
"A single-component, 8-bit unsigned-normalized-integer format that supports 8 bits for the red channel.",
"A single-component, 8-bit unsigned-integer format that supports 8 bits for the red channel.",
"A single-component, 8-bit signed-normalized-integer format that supports 8 bits for the red channel.",
"A single-component, 8-bit signed-integer format that supports 8 bits for the red channel.",
"A single-component, 8-bit unsigned-normalized-integer format for alpha only.",
"A single-component, 1-bit unsigned-normalized integer format that supports 1 bit for the red channel.",
"Three partial-precision floating-point numbers encoded into a single 32-bit value all sharing the same 5-bit exponent (variant of s10e5, which is sign bit, 10-bit mantissa, and 5-bit biased (15) exponent). There is no sign bit,and there is a shared 5-bit biased (15) exponent and a 9-bit mantissa for each channel.",
"A four-component, 32-bit unsigned-normalized-integer format. This packed RGB format is analogous to the UYVY format.Each 32-bit block describes a pair of pixels : (R8, G8, B8) and (R8, G8, B8) where the R8 / B8 values are repeated,and the G8 values are unique to each pixel. Width must be even.",
"A four-component, 32-bit unsigned-normalized-integer format. This packed RGB format is analogous to the YUY2 format.Each 32-bit block describes a pair of pixels : (R8, G8, B8) and (R8, G8, B8) where the R8 / B8 values are repeated,and the G8 values are unique to each pixel. Width must be even.",
"Four-component typeless block-compression format. Three color channels (5 bits:6 bits:5 bits), with 0 or 1 bit(s) of alpha.",
"Four-component block-compression format. Three color channels (5 bits:6 bits:5 bits), with 0 or 1 bit(s) of alpha.",
"Four-component block-compression format for sRGB data. Three color channels (5 bits:6 bits:5 bits), with 0 or 1 bit(s) of alpha.",
"Four-component typeless block-compression format. Three color channels (5 bits:6 bits:5 bits), with 4 bits of alpha.",
"Four-component block-compression format. Three color channels (5 bits:6 bits:5 bits), with 4 bits of alpha.",
"Four-component block-compression format for sRGB data. Three color channels (5 bits:6 bits:5 bits), with 4 bits of alpha.",
"Four-component typeless block-compression format. Three color channels (5 bits:6 bits:5 bits) with 8 bits of alpha.",
"Four-component block-compression format. Three color channels (5 bits:6 bits:5 bits) with 8 bits of alpha.",
"Four-component block-compression format for sRGB data. Three color channels (5 bits:6 bits:5 bits) with 8 bits of alpha.",
"One-component typeless block-compression format. One color channel (8 bits).",
"One-component block-compression format. One color channel (8 bits).",
"One-component block-compression format. One color channel (8 bits).",
"Two-component typeless block-compression format. Two color channels (8 bits:8 bits).",
"Two-component block-compression format. Two color channels (8 bits:8 bits).",
"Two-component block-compression format. Two color channels (8 bits:8 bits).",
"A three-component, 16-bit unsigned-normalized-integer format that supports 5 bits for blue, 6 bits for green, and 5 bits for red.",
"A four-component, 16-bit unsigned-normalized-integer format that supports 5 bits for each color channel and 1-bit alpha.",
"A four-component, 32-bit unsigned-normalized-integer format that supports 8 bits for each color channel and 8-bit alpha.",
"A four-component, 32-bit unsigned-normalized-integer format that supports 8 bits for each color channel and 8 bits unused.",
"A four-component, 32-bit 2.8-biased fixed-point format that supports 10 bits for each color channel and 2-bit alpha.",
"A four-component, 32-bit typeless format that supports 8 bits for each channel including alpha.",
"A four-component, 32-bit unsigned-normalized standard RGB format that supports 8 bits for each channel including alpha.",
"A four-component, 32-bit typeless format that supports 8 bits for each color channel, and 8 bits are unused.",
"A four-component, 32-bit unsigned-normalized standard RGB format that supports 8 bits for each color channel, and 8 bits are unused.",
"A typeless block-compression format. Three color channels (16 bits:16 bits:16 bits) in \"half\" floating point.",
"A block-compression format. Three color channels (16 bits:16 bits:16 bits) in \"half\" floating point.",
"A block-compression format. Three color channels (16 bits:16 bits:16 bits) in \"half\" floating point.",
"A typeless block-compression format. Three color channels (4 to 7 bits per channel) with 0 to 8 bits of alpha.",
"A block-compression format. Three color channels (4 to 7 bits per channel) with 0 to 8 bits of alpha.",
"A block-compression format. Three color channels (4 to 7 bits per channel) with 0 to 8 bits of alpha."
"AYUV",
"Y4102",
"Y416",
"NV12",
"P010",
"P016",
"420_OPAQUE",
"YUY2",
"Y210",
"Y216",
"NV11",
"AI44",
"IA44",
"P8",
"A8P8",
"B4G4R4A4_UNORM",
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
"P208",
"V208",
"V408"
};

const Char* m3d::dxgiformat::ToCommentString(DXGI_FORMAT fmt)
{
	if (fmt < sizeof(_DXGI_FORMAT_COMMENT_STR) / sizeof(const Char*))
		return _DXGI_FORMAT_COMMENT_STR[fmt];
	return 0;
}

