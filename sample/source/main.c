/*
 * Nunki (simple rendering engine)
 * Copyright (C) 2015 Canio Massimo Tristano <massimo.tristano@gmail.com>.
 * For licensing info see LICENSE.
 */

#include <nunki.h>

int main()
{
	NuInitializeInfo info = {
		.versionMajor = NUNKI_VERSION_MAJOR,
		.versionMinor = NUNKI_VERSION_MINOR,
		.versionPatch= NUNKI_VERSION_PATCH,
	};

	nuInitialize(&info, NULL);

	nuTerminate();
}
