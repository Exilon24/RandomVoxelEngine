#version 430 core

#define MAX_DISTANCE 1000
#define MAX_STEPS 100
#define EPSILON 0.001

#define BRICK_SIZE 32

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
uniform vec3 chunkPos;

uniform mat4 iProjMat;
uniform mat4 iViewMat;
uniform mat4 iMatTransform;

uniform int voxelCount;


vec2 resolution = vec2(1920, 1080);

float ufract(float x)
{
	return (1 - x + floor(x));
}

uint indexVoxels(ivec3 voxel_i)
{
	uint voxel_index = voxel_i.x * BRICK_SIZE * BRICK_SIZE + voxel_i.y * BRICK_SIZE + voxel_i.z;
	uint word_index = voxel_index / 32;
	uint in_word_index = voxel_index % 32;
	uint voxel_bit = (voxels[word_index] >> in_word_index) & 1;
	return voxel_bit;
}

vec3 render()
{
	vec3 rayDirection = normalize(vertWorldPos - camPos);

	///////////////////////////////////////////////////////////////////////////////////
	// For the love of god and all holy use floats instead of intergers for division //
	///////////////////////////////////////////////////////////////////////////////////
	ivec3 currentVoxel = ivec3(min(floor((CubeUv * BRICK_SIZE) + rayDirection * 0.0001),vec3(BRICK_SIZE - 0.001))) ;
	vec3 rayOrigin = CubeUv * BRICK_SIZE;

	float t = 0;

	// Floating point and integer representation of the ray's direction as a sign (1, -1, 0)
	vec3 signrd = sign(rayDirection);
    ivec3 rayStep = ivec3(signrd + 0.);


	vec3 rdinv = 1 / rayDirection;
	vec3 delta = rdinv * signrd;
	vec3 tMax = abs((currentVoxel + max(signrd, vec3(0.0)) - rayOrigin) * rdinv);

	for (int i = 0; i < 128; i++) // Will change this to a more appropriate loop later
	{
		
		// If ray escapes voxel
 		if(any(greaterThan(currentVoxel,vec3(BRICK_SIZE - 1.0))) || any(lessThan(currentVoxel,vec3(0)))) break;

		// Check if voxel is set
		if (indexVoxels(currentVoxel) == 1) return vec3(0, currentVoxel.y + (32 * chunkPos.y), 0) / 64;

		if (tMax.x < tMax.y)
		{
			if (tMax.x < tMax.z)
			{
				tMax.x += delta.x;
				currentVoxel.x += rayStep.x;

			}
			else
			{
				tMax.z += delta.z;
				currentVoxel.z += rayStep.z;
			}
		}
		else 
		{
			if (tMax.y < tMax.z)
			{
				tMax.y += delta.y;
				currentVoxel.y += rayStep.y;
			}
			else
			{
				tMax.z += delta.z;
				currentVoxel.z += rayStep.z;
			}
		}

	}

	discard;

}

void main()
{
	vec3 color = render();
	FragColor = vec4(color, 1.0);
}
