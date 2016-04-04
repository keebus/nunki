/*
 * Nunki (simple rendering engine)
 * Copyright (C) 2015 Canio Massimo Tristano <massimo.tristano@gmail.com>.
 * For licensing info see LICENSE.
 */

#include <nunki.h>
#include <stdio.h>
#include <malloc.h>

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

	NuTextureCreateInfo textureInfo = {
		.type = NU_TEXTURE_TYPE_2D,
		.size = { 2, 2, 1 },
		.format = NU_TEXTURE_FORMAT_R8G8B8_UNORM,
	};

	NuFont font;
	nuCreateFont(&(NuFontCreateInfo) {
		.resolution = 90,
		.textureWidth = 1024,
		.textureHeight = 1024,
		.faceLoader = &nuFileLoader,
		.numFaces = 2,
		.faces = (NuFaceInfo[]) {
			{
				.userData = "../../sample/data/AmaticSC-Regular.ttf",
				.size = 30,
				.charSets = (NuCharSet[]) { 32, 122 },
				.numCharSets = 1,
			},
			{
				.userData = "../../sample/data/GreatVibes-Regular.otf",
				.size = 60,
				.charSets = (NuCharSet[]) { 32, 122 },
				.numCharSets = 1,
			}		
		}
	}, NULL, nResetTempAlloc(), &font);

	NuTextStyle styles[] = {
		0, nuRGBA(255, 255, 255, 255), /* welcome-to */
		1, nuRGBA(100, 200, 255, 255), /* nunki */
	};


	NuScene2D scene;
	nuCreateScene2D(NULL, &scene);
	nu2dReset(scene, (NuRect2i) { 0, 0, 1270, 720 });
	nu2dBeginText(scene, font);
	nu2dText(scene, "Welcome to\n<1>Nunki</>", (NuPoint2i) { 50, 50 }, styles, 0);

	NuWindowEvent e;
	for (;;)
	{
		while (nuWindowPollEvents(window, &e)) {
			if (e.type == NU_WINDOW_EVENT_TYPE_CLOSE) {
				goto after_mainloop;
			}
		}

		nuDeviceClear(context, NU_CLEAR_COLOR, (float[]) { 0.2f, 0.3f, 0.5f, 1.0f }, 0, 0);

		Nu2dBeginImmediateInfo imm2dInfo = {
			context,
			.viewport = (NuRect2i) { 0, 0, 1280, 720 },
		};
		
		nu2dPresent(scene, context);

		nuDeviceSwapBuffers(context);
	}

after_mainloop:
	nuDestroyFont(font, NULL);
	nuDestroyContext(context, NULL);
	nuDestroyWindow(window, NULL);
	nuTerminate();
}
