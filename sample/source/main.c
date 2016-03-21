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

	NuBlendState alphaBlendState = {
		.srcRgbFactor = NU_BLEND_FACTOR_SRC_ALPHA,
		.dstRgbFactor = NU_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
		.rgbOp = NU_BLEND_FUNC_ADD,
		.srcAlphaFactor = NU_BLEND_FACTOR_ONE,
		.dstAlphaFactor = NU_BLEND_FACTOR_ZERO,
		.alphaOp = NU_BLEND_FUNC_ADD,
	};

	NuWindowEvent e;
	for (;;)
	{
		while (nuWindowPollEvents(window, &e)) {
			if (e.type == NU_WINDOW_EVENT_TYPE_CLOSE) {
				goto after_mainloop;
			}
		}

		nuDeviceClear(context, NU_CLEAR_COLOR, (float[]) { 0.2f, 0.3f, 0.5f, 1.0f }, 0, 0);

		NuScene2DResetInfo resetInfo = {
			.context = context,
			.bounds = (NuRect2i) { 0, 0, nuWindowGetSize(window) },
		};

		nu2dReset(scene, &resetInfo);

		nu2dSetBlendState(scene, &alphaBlendState);
		nu2dDrawQuadSolidFlat(scene, (NuRect2){ 10, 10, 100, 100 }, 0xff0000ff);

		nu2dPresent(scene);

		nuDeviceSwapBuffers(context);
	}

after_mainloop:
	nuDestroyContext(context, NULL);
	nuDestroyWindow(window, NULL);
	nuTerminate();
}
