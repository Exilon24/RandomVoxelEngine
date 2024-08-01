#version 430 core
layout (location = 0) in vec3 aPos;

out vec3 CubeUv;

out vec3 cubeMin;
out vec3 cubeMax;

uniform mat4 perspective;
uniform mat4 view;
uniform mat4 model;

void main()
{
	CubeUv = aPos * 0.5 + 0.5;

	// find min and max
	if (CubeUv == vec3(0.0)) cubeMin = aPos;
	else if (CubeUv == vec3(1.0)) cubeMax = aPos;

	gl_Position = perspective * view * model * vec4(aPos, 1.0);
}
