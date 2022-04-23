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
#include "D3D12Include.h"
#include "M3DEngine/DocumentSaver.h"
#include "M3DEngine/DocumentLoader.h"
#include "M3DEngine/DocumentSaveLoadUtil.h"



namespace m3d
{

template<typename T, typename R>
class ComPtr;

template<typename T, typename R>
struct SaveDataT<ComPtr<T, R>>
{
	static bool Serialize(DocumentSaver &saver, const ComPtr<T, R> &t) { return SaveDataT<T*>::Serialize(saver, t.get()); }
};

template<typename T, typename R>
struct LoadDataT<ComPtr<T, R>>
{
	static bool Deserialize(DocumentLoader &loader, ComPtr<T, R> &t) 
	{
		T *tmp = nullptr;
		if (!LoadDataT<T*>::Deserialize(loader, tmp))
			return false;
		t = tmp;
		if (tmp)
			tmp->Release();
		return true;
	}
};

bool GRAPHICSCHIPS_API SerializeDocumentData(DocumentSaver &saver, const ID3DBlob *data);
bool GRAPHICSCHIPS_API DeserializeDocumentData(DocumentLoader &loader, ID3DBlob *&data);
/*
class InputElementDescs;

bool D312DCHIPS_API SerializeDocumentData(DocumentSaver &saver, const InputElementDescs &data);
bool GRAPHICSCHIPS_API DeserializeDocumentData(DocumentLoader &loader, InputElementDescs &data);

bool GRAPHICSCHIPS_API SerializeDocumentData(DocumentSaver &saver, const D3D11_INPUT_ELEMENT_DESC &data);
bool GRAPHICSCHIPS_API DeserializeDocumentData(DocumentLoader &loader, D3D11_INPUT_ELEMENT_DESC &data); // IMPORTANT: Remember to delete SemanticName!!!
bool GRAPHICSCHIPS_API SerializeDocumentData(DocumentSaver &saver, const D3D11_BUFFER_DESC &data);
bool GRAPHICSCHIPS_API DeserializeDocumentData(DocumentLoader &loader, D3D11_BUFFER_DESC &data);
bool GRAPHICSCHIPS_API SerializeDocumentData(DocumentSaver &saver, const D3D11_BLEND_DESC &data);
bool GRAPHICSCHIPS_API DeserializeDocumentData(DocumentLoader &loader, D3D11_BLEND_DESC &data);
bool GRAPHICSCHIPS_API SerializeDocumentData(DocumentSaver &saver, const D3D11_DEPTH_STENCIL_DESC &data);
bool GRAPHICSCHIPS_API DeserializeDocumentData(DocumentLoader &loader, D3D11_DEPTH_STENCIL_DESC &data);
bool GRAPHICSCHIPS_API SerializeDocumentData(DocumentSaver &saver, const D3D11_RASTERIZER_DESC &data);
bool GRAPHICSCHIPS_API DeserializeDocumentData(DocumentLoader &loader, D3D11_RASTERIZER_DESC &data);

*/

bool GRAPHICSCHIPS_API SerializeDocumentData(DocumentSaver &saver, const D3D12_SAMPLER_DESC &data);
bool GRAPHICSCHIPS_API DeserializeDocumentData(DocumentLoader &loader, D3D12_SAMPLER_DESC &data);

}