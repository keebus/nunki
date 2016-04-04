/*
 * Nunki (simple rendering engine)
 * Copyright (C) 2015 Canio Massimo Tristano <massimo.tristano@gmail.com>.
 * For licensing info see LICENSE.
 */

#include "nunki/base.h"
#include "nu_libs.h"
#include <malloc.h>
#include <stdio.h>

typedef struct
{
	NuAllocator allocator;
	char* cursor;
	char* buffer;
	char* bufferEnd;
	char* lastAllocationPtr;
	size_t lastAllocationSize;
#ifdef DEBUG
	uint  mallocLocked : 1;
#endif
} TempAllocator;

static n_threadlocal TempAllocator gTempAllocator;

static void* DefaultMalloc(size_t size, size_t alignment, void* userData)
{
	return _aligned_malloc(size, alignment);
}

static void* DefaultRealloc(void* ptr, size_t newSize, size_t alignment, void* userData)
{
	return _aligned_realloc(ptr, newSize, alignment);
}

static void DefaultFree(void* ptr, void* userData)
{
	_aligned_free(ptr);
}

static void* TempMalloc(size_t size, size_t alignment, void* userData)
{
	TempAllocator* tempAllocator = userData;
	nAssert (!tempAllocator->mallocLocked);

	char* cursor = nAlignPtrUp(tempAllocator->cursor, (uint)alignment);
	char* allocationPtr = cursor;
	cursor += size;
	if (cursor >= tempAllocator->bufferEnd) {
		nDebugError("Exhausted temporary allocator. This is critical, please report to the developers.");
		return NULL;
	}
	tempAllocator->cursor = cursor;
	tempAllocator->lastAllocationPtr = allocationPtr;
	tempAllocator->lastAllocationSize = size;
	return allocationPtr;
}

static void* TempRealloc(void* ptr, size_t newSize, size_t alignment, void* userData)
{
	TempAllocator* tempAllocator = userData;
	char* allocationPtr = tempAllocator->lastAllocationPtr;
	if (ptr == allocationPtr) {
		nAssert((uintptr_t)allocationPtr % alignment == 0);
		tempAllocator->cursor = allocationPtr + newSize;
		if (tempAllocator->cursor >= tempAllocator->bufferEnd) {
			nDebugError("Exhausted temporary allocator. This is critical, please report to the developers.");
			return NULL;
		}
	}
	else {
		allocationPtr = TempMalloc(newSize, alignment, userData);
	}
	return allocationPtr;
}

static void TempFree(void* ptr, void* userData)
{
	TempAllocator* tempAllocator = userData;
	if (ptr && ptr == tempAllocator->lastAllocationPtr) {
		tempAllocator->cursor = (char*)ptr;
		tempAllocator->lastAllocationPtr = NULL;
	}
}

const NuAllocator nDefaultAllocator = {
	NULL,
	DefaultMalloc,
	DefaultRealloc,
	DefaultFree,
};

NuTempAllocator nResetTempAlloc(void)
{
	nEnforce(gTempAllocator.buffer, "Uninitialized temp allocator, please use nuInitThread() if this is a separate thread from which Nunki was initialized on.");
	gTempAllocator.cursor = gTempAllocator.buffer;
	return (NuTempAllocator)&gTempAllocator.allocator;
}

NuResult nInitThreadTempAllocator(NuAllocator* allocator)
{
	if (gTempAllocator.buffer) {
		nDebugWarning("Nunki temporary allocator already initialized on this thread.");
		return NU_SUCCESS;
	}

	size_t capacity = 1024 * 1024 * 10; // 10Mb
	gTempAllocator.buffer = n_malloc(capacity, nGetDefaultOrAllocator(allocator));
	gTempAllocator.bufferEnd = gTempAllocator.buffer + capacity;
	gTempAllocator.cursor = gTempAllocator.buffer;

	gTempAllocator.allocator = (NuAllocator) {
		&gTempAllocator,
		TempMalloc,
		TempRealloc,
		TempFree,
	};

	return gTempAllocator.buffer ? NU_SUCCESS : NU_ERROR_OUT_OF_MEMORY;
}

void nDeinitThreadTempAllocator(NuAllocator* allocator)
{
	if (!gTempAllocator.buffer) {
		nDebugWarning("Nunki temporary allocator already deinitialized on this thread.");
		return;
	}
	n_free(gTempAllocator.buffer, nGetDefaultOrAllocator(allocator));
	nZero(&gTempAllocator);
}

void nTempAllocatorLockMalloc(void)
{
#ifdef DEBUG
	gTempAllocator.mallocLocked = true;
#endif
}

void nTempAllocatorUnlockMalloc(void)
{
#ifdef DEBUG
	gTempAllocator.mallocLocked = false;
#endif
}

size_t nuFileLoader(void* buffer, size_t bufferSize, void* faceUserData, void* loaderUserData)
{
	FILE* file = 0;
	fopen_s(&file, (const char*)faceUserData, "rb");
	if (!file) {
		return 0;
	}

	fseek(file, 0, SEEK_END);
	size_t fileSize = ftell(file);

	if (fileSize > bufferSize) {
		fclose(file);
		return fileSize;
	}

	fseek(file, 0, SEEK_SET);
	fread(buffer, 1, fileSize, file);

	fclose(file);
	return fileSize;
}
