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
#include "Shader.h"
#include "M3DCore/HighPrecisionTimer.h"
#include "GraphicsChips/GraphicsException.h"
#include <regex>
#include "M3DEngine/DocumentSaveLoadUtil.h"
#include "D3DSaveLoadUtil.h"
#include "RenderSettings.h"

using namespace m3d;


namespace m3d
{

	bool SerializeDocumentData(DocumentSaver& saver, const ShaderInputBindDesc& data)
	{
		SAVE("register", data.Name);
		SAVE("type", data.Type);
		SAVE("bindPoint", data.BindPoint);
		SAVE("bindCount", data.BindCount);
		SAVE("uFlags", data.uFlags);
		SAVE("returnType", data.ReturnType);
		SAVE("dimension", data.Dimension);
		SAVE("numSamples", data.NumSamples);
		SAVE("space", data.Space);
		SAVE("size", data.Size);
		SAVE("usageMask", data.usageMask);
		SAVE("bufferLayout", data.bufferLayout.GetConfig());
		return true;
	}

	bool DeserializeDocumentData(DocumentLoader& loader, ShaderInputBindDesc& data)
	{
		LOAD("register", data.Name);
		if (loader.GetDocumentVersion() < Version(1, 2, 7, 0)) {
			LOAD("space", data.Type); // <= Nasty bug.
		}
		else {
			LOAD("type", data.Type);
		}
		LOAD("bindPoint", data.BindPoint);
		LOAD("bindCount", data.BindCount);
		LOAD("uFlags", data.uFlags);
		LOAD("returnType", data.ReturnType);
		LOAD("dimension", data.Dimension);
		LOAD("numSamples", data.NumSamples);
		LOAD("space", data.Space);
		LOAD("size", data.Size);
		LOADDEF("usageMask", data.usageMask, 0);
		String config;
		LOADDEF("bufferLayout", config, String());
		if (!config.empty()) {
			data.bufferLayout.Init(config);
			data.bufferLayoutID = BufferLayoutManager::GetInstance().RegisterLayout(data.bufferLayout);
		}
		else {
			data.bufferLayout = BufferLayout();
			data.bufferLayoutID = InvalidBufferLayoutID;
		}
		return true;
	}

	bool SerializeDocumentData(DocumentSaver& saver, const ShaderInputBindDescKey& data)
	{
		SAVE("register", data.Register);
		SAVE("space", data.Space);
		SAVE("bindPoint", data.BindPoint);
		return true;
	}

	bool DeserializeDocumentData(DocumentLoader& loader, ShaderInputBindDescKey& data)
	{
		LOAD("register", data.Register);
		LOAD("space", data.Space);
		LOAD("bindPoint", data.BindPoint);
		return true;
	}

	bool SerializeDocumentData(DocumentSaver& saver, const SignatureParameter& data)
	{
		SAVE("semanticName", data.semanticName);
		SAVE("semanticIndex", data.semanticIndex);
		SAVE("componentType", data.componentType);
		SAVE("mask", data.mask);
		SAVE("readWriteMask", data.readWriteMask);
		SAVE("stream", data.stream);
		SAVE("minPrecision", data.minPrecision);
		return true;
	}

	bool DeserializeDocumentData(DocumentLoader& loader, SignatureParameter& data)
	{
		LOAD("semanticName", data.semanticName);
		LOAD("semanticIndex", data.semanticIndex);
		LOAD("componentType", data.componentType);
		LOAD("mask", data.mask);
		LOAD("readWriteMask", data.readWriteMask);
		LOAD("stream", data.stream);
		LOAD("minPrecision", data.minPrecision);
		return true;
	}

	bool SerializeDocumentData(DocumentSaver& saver, const ShaderDesc& data)
	{
		SAVE("byteCode", data.byteCode);
		SAVE("inputParams", data.inputParameters);
		SAVE("outputParams", data.outputParameters);
		SAVE("uniforms", data.uniforms);
		SAVEDEF("threadGroupSizeX", data.ThreadGroupSize[0], 0);
		SAVEDEF("threadGroupSizeY", data.ThreadGroupSize[1], 0);
		SAVEDEF("threadGroupSizeZ", data.ThreadGroupSize[2], 0);
		return true;
	}

	bool DeserializeDocumentData(DocumentLoader& loader, ShaderDesc& data)
	{
		LOAD("byteCode", data.byteCode);
		LOAD("inputParams", data.inputParameters);
		LOAD("outputParams", data.outputParameters);
		LOAD("uniforms", data.uniforms);
		LOADDEF("threadGroupSizeX", data.ThreadGroupSize[0], 0);
		LOADDEF("threadGroupSizeY", data.ThreadGroupSize[1], 0);
		LOADDEF("threadGroupSizeZ", data.ThreadGroupSize[2], 0);
		return true;
	}
}


