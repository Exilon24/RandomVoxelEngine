#version 430 core

layout (local_size_x = 8, local_size_y = 8, local_size_z = 1)  in;
layout(rgba32f, binding = 0) uniform image2D colorbuffer;

void main()
{
	ivec2 pixelPos = ivec2(gl_GlobalInvocationID.xy);
	ivec2 screenSize = imageSize(colorbuffer);
	if (pixelPos.x >= screenSize.x || pixelPos.y >= screenSize.y)
	{
		return;
	}

	vec3 colr = vec3(
	float(pixelPos.x) / gl_NumWorkGroups.x,
	float(pixelPos.y) / gl_NumWorkGroups.y,
	0.0
	);

	imageStore(colorbuffer, pixelPos, vec4(colr, 1.0));
}