#version 460 core

layout (local_size_x = 8, local_size_y = 4, local_size_z = 1)  in;
layout(rgba32f, binding = 0) uniform image2D colorbuffer;

layout(std430, binding = 1) buffer accelBuffer
{
    uint accelData[];
};

layout(std430, binding = 2) buffer chunkBuffer
{
    uint chunkData[];
};

uniform mat4 InvView;
uniform mat4 InvPerspective;
uniform vec3 camPos;

void main()
{
	ivec2 pixelPos = ivec2(gl_GlobalInvocationID.xy);
	ivec2 screenSize = imageSize(colorbuffer);
	if (pixelPos.x >= screenSize.x || pixelPos.y >= screenSize.y)
	{
		return;
	}

	vec2 uv = vec2(
		(float(pixelPos.x) +0.5) / (gl_NumWorkGroups.x * gl_WorkGroupSize.x),
		(float(pixelPos.y) + 0.5) / (gl_NumWorkGroups.y * gl_WorkGroupSize.y)
	);

	uv = (uv - 0.5) * 2;
	vec3 clipCoords = ( InvView * InvPerspective * vec4(uv.xy, 1.0, 1.0)).xyz;
	
	vec3 rayDir = normalize(clipCoords);

	imageStore(colorbuffer, pixelPos, vec4(rayDir, 1.0));
}