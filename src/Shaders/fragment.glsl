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
in vec3 worldPos;

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

	vec3 startVoxelCoord = uv * 8;

	int stepX, stepY, stepZ;
	float tMaxX, tMaxY, tMaxZ; // ??
	float tDeltaX, tDeltaY, tDeltaZ; // ??

	tDeltaX = GRID_CELL_SIZE / (v.x - u.x);
	tMaxX = tDeltaX * (1.0 - fract(u.x / GRID_CELL_SIZE));

	tDeltaY = GRID_CELL_SIZE / (v.y - u.y);
	tMaxY = tDeltaY * (1.0 - fract(u.y / GRID_CELL_SIZE));

	tDeltaZ = GRID_CELL_SIZE / (v.z - u.z);
	tMaxZ = tDeltaZ * (1.0 - fract(u.z / GRID_CELL_SIZE));

	stepX = int(floor(v.x / abs(v.x)));
	stepY = int(floor(v.y / abs(v.y)));
	stepZ = int(floor(v.z / abs(v.z)));

	vec3 color = vec3(stepX, stepY, stepZ) * 0.5 + 0.5;
	
	return color;
}

void main()
{
	vec3 uv = (CubeUv);

	vec3 colr = render(uv);
	FragColor = vec4(colr, 1.0);
}
