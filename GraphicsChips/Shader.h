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
#include "GraphicsDefines.h"
#include "ShaderDefs.h"

namespace m3d
{

static const Guid SHADER_GUID = { 0x7d2c71e9, 0x5b6d, 0x403f,{ 0x99, 0xeb, 0xf0, 0x2, 0xaf, 0x26, 0xe0, 0x9e } };


class GRAPHICSCHIPS_API Shader : public Chip
{
	CHIPDESC_DECL;
public:
	Shader();
	~Shader();

	bool CopyChip(Chip* chip) override;
	bool LoadChip(DocumentLoader& loader) override;
	bool SaveChip(DocumentSaver& saver) const override;

	virtual ShaderType GetShaderType() const { return _st; }
	virtual void SetShaderType(ShaderType st) { _st = st; }

	virtual bool IsManualMode() const { return _manualMode; }
	virtual void SetManualMode(bool b) { _manualMode = b; }

	virtual String GetShaderSource() const { return _source; }
	virtual void SetShaderSource(String src) { _source = src; }

	virtual UINT GetCompileFlags() const { return _compileFlags; }
	virtual void SetCompileFlags(UINT flags) { _compileFlags = flags; }

	virtual ShaderModel GetShaderModel() const { return _sm; }
	virtual void SetShaderModel(ShaderModel sm) { _sm = sm; }

	virtual bool Compile();

	virtual SID3DBlob GetByteCode() const { return _shader.byteCode; }
	virtual void SetByteCode(SID3DBlob bc) { _shader.byteCode = bc; }

	static HRESULT Compile(ID3DBlob** bc, ShaderType st, ShaderModel sm, UINT compileFlags, String source, String& msg);

	String GetCompileMessages() const { return _messages; }
	void SetCompileMessages(String msg) { _messages = msg; }

	// st can not be AUTO.
	virtual const ShaderDesc& GetShader(ShaderType st, const List<SignatureParameter>* outputParametersFromPrevStage = nullptr);

protected:
	ShaderType _st;
	bool _manualMode;
	UINT _compileFlags;
	ShaderModel _sm;
	String _source;

	String _messages;
	ShaderDesc _shader;

	bool _validateInputParameters(const List<SignatureParameter>* outputParametersFromPrevStage);
};


}