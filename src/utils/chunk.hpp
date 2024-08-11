#ifndef CHUNK_UTIL
#define CHUNK_UTIL

#include<glm/fwd.hpp>
#include "glm/geometric.hpp"

#include<shader.hpp>
#include<vector>
#include<iostream>

struct Chunk
{
	glm::vec3 position = glm::vec3(0.0);
	std::vector<unsigned int> voxelData;
};

struct vecKeyTrait
{
	size_t operator()(const glm::vec3& x) const
	{
		return std::hash<float>()(x.x) ^ std::hash<float>()(x.y) ^ std::hash<float>()(x.z);
	}

	bool operator()(const glm::vec3& a, const glm::vec3& b) const
	{
		return a.x == b.x && a.y == b.y && a.z == b.z;
	}
};

#endif // !CHUNK_UTIL