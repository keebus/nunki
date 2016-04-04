/*
 * Nunki (simple rendering engine)
 * Copyright (C) 2015 Canio Massimo Tristano <massimo.tristano@gmail.com>.
 * For licensing info see LICENSE.
 */

#pragma once

#include "./base.h"

/**
 * Write the #documentation.
 */
typedef struct {
	uint versionMajor;
	uint versionMinor;
	uint versionPatch;
} NuInitializeInfo;

/**
* Write the #documentation.
*/
NUNKI_API uint nuGetVersionMajor(void);

/**
* Write the #documentation.
*/
NUNKI_API uint nuGetVersionMinor(void);

/**
* Write the #documentation.
*/
NUNKI_API uint nyuGetVersionPatch(void);
 
/**
* Write the #documentation.
*/
NUNKI_API const char* nuGetVersionString(void);

/**
 * Write the #documentation.
 */
NUNKI_API NuResult nuInitialize(NuInitializeInfo const* info, NuAllocator* allocator);

/**
 * Write the #documentation.
 */
NUNKI_API void nuTerminate(void);

/**
 * Write the #documentation.
 */
NUNKI_API NuResult nuInitThread(NuAllocator* allocator);

/**
 * Write the #documentation.
 */
NUNKI_API void nuDeinitThread(NuAllocator* allocator);
