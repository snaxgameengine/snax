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
#include "MemoryManager.h"
#include <set>
#include <cassert>


using namespace m3d;

namespace m3d
{
	static const int32 DEBUG_EXTRA = 2 * MemoryManager::ALIGNMENT;

	struct MemStat
	{

		std::set<void*> allocated;
	};
}

MemStack::MemStack() : _stack(0), _stackSize(0), _size(0), _capacity(0), _fragmentSize(0), _allocFailed(false)
{
}

MemStack::~MemStack()
{
	assert(_size == _capacity);
	if (_stack)
		delete[] _stack;
}

uint8 *MemStack::pop()
{
	assert(_size != 0);
	return _stack[--_size];
}

void MemStack::push(uint8 *mem)
{
	assert(_size < _capacity);
	_stack[_size++] = mem;
}

bool MemStack::pushNew(uint8 *mem)
{
	if (_stackSize == _capacity) {
		uint8 **s = _stack;
		uint32 newStackSize = _stackSize == 0 ? 1024 : (_stackSize * 2);
		uint8 **newStack = new uint8*[newStackSize];
		if (!newStack)
			return false;
		if (_capacity) {
			std::memcpy(newStack, _stack, sizeof(uint8*) * _capacity);
			delete[] _stack;
		}
		_stack = newStack;
		_stackSize = newStackSize;
	}
	_capacity++;
	push(mem);
	return true;
}


MemBlock::MemBlock() : _mem(0), _size(0), _free(0) 
{
}

MemBlock::~MemBlock()
{
	release();
}

bool MemBlock::init(size_t size)
{
	assert(_mem == 0);
	_mem = (uint8*)_aligned_offset_malloc(size, MemoryManager::ALIGNMENT, 1); // guaranties ALIGNMENT-bit alignment of _mem+1!
	if (!_mem)
		return false;
	_size = _free = size;
	return true;
}

void MemBlock::release()
{
	if (_mem)
		::_aligned_free(_mem);
	_size = 0;
	_free = 0;
}

uint32 MemBlock::fill(MemStack &ms, uint32 maxCount, uint8 stackID)
{
	uint32 i = 0;
	for (; i < maxCount; i++) {
		if (_free < (ms.getFragmentSize() + 1)) // + 1 because of the extra byte needed for meta data!
			break; // out of memory!
		uint8 *m = _mem + (_size - _free);
		m[0] = stackID; // stack id! (This is the extra byte meta data!)
		if (ms.pushNew(m)) 
			_free -= ((ms.getFragmentSize() + MemoryManager::ALIGNMENT) / MemoryManager::ALIGNMENT) * MemoryManager::ALIGNMENT; // This formula ensures alignment for every fragment!
	}
	return i;
}

MemoryManager::MemoryManager(size_t blockSize) : _blockSize(blockSize), _stats(new MemStat())
{
	assert(STACK_COUNT < 256);
	// This formula may be improved? now doing a linear distribution
	// Make sure MAX_MEMORY/STACK_COUNT==ALIGNMENT. This ensures max utilization (no waste!) of ALIGNMENT-bit aligned memory!
	for (uint32 i = 0, j = 0; i < STACK_COUNT; i++) {
		_stack[i].setFragmentSize(MAX_MEMORY * (i + 1) / STACK_COUNT - 1); // - 1 becuase of the extra byte needed for meta data!
		for (; j < _stack[i].getFragmentSize(); j++)
			_stackMap[j] = i;
	}
}

MemoryManager::~MemoryManager() 
{
	if (!_stats->allocated.empty()) { // Memory leaks?
#if defined( DEBUG ) || defined( _DEBUG ) // TODO: Print info in release?
		OutputDebugStringA("MemoryManager: Leaks detected!\n");
		char c[256];
		for (std::set<void*>::const_iterator itr = _stats->allocated.begin(); itr != _stats->allocated.end(); itr++) {
			char *v = (char*)*itr;
			OutputDebugStringA("StackNr ");
			int32 s = *((uint8*)v - 1);
			_itoa_s(s, c, 256, 10);
			OutputDebugStringA(c);
			size_t size = (size_t)*v;
			int16 ln = (int16)*(v + sizeof(size_t));
			OutputDebugStringA(": ");
			_ui64toa_s(size, c, 256, 10);
			OutputDebugStringA(c);
			OutputDebugStringA(" bytes in file ");
			OutputDebugStringA(v + sizeof(size_t) + sizeof(int16));
			OutputDebugStringA(" (");
			_itoa_s((int32)ln, c, 256, 10);
			OutputDebugStringA(c);
			OutputDebugStringA("): ");
			v += DEBUG_EXTRA;
			std::memcpy(c, v, std::min(size, 32ull));
			c[32] = '\0';
			for (uint32 i = 0; i < 32; i++)
				c[i] = std::max(c[i], (char)32);
			OutputDebugStringA(c);
			OutputDebugStringA("\n");
		}
#endif
	}
	delete _stats;
}

