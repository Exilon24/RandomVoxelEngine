#ifndef CHUNK_UTIL
#define CHUNK_UTIL

#include<glm/fwd.hpp>
#include "glm/geometric.hpp"

#include<shader.hpp>
#include<vector>

class Chunk
{
public:
	Chunk();
	Chunk(Shader& shader);
	void setShader(Shader& shader);

	glm::vec3 position = glm::vec3(0.0);
	glm::vec3 voxelDimentions = glm::vec3(8);
	std::vector<unsigned int> voxels;

};

#endif // !CHUNK_UTIL