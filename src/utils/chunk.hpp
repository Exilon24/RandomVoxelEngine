#ifndef CHUNK_UTIL
#define CHUNK_UTIL

#include<glm/fwd.hpp>
#include "glm/geometric.hpp"

#include<shader.hpp>
#include <perlin.hpp>

#include <thread>
#include <mutex>
#include <vector>
#include <iostream>
#include <bitset>
#include <fstream>
#include <condition_variable>

#define DEBUG_VOXELGEN

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

struct AABB
{
	glm::vec3 min;
	glm::vec3 max;
};

struct TreeInfo
{
	uint16_t maxLevel = 6;
};

struct Acceleration64tree
{
	uint32_t children[4 * 4 * 4];
};

struct Chunk {
	uint32_t bitmask[32 * 32];
};

//Tree
std::vector<Acceleration64tree> accel;
TreeInfo accelTreeInfo;

// Chunkdata
std::vector<Chunk> chunk_data;
std::stack<unsigned int> free_chunks;
std::unordered_map<glm::ivec3, uint32_t, vecKeyTrait, vecKeyTrait> chunkPositions;

//Loading
std::unordered_set<glm::ivec3, vecKeyTrait, vecKeyTrait> processingChunks;
std::queue<glm::ivec3> chunksToLoad;

// Threading
std::mutex loadChunkMutex;
bool stopWork = false;
std::condition_variable mutex_condition;

#ifdef DEBUG_VOXELGEN
std::ofstream chunkLog("log.txt");
std::ofstream treeLog("treeLog.txt");
#endif


/// <summary>
/// Allocate a new chunk to the chunkdata vector
/// </summary>
/// <returns>The index to the new chunk</returns>
uint32_t AllocateChunk()
{
	size_t index;
	if (free_chunks.empty())
	{
		index = chunk_data.size(); // Create a new chunk
	}
	else
	{
		index = free_chunks.top();
		free_chunks.pop();
	}

	return index;
}

void FreeChunkData(uint32_t index)
{
	chunk_data.erase(chunk_data.begin() + index);
	free_chunks.push(index);
}

std::vector<uint32_t> loadChunk(glm::ivec3 chunkPosition)
{
	std::vector<uint32_t> voxels;

	uint32_t buffer = 0;

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

AABB calculateChunkExtents()
{
	std::unique_lock<std::mutex> lock(loadChunkMutex);
	if (chunkPositions.size() < 1)
	{
		std::cerr << "WHAT THE FUCK BRO";

	}

	AABB extents;
	auto it = chunkPositions.begin();
	extents.min = glm::vec3(it->first);
	extents.max = glm::vec3(it->first);
	it++;

	for (; it != chunkPositions.end(); it++)
	{
		extents.min = glm::min(glm::vec3(it->first), extents.min); 
		extents.max = glm::max(glm::vec3(it->first), extents.max);
	}

	return extents;
}

int ceil_div(float a, float b)
{
	return 1 + ((a - 1) / b);
}

void buildTree(int lvl)
{
	if (chunkPositions.size() < 1 || chunk_data.size() < 1) return;

	AABB extents = calculateChunkExtents();
	glm::vec3 size = extents.max - extents.min; // bounding volume size
	float maxSize = std::max(std::max(size.x, size.y), size.z); // bounding cube size
	float mssb = std::ceil(std::log2f(maxSize));
	int maxLevels = ceil_div(mssb, 2);

	accelTreeInfo.maxLevel = maxLevels;

#ifdef DEBUG_VOXELGEN
	treeLog << "Building tree:\nMin: "
		<< extents.min.x << " " << extents.min.y << " " << extents.min.z << "\nMax: "
		<< extents.max.x << " " << extents.max.y << " " << extents.max.z << "\n";
	treeLog << "CubicSize: " << maxLevels << '\n';
	treeLog << "\n";
#endif

	//Build tree
}

void ChunkUpdate()
{
	while (true)
	{
		glm::ivec3 chunkToLoad;
		{
			std::unique_lock<std::mutex> lock(loadChunkMutex);
			mutex_condition.wait(lock, []
				{
					return !chunksToLoad.empty() || stopWork;
				});

			if (stopWork) return;
			chunkToLoad = chunksToLoad.front();
			chunksToLoad.pop();
		}

		std::vector<uint32_t> chunkInfo = loadChunk(chunkToLoad);

		{
			std::unique_lock<std::mutex> lock(loadChunkMutex);

			// Load the chunk and it's position
			Chunk loadedChnk;

			uint32_t allocatedChunk = AllocateChunk();

			std::copy(chunkInfo.begin(), chunkInfo.end(), loadedChnk.bitmask);

			chunk_data.insert(chunk_data.begin() + allocatedChunk, std::move(loadedChnk));
			chunkPositions[chunkToLoad] = allocatedChunk; // store the current chunks position to its index

#ifdef DEBUG_VOXELGEN


			int accum = 0;
			for (auto& valu : loadedChnk.bitmask)
			{
				accum += valu;
			}
			chunkLog << "AllocatedSlot: " << allocatedChunk << '\n';
			chunkLog << "LoadedValue: " << std::to_string(accum) << '\n';
			chunkLog << "ChunkPosition: " << chunkToLoad.x << ", " << chunkToLoad.y << ", " << chunkToLoad.z << '\n';
			chunkLog << "ChunkPositionContainerSize: " << chunkPositions.size() << '\n';

			chunkLog << "\n";

#endif // DEBUG_VOXELGEN

			processingChunks.erase(processingChunks.find(chunkToLoad));
		}
	}
}

#endif // !CHUNK_UTIL