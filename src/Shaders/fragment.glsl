#version 410 core

#define MAX_DISTANCE 1000 
#define MAX_STEPS 100
#define EPSILON 0.001

out vec4 FragColor;

in vec2 TexCoord;

uniform float tickingAway;
uniform vec3 camPos;
uniform mat4 camDirection;

uniform samplerBuffer voxelData;

vec2 resolution = vec2(1920, 1080);
vec3 lightPos = vec3(sin(tickingAway) * 3, 6, 2);

float sphere(vec3 rayPos, float radius, vec3 center)
{
	return length(rayPos - center) - radius;
}

float Box(vec3 rayPos, vec3 b, vec3 centre)
{
  vec3 q = abs(centre - rayPos) - b;
  return length(max(q,0.0)) + min(max(q.x,max(q.y,q.z)),0.0);
}

float sdPlane( vec3 p, vec3 n, float h )
{
  // n must be normalized
  return dot(p,n) + h;
}

float map(in vec3 rayPos)
{
	float result = 0;
	vec3 pos = vec3(
		texelFetch(voxelData, 8).x, 
		texelFetch(voxelData, 9).x,
		texelFetch(voxelData, 10).x
	);
	return min(Box(rayPos, vec3(0.2), pos), Box(rayPos, vec3(0.2), vec3(0.0)));
}

vec3 getNormal(vec3 p)
{ 
    float d = map(p); // Distance
    vec2 e = vec2(EPSILON,0); // Epsilon
    vec3 n = d - vec3(
    map(p-e.xyy),  
    map(p-e.yxy),
    map(p-e.yyx));
   
    return normalize(n);
}



float shadow( in vec3 ro, in vec3 rd ,float minT, float maxT, int k)
{
	float res = 1.0;
	float t = minT;

	// March again
	for (int i = 0; i < 256 && t<maxT; i++)
	{
		float h = map(ro + rd * t);
		if (h < EPSILON)
		{
			return 0.0;
		}
		res = min(res, k*h/t);
		t += h;
	}
	return res;
}

vec3 render(in vec2 uv)
{
	vec3 color = vec3 (0.2);

	vec3 projection = vec3(uv.x * 1.6, uv.y,1.); 

	vec3 rd = normalize(projection)* mat3(camDirection);
	vec3 position;

	float distance = 0.0;

	for (float i = 0.0; i < MAX_STEPS; i++)
	{
		position = camPos + rd * distance;

		float sdfCheck = map(position);
		
		distance += sdfCheck;

		if (sdfCheck < EPSILON || distance > MAX_DISTANCE) break;
	}

	float shadows = shadow(position, normalize(lightPos - position), 1, MAX_DISTANCE, 32);
	color = vec3(1.0); //* max(dot(getNormal(position), normalize(lightPos - position)), 0) * shadows;
	if (distance < 10)
	{
		color *= 0;
	}

	return color;
}

void main()
{
	vec2 uv = (2.0 * gl_FragCoord.xy / resolution - 1);

	vec3 colr = render(uv);
 
	FragColor = vec4(colr, 1.0);
}
