/*
 * Nunki (simple rendering engine)
 * Copyright (C) 2015 Canio Massimo Tristano <massimo.tristano@gmail.com>.
 * For licensing info see LICENSE.
 */

#pragma once

#include <stdbool.h>
#include <stdint.h>

#define NUNKI_VERSION_MAJOR 0
#define NUNKI_VERSION_MINOR 1
#define NUNKI_VERSION_PATCH 0

#ifdef _MSC_VER
	#ifdef NUNKI_EXPORT
		#define NUNKI_API __declspec(dllexport)
	#else
		#define NUNKI_API __declspec(dllimport)
	#endif
#else
	#error Implement this.
#endif

typedef unsigned int uint;

#define NU_HANDLE(name) typedef struct name##Impl* name;
#define NU_HANDLE_INDEX(name) typedef size_t name;

typedef enum NuResult {
	NU_SUCCESS,
	NU_FAILURE,
	NU_ERROR_OUT_OF_MEMORY,
	NU_ERROR_INCOMPATIBLE_LIBRARY_VERSION
} NuResult;

typedef struct NuAllocator {
	void*   userData;
	void* (*malloc)(size_t size, size_t alignment, void* userData);
	void* (*realloc)(void* ptr, size_t newSize, size_t alignment, void* userData);
	void  (*free)(void* ptr, void* userData);
} NuAllocator;

NU_HANDLE(NuTempAllocator);

/**
 * Write the #documentation.
 */
NUNKI_API NuTempAllocator nResetTempAlloc(void);

/**
 * Write the #documentation.
 */
NUNKI_API size_t nuFileLoader(void* buffer, size_t bufferSize, void* faceUserData, void* loaderUserData);

typedef struct {
	int x, y;
} NuPoint2i;

typedef struct {
	float x, y;
} NuPoint2;

typedef struct {
	uint width, height;
} NuSize2i;

typedef struct {
	float width, height;
} NuSize2;

typedef struct{
	uint width, height, depth;
} NuSize3i;

typedef struct {
	float width, height, depth;
} NuSize3;

typedef struct {
	NuPoint2i position;
	NuSize2i  size;
} NuRect2i;

typedef struct {
	NuPoint2 position;
	NuSize2  size;
} NuRect2;

inline NuRect2 nuRect2IntToFloat(NuRect2i r)
{
	return (NuRect2) { (float)r.position.x, (float)r.position.y, (float)r.size.width, (float)r.size.height };
}

inline uint nuRGBA(uint red, uint green, uint blue, uint alpha)
{
	return (alpha & 0xff) << 24 | (blue & 0xff) << 16 | (green & 0xff) << 8 | (red & 0xff);
}