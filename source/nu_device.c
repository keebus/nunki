/*
 * Nunki (simple rendering engine)
 * Copyright (C) 2015 Canio Massimo Tristano <massimo.tristano@gmail.com>.
 * For licensing info see LICENSE.
 */

#include "nu_device.h"
#include "nu_base.h"
#include "thirdparty/gl3w.h"

#ifdef _WIN32
#include "nu_device_context_win32.inl"
#endif

typedef struct NuContextImpl {
	NGLContext nglContext;
} Context;

static struct {
	bool initialized;
	NGlContextManager nglContextManager;
} gDevice;

NuResult nInitializeDevice(void* dummyWindowHandle)
{
	nEnforce(!gDevice.initialized, "Device already initialized.");
	NuResult result = nInitGlContextManager(&gDevice.nglContextManager, dummyWindowHandle);
	if (result) result;



	return NU_SUCCESS;
}

void nTerminateDevice(void)
{
	if (!gDevice.initialized) return;
	nDeinitGlContextManager(&gDevice.nglContextManager);
	nZero(&gDevice);
}

NuResult nuCreateContext(NuContextCreateInfo const* info, NuContext* outContext, NuAllocator* allocator)
{
	*outContext = NULL;
	Context* context = nNew(Context, nGetAllocator(allocator));
	if (!context) {
		return NU_ERROR_OUT_OF_MEMORY;
	}

	NuResult result = nInitGlContext(&gDevice.nglContextManager, info->windowHandle, &context->nglContext);
	if (result) {
		nDebugError("Could not initialize OpenGL context for specified window.");
		return result;
	}

	*outContext = context;
	return NU_SUCCESS;
}

void nuDestroyContext(NuContext context, NuAllocator* allocator)
{

}