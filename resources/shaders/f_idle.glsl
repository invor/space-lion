/*
---------------------------------------------------------------------------------------------------
File: f_idle.glsl
Author: Michael Becher
Date of (presumingly) last edit: 21.06.2013

This shader program is part of the space-lion framework.

Description: This GLSL fragment shader simply passes on the rgba information from
an input texture. It is used to render a texture to another framebuffer colorattachment.
---------------------------------------------------------------------------------------------------
*/

#version 330

uniform sampler2D inputImage;

/*	Normalized screen coordinates [0,1]x[0,1] of the fragment. */
in vec2 uvCoord;

out vec4 fragColour;

void main()
{
	fragColour = texture(inputImage, uvCoord);
	/* Debugging. */
	//fragColour = vec4(uvCoord,0.0,1.0);
}