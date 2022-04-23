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
#include "CriticalSection.h"
#include "MTypes.h"

namespace m3d
{

class M3DCORE_API MemStack
{
private:
	uint8 **_stack;
	uint32 _stackSize;
	uint32 _size;
	uint32 _capacity;
	size_t _fragmentSize;
	bool _allocFailed;

public:
	MemStack();
	~MemStack();

	uint32 getSize() const { return _size; }
	uint32 getCapacity() const { return _capacity; }

	void setFragmentSize(size_t fs) { _fragmentSize = fs; }
	size_t getFragmentSize() const { return _fragmentSize; }

	bool getAllocFailed() const { return _allocFailed; }
	void setAllocFailed(bool b) { _allocFailed = b; }

	uint8 *pop();
	void push(uint8 *mem);
	bool pushNew(uint8 *mem);
};

class M3DCORE_API MemBlock
{
private:
	uint8 *_mem;
	size_t _size;
	size_t _free;

public:
	MemBlock();
	~MemBlock();

	size_t getSize() const { return _size; }
	size_t getFree() const { return _free; }

	bool init(size_t size);
	void release();

	uint32 fill(MemStack &ms, uint32 maxCount, uint8 stackID);

};

struct MemStat;

class M3DCORE_API MemoryManager
{
public:
	static const size_t MAX_MEMORY = 1024; // do not set to high. The memory manager is ment to speed up relatively small (and frequent!) allocations!
	static const uint8 STACK_COUNT = 64; // max 255  (1024/64=16 byte steps.. you waste max 15 bytes pr allocation. average 7-8 bytes?)
	static const size_t ALIGNMENT = 16; // Got to be power of 2!
private:
	MemStack _stack[STACK_COUNT];
	MemBlock _block[256];
	size_t _blockSize;
	uint8 _stackMap[MAX_MEMORY];

	CriticalSection _cs;

	MemStat * _stats;

	void *_getCustomMemory(size_t size);

public:
	MemoryManager(size_t blockSize = 1024 * 1024);
	~MemoryManager();
	
	void *get(size_t size); // Every allocation adds 1 extra byte of metadata in front of the returned memory address! Memory is guarantied ALIGNMENT memory alignement!
	void free(void *mem);
	void *getDebug(size_t size, const char *file, int32 line); // Debug version. Use freeDebug() for memory allocated with this! This method add several bytes of meta data, but guaranties alignment.
	void freeDebug(void *mem); // Debug version
	void *realloc(void *mem, size_t size);
	void *reallocDebug(void *mem, size_t size, const char *file, int32 line);
};


extern M3DCORE_API MemoryManager &mm(); // Global memory manager

#if defined(DEBUG) | defined(_DEBUG)
	#define mmnew new //new(mm(), __FILE__, __LINE__)

	template<typename T> void mmdelete(const T* p)
	{
		//p->~T();
		//mm().freeDebug((T*)p);
		delete p;
	}
#else
	// DO NOT USE mmnew for arrays!!!
	#define mmnew new // new(mm())

	// DO NOT USE mmdelete for arrays!!!
	template<typename T> void mmdelete(const T* p)
	{
		//p->~T();
		//mm().free((T*)p);
		delete p;
	}
#endif



#if defined(DEBUG) | defined(_DEBUG)
	#define mmalloc(size) mm().getDebug(size, __FILE__, __LINE__)
	#define mmfree(pMem) mm().freeDebug(pMem);
	#define mrealloc(pMem, size) mm().reallocDebug(pMem, size, __FILE__, __LINE__)
#else
	#define mmalloc(size) mm().get(size)
	#define mmfree(pMem) mm().free(pMem);
	#define mrealloc(pMem, size) mm().realloc(pMem, size)
#endif

}
/*
static void* operator new(size_t size, m3d::MemoryManager& memManager)
{
	return memManager.get(size);
}

static void* operator new(size_t size, m3d::MemoryManager& memManager, const char *file, int32 line) // Debug version
{
	return memManager.getDebug(size, file, line);
}

static void operator delete(void *mem, m3d::MemoryManager &memManager)
{
	return memManager.free(mem);
}

static void operator delete(void *mem, m3d::MemoryManager &memManager, const char *file, int32 line) // Debug version
{
	return memManager.freeDebug(mem);
}
*/

/*
template<typename T> void mmdeletea(T* p, MemoryManager &memManager = mm)
{
//	uint8 *m = (uint8*)p - sizeof(size_t);
//	while ((size_t&)*m > 0)
//		p[--(size_t&)*m].~T();
//	memManager.freeMemory(m);
//	operator delete[](p, mm);
}

static void* operator new[](size_t size, m3d::MemoryManager& memManager)
{
//	void *m = memManager.getMemory(size + sizeof(size_t));
//	*((size_t*)m) = size;
//	return (uint8*)m + sizeof(size_t);
	return memManager.getMemory(size); // the run-time adds sizeof(size_t) bytes of data to the front of the data returned. this contains size. Is this always a safe assumtion?
}
*/
//static void operator delete[](void *mem, m3d::MemoryManager &memManager)
//{
//	return memManager.freeMemory((uint8*)mem);// - sizeof(size_t));
//}
