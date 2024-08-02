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

uniform float tickingAway;
uniform vec3 camPos;
uniform mat4 camDirection;
uniform mat4 model;

uniform mat4 iProjMat;
uniform mat4 iViewMat;

uniform int voxelCount;

vec2 resolution = vec2(1920, 1080);

vec3 getRayDir(mat4 IViewMatrix, mat4 IProjectionMatrix)
{
	vec2 uv = (2.0 * gl_FragCoord.xy / resolution - 1);
	vec4 rayEye = IProjectionMatrix * vec4(uv.x, uv.y, -1, 1);
	rayEye.z = -1;
	rayEye.w = 0;
	return(normalize(IViewMatrix * rayEye).xyz);
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

	vec3 rayDirection = getRayDir(iViewMat ,iProjMat);
	float t = 0;

	uvec3 startVoxelCoord = uvec3(uv * 7);

	// AABB test

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

	// Calculate the T values
	vec3 tMin = (cubeMin - camPos) / rayDirection;
	vec3 tMax = (cubeMax - camPos ) / rayDirection;

	vec3 t1 = min(tMin, tMax);
	vec3 t2 = max(tMin, tMax);

	float tNear = max(max(t1.x, t1.y), t1.z);
	float tFar = min(min(t2.x, t2.y), t2.z);
    
    if (tNear <= tFar && tFar > 0.0) {
		t = tNear;
    }

	// Find the point where the ray intersects with the cube
	vec3 rayOrigin = camPos + rayDirection * t;

	vec3 color = rayOrigin;
	return color;
}

void main()
{
	vec3 uv = (CubeUv);

	vec3 colr = render(uv);
	FragColor = vec4(colr, 1.0);
}
