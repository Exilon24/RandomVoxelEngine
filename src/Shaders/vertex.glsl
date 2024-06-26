#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 texCoords;

out vec2 TexCoord;
out vec3 worldPos;

uniform mat4 perspective;
uniform mat4 camDirection;
uniform mat4 model;

void main()
{
	TexCoord = texCoords;
	worldPos = mat3(model) * aPos;
	gl_Position = perspective * camDirection * model * vec4(aPos, 1.0);
}
