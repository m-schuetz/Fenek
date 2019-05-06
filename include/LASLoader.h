
#include <string>
#include <vector>
#include <fstream>
#include <chrono>
#include <thread>
#include <mutex>
#include <atomic>
#include <iostream>
#include <algorithm>

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

	static long long ll_start_time = std::chrono::high_resolution_clock::now().time_since_epoch().count();

	double llnow() {
		auto now = std::chrono::high_resolution_clock::now();
		long long nanosSinceStart = now.time_since_epoch().count() - ll_start_time;

		double secondsSinceStart = double(nanosSinceStart) / 1'000'000'000;

		return secondsSinceStart;
	}

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
		uint32_t numPoints = 0;
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

	struct Points {

		vector<float> position;
		//vector<double> position;
		vector<uint8_t> rgba;
		uint32_t size = 0;

		Points() {

		}

	};

	class LASLoader {

	public:

		string file;

		LASHeader header;

		vector<char> headerBuffer;
		vector<Points> chunks;

		mutex mtx_processing_chunk;
		mutex mtc_access_chunk;

		atomic<uint64_t> numLoaded = 0;
		atomic<uint64_t> numParsed = 0;

		uint32_t defaultChunkSize = 10'000'000;

		LASLoader(string file) {
			this->file = file;

			loadHeader();

			loadPointRecordBuffers();
		}

		void waitUntilFullyParsed() {

			while (!fullyParsed()) {
				std::this_thread::sleep_for(std::chrono::milliseconds(100));

				if (hasChunkAvailable()) {
					auto chunk = getNextChunk();

					cout << "new chunk available, size: " << chunk.size << endl;
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

		Points getNextChunk() {
			lock_guard<mutex> lock(mtc_access_chunk);

			Points chunk;

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

			header.headerSize = reinterpret_cast<uint16_t*>(headerBuffer.data() + 94)[0];
			header.offsetToPointData = reinterpret_cast<uint32_t*>(headerBuffer.data() + 96)[0];
			header.pointDataFormat = reinterpret_cast<uint8_t*>(headerBuffer.data() + 104)[0];
			header.pointDataRecordLength = reinterpret_cast<uint16_t*>(headerBuffer.data() + 105)[0];
			header.numPoints = reinterpret_cast<uint32_t*>(headerBuffer.data() + 107)[0];
			header.numPoints = reinterpret_cast<uint32_t*>(headerBuffer.data() + 107)[0];

			cout << "header.headerSize: " << header.headerSize << endl;
			cout << "header.offsetToPointData: " << header.offsetToPointData << endl;
			cout << "header.pointDataFormat: " << header.pointDataFormat << endl;
			cout << "header.pointDataRecordLength: " << header.pointDataRecordLength << endl;
			cout << "header.numPoints: " << header.numPoints << endl;

			fhandle.close();
		}

		void parseBinaryChunk(vector<char> *chunkBuffer) {

			thread t([this, chunkBuffer]() {

				auto start = llnow();

				// lock mutex until parsing is done.
				// if the loading thread tries to acquire the mutex,
				// it will block until parsing is done.
				unique_lock<mutex> lock(mtx_processing_chunk);

				int n = (int)chunkBuffer->size() / header.pointDataRecordLength;
				Points points;
				points.size = n;
				points.position.reserve(3 * n);
				points.rgba.reserve(4 * n);

				int positionOffset = 0;
				int rgbOffset = 20; // format 2

				for (int i = 0; i < n; i++) {

					int32_t *uXYZ = reinterpret_cast<int32_t*>(chunkBuffer->data() + positionOffset);
					uint16_t *uRGB = reinterpret_cast<uint16_t*>(chunkBuffer->data() + rgbOffset);

					int32_t ux = uXYZ[0];
					int32_t uy = uXYZ[1];
					int32_t uz = uXYZ[2];

					double x = double(ux) * header.scaleX;
					double y = double(uy) * header.scaleY;
					double z = double(uz) * header.scaleZ;

					uint16_t r16 = uRGB[0];
					uint16_t g16 = uRGB[1];
					uint16_t b16 = uRGB[2];

					uint8_t r = r16 / 256;
					uint8_t g = g16 / 256;
					uint8_t b = b16 / 256;
					uint8_t a = 255;

					points.position.emplace_back(float(x));
					points.position.emplace_back(float(y));
					points.position.emplace_back(float(z));

					points.rgba.emplace_back(r);
					points.rgba.emplace_back(g);
					points.rgba.emplace_back(b);
					points.rgba.emplace_back(a);
				}

				mtc_access_chunk.lock();
				chunks.emplace_back(points);
				mtc_access_chunk.unlock();

				numParsed += n;

				delete chunkBuffer;

				auto end = llnow();
				auto duration = end - start;
				cout << "process duration: " << duration << "s" << endl;

			});
			t.detach();

		}

		void loadPointRecordBuffers() {

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
					handle.read(chunkBuffer->data(), chunkSizeBytes);

					offset += chunkSizeBytes;
					pointsLoaded += chunkSizePoints;
					numLoaded = pointsLoaded;

					if ((pointsLoaded % 10'000'000) == 0) {
						cout << pointsLoaded << endl;
					}

					// block if a chunk is already being parsed
					// otherwise, start parsing it.
					mtx_processing_chunk.lock();
					mtx_processing_chunk.unlock();
					parseBinaryChunk(chunkBuffer);

					if (pointsLoaded >= header.numPoints) {
						break;
					}

				}

				cout << pointsLoaded << endl;

				double end = llnow();
				double duration = end - start;

				cout << "duration: " << duration << endl;

			});

			t.detach();



			//vector<char> chunkBuffer(chunkSizeBytes);

		}



	};

}