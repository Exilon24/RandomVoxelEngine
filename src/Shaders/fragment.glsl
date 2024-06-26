#version 410 core

#define MAX_DISTANCE 1000
#define MAX_STEPS 100
#define EPSILON 0.001

// shapes in raymarch space are twice as large (?)
#define VOXEL_SIZE 0.015625 

out vec4 FragColor;

in vec2 TexCoord;
in vec3 worldPos;

uniform float tickingAway;
uniform vec3 camPos;
uniform mat4 camDirection;
uniform mat4 model;

uniform int voxelCount;

uniform isamplerBuffer voxelData;

vec2 resolution = vec2(1920, 1080);
vec3 lightPos = vec3(sin(tickingAway) * 3, 4, 0);

float Box(vec3 rayPos, vec3 b, vec3 centre)
{
  vec3 q = abs(centre - rayPos) - b;
  return length(max(q,0.0)) + min(max(q.x,max(q.y,q.z)),0.0);
}

float map(in vec3 rayPos)
{
	float result = 1;
	
	for (int i = 0; i < voxelCount; i++)
	{
		if (texelFetch(voxelData, i).x == 1)
		{
			// Still figuring out this part out
			vec3 pos = vec3(
				(VOXEL_SIZE/ 2) * i,
				(VOXEL_SIZE/2) * fract(i / 6), 
				0) - vec3(0.5);
			if (i==0)
			{
				result = Box(rayPos, vec3(VOXEL_SIZE), pos);
			}
			else
			{
				result = min(Box(rayPos, vec3(VOXEL_SIZE), pos), result);
			}
		}
	}
	
	return result;
}


vec3 render(in vec2 uv)
{
	// TODO FACE CULLING
	vec3 color = vec3 (1.0);

	vec3 projection = vec3(uv.x * 1.6, uv.y,1.); 

	vec3 rd = normalize(worldPos - camPos);
	vec3 position;

	float distance = 0.0;

	for (float i = 0.0; i < MAX_STEPS; i++)
	{
		position = camPos + rd * distance;

		float sdfCheck = map(position);
		
		distance += sdfCheck;

		if (sdfCheck < EPSILON || distance > MAX_DISTANCE) break;
	}

	color = vec3(1.0);

	if (distance < 100)
	{
		color *= 0;
	}
	return color;
}

void main()
{
	vec2 uv = (2.0 * gl_FragCoord.xy / resolution - 1);

	vec3 colr = render(uv);
 
	FragColor =vec4(colr, 1.0);
}