void *MemoryManager::_getCustomMemory(size_t size)
{
	if (size == 0)
		return 0;
	uint8 *m = (uint8*)_aligned_offset_malloc(size + 1, ALIGNMENT, 1); // guaranties ALIGNMENT-bit alignment of returned memory from this function!
	if (!m)
		return 0;
	m[0] = 255;
	return m + 1;
}



void *MemoryManager::get(size_t size)
{
	return ::malloc(size); // MemoryManager disabled!

	if (size == 0 || size > (MAX_MEMORY - 1))
		return _getCustomMemory(size);

	uint8 stackNr = _stackMap[size - 1];

	assert(stackNr < STACK_COUNT);

	MemStack &stack = _stack[stackNr];

	volatile CriticalBlock critical(_cs);

	if (stack.getSize() == 0) { // Stack is empty?
		if (stack.getAllocFailed()) // Previous alloc failed?
			return _getCustomMemory(size);
		for (uint32 i = 0; i < 256; i++)
			if (_block[i].getFree() >= (stack.getFragmentSize() + 2))
				_block[i].fill(stack, stack.getCapacity() == 0 ? 64 : stack.getCapacity(), stackNr); // try to double capacity!
		if (stack.getSize() == 0) { // Didn't find any free block mem?
			for (uint32 i = 0; i < 256; i++)
				if (_block[i].getSize() == 0) // Empty block?
					if (_block[i].init(_blockSize)) { // allocate block mem!
						_block[i].fill(stack, stack.getCapacity() == 0 ? 64 : stack.getCapacity(), stackNr); // try to double capacity!
						break;
					}
		}
		if (stack.getSize() == 0) { // Could not create any new block mem?
			stack.setAllocFailed(true);
			return _getCustomMemory(size);
		}
	}

	uint8 *m = stack.pop();
	assert(((size_t)(m + 1) & (ALIGNMENT - 1)) == 0);

	return m + 1;
}

void MemoryManager::free(void *mem)
{
	return ::free(mem); // MemoryManager disabled!

	if (mem) {
		uint8 *m = (uint8 *)mem - 1;
		if (*m == 255) {
			::_aligned_free(m);
			return;
		}
		volatile CriticalBlock critical(_cs);

		assert(_stack[*m].getSize() < _stack[*m].getCapacity());
		
		_stack[*m].push(m);

		// when do we free exessive block mem?
	}
}

void *MemoryManager::realloc(void *mem, size_t size)
{
	return ::realloc(mem, size); // MemoryManager disabled!

	if (mem) {
		uint8 *m = (uint8 *)mem - 1;
		if (*m == 255) {
			mem = (uint8*)::_aligned_offset_realloc(m, size + 1, ALIGNMENT, 1) + 1;
		}
		else {
			size_t s = _stack[*m].getFragmentSize();
			if (s < size) {
				void *n = get(size);
				std::memcpy(n, mem, s);
				free(mem);
				mem = n;
			}
		}
	}
	else
		mem = get(size);
	return mem;
}


void *MemoryManager::getDebug(size_t size, const char *file, int32 line)
{
	return ::malloc(size); // MemoryManager disabled!

	if (size == 0)
		return nullptr;
	char *m = (char*)get(size + DEBUG_EXTRA);
	*((size_t*)m) = size;
	*((int16*)(m + sizeof(size_t))) = (int16)line;
	size_t e = sizeof(size_t) + sizeof(int16);
	int32 n = int32(strlen(file) + 1) - int32(DEBUG_EXTRA - e);
	strcpy_s(m + e, DEBUG_EXTRA - e, file + std::max(0, n));

	volatile CriticalBlock critical(_cs);
	_stats->allocated.insert(m);

	m += DEBUG_EXTRA;
	assert(((size_t)m & (ALIGNMENT - 1)) == 0);
	return m;
}

void MemoryManager::freeDebug(void *mem)
{
	return ::free(mem); // MemoryManager disabled!

	if (!mem)
		return;
	char *c = (char*)mem - DEBUG_EXTRA;

	volatile CriticalBlock critical(_cs);
	_stats->allocated.erase(c);

	free(c);
}
	
void *MemoryManager::reallocDebug(void *mem, size_t size, const char *file, int32 line)
{
	return ::realloc(mem, size); // MemoryManager disabled!
	/*if (mem) {
		uint8 *mr = (uint8*)mem - DEBUG_EXTRA;
		uint8 *m = (uint8 *)mr - 1;
		if (*m == 255) {
			//mem = (uint8*)::_aligned_offset_realloc(m, size + 1, ALIGNMENT, 1) + 1;
		}
		else {
			size_t s = _stack[*m].getFragmentSize();
			if (s < size) {
				void *n = get(size);
				std::memcpy(n, mem, s);
				free(mem);
				mem = n;
			}
		}
	}
	else
		mem = getDebug(size, file, line);
	*/return mem;
}

MemoryManager &m3d::mm()
{
	static MemoryManager manager;
	return manager; 
}
