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

	NuTextureCreateInfo textureInfo = {
		.type = NU_TEXTURE_TYPE_2D,
		.size = { 2, 2, 1 },
		.format = NU_TEXTURE_FORMAT_R8G8B8_UNORM,
	};

	NuTexture texture;
	nuCreateTexture(&textureInfo, NULL, &texture);

	NuImageView imageView = {
		.format = NU_IMAGE_FORMAT_RGBA8,
		.size = { 2, 2 },
		.data = (uint32_t[]) { 0xff0000ff, 0xff00ff00, 0xffff0000, 0xffffffff }
	};

	nuTextureUpdateLevels(texture, 0, 1, &imageView);

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
		
		
		nu2dImmediateBegin(&imm2dInfo);

		nu2dBeginQuadsTextured(NULL, &(Nu2dQuadsTexturedBeginInfo) {
			.blendState = &nuDeviceGetDefaults()->defaultBlendState,
			.texture = texture,
			.sampler = nuDeviceGetDefaults()->linearSampler,
		});

		nu2dQuadTextured(NULL, (NuRect2) { 10, 10, 100, 100 }, 0xffffffff, (NuRect2) { 0, 0, 1, 1 }, 0);
		
		nu2dImmediateEnd();

		nuDeviceSwapBuffers(context);
	}

after_mainloop:
	nuDestroyTexture(texture, NULL);
	nuDestroyContext(context, NULL);
	nuDestroyWindow(window, NULL);
	nuTerminate();
}
