/*
 * Nunki (simple rendering engine)
 * Copyright (C) 2015 Canio Massimo Tristano <massimo.tristano@gmail.com>.
 * For licensing info see LICENSE.
 */

#pragma once

#include "./base.h"

NU_HANDLE(NuImage);

typedef enum {
	NU_IMAGE_FORMAT_RGBA8,
	NU_IMAGE_FORMAT_COUNT_,
} NuImageFormat;

typedef struct {
	NuImageFormat format;
	NuSize2i      size;
	const void*   data;
} NuImageView;

typedef struct {
	NuImageFormat format;
	NuSize2i      size;
	uint          initializeMemory : 1;
} NuImageCreateInfo;

/**
 * Write the #documentation.
 */
NUNKI_API NuResult nuCreateImage(NuImageCreateInfo const* info, NuAllocator* allocator, NuImage* pImage);

/**
 * Write the #documentation.
 */
NUNKI_API void nuDestroyImage(NuImage image, NuAllocator* allocator);

/**
 * Write the #documentation.
 */
NUNKI_API NuImageView nuImageGetView(NuImage const image);

/**
 * Write the #documentation.
 */
NUNKI_API void* nuImageGetWritableDataPtr(NuImage image);
