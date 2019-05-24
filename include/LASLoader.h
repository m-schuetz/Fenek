
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

namespace LASLoaderThreaded {

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

	static long long ll_start_time = std::chrono::high_resolution_clock::now().time_since_epoch().count();

	double llnow() {
		auto now = std::chrono::high_resolution_clock::now();
		long long nanosSinceStart = now.time_since_epoch().count() - ll_start_time;

		double secondsSinceStart = double(nanosSinceStart) / 1'000'000'000;

		return secondsSinceStart;
	}

	class ShuffleGenerator {

		vector<uint32_t> indices;

		// max value of uint32_t
		static const uint32_t dval = -1;

		uint32_t current = 0;

		uint32_t n = 0;

	public:

		ShuffleGenerator(uint32_t size) {
			n = size;
			indices = vector<uint32_t>(n, dval);
		}

		/// get the next value
		uint32_t getNextValue() {

			if (current >= n) {
				return dval;
			}

			uint32_t index = xorshf96() % (n - current) + current;

			uint32_t a = indices[current];
			uint32_t b = indices[index];

			a = a == dval ? current : a;
			b = b == dval ? index : b;

			indices[current] = b;
			indices[index] = a;

			current++;

			return b;
		}

		/// get the next few values
		vector<uint32_t> getNextValues(int chunkSize) {

			int start = current;
			int end = std::min(current + chunkSize, n);
			int size = end - start;

			//vector<uint32_t> values(size);
			vector<uint32_t> values;
			values.reserve(size);

			for (int i = start; i < end; i++) {
				//values[i - start] = getNextValue();
				values.emplace_back(getNextValue());
			}

			return values;
		}

		/// see 
		/// * https://stackoverflow.com/questions/1640258/need-a-fast-random-generator-for-c
		/// * https://github.com/raylee/xorshf96
		///
		/// not recommended according to the latter but will use for now until issues arise
		static uint32_t xorshf96(void) {

			static uint32_t x = 123456789, y = 362436069, z = 521288629;

			uint32_t t;
			x ^= x << 16;
			x ^= x >> 5;
			x ^= x << 1;

			t = x;
			x = y;
			y = z;
			z = t ^ x ^ y;

			return z;
		}

	};


	struct LASHeader {

		uint16_t fileSourceID = 0;
		uint16_t globalEncoding = 0;
		uint32_t project_ID_GUID_data_1 = 0;
		uint16_t project_ID_GUID_data_2 = 0;
		uint16_t project_ID_GUID_data_3 = 0;
		uint16_t project_ID_GUID_data_4 = 0;

		uint8_t versionMajor = 0;
		uint8_t versionMinor = 0;
		string systemIdentifier = "";
		string generatingSoftware = "";
		uint16_t fileCreationDay = 0;
		uint16_t fileCreationYear = 0;

		uint16_t headerSize = 0;
		uint32_t offsetToPointData = 0;
		uint32_t numVLRs = 0;
		uint8_t pointDataFormat = 0;
		uint16_t pointDataRecordLength = 0;
		uint64_t numPoints = 0;
		vector<uint32_t> numPointsPerReturn;

		double scaleX = 0;
		double scaleY = 0;
		double scaleZ = 0;

		double offsetX = 0;
		double offsetY = 0;
		double offsetZ = 0;

		double maxX = 0;
		double minX = 0;

		double maxY = 0;
		double minY = 0;

		double maxZ = 0;
		double minZ = 0;

	};

	struct XYZRGBA {
		float x;
		float y;
		float z;
		uint8_t r;
		uint8_t g;
		uint8_t b;
		uint8_t a;
	};

	struct Points {

		vector<float> position;
		vector<XYZRGBA> xyzrgba;
		vector<uint8_t> rgba;
		vector<uint32_t> shuffledOrder;
		uint32_t size = 0;

		Points() {

		}

	};

	class LASLoader {

	public:

		string file;

		LASHeader header;

		vector<char> headerBuffer;
		vector<vector<char>*> binaryChunks;
		vector<Points*> chunks;

