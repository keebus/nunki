/*
 * Nunki (simple rendering engine)
 * Copyright (C) 2015 Canio Massimo Tristano <massimo.tristano@gmail.com>.
 * For licensing info see LICENSE.
 */

#include "nunki/root.h"
#include "nu_window.h"
#include "nu_device.h"
#include "nu_builtin_resources.h"
#include "nu_scene2d.h"
#include "nu_font.h"
#include "nu_base.h"
#include "nu_libs.h"

static struct {
	bool initialized;
	NuAllocator allocator;
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
	
	NuResult result;
	if (result = nInitThreadTempAllocator(allocator)) {
		nDebugError("Could not initialize thread temporary allocator.");
		return result;
	}

	gRoot.initialized = true;
	allocator = nGetDefaultOrAllocator(allocator);
	gRoot.allocator = *allocator;
	
	if (result = nInitWindowModule()) {
		nuTerminate();
		return result;
	}

	if (result = nInitDevice(allocator, nGetDummyWindowHandle())) {
		nuTerminate();
		return result;
	}

	if (result = nInitFontModule()) {
		nDebugError("Could not initialize font module.");
		nuTerminate();
		return result;
	}

	nInitBuiltinResources(allocator);

	if (result = nInitScene2D(allocator)) {
		nuTerminate();
		return result;
	}

	return NU_SUCCESS;
}

void nuTerminate(void)
{
	EnforceInitialized();
	nDeinitScene2D(&gRoot.allocator);
	nDeinitBuiltinResources(&gRoot.allocator);
	nDeinitFontModule();
	nDeinitDevice();
	nDeinitWindowModule();
	nDeinitThreadTempAllocator(&gRoot.allocator);
	gRoot.initialized = false;
}

NuResult nuInitThread(NuAllocator* allocator)
{
	return nInitThreadTempAllocator(allocator);
}

void nuDeinitThread(NuAllocator* allocator)
{
	nDeinitThreadTempAllocator(allocator);
}
