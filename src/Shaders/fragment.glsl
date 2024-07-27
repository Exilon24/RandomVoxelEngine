#version 430 core

#define MAX_DISTANCE 1000
#define MAX_STEPS 100
#define EPSILON 0.001

//uvec3 voxel_i = ...;
//uint voxel_index = voxel_i.x + voxel_i.y * BRICK_SIZE + voxel_i.z * BRICK_SIZE * BRICK_SIZE; // pitch linear index
//uint word_index = voxel_index / 32;
//uint in_word_index = voxel_index % 32;
//uint voxel_bit = (voxels[word_index] >> in_word_index) & 1;

layout(std430, binding = 0) buffer voxelData
{
    uint voxels[];
};

out vec4 FragColor;

in vec2 TexCoord;
in vec3 worldPos;

uniform float tickingAway;
uniform vec3 camPos;
uniform mat4 camDirection;
uniform mat4 model;

uniform int voxelCount;


vec2 resolution = vec2(1920, 1080);


vec3 render(in vec2 uv)
{

	vec3 color = vec3(1.0);
	return color;
}

void main()
{
	vec2 uv = (2.0 * gl_FragCoord.xy / resolution - 1);

	vec3 colr = render(uv);
	FragColor = vec4( colr, 1.0);
}
