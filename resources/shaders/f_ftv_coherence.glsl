/*
---------------------------------------------------------------------------------------------------
File: f_ftv_coherence.glsl
Author: Michael Becher
Date of (presumingly) last edit: 21.06.2013

This shader program is being developed in the scope of FeTol at University Stuttgart (VISUS).
http://www.vis.uni-stuttgart.de/en/projects/fetol.html

Description: This GLSL fragment shader computes coherence flow and coherence strength using a
given structure tensor.
---------------------------------------------------------------------------------------------------
*/

#version 330

uniform sampler2D structureTensor;

/*	Normalized coordinates [0,1]x[0,1] of the fragment. */
in vec2 uvCoord;

/*	Fragment shader output variable. */
out vec3 coherenceOuput;


void main()
{
	vec4 tensor  = texture(structureTensor,uvCoord);
	
	float diskr = sqrt( pow((tensor.x - tensor.z),2.0)+4.0*tensor.y*tensor.y);
	float lambda_1 = 0.5 * (tensor.x + tensor.z + diskr);
	float lambda_2 = 0.5 * (tensor.x + tensor.z - diskr);
	
	vec2 v_1 = vec2( (2.0*tensor.y) , (tensor.x - tensor.z + diskr) );
	vec2 v_2 = vec2( -v_1.y , v_1.x );

	float coherenceStrength;
	/* TODO FIND VALUES */
	float quant = 1.0/10.0;
	float k = 1.0;
	if(lambda_1 == lambda_2)
	{
		coherenceStrength = 1.0;
	}
	else
	{
		coherenceStrength = k * exp( -pow(quant,4.0)/pow(lambda_1-lambda_2,2.0) );
	}

	coherenceOuput = vec3(v_2,coherenceStrength);
}