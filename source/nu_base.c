/*
 * Nunki (simple rendering engine)
 * Copyright (C) 2015 Canio Massimo Tristano <massimo.tristano@gmail.com>.
 * For licensing info see LICENSE.
 */

#include "nunki/base.h"
#include <malloc.h>

static void* DefaultMalloc(size_t size, size_t alignment, void* userData)
{
	return _aligned_malloc(size, alignment);
}

static void* DefaultRealloc(void* ptr, size_t alignment, size_t newSize, void* userData)
{
	return _aligned_realloc(ptr, newSize, alignment);
}

static void DefaultFree(void* ptr, void* userData)
{
	_aligned_free(ptr);
}

const NuAllocator nDefaultAllocator = {
	NULL,
	DefaultMalloc,
	DefaultRealloc,
	DefaultFree,
};