		mutex mtx_processing_chunk;
		mutex mtc_access_chunk;
		mutex mtx_binary_chunks;

		atomic<uint64_t> numLoaded = 0;
		atomic<uint64_t> numParsed = 0;

		uint32_t defaultChunkSize = 500'000;

		ShuffleGenerator* shuffle = nullptr;

		LASLoader(string file) {
			this->file = file;

			loadHeader();

			shuffle = new ShuffleGenerator(header.numPoints);

			createBinaryLoaderThread();
			createBinaryChunkParserThread();
			//createBinaryChunkParserThread();
			//createBinaryChunkParserThread();
			//createBinaryChunkParserThread();
			//createBinaryChunkParserThread();
			//createBinaryChunkParserThread();
			//createBinaryChunkParserThread();
			//createBinaryChunkParserThread();
			//createBinaryChunkParserThread();
		}

		void waitUntilFullyParsed() {

			while (!fullyParsed()) {
				std::this_thread::sleep_for(std::chrono::milliseconds(100));

				if (hasChunkAvailable()) {
					auto chunk = getNextChunk();

					cout << "new chunk available, size: " << chunk->size << endl;
				}

				//cout << "sleep " << numLoaded << ", " << numParsed << endl;
			}

		}

		bool fullyLoaded() {
			bool result = numLoaded == header.numPoints;

			return result;
		}

		bool fullyParsed() {
			bool result = numParsed == header.numPoints;

			return result;
		}

		bool hasChunkAvailable() {
			lock_guard<mutex> lock(mtc_access_chunk);
			bool result = chunks.size() > 0;

			return result;
		}

		bool allChunksServed() {
			lock_guard<mutex> lock(mtc_access_chunk);
			bool result = fullyParsed() && chunks.size() == 0;

			return result;
		}

		Points* getNextChunk() {
			lock_guard<mutex> lock(mtc_access_chunk);

			Points* chunk = nullptr;

			if (chunks.size() > 0) {
				chunk = chunks.back();
				chunks.pop_back();
			}

			return chunk;
		}

	private:



		void loadHeader() {
			ifstream fhandle(file, ios::binary | ios::ate);
			streamsize size = fhandle.tellg();
			fhandle.seekg(0, std::ios::beg);

			int headerSize = 227;
			headerBuffer.resize(headerSize);

			if (fhandle.read(headerBuffer.data(), headerSize)) {
				cout << "header buffer loaded" << endl;
			} else {
				cout << "header buffer not loaded :x" << endl;
			}

			header.versionMajor = reinterpret_cast<uint8_t*>(headerBuffer.data() + 24)[0];
			header.versionMinor = reinterpret_cast<uint8_t*>(headerBuffer.data() + 25)[0];

			if (header.versionMajor >= 1 && header.versionMinor >= 4) {
				fhandle.seekg(0, std::ios::beg);

				headerSize = 375;
				headerBuffer.resize(headerSize);

				if (fhandle.read(headerBuffer.data(), headerSize)) {
					cout << "extended header buffer loaded" << endl;
				} else {
					cout << "extended header buffer not loaded :(" << endl;
				}
			}
			

			header.headerSize = reinterpret_cast<uint16_t*>(headerBuffer.data() + 94)[0];
			header.offsetToPointData = reinterpret_cast<uint32_t*>(headerBuffer.data() + 96)[0];
			header.pointDataFormat = reinterpret_cast<uint8_t*>(headerBuffer.data() + 104)[0];
			header.pointDataRecordLength = reinterpret_cast<uint16_t*>(headerBuffer.data() + 105)[0];
			header.numPoints = reinterpret_cast<uint32_t*>(headerBuffer.data() + 107)[0];
			header.scaleX = reinterpret_cast<double*>(headerBuffer.data() + 131)[0];
			header.scaleY = reinterpret_cast<double*>(headerBuffer.data() + 139)[0];
			header.scaleZ = reinterpret_cast<double*>(headerBuffer.data() + 147)[0];

			if (header.versionMajor >= 1 && header.versionMinor >= 4) {
				header.numPoints = reinterpret_cast<uint64_t*>(headerBuffer.data() + 247)[0];
			}

			int maxPoints = 134'000'000;
			if (header.numPoints > maxPoints) {
				cout << "#points limited to " << maxPoints << ", was " << header.numPoints << endl;
				header.numPoints = maxPoints;
			}

			cout << "header.headerSize: " << header.headerSize << endl;
			cout << "header.offsetToPointData: " << header.offsetToPointData << endl;
			cout << "header.pointDataFormat: " << header.pointDataFormat << endl;
			cout << "header.pointDataRecordLength: " << header.pointDataRecordLength << endl;
			cout << "header.numPoints: " << header.numPoints << endl;

			fhandle.close();
		}


