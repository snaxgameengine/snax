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
#include "M3DEngine/Chip.h"
#include "Graphics.h"
#include "PipelineStatePool.h"
#include "M3DCore/Containers.h"
#include <regex>
#include <initializer_list>

namespace m3d
{



template<typename T, typename S>
struct Mapping
{
	Map<T, S> map;
	const S UNDEFINED;

	Mapping(std::initializer_list<typename Map<T, S>::value_type> v, const S &UNDEFINED) : map(v), UNDEFINED(UNDEFINED)
	{
	}

	S operator[](const T &key) const 
	{
		auto itr = map.find(key);
		if (itr == map.end())
			return UNDEFINED;
		return itr->second;
	}
};




static const Mapping<ShaderType, String> ShaderTypeText ({ { ShaderType::VS, "vs"}, { ShaderType::HS, "hs" } ,{ ShaderType::DS, "ds" } ,{ ShaderType::GS, "gs" },{ ShaderType::PS, "ps" } , { ShaderType::CS, "cs"} }, "error" );





static const String STD_CB1_CODE = 
	"cbuffer std1 : register(b0)\n" \
	"{\n" \
	"    float4x4 view;\n" \
	"    float4x4 projection;\n" \
	"    float4x4 viewInverse;\n" \
	"    float4x4 viewInverseTranspose;\n" \
	"    float4x4 projectionInverse;\n" \
	"    float4x4 viewProjection;\n" \
	"    float4x4 viewProjectionInverse;\n" \
	"};\n";

static const String STD_CB2_CODE =
	"cbuffer std2 : register(b1)\n" \
	"{\n" \
	"    float4x4 world;\n" \
	"    float4x4 worldInverse;\n" \
	"    float4x4 worldInverseTranspose;\n" \
	"    float4x4 worldView;\n" \
	"    float4x4 worldViewInverse;\n" \
	"    float4x4 worldViewProjection;\n" \
	"    float4x4 worldViewProjectionInverse;\n" \
	"    float4x4 shadow;\n" \
	"    float4x4 worldShadow;\n" \
	"};\n";


}