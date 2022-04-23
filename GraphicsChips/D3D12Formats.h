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
#include <DXGIFormat.h>

namespace m3d
{

namespace dxgiformat
{
	uint32 GRAPHICSCHIPS_API BitsPerPixel( DXGI_FORMAT format );

	bool GRAPHICSCHIPS_API IsValid( DXGI_FORMAT fmt );
	bool GRAPHICSCHIPS_API IsCompressed( DXGI_FORMAT fmt );
	bool GRAPHICSCHIPS_API IsPacked( DXGI_FORMAT fmt );
	bool GRAPHICSCHIPS_API IsVideo( DXGI_FORMAT fmt );
	bool GRAPHICSCHIPS_API IsSRGB( DXGI_FORMAT fmt );
	bool GRAPHICSCHIPS_API IsTypeless( DXGI_FORMAT fmt, bool partialTypeless =  true);
	bool GRAPHICSCHIPS_API HasAlpha( DXGI_FORMAT fmt );
	bool GRAPHICSCHIPS_API IsDepthStencil(DXGI_FORMAT fmt);

	DXGI_FORMAT GRAPHICSCHIPS_API MakeSRGB( DXGI_FORMAT fmt );
	DXGI_FORMAT GRAPHICSCHIPS_API MakeTypeless( DXGI_FORMAT fmt );
	DXGI_FORMAT GRAPHICSCHIPS_API MakeTypelessUNORM( DXGI_FORMAT fmt );
	DXGI_FORMAT GRAPHICSCHIPS_API MakeTypelessFLOAT( DXGI_FORMAT fmt );

	DXGI_FORMAT GRAPHICSCHIPS_API PrevFormat(DXGI_FORMAT fmt);
	DXGI_FORMAT GRAPHICSCHIPS_API NextFormat(DXGI_FORMAT fmt);

	const Char GRAPHICSCHIPS_API *ToString( DXGI_FORMAT fmt );
	const Char GRAPHICSCHIPS_API* ToCommentString(DXGI_FORMAT fmt);
}

}