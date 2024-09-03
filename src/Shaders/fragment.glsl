#version 460 core

out vec4 FragColor;

uniform sampler2D texr;

in vec2 TexCoords;

void main()
{
	vec3 texCol = texture(texr, TexCoords).rgb;   
	FragColor = vec4(texCol ,1.0);
}
