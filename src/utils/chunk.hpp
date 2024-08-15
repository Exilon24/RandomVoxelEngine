#ifndef CHUNK_UTIL
#define CHUNK_UTIL

#include<glm/fwd.hpp>
#include "glm/geometric.hpp"

#include<shader.hpp>
#include<vector>
#include<iostream>
#include<bitset>
#include <perlin.hpp>

int Fltsign(float x) {
	return (x > 0) - (x < 0);
}

std::vector<unsigned int> loadChunk(glm::vec3 chunkPosition)
{
	std::vector<unsigned int> voxels;

	unsigned int buffer = 0;
	chunkPosition = chunkPosition;

	for (int x = 0; x < 32; x++)
	{
		for (int y = 0; y < 32; y++)
		{
			for (int z = 0; z < 32; z++)
			{
				float xCoord = (float)x / 32 + chunkPosition.x;
				float zCoord = (float)z / 32 - chunkPosition.z;

				int height = (int)((perlin2D(xCoord, zCoord) * 0.5 + 0.5) * 64);
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

	return voxels;
}

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