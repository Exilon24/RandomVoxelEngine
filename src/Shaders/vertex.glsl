#version 430 core
layout (location = 0) in vec3 aPos;

out vec3 CubeUv;

uniform mat4 perspective;
uniform mat4 view;
uniform mat4 model;

void main()
{
	CubeUv = aPos * 0.5 + 0.5;
	gl_Position = perspective * view * model * vec4(aPos, 1.0);
}
