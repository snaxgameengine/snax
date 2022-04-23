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


namespace m3d
{

template<typename T>
struct ComPtrTraits
{
	static void AddRef(T *t) { t->AddRef(); }
	static void Release(T *t) { t->Release(); }
};

template<typename T, typename R = ComPtrTraits<T>>
class ComPtr
{
private:
    T *_t;

public:
	ComPtr() : _t(nullptr) {}

	ComPtr(const ComPtr &ptr) : _t(ptr._t)
	{
		if (_t)
			R::AddRef(_t);
	}

	ComPtr(T *t) : _t(t)
	{
		if (_t)
			R::AddRef(_t);
	}

	~ComPtr()
	{
		if (_t)
			R::Release(_t); 
	}

	ComPtr &operator=(T *t)
	{
		if (_t != t) {
			if (_t)
				R::Release(_t);

			_t = t;

			if (_t)
				R::AddRef(_t);
		}
		return *this;
	}

	ComPtr &operator=(const ComPtr &ptr)
	{
		return *this = ptr._t;
	}

//	__declspec(deprecated)
	T * const *operator&() const { return &_t; }

//	__declspec(deprecated)
	T **operator&() 
	{
		// This operator is not "standard". It will release our current object before returning.
		// It is ment to be used for functions taking a **T with the intention to set the pointer.
		// This assumes that the function will increment the refptr before returning. (All dx funtions does this!)
		if (_t)
			R::Release(_t);
		_t = nullptr;
		return &_t; 
	}

	operator T*() const { return _t; }
//	T& operator*() const { return *_t; }
	T* operator->() const { return _t; }

	// the get() function is needed in cases where you want to cast the pointer.
	// Example:
	// ComPtr<ID3D11Resource> r;
	// (ID3D11Texture2D*)r.get();
	// I do not want to add any extra template casting functions to fix this, becuase this messes things up,
	// and I would probably then have to get rid of the T*() function and use the get() function all the time instead (and add comp. functions)!
	T *get() const { return _t; }

	// Use this togheter with dx-functions accepting T** as parameters. 
	// They will set our pointer and have the ref-count increased automatically!
	T **fill() 
	{
		if (_t)
			R::Release(_t);
		_t = nullptr;
		return &_t; 
	}

	// Use this togheter with dx-functions accepting T** as parameters IF WE are the first element of an ARRAY of ComPtrs and the function is to fill this array!
	// They will set our pointer and have the ref-count increased automatically!
	T **fill(uint32 n)
	{
		T **t = &_t;
		for (uint32 i = 0; i < n; i++) {
			if (t[i]) {
				R::Release(t[i]);
				t[i] = nullptr;
			}
		}
		return &_t;
	}

	T * const *put() const { return &_t; }

};




}