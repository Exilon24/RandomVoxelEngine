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

float ufract(float x)
{
	return (1 - x + floor(x));
}

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
	vec3 rayDirection = normalize(vertWorldPos - camPos);
	uvec3 currentVoxel = uvec3(min((uv * 8) + rayDirection * 0.0001, 7)) ;
	vec3 rayOrigin = uv * 8;

	float t = 0;

	ivec3 stepC = ivec3(sign(rayDirection));
	vec3 rdinv = 1.0/ rayDirection;
	vec3 delta = min(rdinv * stepC, 8.0);
	vec3 tMax = abs((currentVoxel + max(stepC, vec3(0.0)) - rayOrigin) * rdinv);

	//return tMax;

	for (int i = 0; i < 20; ++i) // Will change this to a more appropriate loop later
	{
		if (tMax.x < tMax.y)
		{
			if (tMax.x < tMax.z)
			{
				tMax.x += delta.x;
				currentVoxel.x += stepC.x;
			}
			else
			{
				tMax.z += delta.z;
				currentVoxel.z += stepC.z;
			}
		}
		else 
		{
			if (tMax.y < tMax.z)
			{
				tMax.y += delta.y;
				currentVoxel.y += stepC.y;
			}
			else
			{
				tMax.z += delta.z;
				currentVoxel.z += stepC.z;
			}
		}

		if (indexVoxels(currentVoxel) == 1) return vec3(1.0);
	}

	discard;

}

void main()
{
	vec3 uv = CubeUv;

	vec3 colr = render(uv);
	FragColor = vec4(colr, 1.0);
}
