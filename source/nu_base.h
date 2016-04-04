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

static inline void* n_newEx(NuAllocator* allocator, size_t size, size_t alignment)
{
	void* ptr = allocator->malloc(size, alignment, allocator->userData);
	memset(ptr, 0, size);
	return ptr;
}

#define n_malloc(size, pallocator) (pallocator->malloc(size, n_alignof(void*), pallocator->userData))
#define n_new(type, pallocator) ((type*)n_newEx(pallocator, sizeof(type), n_alignof(type)))
#define n_newarray(type, length, pallocator) ((type*)n_newEx(pallocator, sizeof(type) * length, n_alignof(type)))
#define n_realloc(ptr, size, pallocator) (pallocator->realloc(ptr, size, n_alignof(void*), pallocator->userData))
#define n_free(ptr, pallocator) (pallocator->free(ptr, pallocator->userData))

/**
 * Write the #documentation.
 */
NuResult nInitThreadTempAllocator(NuAllocator* allocator);

/**
 * Write the #documentation.
 */
void nDeinitThreadTempAllocator(NuAllocator* allocator);

/**
 * Write the #documentation.
 */
inline NuAllocator* nAllocatorFromTemp(NuTempAllocator tempAllocator) { return (NuAllocator*)tempAllocator; }

/**
 * Write the #documentation.
 */
void nTempAllocatorLockMalloc(void);

/**
 * Write the #documentation.
 */
void nTempAllocatorUnlockMalloc(void);