#version 330

//in vec3 v_position;
//in vec2 v_uvCoord;

out vec2 uvCoord;

void main()
{
	const vec4 vertices[6] = vec4[6]( vec4( -1.0,-1.0,0.0,0.0 ),
									vec4( 1.0,1.0,1.0,1.0 ),
									vec4( -1.0,1.0,0.0,1.0 ),
									vec4( 1.0,1.0,1.0,1.0 ),
									vec4( -1.0,-1.0,0.0,0.0 ),
                                	vec4( 1.0,-1.0,1.0,0.0 ) );

	vec4 vertex = vertices[gl_VertexID];
	
	uvCoord = vertex.zw;
	gl_Position =  vec4(vertex.xy, -0.9999, 1.0);

	//uvCoord = v_uvCoord;
	//gl_Position =  vec4(v_position, 1.0);
}