CHIPDESCV1_DEF(Shader, MTEXT("Shader"), SHADER_GUID, CHIP_GUID);



Shader::Shader()
{
	_st = ShaderType::UNDEFINED;
	_manualMode = true;
	_compileFlags = 0;
	_sm = ShaderModel::SM51;
}

Shader::~Shader()
{
}

bool Shader::CopyChip(Chip* chip)
{
	Shader* c = dynamic_cast<Shader*>(chip);
	B_RETURN(Chip::CopyChip(c));
	_st = c->_st;
	_manualMode = c->_manualMode;
	_compileFlags = c->_compileFlags;
	_sm = c->_sm;
	_source = c->_source;
	_shader = c->_shader;
	_messages = c->_messages;
	SetUpdateStamp();
	return true;
}

bool Shader::LoadChip(DocumentLoader& loader)
{
	B_RETURN(Chip::LoadChip(loader));
	LOAD("shaderType", _st);
	LOAD("manualMode", _manualMode);
	LOAD("compileFlags", _compileFlags);
	LOAD("shaderModel", _sm);
	LOAD("sourceCode", _source);
	LOAD("shader", _shader);
	_messages.clear();
	SetUpdateStamp();
	return true;
}

bool Shader::SaveChip(DocumentSaver& saver) const
{
	B_RETURN(Chip::SaveChip(saver));
	SAVE("shaderType", _st);
	SAVE("manualMode", _manualMode);
	SAVE("compileFlags", _compileFlags);
	SAVE("shaderModel", _sm);
	SAVE("sourceCode", _source);
	SAVE("shader", _shader);
	return true;
}

const ShaderDesc& Shader::GetShader(ShaderType st, const List<SignatureParameter>* outputParametersFromPrevStage)
{
	RefreshT refresh(Refresh);

	if (st == ShaderType::UNDEFINED)
		throw GraphicsException(this, MTEXT("No shader type specified."), FATAL);

	if (!_shader.byteCode)
		throw GraphicsException(this, MTEXT("No shader is defined."), FATAL);
	if (st != _st)
		throw GraphicsException(this, MTEXT("Incorrect shader type."), FATAL);
	if (!_validateInputParameters(outputParametersFromPrevStage))
		throw GraphicsException(this, MTEXT("Shader input is incompatible with output from previous stage."), FATAL);
	return _shader;
}

bool Shader::_validateInputParameters(const List<SignatureParameter>* outputParametersFromPrevStage)
{
	if (!outputParametersFromPrevStage)
		return true;
	for (size_t i = 0, o = 0; i < _shader.inputParameters.size(); i++) {
		// Some system-generated values can be ignored from the validation. TODO: There are probably more of these...
		if (strUtils::compareNoCase(_shader.inputParameters[i].semanticName, String("SV_IsFrontFace")) == 0)
			continue;
		if (strUtils::compareNoCase(_shader.inputParameters[i].semanticName, String("SV_SampleIndex")) == 0)
			continue;
		if (strUtils::compareNoCase(_shader.inputParameters[i].semanticName, String("SV_Coverage")) == 0)
			continue;
		if (o >= outputParametersFromPrevStage->size() || !_shader.inputParameters[i].isCompatibleWithOutputParameter(outputParametersFromPrevStage->at(o++)))
			return false;
	}
	return true;
}

class ShaderInclude : public ID3DInclude
{
public:
	ShaderInclude() {}
	~ShaderInclude() {}

	HRESULT Open(D3D_INCLUDE_TYPE IncludeType, LPCSTR pFileName, LPCVOID pParentData, LPCVOID* ppData, UINT* pBytes)
	{
		if (String("Std") == String(pFileName)) {
			*ppData = STD_TEXT;
			*pBytes = sizeof(STD_TEXT) - 1; // -1 to exclude final null.
			return S_OK;
		}
		return E_FAIL;
	}

	HRESULT Close(LPCVOID pData) { return S_OK; }
};

bool _getTypeStr(const D3D12_SHADER_TYPE_DESC& tDesc, String& str)
{
	switch (tDesc.Type)
	{
	case D3D_SVT_BOOL: str = "bool"; break;
	case D3D_SVT_INT: str = "int"; break;
	case D3D_SVT_FLOAT: str = "float"; break;
	case D3D_SVT_UINT: str = "uint"; break;
	case D3D_SVT_DOUBLE: str = "double"; break;
	default: return false;
	}
	switch (tDesc.Class)
	{
	case D3D_SVC_SCALAR: break;
	case D3D_SVC_VECTOR: str = strUtils::ConstructString("%1%2").arg(str).arg(tDesc.Columns); break;
	case D3D_SVC_MATRIX_COLUMNS: str = strUtils::ConstructString("%1%2x%3").arg(str).arg(tDesc.Rows).arg(tDesc.Columns); break;
	default: return false;
	}
	return true;
}

