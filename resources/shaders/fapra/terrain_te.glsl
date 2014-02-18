#version 430

#define TESS_LVL 16

layout(quads, equal_spacing, ccw) in;

uniform mat4 projection_matrix;
uniform mat4 model_view_matrix;

uniform sampler2D heightmap_tx2D;
uniform int size;
uniform float range;

in vec3 eval_position[];
out vec3 position;
out vec2 uv_coord;
out vec3 viewer_direction;
out mat3 tangent_space_matrix;
 
void main()
{
    vec3 p0 = gl_TessCoord.x * eval_position[0] + (1.0 - gl_TessCoord.x) * eval_position[1];
    vec3 p1 = gl_TessCoord.x * eval_position[3] + (1.0 - gl_TessCoord.x) * eval_position[2];
	
	position = gl_TessCoord.y * p0 + (1.0 - gl_TessCoord.y) * p1;
	
	/* compute uv coordinate */
	uv_coord = (position).xz / vec2(size);
	
	/*	use normalized position to access height-map and retrieve height-value (y-coordinate) */
	position.y = texture(heightmap_tx2D,uv_coord).x  * range;
	position	*=0.1;
	
	float patch_size = 1.0/float(size);
	
	/*	Compute tangent and bitangent from heightmap */
	vec2 x_neighbour_uv = uv_coord + vec2(patch_size/float(TESS_LVL),0.0);
	vec2 z_neighbour_uv = uv_coord + vec2(0.0,patch_size/float(TESS_LVL));
	float x_neighbour_height = texture(heightmap_tx2D,x_neighbour_uv).x * range *0.1;
	float z_neighbour_height = texture(heightmap_tx2D,z_neighbour_uv).x * range *0.1;
	
	vec3 tangent = normalize(vec3(0.0,z_neighbour_height-position.y,0.1 * 1.0/float(TESS_LVL)));
	vec3 bitangent = normalize(vec3(0.1 * 1.0/float(TESS_LVL),x_neighbour_height-position.y,0.0));
	vec3 normal = -normalize(cross(bitangent, tangent));
	
	/*	Construct matrices that use the model matrix*/
	mat3 normal_matrix = transpose(inverse(mat3(model_view_matrix)));
	
	/*	Transform input vectors into view space */
	//normal = normalize(normal_matrix * normal);
	//tangent = normalize(normal_matrix * tangent);
	//bitangent = normalize(normal_matrix * bitangent);
    
	/*	Compute transformation matrix for tangent space transformation */
	tangent_space_matrix = mat3(
		tangent.x, bitangent.x, normal.x,
		tangent.y, bitangent.y, normal.y,
		tangent.z, bitangent.z, normal.z);
	
	/*	Transform vertex position to view space */
	position = (model_view_matrix * vec4(position,1.0)).xyz;
	
	/*	Compute direction to the viewer/camer and light source into tangent space */
	viewer_direction = normalize(tangent_space_matrix * normalize( -position ));
	
	gl_Position =  projection_matrix  * vec4(position, 1.0);
}