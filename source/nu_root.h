/*
 * Nunki (simple rendering engine)
 * Copyright (C) 2015 Canio Massimo Tristano <massimo.tristano@gmail.com>.
 * For licensing info see LICENSE.
 */

#include "nunki/root.h"
#include "nu_window.h"
#include "nu_device.h"
#include "nu_builtin_resources.h"
#include "nu_base.h"
#include "nu_libs.h"

static struct {
	bool initialized;
	NuAllocator allocator;
	NBuiltinResources builtins;
} gRoot;

#define EnforceInitialized() nEnforce(gRoot.initialized, "Nunki uninitialized.");

uint yuGetVersionMajor(void) { return NUNKI_VERSION_MAJOR; }
uint yuGetVersionMinor(void) { return NUNKI_VERSION_MINOR; }
uint yuGetVersionPatch(void) { return NUNKI_VERSION_PATCH; }

const char* yuGetVersionString(void)
{
	return nStringify(NUNKI_VERSION_MAJOR) "." nStringify(NUNKI_VERSION_MINOR) "." nStringify(NUNKI_VERSION_PATCH);
}

NuResult nuInitialize(NuInitializeInfo const* info, NuAllocator* allocator)
{
	nEnforce(!gRoot.initialized, "Nunki already initialized.");
	
	gRoot.initialized = true;
	allocator = nGetDefaultOrAllocator(allocator);
	gRoot.allocator = *allocator;

	NuResult result;

	if (result = nInitWindowModule()) {
		nuTerminate();
		return result;
	}

	if (result = nInitDevice(allocator, nGetDummyWindowHandle())) {
		nuTerminate();
		return result;
	}

	nInitBuiltinResources(&gRoot.builtins, allocator);

	return NU_SUCCESS;
}

void nuTerminate(void)
{
	EnforceInitialized();
	nDeinitBuiltinResources(&gRoot.builtins, &gRoot.allocator);
	nDeinitDevice();
	nDeinitWindowModule();
	gRoot.initialized = false;
}
