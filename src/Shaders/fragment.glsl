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

vec3 render(in vec3 uv)
{

	// Raycast towards cube
	vec3 u = camPos;
	vec3 v = getRayDir(iViewMat ,iProjMat);
	float t = 0;

	vec3 rayPos = u + v * t;

	uvec3 startVoxelCoord = uvec3(uv * 8);

	int stepX, stepY, stepZ;
	float tMaxX, tMaxY, tMaxZ, tMinX, tMinY, tMinZ; // ??
	float tDeltaX, tDeltaY, tDeltaZ; // ??

	// Initialize step
	if (v.x > 0) stepX = 1;
	else if (v.x < 0) stepX = -1;
	else stepX = 0;

	if (v.y > 0) stepY = 1;
	else if (v.y < 0) stepY = -1;
	else stepY = 0;

	if (v.z > 0) stepZ = 1;
	else if (v.z < 0) stepZ = -1;
	else stepZ = 0;

	// Calculate ray intersect with box
	tMinX = (cubeMin.x - u.x) / v.x;
	tMaxX = (cubeMax.x - u.x) / v.x;

	tMinY = (cubeMin.y - u.y) / v.y;
	tMaxY = (cubeMax.y - u.y) / v.y;

	tMinZ = (cubeMin.z - u.z) / v.z;
	tMaxZ = (cubeMax.z - u.z) / v.z;

	float tMin = max(
	max( 
            min(tMinX, tMaxX), 
            min(tMinY, tMaxY) 
          ), 
          min(tMinZ, tMaxZ)
        );

	float tMax = min( 
          min(
            max(tMinX, tMaxX),
            max(tMinY, tMaxY)
          ), 
          max(tMinZ, tMaxZ)
        );

	if (tMax > 0 && tMin < tMax)
	{
		if (tMin < 0)
		{
			t = tMax;
		}
		else
		{
			t = tMin;
		}
	}

	vec3 color = u + v * t;

	return color;
}

void main()
{
	vec3 uv = (CubeUv);

	vec3 colr = render(uv);
	FragColor = vec4(colr, 1.0);
}
