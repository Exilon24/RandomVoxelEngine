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

int ceil_div(int a, int b)
{
	return 1 + ((a - 1) / b);
}

void buildNode(Acceleration64tree node, int level)
{
	if (level < accelTreeInfo.maxLevel)
	{
		std::vector<uint32_t> indexes;

		for (int i = 0; i < 4 * 4 * 4; i++)
		{
			Acceleration64tree newNode;
			accel.push_back(newNode);
			indexes.push_back(accel.size() - 1);
		}

		std::copy(indexes.begin(), indexes.end(), node.children);
	}
}

glm::vec3 getChunkPosition(int currentChunk)
{
	for (auto it = chunkPositions.begin(); it != chunkPositions.end(); it++)
	{
		if (it->second == currentChunk)
		{
			return it->first;
		}

		throw std::invalid_argument("COULDN'T FIND CHUNK");
	}
}

void buildTree()
{
	if (chunkPositions.size() < 1 || chunk_data.size() < 1) return;

	AABB extents = calculateChunkExtents();
	glm::ivec3 size = glm::ivec3(extents.max - extents.min); // bounding volume size
	int maxSize = std::max(std::max(size.x, size.y), size.z); // bounding cube size
	int mssb = std::ceil(std::log2(maxSize));
	int maxLevels = ceil_div(mssb, 2);

	accelTreeInfo.maxLevel = maxLevels;

#ifdef DEBUG_VOXELGEN
	treeLog << "Building tree:\nMin: "
		<< extents.min.x << " " << extents.min.y << " " << extents.min.z << "\nMax: "
		<< extents.max.x << " " << extents.max.y << " " << extents.max.z << "\n";
	treeLog << "Max Level: " << maxLevels << '\n';
	treeLog << "\n";
#endif

	accel.emplace_back(); //add rootNode

	uint32_t chunk_index = 0;
	for (auto& chunk : chunk_data) {

		uint32_t node_index = 0;

		// Get half root node size
		uint32_t node_size_log2 = maxLevels * 2; // what is this?
		uint32_t node_size = 1 << node_size_log2;
		uint32_t root_size_half = 1 << (node_size_log2 - 1);


		// Get chunk position
		glm::ivec3 chunk_pos = getChunkPosition(chunk_index) + glm::vec3(root_size_half);

		for (int level = maxLevels; level > 0; --level) {

			// 0000	0000
			uint8_t child_position;

			uint32_t position_mask = (node_size - 1) & ((1 << (node_size_log2 - 2)) - 1); // what the fuck?

			child_position = (position_mask & chunk_pos.x) << 0; // 0000 00xx
			child_position |= (position_mask & chunk_pos.y) << 2; // 0000 yy00
			child_position |= (position_mask & chunk_pos.z) << 4; // 00zz 0000

			// 00zz yyxx
			// 2 bits per axis???

			Acceleration64tree& parent = accel[node_index];
			uint32_t child_index = parent.children[child_position]; 

			// Check if child exists
			if (!child_index) {
				child_index = accel.size();
				accel[node_index].children[child_position] = child_index;
				accel.emplace_back();
			}

			node_index = child_index;

			node_size_log2 = level * 2;
			node_size = 1 << level;
		}

		// What is this section down here for?
		uint8_t child_position;
		uint32_t position_mask = (node_size - 1) & ((1 << (node_size_log2 - 2)) - 1);
		child_position = (position_mask & chunk_pos.x) << 0;
		child_position |= (position_mask & chunk_pos.y) << 2;
		child_position |= (position_mask & chunk_pos.z) << 4;

		accel[node_index].children[child_position] = chunk_index;
		chunk_index++;
	}
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