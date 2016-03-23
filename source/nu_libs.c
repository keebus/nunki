/*
 * Nunki (simple rendering engine)
 * Copyright (C) 2015 Canio Massimo Tristano <massimo.tristano@gmail.com>.
 * For licensing info see LICENSE.
 */

#include "nu_libs.h"
#include "nu_base.h"
#include <stdarg.h>
#include <stdio.h>

#ifdef _WIN32
#include <Windows.h>
#endif

NDebugDialogResult nDebugShowDialog(NDebugDialogType type, const char* title, const char* format, ...)
{
	va_list args;
	va_start(args, format);

	//MB_OKCANCEL

	char buffer[1024];
	vsnprintf(buffer, 1024, format, args);

	uint wtype = 0;
	switch (type) {
		case N_DEBUG_DIALOG_TYPE_OK: wtype = MB_OK; break;
		case N_DEBUG_DIALOG_TYPE_OK_IGNORE_ABORT_RETRY: wtype = MB_ABORTRETRYIGNORE; break;
	}

	int result = MessageBoxA(0, buffer, title, wtype);

	switch (result) {
		case IDOK: return N_DEBUG_DIALOG_RESULT_OK;
		//case IDCANCEL: return N_dialog_result_ok;
		case IDIGNORE: return N_DEBUG_DIALOG_RESULT_IGNORE;
		case IDABORT: return N_DEBUG_DIALOG_RESULT_ABORT;
		case IDRETRY: return N_DEBUG_DIALOG_RESULT_RETRY;
		default: return 0;
	}

#ifdef _MSC_VER
	OutputDebugStringA(buffer);
#else
	printf(buffer);
#endif

	va_end(args);
}

void nDebugPrint(const char* format, ...)
{
	va_list args;
	va_start(args, format);

#ifdef _MSC_VER
	char buffer[1024];
	vsnprintf(buffer, 1024, format, args);
	OutputDebugStringA(buffer);
#else
	vprintf(format, args);
#endif

	va_end(args);
}

/*-------------------------------------------------------------------------------------------------
 * array
 *-----------------------------------------------------------------------------------------------*/
typedef struct {
	uint capacity;
	uint length;
} ArrayHeader;

static inline ArrayHeader* getArrayHeader(void* array)
{
	return nAlignPtrDown((ArrayHeader*)array - 1, n_alignof(void*));
}
bool nArrayReserveEx(void** parray, NuAllocator* allocator, uint elementSize, uint capacity)
{
	ArrayHeader* header = getArrayHeader(*parray);

	/* reallocate if necessary */
	if (!*parray || header->capacity < capacity) {
		const uint old_capacity = *parray ? header->capacity : 10u;
		const uint oldLength = *parray ? header->length : 0;
		const uint newCapacity = max_uint(old_capacity * 2, capacity);

		/* compute the total size */
		uint totalSize = n_alignof(void*) + sizeof(ArrayHeader); // max alignment + size of header
		totalSize = (uint)nAlignUintUp(totalSize, n_alignof(void*)); // add padding for type alignment

		uint dataOffset = totalSize - n_alignof(void*);  // remember the offset in bytes from the allocation pointer to the actual data array

		totalSize += elementSize * newCapacity; // actual storage of values
		totalSize -= n_alignof(void*); // remove max alignment

		/* allocate the new array and copy the data over */
		ArrayHeader* newArrayHeader = nMalloc(totalSize, allocator);
		if (!newArrayHeader) return false;

		void* new_array = (char*)newArrayHeader + dataOffset;

		if (*parray) {
			memcpy(new_array, *parray, elementSize * oldLength);
			nFree(header, allocator);
		}

		header = newArrayHeader;
		header->capacity = newCapacity;
		header->length = oldLength;

		*parray = new_array;
	}

	return true;
}

void* nArrayPushEx(void** parray, NuAllocator* allocator, uint elementSize, uint count)
{
	uint old_length = nArrayLen(*parray);
	uint new_length = old_length + count;

	if (!nArrayReserveEx(parray, allocator, elementSize, new_length)) {
		return NULL;
	}

	getArrayHeader(*parray)->length = new_length;

	return ((char*)*parray) + old_length * elementSize;
}

void nArrayFree(void* array, NuAllocator* allocator)
{
	if (array) nFree(getArrayHeader(array), allocator);
}

void nArrayClear(void* array)
{
	ArrayHeader* header = getArrayHeader(array);
	if (array) header->length = 0;
}

uint nArrayLen(void* array)
{
	return array ? getArrayHeader(array)->length : 0;
}

bool nArrayAlignUp(void** parray, NuAllocator* allocator, uint alignment)
{
	if (!*parray) {
		if (!nArrayReserveEx(parray, allocator, 1, 1)) {
			return false;
		}
	}
	ArrayHeader* header = getArrayHeader(*parray);
	char* back = (char*)*parray + header->length;
	uint64_t offset = (char*)nAlignPtrUp(back, alignment) - back;
	return nArrayPushEx(parray, allocator, 1, (uint)offset) != NULL;
}
