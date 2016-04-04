/*
 * Nunki (simple rendering engine)
 * Copyright (C) 2015 Canio Massimo Tristano <massimo.tristano@gmail.com>.
 * For licensing info see LICENSE.
 */

#pragma once

#include "./base.h"

NU_HANDLE(NuTexture);
NU_HANDLE(NuFont);

typedef struct {
	uint fromCodePoint;
	uint toCodePoint;
} NuCharSet;

typedef struct {
	void*            userData;
	uint             size;
	NuCharSet const* charSets;
	uint             numCharSets;
} NuFaceInfo;

typedef size_t (*NuFaceLoader) (void* buffer, size_t bufferSize, void* faceUserData, void* loaderUserData);

typedef struct {
	uint resolution;
	uint textureWidth;
	uint textureHeight;
	NuFaceLoader faceLoader;
	void* faceLoaderUserData;
	uint numFaces;
	NuFaceInfo const* faces; /* null or null-terminated */
} NuFontCreateInfo;

typedef struct {
	/** The vertical distance from the horizontal baseline to the highest ‘character’ coordinate. */
	int ascender;

	/** The vertical distance from the horizontal baseline to the lowest ‘character’ coordinate. */
	int descender;

	/** Height of line of text (distance between two baselines).*/
	int lineHeight;
} NuFaceMeasures;

typedef struct
{
	NuPoint2i offset;
	NuSize2i  size;
	int       advance;
	NuRect2   textureRect;
} NuFaceGlyph;

typedef struct
{
	uint faceId;
	uint color;
} NuTextStyle;

typedef void (*NuFontProcessCharCallback)(NuTextStyle const* style, NuRect2i bounds, NuRect2 textureBounds, void* userdata);

/**
 * Write the #documentation.
 */
NUNKI_API NuResult nuCreateFont(NuFontCreateInfo const* info, NuAllocator* allocator, NuTempAllocator tempAllocator, NuFont* pFont);

/**
 * Write the #documentation.
 */
NUNKI_API void nuDestroyFont(NuFont fontSet, NuAllocator* allocator);

/**
 * Write the #documentation.
 */
NUNKI_API NuTexture nuFontGetTexture(NuFont fontSet);

/**
 * Write the #documentation.
 */
NUNKI_API NuSize2i nuFontProcessText(NuFont fontSet, const char* text, NuTextStyle const* styles, uint initialStyleId, NuFontProcessCharCallback callback, void* userdata);
