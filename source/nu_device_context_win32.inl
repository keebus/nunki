/*
 * Nunki (simple rendering engine)
 * Copyright (C) 2015 Canio Massimo Tristano <massimo.tristano@gmail.com>.
 * For licensing info see LICENSE.
 */

#include "thirdparty/gl3w.h"
#include "thirdparty/wglext.h"
#include <string.h>

typedef struct {
	bool initialized;
	HWND dummyWindowHandle;
	HDC hDC;
	HGLRC context;
	int maxglmajor;
	int maxglminor;
	PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB_proc;
	PFNWGLGETPIXELFORMATATTRIBIVARBPROC wglGetPixelFormatAttribivARB_proc;
	PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB_proc;
	PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT_proc;
	NuContext activeContext;
} NGlContextManager;

typedef struct {
	HDC hdc;
	HGLRC hglrc;
	HWND hwnd;
} NGLContext;

void nDeinitGlContextManager(NGlContextManager* glCM);
void nDeinitGlContext(NGlContextManager* glCM, NGLContext* context);

NuResult nInitGlContextManager(NGlContextManager* glCM, void* dummyWindowHandle)
{
	nEnforce(!glCM->initialized, "Context already initialized.");

	glCM->initialized = true;
	glCM->dummyWindowHandle = dummyWindowHandle;
	HWND handle = dummyWindowHandle;

	/* Retrieve the device context from the window */
	glCM->hDC = GetDC(handle);
	if (!glCM->hDC) {
		nDebugError("Could not get Device Context from context dummy window.");
		nDeinitGlContextManager(glCM);
		return false;
	}

	/* Try setting an empty pixel format */
	PIXELFORMATDESCRIPTOR pfd;
	if (!SetPixelFormat(glCM->hDC, 1, &pfd)) {
		nDebugError("Could not set the preliminary pixel format.");
		nDeinitGlContextManager(glCM);
		return false;
	}

	glCM->context = wglCreateContext(glCM->hDC);
	if (!glCM->context) {
		nDebugError("Could not create OpenGL context.");
		nDeinitGlContextManager(glCM);
		return false;
	}

	/* Activate the preliminary context */
	if (!wglMakeCurrent(glCM->hDC, glCM->context)) {
		nDebugError("Could not set context manager OpenGL context as current.");
		nDeinitGlContextManager(glCM);
		return false;
	}

	if (gl3wInit()) {
		nDebugError("Could not initialize OpenGL extensions.");
		nDeinitGlContextManager(glCM);
		return false;
	}

	int maxglmajor, maxglminor;

	/* Finally retrieve version of OpenGL available on this machine. */
	glGetIntegerv(GL_MAJOR_VERSION, &maxglmajor);
	glGetIntegerv(GL_MINOR_VERSION, &maxglminor);

	nDebugInfo("Supported OpenGL %d.%d by vendor %s.", maxglmajor, maxglminor, glGetString(GL_VENDOR));

	/* Retrieve a few fundamental functions */
	glCM->wglChoosePixelFormatARB_proc = (PFNWGLCHOOSEPIXELFORMATARBPROC)wglGetProcAddress("wglChoosePixelFormatARB");
	glCM->wglGetPixelFormatAttribivARB_proc = (PFNWGLGETPIXELFORMATATTRIBIVARBPROC)wglGetProcAddress("wglGetPixelFormatAttribivARB");
	glCM->wglCreateContextAttribsARB_proc = (PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress("wglCreateContextAttribsARB");
	glCM->wglSwapIntervalEXT_proc = (PFNWGLSWAPINTERVALEXTPROC)wglGetProcAddress("wglSwapIntervalEXT");

	return NU_SUCCESS;
}

void nDeinitGlContextManager(NGlContextManager* glCM)
{
	if (!glCM->initialized) return;
	if (glCM->context) {
		wglMakeCurrent(NULL, NULL);
		wglDeleteContext(glCM->context);
	}
	if (glCM->hDC) ReleaseDC(glCM->dummyWindowHandle, glCM->hDC);
	nZero(glCM);
}

bool nInitGlContext(NGlContextManager* glCM, void* windowHandle, NGLContext* context)
{
	HWND handle = windowHandle;
	context->hwnd = handle;

	/* Retrieve the window device context */
	context->hdc = GetDC(context->hwnd);
	if (!context->hdc) {
		nDebugError("Could not obtain a device context for the window.");
		return false;
	}

	/* Create the GL context */
	int attrib_list[] =
	{
		WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
		WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
		WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
		WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
		WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
		WGL_COLOR_BITS_ARB, (int)32,
		WGL_DEPTH_BITS_ARB, (int)0,
		WGL_STENCIL_BITS_ARB, (int)0,
		WGL_SAMPLE_BUFFERS_ARB, 0,
		WGL_SAMPLES_ARB, 0,
		0 /* msaa */
	};

	int pixelformat = -1;
	UINT numformats = 0;
	glCM->wglChoosePixelFormatARB_proc(context->hdc, attrib_list, NULL, 1, &pixelformat, &numformats);

	/* If pixelformat < 0 it means that it couldnt obtain a pixel format compatible to requested one. */
	if (pixelformat < 0) {
		nDebugError("Could not obtain a pixel format compatible to requested parameters.");
		nDeinitGlContext(glCM, context);
		return false;
	}

	int attrib[] = { WGL_SAMPLES_ARB };
	int max_fsaa = 0;
	glCM->wglGetPixelFormatAttribivARB_proc(context->hdc, pixelformat, 0, 1, attrib, &max_fsaa);

	PIXELFORMATDESCRIPTOR pfd = { 0 };
	if (!SetPixelFormat(context->hdc, pixelformat, &pfd)) {
		DWORD error = GetLastError();
		nDebugError("Could not set window pixel format.");
		nDeinitGlContext(glCM, context);
		return false;
	}

	/* Prepare the attributes used for creating the GL context */
	GLint attribs[] = {
		WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
		WGL_CONTEXT_MINOR_VERSION_ARB, 3,
		WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
		WGL_CONTEXT_FLAGS_ARB,
#ifndef EVE_RELEASE
		WGL_CONTEXT_DEBUG_BIT_ARB,
#else
		0,
#endif
		0
	};

	context->hglrc = glCM->wglCreateContextAttribsARB_proc(context->hdc, glCM->context, attribs);
	if (!context->hglrc) {
		nDebugError("Could not create OpenGL context.");
		nDeinitGlContext(glCM, context);
		return false;
	}

	if (!wglMakeCurrent(context->hdc, context->hglrc)) {
		nDebugError("Could not set newly created OpenGL context as current.");
		nDeinitGlContext(glCM, context);
		return false;
	}

	if (gl3wInit()) {
		nDebugError("Could not initialize OpenGL extensions.");
		nDeinitGlContext(glCM, context);
		return false;
	}

	/* initialize the context */
	GLuint vao;
	glGenVertexArrays(1, &vao);
	if (!vao) {
		nDebugError("Could not create context default VAO.");
		nDeinitGlContext(glCM, context);
		return false;
	}
	glBindVertexArray(vao);

	/* reset the context to manager's */
	if (!wglMakeCurrent(glCM->hDC, glCM->context)) {
		nDebugError("Could not restore context manager GL context.");
		nDeinitGlContext(glCM, context);
		return false;
	}

	return true;
}

void nDeinitGlContext(NGlContextManager* glCM, NGLContext* context)
{
	wglMakeCurrent(NULL, NULL);
	if (context->hglrc) wglDeleteContext(context->hglrc);
	if (context->hdc) ReleaseDC(context->hwnd, context->hdc);
}

void nGlContextSwapBuffers(NGLContext* context)
{
	nAssert(context && "Invalid context provided.");
	SwapBuffers(context->hdc);
}
