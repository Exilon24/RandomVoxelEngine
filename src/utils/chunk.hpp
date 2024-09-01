#ifndef CHUNK_UTIL
#define CHUNK_UTIL

#include<glm/fwd.hpp>
#include "glm/geometric.hpp"

#include<shader.hpp>
#include<vector>
#include<iostream>
#include<bitset>
#include <perlin.hpp>

struct vecKeyTrait
{

	size_t operator()(const glm::ivec3& a) const
	{
		unsigned int hash = 17;
		hash = 31 * hash + a.x;
		hash = 31 * hash + a.y;
		hash = 31 * hash + a.z;
		return hash;
	}

	bool operator()(const glm::ivec3& a, const glm::ivec3& b) const
	{
		return a.x == b.x && a.y == b.y && a.z == b.z;
	}
};

struct TreeInfo
{
	static constexpr uint16_t maxLevel = 6;
};

struct AccelerationQuadtree
{
	uint32_t children[4 * 4 * 4];
};

struct Chunk {
	uint32_t bitmask[32 * 32];
};

struct ChunkStore {
	std::unique_ptr<Chunk> data;
	GLuint gpuDataBuffer;
};
std::vector<unsigned int> loadChunk(glm::ivec3 chunkPosition)
{
	std::vector<unsigned int> voxels;

	unsigned int buffer = 0;

	for (int x = 0; x < 32; x++)
	{
		for (int y = 0; y < 32; y++)
		{
			for (int z = 0; z < 32; z++)
			{
				float xCoord = (float)x / 32 + (chunkPosition.x + 0x000F0000);
				float zCoord = (float)z / 32 - (chunkPosition.z + 0x000F0000);

				int height = (int)((perlin2D(std::abs(xCoord) / 12 , std::abs(zCoord )/12) * 0.5 + 0.5) * 32);
				if (y + (32 * chunkPosition.y) < height)
				{
					buffer += 1;
					if (z < 31)
					{
						buffer = buffer << 1;
					}
				}
				else
				{
					if (z < 31)
					{
						buffer = buffer << 1;
					}
				}
			}
			voxels.push_back(buffer);
			buffer = 0;
		}
	}
	
	return std::move(voxels);
}

#endif // !CHUNK_UTIL