/*
 * Nunki (simple rendering engine)
 * Copyright (C) 2015 Canio Massimo Tristano <massimo.tristano@gmail.com>.
 * For licensing info see LICENSE.
 */

#version 330

uniform sampler2D sTexture;

flat in vec4 vColor;
in vec3 vUV;

out vec4 fFragColor;

void main()
{
	vec4 texColor = textureLod(sTexture, vUV.xy, 0);
	fFragColor = vColor * texColor;
}