bool Shader::Compile()
{
	SetUpdateStamp();

	_shader = ShaderDesc();

	SetUpdateStamp();

	ShaderDesc shader;
	String messages;

	HRESULT hr = Compile(&shader.byteCode, _st, _sm, _compileFlags, _source, messages);

	if (FAILED(hr)) {
		_messages = messages;
		return false;
	}

	_messages = "\n\nShader compiled, but an error happened during the shader reflection process! Most likely you are using syntax/features not supported by SnaX!";


	SID3D12ShaderReflection reflection;
	hr = D3DReflect(shader.byteCode->GetBufferPointer(), shader.byteCode->GetBufferSize(), IID_ID3D11ShaderReflection, (void**)&reflection);
	if (FAILED(hr))
		return false;

	reflection->GetThreadGroupSize(shader.ThreadGroupSize, shader.ThreadGroupSize + 1, shader.ThreadGroupSize + 2);

	D3D12_SHADER_DESC sDesc = { 0 };
	hr = reflection->GetDesc(&sDesc);
	if (FAILED(hr))
		return false;

	for (UINT i = 0; i < sDesc.InputParameters; i++) {
		D3D12_SIGNATURE_PARAMETER_DESC pDesc = { 0 };
		hr = reflection->GetInputParameterDesc(i, &pDesc);
		if (FAILED(hr))
			return false;
		if (pDesc.Register == -1)
			continue; // This does not use a register.
		SignatureParameter sp;
		sp.semanticName = pDesc.SemanticName;
		sp.semanticIndex = pDesc.SemanticIndex;
		sp.componentType = pDesc.ComponentType;
		sp.mask = pDesc.Mask;
		sp.readWriteMask = pDesc.ReadWriteMask;
		sp.stream = pDesc.Stream;
		sp.minPrecision = pDesc.MinPrecision;
		if (pDesc.Register >= shader.inputParameters.size())
			shader.inputParameters.resize(pDesc.Register + 1);
		shader.inputParameters[pDesc.Register] = sp;
	}

	for (UINT i = 0; i < sDesc.OutputParameters; i++) {
		D3D12_SIGNATURE_PARAMETER_DESC pDesc = { 0 };
		hr = reflection->GetOutputParameterDesc(i, &pDesc);
		if (FAILED(hr))
			return false;
		if (pDesc.Register == -1)
			continue; // This does not use a register.
		SignatureParameter sp;
		sp.semanticName = pDesc.SemanticName;
		sp.semanticIndex = pDesc.SemanticIndex;
		sp.componentType = pDesc.ComponentType;
		sp.mask = pDesc.Mask;
		sp.readWriteMask = pDesc.ReadWriteMask;
		sp.stream = pDesc.Stream;
		sp.minPrecision = pDesc.MinPrecision;
		if (pDesc.Register >= shader.outputParameters.size())
			shader.outputParameters.resize(pDesc.Register + 1);
		shader.outputParameters[pDesc.Register] = sp;
	}

	for (UINT i = 0; i < sDesc.BoundResources; i++) {
		D3D12_SHADER_INPUT_BIND_DESC sibDesc = { 0 };
		HRESULT hr = reflection->GetResourceBindingDesc(i, &sibDesc);
		if (FAILED(hr))
			return false;
		ShaderInputBindDesc d = ShaderInputBindDesc(sibDesc);
		d.Stages = (UINT)_st;
		switch (sibDesc.Type)
		{
		case D3D_SIT_CBUFFER:
		case D3D_SIT_TBUFFER:
		{
			ID3D12ShaderReflectionConstantBuffer* cb = reflection->GetConstantBufferByName(sibDesc.Name);
			assert(cb);
			D3D12_SHADER_BUFFER_DESC sbDesc = { 0 };
			hr = cb->GetDesc(&sbDesc);
			if (FAILED(hr))
				return false;
			d.Size = sbDesc.Size;
			String configStr;
			if (sbDesc.Variables == 1) {
				ID3D12ShaderReflectionVariable* v = cb->GetVariableByIndex(0);
				ID3D12ShaderReflectionType* t = v->GetType();
				D3D12_SHADER_VARIABLE_DESC vDesc = { 0 };
				hr = v->GetDesc(&vDesc);
				if (FAILED(hr))
					return false;
				D3D12_SHADER_TYPE_DESC tDesc;
				hr = t->GetDesc(&tDesc);
				if (FAILED(hr))
					return false;
				if (tDesc.Class == D3D_SVC_STRUCT) {
					bool lastFailed = false;
					for (UINT j = 0; j < tDesc.Members; j++) {
						String elStr;
						ID3D12ShaderReflectionType* srt = t->GetMemberTypeByIndex(j);
						D3D12_SHADER_TYPE_DESC tDesc2 = { 0 };
						hr = srt->GetDesc(&tDesc2);
						if (FAILED(hr))
							return false;
						if (!_getTypeStr(tDesc2, elStr)) {
							lastFailed = true;
							continue;
						}
						String name = t->GetMemberTypeName(j);
						elStr = strUtils::ConstructString("%1 %2").arg(elStr).arg(name);
						if (tDesc2.Elements > 0)
							elStr += strUtils::ConstructString("[%1]").arg(tDesc2.Elements);
						if (lastFailed)
							elStr += strUtils::ConstructString(" : offset(%1)").arg(tDesc2.Offset);
						elStr += ";\n";
						configStr += elStr;
						lastFailed = false;
					}
				}
			}
			if (configStr.empty()) {
				bool lastFailed = false;
				for (UINT j = 0; j < sbDesc.Variables; j++) {
					String elStr;
					ID3D12ShaderReflectionVariable* v = cb->GetVariableByIndex(j);
					ID3D12ShaderReflectionType* srt = v->GetType();
					D3D12_SHADER_VARIABLE_DESC vDesc = { 0 };
					hr = v->GetDesc(&vDesc);
					if (FAILED(hr))
						return false;
					D3D12_SHADER_TYPE_DESC tDesc2 = { 0 };
					hr = srt->GetDesc(&tDesc2);
					if (FAILED(hr))
						return false;
					if (!_getTypeStr(tDesc2, elStr)) {
						lastFailed = true;
						continue;
					}
					if (vDesc.uFlags & D3D_SVF_USED)
						d.usageMask |= (1 << j);

					elStr = strUtils::ConstructString("%1 %2").arg(elStr).arg(vDesc.Name);
					if (tDesc2.Elements > 0)
						elStr += strUtils::ConstructString("[%1]").arg(tDesc2.Elements);
					if (lastFailed)
						elStr += strUtils::ConstructString(" : offset(%1)").arg(vDesc.StartOffset);
					elStr += ";\n";
					configStr += elStr;
					lastFailed = false;
				}
			}
			bool bufferLayoutOK = d.bufferLayout.Init(configStr);
			if (bufferLayoutOK)
				d.bufferLayoutID = BufferLayoutManager::GetInstance().RegisterLayout(d.bufferLayout);
			else
			{
				msg(WARN, MTEXT("Ugh! BufferLayout init failed."));
			}
			break;
		}
		case D3D_SIT_TEXTURE:
		case D3D_SIT_SAMPLER:
		case D3D_SIT_UAV_RWTYPED:
		case D3D_SIT_STRUCTURED:
		case D3D_SIT_UAV_RWSTRUCTURED:
		case D3D_SIT_BYTEADDRESS:
		case D3D_SIT_UAV_RWBYTEADDRESS:
		case D3D_SIT_UAV_APPEND_STRUCTURED:
		case D3D_SIT_UAV_CONSUME_STRUCTURED:
		case D3D_SIT_UAV_RWSTRUCTURED_WITH_COUNTER:
		default:;
		}
		shader.uniforms.insert(std::make_pair(ShaderInputBindDescKey(sibDesc.Type, sibDesc.BindPoint, sibDesc.Space), d));
	}

	SID3DBlob strippedByteCode;

	if (SUCCEEDED(D3DStripShader(shader.byteCode->GetBufferPointer(), shader.byteCode->GetBufferSize(), D3DCOMPILER_STRIP_REFLECTION_DATA | /*D3DCOMPILER_STRIP_DEBUG_INFO | */D3DCOMPILER_STRIP_ROOT_SIGNATURE, &strippedByteCode)))
		shader.byteCode = strippedByteCode;
	else
		msg(WARN, MTEXT("Failed to strip shader byte code."));

	shader.sm = _sm;
	shader.st = _st;

	_shader = std::move(shader);

	_messages = messages;

	return true;

}

