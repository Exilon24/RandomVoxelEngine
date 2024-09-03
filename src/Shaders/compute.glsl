#version 460 core

layout (local_size_x = 8, local_size_y = 4, local_size_z = 1)  in;
layout(rgba32f, binding = 0) uniform image2D colorbuffer;

uniform mat4 InvView;
uniform mat4 InvPerspective;

void main()
{
	ivec2 pixelPos = ivec2(gl_GlobalInvocationID.xy);
	ivec2 screenSize = imageSize(colorbuffer);
	if (pixelPos.x >= screenSize.x || pixelPos.y >= screenSize.y)
	{
		return;
	}

	vec2 uv = vec2(
		float(pixelPos.x) / (gl_NumWorkGroups.x * gl_WorkGroupSize.x),
		float(pixelPos.y) / (gl_NumWorkGroups.y * gl_WorkGroupSize.y)
	);

	//uv = (uv + 0.5) / vec2(1920, 1080);
	uv = (uv - 0.5) * 2;
	vec3 clipCoords = ( InvView * InvPerspective * vec4(uv.xy, 1.0, 1.0)).xyz;
	
	vec3 rayDir = normalize(clipCoords);

	imageStore(colorbuffer, pixelPos, vec4(sign(rayDir), 1.0));
}