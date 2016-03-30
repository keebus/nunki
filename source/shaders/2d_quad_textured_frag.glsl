/*
 * Nunki (simple rendering engine)
 * Copyright (C) 2015 Canio Massimo Tristano <massimo.tristano@gmail.com>.
 * For licensing info see LICENSE.
 */

#version 330

uniform texture2D tTexture;
uniform sampler2D sSampler;

flat in vec4 vColor;
in vec3 vUV;

out vec4 fFragColor;

void main()
{
	vec4 texColor = textureLod(sSampler, tTexture, vUV.xy, 0);
	fFragColor = vColor * texColor;
}
