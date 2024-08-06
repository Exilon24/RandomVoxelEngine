#version 430 core
layout (location = 0) in vec3 aPos;

out vec3 CubeUv;

out vec3 cubeMin;
out vec3 cubeMax;

out vec3 vertWorldPos;

uniform mat4 perspective;
uniform mat4 view;
uniform mat4 model;

void main()
{
CubeUv = aPos;
vertWorldPos = (model * vec4(aPos, 1)).xyz;

gl_Position = perspective * view *  model * vec4(aPos, 1.0);
}