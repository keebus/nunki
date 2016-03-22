/*
 * Nunki (simple rendering engine)
 * Copyright (C) 2015 Canio Massimo Tristano <massimo.tristano@gmail.com>.
 * For licensing info see LICENSE.
 */

#pragma once

#include "nu_base.h"
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

typedef unsigned uint;

 /*-------------------------------------------------------------------------------------------------
 * preprocessor
 *-----------------------------------------------------------------------------------------------*/
#ifdef _MSC_VER
	#define n_threadlocal __declspec(thread)
	#define n_forceinline __forceinline
	#define n_alignof(type) _Alignof(type)
#else
	#error implement this
#endif

#define nStringifyX(x) #x
#define nStringify(x) nStringifyX(x)

/*-------------------------------------------------------------------------------------------------
 * debugging
 *-----------------------------------------------------------------------------------------------*/

typedef enum {
	N_DEBUG_DIALOG_RESULT_OK,
	N_DEBUG_DIALOG_RESULT_IGNORE,
	N_DEBUG_DIALOG_RESULT_ABORT,
	N_DEBUG_DIALOG_RESULT_RETRY,
} NDebugDialogResult;

typedef enum
{
	N_DEBUG_DIALOG_TYPE_OK,
	N_DEBUG_DIALOG_TYPE_OK_IGNORE_ABORT_RETRY,
} NDebugDialogType;

#ifdef _DEBUG

#ifdef _MSC_VER
	#define nDebugBreak() __debugbreak()
#else
	#define nDebugBreak()
#endif

#define nDebugError(format, ...) {\
	static bool ___n_ignore = false;\
	if (___n_ignore) {\
		nDebugPrint("[error] " format "\n", __VA_ARGS__);\
	}\
	else {\
		NDebugDialogResult ___n_result = nDebugShowDialog(N_DEBUG_DIALOG_TYPE_OK_IGNORE_ABORT_RETRY, "Debug Error", "[error] " format, __VA_ARGS__); \
		if (___n_result == N_DEBUG_DIALOG_RESULT_IGNORE) ___n_ignore = true; \
		else if (___n_result == N_DEBUG_DIALOG_RESULT_RETRY) nDebugBreak(); \
		else exit(-1);\
	}\
}

#define nDebugWarning(format, ...)	nDebugPrint("[warning] " format "\n", __VA_ARGS__)

#define nDebugInfo(format, ...) 	nDebugPrint("[info] " format "\n", __VA_ARGS__)

#define nAssert(condition) nEnforce(condition, "Assertion failure:\n\t" # condition)

#define nEnforce(condition, message, ...) \
	if (!(condition)) {\
		static bool ___n_ignore = false;\
		if (!___n_ignore) {\
			NDebugDialogResult ___n_result = nDebugShowDialog(N_DEBUG_DIALOG_TYPE_OK_IGNORE_ABORT_RETRY, "Error", message, __VA_ARGS__); \
			if (___n_result == N_DEBUG_DIALOG_RESULT_IGNORE) ___n_ignore = true; \
			else if (___n_result == N_DEBUG_DIALOG_RESULT_RETRY) nDebugBreak(); \
			else exit(-1);\
		}\
	}

#else

#define nDebugBreak()
#define nDebugError(format, ...)
#define nDebugWarning(format, ...)
#define nDebugInfo(format, ...)
#define nAssert(condition)

#define nEnforce(condition, message, ...) \
	if (!(condition)) {\
		nDebugShowDialog(N_DEBUG_DIALOG_TYPE_OK, "Fatal error", message ## "\nThe application will now close.", __VA_ARGS__); \
		exit(-1);\
	}

#endif

NDebugDialogResult  nDebugShowDialog(NDebugDialogType, const char* title, const char* format, ...);
void                nDebugPrint(const char* format, ...);

/*-------------------------------------------------------------------------------------------------
 * Fundamental helper functions
 *-----------------------------------------------------------------------------------------------*/
/**
 * \returns the smallest address after \p ptr aligned to \p alignment.
 * \example align(10, 4) would return 12.
 */
n_forceinline uintptr_t nAlignUintUp(uintptr_t ptr, uint alignment)
{
	const uintptr_t alignment_mins_one = alignment - 1;
	return (ptr + alignment_mins_one) & ~alignment_mins_one;
}

/**
 * \returns the smallest address after \p ptr aligned to \p alignment.
 * \example align(10, 4) would return 12.
 */
n_forceinline void* nAlignPtrUp(void* ptr, uint alignment)
{
	return (void*)(nAlignUintUp((uintptr_t)ptr, alignment));
}

n_forceinline uintptr_t nAlignUintDown(uintptr_t ptr, uint alignment)
{
	return ptr & -(int)alignment;
}

n_forceinline void* nAlignPtrDown(void* ptr, uint alignment)
{
	return (void*)(nAlignUintDown((uintptr_t)ptr, alignment));
}

#define nunki_define_min_max(type)\
	static n_forceinline type max_##type(type a, type b) { return a > b ? a : b; }\
	static n_forceinline type min_##type(type a, type b) { return a < b ? a : b; }\

nunki_define_min_max(int)
nunki_define_min_max(uint)
nunki_define_min_max(size_t)
nunki_define_min_max(float)
nunki_define_min_max(double)

#undef nunki_define_min_max

#define nZero(structPtr) memset(structPtr, 0, sizeof *structPtr)

inline uint nSwizzleUInt(uint ui)
{
	return (ui >> 24) | ((ui >> 8) & 0x0000ff00) | ((ui << 8) & 0x00ff0000) | (ui << 24);
}

/*-------------------------------------------------------------------------------------------------
 * array
 *-----------------------------------------------------------------------------------------------*/
void  nArrayReserveEx(void** parray, NuAllocator* allocator, uint elementSize, uint capacity);
void* nArrayPushEx(void** parray, NuAllocator* allocator, uint elementSize, uint count);
void  nArrayFree(void* array, NuAllocator* allocator);
void  nArrayClear(void* array);
uint  nArrayLen(void* array);

#define nArrayReserve(parray, allocator, type, capacity) nArrayReserveEx(parray, allocator, sizeof(type), capacity)
#define nArrayPush(parray, allocator, type) (type*)nArrayPushEx(parray, allocator, sizeof(type), 1)
#define nArrayPushN(parray, allocator, type, N) (type*)nArrayPushEx(parray, allocator, sizeof(type), N)

/**
 * Write the #documentation.
 */
void nArrayAlignUp(void** parray, NuAllocator* allocator, uint alignment);
