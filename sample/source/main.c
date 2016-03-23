/*
 * Nunki (simple rendering engine)
 * Copyright (C) 2015 Canio Massimo Tristano <massimo.tristano@gmail.com>.
 * For licensing info see LICENSE.
 */

#include <nunki.h>

int main()
{
	NuInitializeInfo initInfo = {
		.versionMajor = NUNKI_VERSION_MAJOR,
		.versionMinor = NUNKI_VERSION_MINOR,
		.versionPatch= NUNKI_VERSION_PATCH,
	};

	nuInitialize(&initInfo, NULL);

	NuWindowCreateInfo winInfo = {
		.title = "Nunki Sample",
		.width = 1280,
		.height = 720,
	};

	NuWindow window;
	nuCreateWindow(&winInfo, NULL, &window);

	NuContextCreateInfo contextInfo = {
		.windowHandle = nuWindowGetNativeHandle(window),
	};

	NuContext context;
	nuCreateContext(&contextInfo, NULL, &context);

	NuScene2D scene;
	nuCreateScene2D(NULL, &scene);

	nu2dReset(scene, (NuRect2i) { 0, 0, 1280, 720 });
	nu2dBeginQuadsSolid(scene, &nuDeviceGetDefaultStates()->alphaBlendState);
	nu2dQuadSolid(scene, (NuRect2) { 10, 10, 100, 100 }, 0xff0000ff);
	nu2dQuadSolid(scene, (NuRect2) { 20, 20, 100, 100 }, 0xfff000ff);
	nu2dQuadSolid(scene, (NuRect2) { 30, 30, 100, 100 }, 0xff0f00ff);
	nu2dQuadSolid(scene, (NuRect2) { 40, 40, 100, 100 }, 0xff00f0ff);
	nu2dQuadSolid(scene, (NuRect2) { 50, 50, 100, 100 }, 0xff000fff);

	NuWindowEvent e;
	for (;;)
	{
		while (nuWindowPollEvents(window, &e)) {
			if (e.type == NU_WINDOW_EVENT_TYPE_CLOSE) {
				goto after_mainloop;
			}
		}

		nuDeviceClear(context, NU_CLEAR_COLOR, (float[]) { 0.2f, 0.3f, 0.5f, 1.0f }, 0, 0);

		//Nu2dBeginImmediateInfo imm2dInfo = {
		//	context, .viewport = (NuRect2i) { 0, 0, 1280, 720 },
		//};
		//nu2dImmediateBegin(&imm2dInfo);
		
		nu2dPresent(scene, context);

		//nu2dImmediateEnd();

		nuDeviceSwapBuffers(context);
	}

after_mainloop:
	nuDestroyScene2D(scene, NULL);
	nuDestroyContext(context, NULL);
	nuDestroyWindow(window, NULL);
	nuTerminate();
}
