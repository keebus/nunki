/*
 * Nunki (simple rendering engine)
 * Copyright (C) 2015 Canio Massimo Tristano <massimo.tristano@gmail.com>.
 * For licensing info see LICENSE.
 */

#pragma once

#include "nunki/base.h"
#include <string.h>

extern const NuAllocator nDefaultAllocator;

inline NuAllocator* nGetDefaultOrAllocator(NuAllocator* allocator)
{
	return allocator ? allocator : &nDefaultAllocator;
}

static inline void* nNewEx(NuAllocator* allocator, size_t size, size_t alignment)
{
	void* ptr = allocator->malloc(size, alignment, allocator->userData);
	memset(ptr, 0, size);
	return ptr;
}

#define nMalloc(size, pallocator) (pallocator->malloc(size, n_alignof(void*), pallocator->userData))
#define nNew(type, pallocator) ((type*)nNewEx(pallocator, sizeof(type), n_alignof(type)))
#define nNewArray(type, length, pallocator) ((type*)nNewEx(pallocator, sizeof(type) * length, n_alignof(type)))
#define nRealloc(ptr, size, pallocator) (pallocator->realloc(ptr, size, n_alignof(void*), pallocator->userData))
#define nFree(ptr, pallocator) (pallocator->free(ptr, pallocator->userData))