		void createBinaryChunkParserThread() {

			thread t([this]() {

				int i = 0;

				bool done = false;
				while (!done) {
					
					mtx_binary_chunks.lock();
					if (binaryChunks.size() == 0) {
						mtx_binary_chunks.unlock();
						continue;
					}

					{
						//cout << "num chunks available: " << binaryChunks.size() << endl;
						stringstream ss;
						ss << "parsing by thread: " << std::this_thread::get_id();
						ss << ", numParsed: " << numParsed;
						ss << ", available: " << binaryChunks.size();
						ss << endl;
						cout << ss.str();
					}

					auto binaryChunk = binaryChunks.back();
					binaryChunks.pop_back();
					mtx_binary_chunks.unlock();

					i++;
					//cout << "parsing chunk[" << i << "], size: " << binaryChunk->size() << endl;

					{
						auto start = llnow();
				
						// lock mutex until parsing is done.
						// if the loading thread tries to acquire the mutex,
						// it will block until parsing is done.
						//unique_lock<mutex> lock(mtx_processing_chunk);
				
						int n = (int)binaryChunk->size() / header.pointDataRecordLength;
						Points* points = new Points();
						points->size = n;
						points->position.reserve(3 * n);
						points->rgba.reserve(4 * n);
						points->xyzrgba.reserve(n);
						points->shuffledOrder = shuffle->getNextValues(n);
				
						int positionOffset = 0;

						int rgbOffset = 20;

						if (header.pointDataFormat == 2) {
							rgbOffset = 20;
						} else if (header.pointDataFormat == 3) {
							rgbOffset = 28;
						} else if (header.pointDataFormat == 6) {
							rgbOffset = 0;
						} else if (header.pointDataFormat == 7) {
							rgbOffset = 30;
						}

						int beamVectorOffset = 36 + 2 + 2 + 2 + 4;
						int normalVectorOffset = beamVectorOffset + 3 * 2;
						
				
						for (int i = 0; i < n; i++) {
				
							int byteOffset = i * header.pointDataRecordLength;
				
							int32_t *uXYZ = reinterpret_cast<int32_t*>(binaryChunk->data() + byteOffset + positionOffset);
							uint16_t *uRGB = reinterpret_cast<uint16_t*>(binaryChunk->data() + byteOffset + rgbOffset);
							int16_t *uBeamVector = reinterpret_cast<int16_t*>(binaryChunk->data() + byteOffset + beamVectorOffset);
							int16_t *uNormalVector = reinterpret_cast<int16_t*>(binaryChunk->data() + byteOffset + normalVectorOffset);
				
							int32_t ux = uXYZ[0];
							int32_t uy = uXYZ[1];
							int32_t uz = uXYZ[2];

							float bvx = double(uBeamVector[0]) * 0.0001;
							float bvy = double(uBeamVector[1]) * 0.0001;
							float bvz = double(uBeamVector[2]) * 0.0001;

							float nvx = double(uNormalVector[0]) * 0.0001;
							float nvy = double(uNormalVector[1]) * 0.0001;
							float nvz = double(uNormalVector[2]) * 0.0001;

							XYZRGBA point;
				
							point.x = double(ux) * header.scaleX;
							point.y = double(uy) * header.scaleY;
							point.z = double(uz) * header.scaleZ;
				
							uint16_t r16 = uRGB[0];
							uint16_t g16 = uRGB[1];
							uint16_t b16 = uRGB[2];
				
							point.r = r16 / 256;
							point.g = g16 / 256;
							point.b = b16 / 256;
							point.a = 255;

							//point.r = std::max(0.0f, nvx) * 255;
							//point.g = std::max(0.0f, nvy) * 255;
							//point.b = std::max(0.0f, nvz) * 255;
				
							points->position.emplace_back(float(point.x));
							points->position.emplace_back(float(point.y));
							points->position.emplace_back(float(point.z));
				
							points->rgba.emplace_back(point.r);
							points->rgba.emplace_back(point.g);
							points->rgba.emplace_back(point.b);
							points->rgba.emplace_back(point.a);

							points->xyzrgba.emplace_back(point);
						}
				
						mtc_access_chunk.lock();
						//cout << "chunk parsed by thread: " << std::this_thread::get_id() << ", numParsed: " << numParsed << endl;
						chunks.emplace_back(points);
						mtc_access_chunk.unlock();
				
						numParsed += n;
				
						delete binaryChunk;
				
						//std::this_thread::sleep_for(std::chrono::milliseconds(200));
				
						auto end = llnow();
						auto duration = end - start;
						//cout << "process duration: " << duration << "s" << endl;
					}





					//cout << "lock" << endl;
					mtx_binary_chunks.lock();
					//cout << "locked" << endl;
					done = fullyLoaded() && binaryChunks.size() == 0;
					//cout << "done: " << (done ? "true" : "false") << endl; 
					mtx_binary_chunks.unlock();
					//cout << "unlock" << endl;
				}

				cout << "done parsing binary chunks" << endl;

			});
			t.detach();
		}

