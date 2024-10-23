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
	glm::ivec3 min;
	glm::ivec3 max;
};

struct TreeInfo
{
	uint16_t maxLevel = 6;
	GLuint chunkDataBuffer;
	GLuint accelTreeBuffer;
};

struct Acceleration64tree
{
	uint32_t children[4 * 4 * 4] = {0};
};

struct Chunk {
	uint32_t bitmask[32 * 32] = {0};
};

int lastAccelSize = 0;
int lastChunkSize = 0;

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

AABB calculateChunkExtents() {
	if (chunkPositions.empty())
		std::cerr << "WHAT THE FUCK BRO";

	AABB extents{};
	auto it = chunkPositions.begin();
	extents.min = it->first;
	extents.max = it->first;
	it++;

	for (; it != chunkPositions.end(); it++) {
		extents.min = glm::min(it->first, extents.min);
		extents.max = glm::max(it->first, extents.max);
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

void buildTree() {
	std::unique_lock<std::mutex> lock(loadChunkMutex);  // This may take a long time to release,
	// maybe using the condition variable would be better

	if (chunkPositions.empty()) return;

	AABB extents = calculateChunkExtents();
	glm::ivec3 size = extents.max - extents.min; // total bounding volume size

	int maxSize = std::max(std::max(size.x, size.y), size.z); // bounding cube size
	float mssb = std::ceil(std::log2f(static_cast<float>(maxSize))); // largest set bit == smallest possible cubic size
	int maxLevels = ceil_div(static_cast<int>(mssb), 2);

	accelTreeInfo.maxLevel = maxLevels;

#ifdef DEBUG_VOXELGEN
	treeLog << "Building tree:\nMin: "
		<< extents.min.x << " " << extents.min.y << " " << extents.min.z << "\nMax: "
		<< extents.max.x << " " << extents.max.y << " " << extents.max.z << "\n";
	treeLog << "Max Level: " << maxLevels << '\n';
	treeLog << "\n";
#endif

	accel.clear();
	accel.emplace_back();

	for (auto& pair : chunkPositions) {
		uint32_t node_index = 0;

		// Relative chunk position
		glm::ivec3 chunk_pos = pair.first - extents.min;

		for (int level = (maxLevels - 1) * 2; level > 0; level -= 2) {

			// 00000000
			uint8_t child_position;

			//std::cout << "Denominator: " << (node_size / 4) << "\n";

			uint8_t childPositionX = (chunk_pos.x >> level) & 0x3; //  Binary 0 - 3 value
			uint8_t childPositionZ = (chunk_pos.y >> level) & 0x3;
			uint8_t childPositionY = (chunk_pos.z >> level) & 0x3;

			child_position = childPositionX;
			child_position |= childPositionY << 2;
			child_position |= childPositionZ << 4;

			// 00zz yyxx

			Acceleration64tree& parent = accel[node_index];
			node_index = parent.children[child_position];

			// Check if a child exists
			if (!node_index) {
				node_index = accel.size();
				parent.children[child_position] = node_index;
				accel.emplace_back();
			}
		}


		uint8_t childPositionX = chunk_pos.x & 0x3; //  Binary 0 - 3 value
		uint8_t childPositionZ = chunk_pos.y & 0x3;
		uint8_t childPositionY = chunk_pos.z & 0x3;

		// Leaf node
		uint8_t child_position;
		child_position = childPositionX;
		child_position |= childPositionY << 2;
		child_position |= childPositionZ << 4;

		accel[node_index].children[child_position] = pair.second;
	}

	if (lastAccelSize != accel.capacity())
	{
		glNamedBufferData(accelTreeInfo.accelTreeBuffer, accel.capacity() * sizeof(Acceleration64tree), nullptr, GL_STATIC_DRAW);
		glNamedBufferSubData(accelTreeInfo.accelTreeBuffer, 0, accel.size() * sizeof(Acceleration64tree), accel.data());
	}
	else
		glNamedBufferSubData(accelTreeInfo.accelTreeBuffer, 0,  accel.size() * sizeof(Acceleration64tree), accel.data());

	lastAccelSize = accel.capacity();
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

			// Send chunks to buffer

			std::cout << chunk_data.size() * sizeof(Chunk) << '\n';

			processingChunks.erase(processingChunks.find(chunkToLoad));
		}
	}
}

void UpdateChunkBuffer()
{
	if (chunk_data.capacity() != lastChunkSize)
	{
		glNamedBufferData(accelTreeInfo.chunkDataBuffer, chunk_data.capacity() * sizeof(Chunk), nullptr, GL_STATIC_DRAW);
		glNamedBufferSubData(accelTreeInfo.chunkDataBuffer, 0, chunk_data.size() * sizeof(Chunk), chunk_data.data());
	}
	else
	{
		glNamedBufferSubData(accelTreeInfo.chunkDataBuffer, 0, chunk_data.size() * sizeof(Chunk), chunk_data.data());
	}
}

#endif // !CHUNK_UTIL
