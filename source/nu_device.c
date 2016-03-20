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

static struct {
	bool initialized;
	NGlContextManager nglContextManager;
} gDevice;

NuResult nInitializeDeviceModule(void* dummyWindowHandle)
{
	nEnforce(!gDevice.initialized, "Device already initialized.");
	nInitGlContextManager(&gDevice.nglContextManager, dummyWindowHandle);
}

void nTerminateDeviceModule(void)
{
	nDeinitGlContextManager(&gDevice.nglContextManager);
}