		void createBinaryLoaderThread() {

			thread t([this]() {
				double start = llnow();

				uint64_t offset = header.offsetToPointData;
				uint64_t pointsLoaded = 0;
				uint64_t bytes = header.numPoints * header.pointDataRecordLength;

				ifstream handle(file, ios::binary | ios::ate);
				streamsize size = handle.tellg();
				handle.seekg(offset, ios::beg);

				while (handle.good()) {

					uint32_t chunkSizePoints = (uint32_t)min(uint64_t(defaultChunkSize), header.numPoints - pointsLoaded);
					uint32_t chunkSizeBytes = chunkSizePoints * header.pointDataRecordLength;

					vector<char> *chunkBuffer = new vector<char>(chunkSizeBytes);
					//char* chunkBuffer = reinterpret_cast<char*>(malloc(chunkSizeBytes));
					handle.read(chunkBuffer->data(), chunkSizeBytes);
					//handle.read(chunkBuffer, chunkSizeBytes);

					mtx_binary_chunks.lock();
					binaryChunks.emplace_back(chunkBuffer);
					mtx_binary_chunks.unlock();

					offset += chunkSizeBytes;
					pointsLoaded += chunkSizePoints;
					numLoaded = pointsLoaded;

					//if ((pointsLoaded % 10'000'000) == 0) {
					//	cout << pointsLoaded << endl;
					//}

					// block if a chunk is already being parsed
					// otherwise, start parsing it.
					//mtx_processing_chunk.lock();
					//mtx_processing_chunk.unlock();
					//parseBinaryChunk(chunkBuffer, chunkSizeBytes);

					

					//int sum = 0;

					//for (int nt = 0; nt < 10; nt++) {
					//	thread tpc([&sum]() {

					//		sum++;

					//	});
					//	tpc.detach();
					//}

					//cout << "sum: " << sum << endl;

					if (pointsLoaded >= header.numPoints) {
						break;
					}

				}

				cout << pointsLoaded << endl;

				double end = llnow();
				double duration = end - start;

				cout << "done loading binary chunks" << endl;
				cout << "duration: " << duration << endl;

			});

			t.detach();



			//vector<char> chunkBuffer(chunkSizeBytes);

		}



	};

}