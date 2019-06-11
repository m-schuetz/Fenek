
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

	// see https://en.wikipedia.org/wiki/Primality_test
	bool isPrime(uint64_t n) {
		if (n <= 3) {
			return n > 1;
		} else if ((n % 2) == 0 || (n % 3) == 0) {
			return false;
		}

		uint64_t i = 5;
		while ((i * i) <= n) {
			if ((n % i) == 0 || (n % (i + 2)) == 0) {
				return false;
			}


			i = i + 6;
		}

		return true;
	}

	//
	// Primes where p = 3 mod 4 allow us to generate random numbers without duplicates in range [0, prime - 1]
	// https://preshing.com/20121224/how-to-generate-a-sequence-of-unique-random-integers/
	uint64_t previousPrimeCongruent3mod4(uint64_t start) {
		for (uint64_t i = start -1; true; i--) {
			if ((i % 4) == 3 && isPrime(i)) {
				return i;
			}
		}
	}

public:

	LASLoader* loader = nullptr;
	uint32_t prime = 0;
	
	vector<GLuint> ssVertexBuffers;
	GLuint ssChunk16B = -1;

	uint32_t pointsUploaded;
	int bytePerPoint = 16;
	vector<Points*> chunks;
	ComputeShader* csDistribute = nullptr;

	int maxPointsPerBuffer = 134'000'000;

	ProgressiveLoader(string path) {

		loader = new LASLoader(path);
		prime = previousPrimeCongruent3mod4(loader->header.numPoints);

		int numBuffers = (loader->header.numPoints / maxPointsPerBuffer) + 1;
		if ((loader->header.numPoints % maxPointsPerBuffer) == 0) {
			numBuffers = numBuffers - 1;
		}

		glCreateBuffers(1, &ssChunk16B);

		GLbitfield usage = GL_DYNAMIC_DRAW;

		int pointsLeft = loader->header.numPoints;
		for (int i = 0; i < numBuffers; i++) {
			int numPointsInBuffer = pointsLeft > maxPointsPerBuffer ? maxPointsPerBuffer : pointsLeft;

			GLuint ssVertexBuffer;

			glCreateBuffers(1, &ssVertexBuffer);
			uint32_t size = numPointsInBuffer * bytePerPoint;
			
			glNamedBufferData(ssVertexBuffer, size, nullptr, usage);

			ssVertexBuffers.emplace_back(ssVertexBuffer);

			pointsLeft = pointsLeft - numPointsInBuffer;
		}
		

		uint32_t chunkSize = loader->defaultChunkSize * 16;
		glNamedBufferData(ssChunk16B, chunkSize, nullptr, usage);

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

		int chunkSize = chunk->size;

		//{
		//	GLint64 max;
		//	glGetInteger64v(GL_MAX_SHADER_STORAGE_BLOCK_SIZE, &max);

		//	cout << "!!!!!!!!!!!!!  " << max << "   !!!!!!!!!!!!!!!!!" << endl;
		//}

		{// upload
			glNamedBufferSubData(ssChunk16B, 0, chunkSize * 16, chunk->xyzrgba.data());
			//glNamedBufferSubData(ssChunkIndices, 0, chunkSize * 4, chunk->shuffledOrder.data());
		}

		{// distribute to shuffled location
			glUseProgram(csDistribute->program);
			
			GLuint ssInput = ssChunk16B;

			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssInput);

			for(int i = 0; i < ssVertexBuffers.size(); i++){
				glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2 + i, ssVertexBuffers[i]);
			}

			//glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, ssVertexBuffers[0]);
			//glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, ssVertexBuffers[1]);

			auto uLocation = csDistribute->uniformLocations["uNumPoints"];
			glUniform1i(uLocation, chunkSize);

			auto uPrime = csDistribute->uniformLocations["uPrime"];
			glUniform1d(uPrime, double(prime));

			auto uOffset = csDistribute->uniformLocations["uOffset"];
			glUniform1i(uOffset, pointsUploaded);

			int groups = ceil(double(chunkSize) / 32.0);
			glDispatchCompute(groups, 1, 1);

			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);

			for (int i = 0; i < ssVertexBuffers.size(); i++) {
				glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2 + i, 0);
			}
			//glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, 0);
			//glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, 0);
			
			glUseProgram(0);

			glMemoryBarrier(GL_ALL_BARRIER_BITS);
		}

		chunks.emplace_back(chunk);

		pointsUploaded += chunk->size;

		return chunk->size;
	}

	void uploadChunk(void* data, int offset, int size) {

		int targetOffset = offset;
		int chunkSize = size;

		{// upload
			glNamedBufferSubData(ssChunk16B, 0, chunkSize * 16, data);
			//glNamedBufferSubData(ssChunkIndices, 0, chunkSize * 4, chunk->shuffledOrder.data());
		}

		{// distribute to shuffled location
			glUseProgram(csDistribute->program);

			int bufferIndex = pointsUploaded / maxPointsPerBuffer;

			GLuint ssInput = ssChunk16B;
			GLuint ssTarget = ssVertexBuffers[bufferIndex];

			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssInput);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, ssTarget);

			auto uLocation = csDistribute->uniformLocations["uNumPoints"];
			glUniform1i(uLocation, chunkSize);

			auto uPrime = csDistribute->uniformLocations["uPrime"];
			glUniform1d(uPrime, double(prime));

			auto uOffset = csDistribute->uniformLocations["uOffset"];
			glUniform1i(uOffset, targetOffset);

			int groups = ceil(double(chunkSize) / 32.0);
			glDispatchCompute(groups, 1, 1);

			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, 0);

			glUseProgram(0);

			glMemoryBarrier(GL_ALL_BARRIER_BITS);
		}

	}

};

