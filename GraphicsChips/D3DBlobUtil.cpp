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
#include "D3DBlobUtil.h"


namespace m3d
{

// My own implementation; 2 advantages: 1. Not dependent on D3DCreateBlob in d3dcompiler.lib.  2. Able to use shared memory for the blob!
struct _D3DBlob : public ID3DBlob
{
	_D3DBlob() : _data(nullptr), _size(0), _ownsData(TRUE), _refCount(1) {}
	~_D3DBlob() 
	{
		if (_ownsData)
			delete[] _data;
	}
	virtual LPVOID STDMETHODCALLTYPE GetBufferPointer( void) { return _data; }
	virtual SIZE_T STDMETHODCALLTYPE GetBufferSize( void) { return _size; }
	virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, _COM_Outptr_ void __RPC_FAR *__RPC_FAR *ppvObject) { return E_NOTIMPL; }
	virtual ULONG STDMETHODCALLTYPE AddRef( void) { return ++_refCount; }
	virtual ULONG STDMETHODCALLTYPE Release( void) { ULONG c = --_refCount; if (_refCount == 0) delete this; return c; }

	LPVOID _data;
	SIZE_T _size;
	BOOL _ownsData;
	ULONG _refCount;
};


HRESULT CreateD3DBlobFromCArray(const LPVOID pData, UINT Size, ID3DBlob **blob) 
{
	_D3DBlob *b = new _D3DBlob();
	b->_data = new BYTE[Size];
	b->_size = Size;
	b->_ownsData = TRUE;
	std::memcpy(b->_data, pData, Size);
	*blob = b;
	return S_OK;
}


HRESULT CreateD3DBlobFromCArrayShared(LPVOID pData, UINT Size, ID3DBlob **blob) 
{
	_D3DBlob *b = new _D3DBlob();
	b->_data = pData;
	b->_size = Size;
	b->_ownsData = FALSE;
	*blob = b;
	return S_OK;
}

}