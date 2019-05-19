#version 450

in vec3 position;
in vec2 uvCoord;
in mat3 tangent_space_matrix;

out vec4 frag_colour;

void main()
{
    frag_colour = vec4(1.0);
}