
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
#include <future>
#include <cstdio>
#include <filesystem>
#include <Windows.h>

namespace fs = std::experimental::filesystem;

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

	struct BArray {
		void* data = nullptr;
		uint8_t* dataU8 = nullptr;
		uint64_t size = 0;

		BArray(uint64_t size) {
			this->data = malloc(size);
			this->dataU8 = reinterpret_cast<uint8_t*>(this->data);
			this->size = size;
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

	struct VariableLengthRecord{
		string userID;
		uint16_t recordID = 0;
		uint16_t recordLengthAfterHeader = 0;
		string description;

		vector<char> buffer;
	};

	struct ExtraBytes{

		uint8_t reserved[2];
		uint8_t dataType;
		uint8_t options;
		int8_t name[32];
		uint8_t unused[4];
		uint8_t noData[24];
		uint8_t min[24];
		uint8_t max[24];
		double scale[3];
		double offset[3];
		int8_t description[32];

		vector<uint64_t> noDataU64(){
			vector<uint64_t> value = {
				reinterpret_cast<uint64_t*>(noData)[0],
				reinterpret_cast<uint64_t*>(noData)[1],
				reinterpret_cast<uint64_t*>(noData)[2]
			};

			return value;
		}

		vector<int64_t> noDataI64(){
			vector<int64_t> value = {
				reinterpret_cast<int64_t*>(noData)[0],
				reinterpret_cast<int64_t*>(noData)[1],
				reinterpret_cast<int64_t*>(noData)[2]
			};

			return value;
		}

		vector<double> noDataDouble(){
			vector<double> value = {
				reinterpret_cast<double*>(noData)[0],
				reinterpret_cast<double*>(noData)[1],
				reinterpret_cast<double*>(noData)[2]
			};

			return value;
		}

		vector<uint64_t> minU64(){
			vector<uint64_t> value = {
				reinterpret_cast<uint64_t*>(min)[0],
				reinterpret_cast<uint64_t*>(min)[1],
				reinterpret_cast<uint64_t*>(min)[2]
			};

			return value;
		}

		vector<int64_t> minI64(){
			vector<int64_t> value = {
				reinterpret_cast<int64_t*>(min)[0],
				reinterpret_cast<int64_t*>(min)[1],
				reinterpret_cast<int64_t*>(min)[2]
			};

			return value;
		}

		vector<double> minDouble(){
			vector<double> value = {
				reinterpret_cast<double*>(min)[0],
				reinterpret_cast<double*>(min)[1],
				reinterpret_cast<double*>(min)[2]
			};

			return value;
		}

		vector<uint64_t> maxU64(){
			vector<uint64_t> value = {
				reinterpret_cast<uint64_t*>(max)[0],
				reinterpret_cast<uint64_t*>(max)[1],
				reinterpret_cast<uint64_t*>(max)[2]
			};

			return value;
		}

		vector<int64_t> maxI64(){
			vector<int64_t> value = {
				reinterpret_cast<int64_t*>(max)[0],
				reinterpret_cast<int64_t*>(max)[1],
				reinterpret_cast<int64_t*>(max)[2]
			};

			return value;
		}

		vector<double> maxDouble(){
			vector<double> value = {
				reinterpret_cast<double*>(max)[0],
				reinterpret_cast<double*>(max)[1],
				reinterpret_cast<double*>(max)[2]
			};

			return value;
		}

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
		//vector<uint32_t> shuffledOrder;
		uint32_t size = 0;

		Points() {

		}

	};

	class LASLoader {

	public:

		string file;

		LASHeader header;
		vector<VariableLengthRecord> variableLengthRecords;

		vector<char> headerBuffer;
		vector<BArray*> binaryChunks;
		vector<Points*> chunks;

		mutex mtx_processing_chunk;
		mutex mtc_access_chunk;
		mutex mtx_binary_chunks;

		atomic<uint64_t> numLoaded = 0;
		atomic<uint64_t> numParsed = 0;

		uint32_t defaultChunkSize = 500'000;

		//ShuffleGenerator* shuffle = nullptr;

		LASLoader(string file) {
			this->file = file;

			loadHeader();
			loadVariableLengthRecords();

			//shuffle = new ShuffleGenerator(header.numPoints);

			createBinaryLoaderThread();
			createBinaryChunkParserThread();
			createBinaryChunkParserThread();
			createBinaryChunkParserThread();
			createBinaryChunkParserThread();
			//createBinaryChunkParserThread();
			//createBinaryChunkParserThread();
			//createBinaryChunkParserThread();
			//createBinaryChunkParserThread();
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
			
			// TODO probably should use that instead of hardcoding 227 and 375?
			header.headerSize = reinterpret_cast<uint16_t*>(headerBuffer.data() + 94)[0];

			header.offsetToPointData = reinterpret_cast<uint32_t*>(headerBuffer.data() + 96)[0];
			header.numVLRs = reinterpret_cast<uint32_t*>(headerBuffer.data() + 100)[0];
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

		void loadVariableLengthRecords(){
			ifstream fhandle(file, ios::binary | ios::ate);
			streamsize size = fhandle.tellg();

			//auto setReadPos = [&fhandle](int offset){fhandle.seekg(offset, std::ios::beg);};

			auto readBytes = [&fhandle](int offset, int length){
				fhandle.seekg(offset, std::ios::beg);
				vector<char> data(length);

				fhandle.read(data.data(), length);

				return data;
			};

			int vlrHeaderSize = 54;
			int offset = header.headerSize;

			for(int i = 0; i < header.numVLRs; i++){
				//fhandle.seekg(offset, std::ios::beg);
				//setReadPos(offset);

				VariableLengthRecord vlr;

				vector<char> buffer = readBytes(offset, vlrHeaderSize);

				vlr.userID = string(buffer.begin() + 2, buffer.begin() + 2 + 16);
				vlr.recordID = reinterpret_cast<uint16_t*>(buffer.data() + 18)[0];
				vlr.recordLengthAfterHeader = reinterpret_cast<uint16_t*>(buffer.data() + 20)[0];
				vlr.description = string(buffer.begin() + 22, buffer.begin() + 22 + 32);

				vlr.buffer = readBytes(offset + vlrHeaderSize, vlr.recordLengthAfterHeader);

				variableLengthRecords.emplace_back(vlr);
				
				offset += 54 + vlr.recordLengthAfterHeader;
			}

			for(VariableLengthRecord &vlr : variableLengthRecords){
				cout << "==== VLR start ===" << endl;

				cout << "description: " <<  vlr.description << endl;
				cout << "length: " <<  vlr.recordLengthAfterHeader << endl;
				cout << "recordID: " <<  vlr.recordID << endl;


				cout << "==== VLR end ===" << endl;
			}
		}

		int getOffsetIntensity() {
			return 12;
		}

		int getOffsetReturnNumber() {
			return 14;
		}

		int getOffsetClassification() {
			int format = header.pointDataFormat;

			if (format <= 5) {
				return 15;
			} else if(format == 6 || format == 7){
				return 16;
			}

			return 0;
		}

		int getOffsetPointSourceID() {
			int format = header.pointDataFormat;

			if (format <= 5) {
				return 18;
			} else if (format == 6 || format == 7) {
				return 20;
			}

			return 0;
		}

		int getOffsetRGB() {
			int format = header.pointDataFormat;

			if (format == 2) {
				return 20;
			} else if (format == 3) {
				return 28;
			} else if (format == 5) {
				return 28;
			} else if (format == 7) {
				return 30;
			}

			return 0;
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

					//{
					//	//cout << "num chunks available: " << binaryChunks.size() << endl;
					//	stringstream ss;
					//	ss << "parsing by thread: " << std::this_thread::get_id();
					//	ss << ", numParsed: " << numParsed;
					//	ss << ", available: " << binaryChunks.size();
					//	ss << endl;
					//	cout << ss.str();
					//}

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
				
						int n = binaryChunk->size / uint64_t(header.pointDataRecordLength);
						Points* points = new Points();
						points->size = n;
						points->position.reserve(3 * n);
						points->rgba.reserve(4 * n);
						points->xyzrgba.reserve(n);

						int positionOffset = 0;

						int offsetRGB = getOffsetRGB();
						int offsetIntensity = getOffsetIntensity();

						int beamVectorOffset = 36 + 2 + 2 + 2 + 4;
						int normalVectorOffset = beamVectorOffset + 3 * 2;
						
				
						for (int i = 0; i < n; i++) {
				
							int byteOffset = i * header.pointDataRecordLength;
				
							int32_t *uXYZ = reinterpret_cast<int32_t*>(binaryChunk->dataU8 + byteOffset + positionOffset);
							uint16_t *uRGB = reinterpret_cast<uint16_t*>(binaryChunk->dataU8 + byteOffset + offsetRGB);
							uint16_t *uIntensity = reinterpret_cast<uint16_t*>(binaryChunk->dataU8 + byteOffset + offsetIntensity);
							int16_t *uBeamVector = reinterpret_cast<int16_t*>(binaryChunk->dataU8 + byteOffset + beamVectorOffset);
							int16_t *uNormalVector = reinterpret_cast<int16_t*>(binaryChunk->dataU8 + byteOffset + normalVectorOffset);
				
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

				//ifstream handle(file, ios::binary | ios::ate);
				//streamsize size = handle.tellg();
				//handle.seekg(offset, ios::beg);

				{ // disable windows file cache for benchmarking
					LPCTSTR lfile = file.c_str();

					auto hFile = CreateFile(lfile, GENERIC_READ,
						FILE_SHARE_READ,
						NULL, OPEN_EXISTING,
						FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN
						| FILE_FLAG_NO_BUFFERING, NULL);
				}

				FILE* in = fopen(file.c_str(), "rb");
				_fseeki64(in, offset, ios::beg);
				auto size = fs::file_size(file);

				bool done = false;
				while(!done){

					uint32_t chunkSizePoints = (uint32_t)min(uint64_t(defaultChunkSize), header.numPoints - pointsLoaded);
					uint32_t chunkSizeBytes = chunkSizePoints * header.pointDataRecordLength;

					//vector<char> *chunkBuffer = new vector<char>(chunkSizeBytes);
					//handle.read(chunkBuffer->data(), chunkSizeBytes);

					BArray* chunkBuffer = new BArray(chunkSizeBytes);
					auto bytesRead = fread(chunkBuffer->data, 1, chunkSizeBytes, in);

					done = bytesRead == 0;

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