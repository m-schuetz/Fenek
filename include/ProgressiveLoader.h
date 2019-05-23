
#pragma once

#include <string>
#include <vector>
#include <fstream>
#include <chrono>
#include <thread>
#include <mutex>
#include <atomic>
#include <iostream>
#include <algorithm>
#include <sstream>

#include "GL\glew.h"
#include "GLFW\glfw3.h"

#include "LASLoader.h"
#include "ComputeShader.h"

using std::string;
using std::vector;
using std::ifstream;
using std::ios;
using std::cout;
using std::endl;
using std::streamsize;
using std::thread;
using std::mutex;
using std::unique_lock;
using std::lock_guard;
using std::atomic;
using std::min;
using std::stringstream;
using LASLoaderThreaded::LASLoader;
using LASLoaderThreaded::Points;


class ProgressiveLoader {

public:

	LASLoader* loader = nullptr;
	
	GLuint ssVertexBuffer = -1;
	GLuint ssChunk16B = -1;
	GLuint ssChunkIndices = -1;

	uint32_t pointsUploaded;
	int bytePerPoint = 16;
	vector<Points*> chunks;
	ComputeShader* csDistribute = nullptr;

	ProgressiveLoader(string path) {

		loader = new LASLoader(path);

		glCreateBuffers(1, &ssVertexBuffer);
		glCreateBuffers(1, &ssChunk16B);
		glCreateBuffers(1, &ssChunkIndices);

		uint32_t size = loader->header.numPoints * bytePerPoint;
		GLbitfield usage = GL_STREAM_DRAW;
		glNamedBufferData(ssVertexBuffer, size, nullptr, usage);

		uint32_t chunkSize = loader->defaultChunkSize * 16;
		glNamedBufferData(ssChunk16B, chunkSize, nullptr, usage);

		uint32_t chunkIndicesSize = loader->defaultChunkSize * 4;
		glNamedBufferData(ssChunkIndices, chunkIndicesSize, nullptr, usage);

		string csPath = "../../resources/shaders/pcp/distribute.cs";
		csDistribute = new ComputeShader(csPath);

	}

	bool isDone() {
		return loader->allChunksServed();
	}

	///
	/// upload a chunk, if available, and return the number of uploaded points.
	/// returns 0, if no chunk was uploaded.
	///
	uint32_t uploadNextAvailableChunk() {
		Points* chunk = loader->getNextChunk();

		if (chunk == nullptr) {
			return 0;
		}

		int targetOffset = pointsUploaded * bytePerPoint;
		int chunkSize = chunk->size;

		{// upload
			glNamedBufferSubData(ssChunk16B, 0, chunkSize * 16, chunk->xyzrgba.data());
			glNamedBufferSubData(ssChunkIndices, 0, chunkSize * 4, chunk->shuffledOrder.data());
		}

		{// distribute to shuffled location
			glUseProgram(csDistribute->program);

			GLuint ssInput = ssChunk16B;
			GLuint ssTarget = ssVertexBuffer;
			GLuint ssIndices = ssChunkIndices;

			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssInput);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ssIndices);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, ssTarget);

			auto uLocation = csDistribute->uniformLocations["uNumPoints"];
			glUniform1i(uLocation, chunkSize);
			int groups = ceil(double(chunkSize) / 32.0);
			glDispatchCompute(groups, 1, 1);
			
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, 0);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, 0);
			
			glUseProgram(0);

			glMemoryBarrier(GL_ALL_BARRIER_BITS);
		}

		chunks.emplace_back(chunk);

		pointsUploaded += chunk->size;

		return chunk->size;
	}

};

