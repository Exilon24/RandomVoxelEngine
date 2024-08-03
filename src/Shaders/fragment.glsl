#version 430 core

#define MAX_DISTANCE 1000
#define MAX_STEPS 100
#define EPSILON 0.001

#define GRID_CELL_SIZE 1

#define BRICK_SIZE 8

layout(std430, binding = 0) buffer voxelData
{
    uint voxels[];
};

out vec4 FragColor;

in vec3 CubeUv;
in vec3 cubeMin;
in vec3 cubeMax;

in vec3 vertWorldPos;

uniform float tickingAway;
uniform vec3 camPos;

uniform mat4 iProjMat;
uniform mat4 iViewMat;
uniform mat4 iMatTransform;

uniform int voxelCount;

vec2 resolution = vec2(1920, 1080);

uint indexVoxels(uvec3 voxel_i)
{
	uint voxel_index = voxel_i.x + voxel_i.y * BRICK_SIZE + voxel_i.z * BRICK_SIZE * BRICK_SIZE;
	uint word_index = voxel_index / 32;
	uint in_word_index = voxel_index % 32;
	uint voxel_bit = (voxels[word_index] >> in_word_index) & 1;
	return voxel_bit;
}

vec3 render(vec3 uv)
{
	vec3 rayOrigin = uv * 8;
	uvec3 startingVoxel = uvec3(min(uv * 8, 7));

	vec3 rayDirection = normalize(vertWorldPos - camPos);
	float t = 0;

	int stepX, stepY, stepZ;

	// Initialize step direction
	if (rayDirection.x > 0) stepX = 1;
	else if (rayDirection.x < 0) stepX = -1;
	else stepX = 0;

	if (rayDirection.y > 0) stepY = 1;
	else if (rayDirection.y < 0) stepY = -1;
	else stepY = 0;

	if (rayDirection.z > 0) stepZ = 1;
	else if (rayDirection.z < 0) stepZ = -1;
	else stepZ = 0;

	vec3 color = rayDirection; 
	return color;
}

void main()
{
	vec3 uv = CubeUv;

	vec3 colr = render(uv);
	FragColor = vec4(colr, 1.0);
}
