/*
 * Nunki (simple rendering engine)
 * Copyright (C) 2015 Canio Massimo Tristano <massimo.tristano@gmail.com>.
 * For licensing info see LICENSE.
 */

#include "nunki/root.h"
#include "nu_window.h"
#include "nu_device.h"
#include "nu_base.h"

static struct {
	bool initialized;
	NuAllocator allocator;
} gInstance;

#define EnforceInitialized() nEnforce(gInstance.initialized, "Nunki uninitialized.");

uint yuGetVersionMajor(void) { return NUNKI_VERSION_MAJOR; }
uint yuGetVersionMinor(void) { return NUNKI_VERSION_MINOR; }
uint yuGetVersionPatch(void) { return NUNKI_VERSION_PATCH; }

const char* yuGetVersionString(void)
{
	return nStringify(NUNKI_VERSION_MAJOR) "." nStringify(NUNKI_VERSION_MINOR) "." nStringify(NUNKI_VERSION_PATCH);
}

NuResult nuInitialize(NuInitializeInfo const* info, NuAllocator* allocator)
{
	nEnforce(!gInstance.initialized, "Nunki already initialized.");
	gInstance.initialized = true;
	NuResult result;

	if (result = nInitializeWindowModule()) {
		nuTerminate();
		return result;
	}

	if (result = nInitializeDevice(nGetDummyWindowHandle())) {
		nuTerminate();
		return result;
	}

	return NU_SUCCESS;
}

void nuTerminate(void)
{
	EnforceInitialized();
	nTerminateDevice();
	nTerminateWindowModule();
	gInstance.initialized = false;
}