HRESULT Shader::Compile(ID3DBlob** bc, ShaderType st, ShaderModel sm, UINT compileFlags, String source, String& msg)
{
	msg.clear();

	time_t now = time(NULL);
	struct tm ts;
	localtime_s(&ts, &now);
	Char buf[64];
	strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &ts);

	HighPrecisionTimer timer;
	timer.Tick();

	msg = strUtils::ConstructString("%1: Compiling...\n\n").arg(buf);

	static const Mapping<ShaderType, uint32> ShaderIndex({ { ShaderType::VS, 0 },{ ShaderType::HS, 1 },{ ShaderType::DS, 2 },{ ShaderType::GS, 3 },{ ShaderType::PS, 4 },{ ShaderType::CS, 5 } }, 0);

	const Char* profiles[7][6] =
	{
		{ nullptr },
		{ "vs_4_0_level_9_1", nullptr, nullptr, nullptr, "ps_4_0_level_9_1", nullptr },
		{ "vs_4_0_level_9_3", nullptr, nullptr, nullptr, "ps_4_0_level_9_3", nullptr },
		{ "vs_4_0", nullptr, nullptr, "gs_4_0", "ps_4_0", "cs_4_0" },
		{ "vs_4_1", nullptr, nullptr, "gs_4_1", "ps_4_1", "cs_4_1" },
		{ "vs_5_0", "hs_5_0", "ds_5_0", "gs_5_0", "ps_5_0", "cs_5_0" },
		{ "vs_5_1", "hs_5_1", "ds_5_1", "gs_5_1", "ps_5_1", "cs_5_1" }
	};

	static const Char* SHADER_MODEL_STR[7] = { "", "491", "493", "4000", "4100", "5000", "5100" };
	static const Char* SHADER_STAGE_STR[6] = { "VS", "HS", "DS", "GS", "PS", "CS" };

	const Char* profileStr = profiles[(uint32)sm][ShaderIndex[st]];
	if (profileStr == nullptr) {
		//msg += String(MTEXT("Error: %1 not supported for %2.\n")).arg(typeName[st]).arg(FROMQSTRING(ui.comboBox_profiles->currentText()));
		msg += "Profile is not supported!\n";
		return E_FAIL;
	}

	List<D3D_SHADER_MACRO> macros;

	List<std::pair<String, String>> defines;
	defines.push_back(std::make_pair("VERSION", SHADER_MODEL_STR[(uint32)sm]));
	defines.push_back(std::make_pair(SHADER_STAGE_STR[ShaderIndex[st]], "1"));

	for (size_t i = 0; i < defines.size(); i++) {
		D3D_SHADER_MACRO m = { defines[i].first.c_str(), defines[i].second.c_str() };
		macros.push_back(m);
	}

	if (compileFlags & D3DCOMPILE_DEBUG) {
		D3D_SHADER_MACRO debugMacro = { "DEBUG", "1" };
		macros.push_back(debugMacro);
	}

	D3D_SHADER_MACRO nullMacro = { nullptr, nullptr };
	macros.push_back(nullMacro);

	SID3DBlob errorMsgs;
	/*
	std::regex r("^(\\[.+\\]\\n)+"/*, std::regex_constants::ECMAScript | std::regex_constants::icase* /);
	std::cmatch rmatch;
	if (std::regex_search(_source.c_str(), _source.c_str() + _source.length(), rmatch, r)) {
	for (size_t i = 0; i < rmatch.size(); i++) {
	std::string fdf = rmatch.str(i);
	int32 gg = 3;
	}
	}
	*/
	String src = String("#line 1 \"Main\"\n") + source;

	ShaderInclude si;
	HRESULT hr = D3DCompile2(src.c_str(), src.length(), nullptr, &macros.front(), &si, "main", profileStr, compileFlags, 0, 0, NULL, 0, bc, &errorMsgs);

	UINT instructionCount = 0;

	if (SUCCEEDED(hr)) {
		SID3D12ShaderReflection reflection;
		hr = D3DReflect((*bc)->GetBufferPointer(), (*bc)->GetBufferSize(), IID_ID3D11ShaderReflection, (void**)&reflection);
		if (SUCCEEDED(hr)) {
			D3D12_SHADER_DESC sDesc;
			hr = reflection->GetDesc(&sDesc);
			instructionCount = sDesc.InstructionCount;
		}
	}

	if (errorMsgs)
		msg += String((const Char*)errorMsgs->GetBufferPointer());

	timer.Tick();
	int64 dt = timer.GetDt_us();

	if (SUCCEEDED(hr)) {
		msg += strUtils::ConstructString("Compile SUCCEEDED in %1 microseconds!\n\nShader is %2 instructions (%3 bytes) long.\n").arg(dt).arg(instructionCount).arg((*bc)->GetBufferSize());
	}
	else {
		msg += String("Compile FAILED!\n");
	}

	return hr;

}
