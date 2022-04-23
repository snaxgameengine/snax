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
#include "Shaders.h"
#include "Shader.h"
#include "RenderSettings.h"

using namespace m3d;


CHIPDESCV1_DEF(Shaders, MTEXT("Shaders"), SHADERS_GUID, CHIP_GUID);



Shaders::Shaders()
{
	CREATE_CHILD(0, SHADER_GUID, false, UP, MTEXT("Vertex Shader"));
	CREATE_CHILD(1, SHADER_GUID, false, UP, MTEXT("Hull Shader"));
	CREATE_CHILD(2, SHADER_GUID, false, UP, MTEXT("Domain Shader"));
	CREATE_CHILD(3, SHADER_GUID, false, UP, MTEXT("Geometry Shader"));
	CREATE_CHILD(4, SHADER_GUID, false, UP, MTEXT("Pixel Shader"));

	memset(_shaderStamps, 0, sizeof(_shaderStamps));
}

Shaders::~Shaders()
{
}

const ShaderPipelineDesc& Shaders::GetProgram()
{
	RefreshT refresh(Refresh);

	if (!refresh) {
		if (_shaders.shaderStages == (UINT)ShaderType::UNDEFINED)
			throw ReinitPreventedException(this);
		return _shaders;
	}

	const List<SignatureParameter>* parameters = nullptr;

	ShaderPipelineDesc shaders;

	ChildPtr<Shader> chShaders[5];
	UpdateStamp shaderStamps[5] = { 0 };

	for (uint32 i = 0; i < 5; i++) {
		chShaders[i] = GetChild(i);
		if (chShaders[i])
			shaderStamps[i] = chShaders[i]->GetUpdateStamp();
	}

	if (std::memcmp(_shaderStamps, shaderStamps, sizeof(_shaderStamps)) == 0)
		return _shaders; // There are no updates to our shaders!

	try {
		for (uint32 i = 0; i < 5; i++) {
			ChildPtr<Shader> ch = chShaders[i];
			if (ch) {
				ShaderType st = (ShaderType)(1 << i);
				const ShaderDesc& sd = ch->GetShader(st, parameters); // throws!
				shaders.byteCodes[i] = sd.byteCode;
				shaders.shaderStages |= (UINT)st;
				if (st == ShaderType::VS) // DO we need the VS?
					shaders.inputParameters = sd.inputParameters;
				parameters = &sd.outputParameters;
				for (const auto& n : sd.uniforms) {
					auto m = shaders.uniforms.find(n.first);
					if (m != shaders.uniforms.end()) {
						if (!m->second.isCompatible(n.second))
							throw GraphicsException(this, MTEXT("Uniforms are incompatible."), FATAL);
						shaders.uniforms[n.first].bufferLayout = shaders.uniforms[n.first].bufferLayout.Merge(n.second.bufferLayout);
						shaders.uniforms[n.first].usageMask |= n.second.usageMask;
					}
					else
						shaders.uniforms[n.first] = n.second;
					shaders.uniforms[n.first].Stages |= (UINT)st;
				}
			}
		}

		for (auto& n : shaders.uniforms) {
			if (!n.second.bufferLayout.GetItems().empty())
				n.second.bufferLayoutID = BufferLayoutManager::GetInstance().RegisterLayout(n.second.bufferLayout);
		}


		// TODO: Check the uniforms again for arrays...

		if (parameters)
			shaders.outputParameters = *parameters; // Correct? What if we have stream-out?

		// NEED VS (for now!)
		if ((shaders.shaderStages & ((UINT)ShaderType::VS)) != ((UINT)ShaderType::VS))
			throw GraphicsException(this, MTEXT("Missing shader stages."), FATAL);
	}
	catch (const ChipException& e) {
		_shaders = ShaderPipelineDesc();
		memset(_shaderStamps, 0, sizeof(_shaderStamps));
		throw;
	}

	if (!_shaders.isEqual(shaders)) {
		SetUpdateStamp();
		_shaders = shaders;
		std::memcpy(_shaderStamps, shaderStamps, sizeof(_shaderStamps));
	}

	return _shaders;
}



