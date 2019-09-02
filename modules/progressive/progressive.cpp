

#include "modules/progressive/progressive.h"

#include <iostream>
#include <memory>

using std::cout;
using std::endl;
using std::make_shared;

static bool loadingLAS = false;

// TODO bad
static ProgressiveLoader* loader = nullptr;

void binaryUploadHook(shared_ptr<BinLoadData> loadData) {

	loadData->loader->uploadNextAvailableChunk();
	loadData->loader->uploadNextAvailableChunk();
	loadData->loader->uploadNextAvailableChunk();
	loadData->loader->uploadNextAvailableChunk();
	loadData->loader->uploadNextAvailableChunk();

	schedule([loadData]() {

		if (!loadData->loader->isDone()) {
			binaryUploadHook(loadData);
		} else {
			loadData->tEndUpload = now();
			double duration = loadData->tEndUpload - loadData->tStartUpload;
			cout << "upload duration: " << duration << "s" << endl;
		}
	});
};


void uploadHook(shared_ptr<LoadData> loadData) {
	auto start = now();
	//cout << "chunks.size(): " << loader->loader->chunks.size() << endl;

	loadData->loader->uploadNextAvailableChunk();
	loadData->loader->uploadNextAvailableChunk();
	loadData->loader->uploadNextAvailableChunk();
	loadData->loader->uploadNextAvailableChunk();
	loadData->loader->uploadNextAvailableChunk();

	schedule([loadData]() {

		if (!loadData->loader->isDone()) {
			uploadHook(loadData);
		} else {
			loadData->tEndUpload = now();
			double duration = loadData->tEndUpload - loadData->tStartUpload;
			cout << "upload duration: " << duration << "s" << endl;


			//{ // OUT OF CORE - FLUSHING
			//
			//	double tStartFlush = now();
			//
			//	auto& chunks = loader->chunks;
			//
			//	for (Attribute& attribute : loader->loader->getAttributes()) {
			//
			//
			//		string file = loader->loader->file;
			//		string folder = file + "/../.progressive/";
			//		string filename = folder + attribute.name + ".bin";
			//		fs::create_directories(folder);
			//
			//		std::fstream myfile = std::fstream(filename, std::ios::out | std::ios::binary);
			//		//FILE* myfile = fopen(filename.c_str(), "wb");
			//
			//		for (auto points : chunks) {
			//			auto& as = points->attributes;
			//			auto a = std::find_if(as.begin(), as.end(), [&attribute](const Attribute& a) {return a.name == attribute.name; });
			//
			//			if (a == as.end()) {
			//				cout << "damn!" << endl;
			//			} else {
			//				BArray* data = (*a).data;
			//				//fwrite(data->data, 1, data->size, myfile);
			//				myfile.write(reinterpret_cast<const char*>(data->data), data->size);
			//			}
			//		}
			//		
			//		myfile.close();
			//		//fclose(myfile);
			//	}
			//
			//	double tEndFlush = now();
			//	double duration = tEndFlush - tStartFlush;
			//	cout << "duration(flush): " << duration << "s" << endl;
			//
			//}

			loadingLAS = false;
		}
	});

	auto duration = now() - start;
	//cout << "uploadHook(): " << duration << "s" << endl;
};

shared_ptr<LoadData> loadLasProgressive(string file) {

	//ProgressiveLoader* loader = new ProgressiveLoader(file);
	loader = new ProgressiveLoader(file);
	shared_ptr<LoadData> load = make_shared<LoadData>();
	load->tStartUpload = now();
	//load.tStart = now();

	load->loader = loader;

	uploadHook(load);

	return load;
}

shared_ptr<BinLoadData> loadBinProgressive(string file) {

	ProgressiveBINLoader* loader = new ProgressiveBINLoader(file);
	shared_ptr<BinLoadData> load = make_shared<BinLoadData>();
	load->tStartUpload = now();

	load->loader = loader;

	binaryUploadHook(load);

	return load;
}


void setAttribute(vector<SetAttributeDescriptor> attributes) {

	if (loader == nullptr) {
		return;
	}

	static atomic<int> pointsUploaded = 0;
	static atomic<int> chunkIndex = 0;

	pointsUploaded = 0;
	chunkIndex = 0;

	// TODO creating mutex pointer ... baaad
	mutex* mtx = new mutex();

	auto setAttributeTask = [attributes, mtx]() {

		auto lasloader = loader->loader;

		auto findAttribute = [lasloader](string name, Points* chunk) {

			auto attributes = chunk->attributes;

			auto it = std::find_if(attributes.begin(), attributes.end(), [name](LASLoaderThreaded::Attribute& a) {
				return a.name == name;
			});

			if (it == attributes.end()) {
				return attributes[0];
			} else {
				return *it;
			}
		};

		while (true) {

			mtx->lock();

			if (chunkIndex >= loader->chunks.size()) {
				mtx->unlock();
				break;
			}

			auto chunk = loader->chunks[chunkIndex];
			chunkIndex++;
			mtx->unlock();

			int chunkSize = chunk->size;
			void* data = malloc(chunkSize * 4);
			uint32_t* target = reinterpret_cast<uint32_t*>(data);
			uint8_t* tu8 = reinterpret_cast<uint8_t*>(target);
			uint16_t* tu16 = reinterpret_cast<uint16_t*>(target);

			int targetByteOffset = 0;
			int packing = 4;
			if (attributes.size() == 2) {
				packing = 2;
			}if (attributes.size() > 2) {
				packing = 1;
			}

			for (SetAttributeDescriptor requestedAttribute : attributes) {

				string name = requestedAttribute.name;
				

				auto attribute = findAttribute(requestedAttribute.name, chunk);

				auto source = attribute.data->data;

				if(requestedAttribute.useScaleOffset){
					
					double scale = requestedAttribute.scale;
					double offset = requestedAttribute.offset;

					if (attributes.size() == 1) {
						if (attribute.elementSize == 1) {
							ProgressiveLoader::transformAttribute<uint8_t, float>(source, target, chunkSize, scale, offset, targetByteOffset);
						} else if (attribute.elementSize == 2) {
							ProgressiveLoader::transformAttribute<uint16_t, float>(source, target, chunkSize, scale, offset, targetByteOffset);
						} else if (attribute.elementSize == 4) {
							ProgressiveLoader::transformAttribute<uint32_t, float>(source, target, chunkSize, scale, offset, targetByteOffset);
						}
					} else if (attributes.size() == 2) {
						if (attribute.elementSize == 1) {
							ProgressiveLoader::transformAttribute<uint8_t, uint16_t>(source, target, chunkSize, scale, offset, targetByteOffset);
						} else if (attribute.elementSize == 2) {
							ProgressiveLoader::transformAttribute<uint16_t, uint16_t>(source, target, chunkSize, scale, offset, targetByteOffset);
						} else if (attribute.elementSize == 4) {
							ProgressiveLoader::transformAttribute<uint32_t, uint16_t>(source, target, chunkSize, scale, offset, targetByteOffset);
						}
					} else if (attributes.size() > 2) {
						if (attribute.elementSize == 1) {
							ProgressiveLoader::transformAttribute<uint8_t, uint8_t>(source, target, chunkSize, scale, offset, targetByteOffset);
						} else if (attribute.elementSize == 2) {
							ProgressiveLoader::transformAttribute<uint16_t, uint8_t>(source, target, chunkSize, scale, offset, targetByteOffset);
						} else if (attribute.elementSize == 4) {
							ProgressiveLoader::transformAttribute<uint32_t, uint8_t>(source, target, chunkSize, scale, offset, targetByteOffset);
						}
					}
				} else if (requestedAttribute.useRange) {

					double start = requestedAttribute.rangeStart;
					double end = requestedAttribute.rangeEnd;

					if (attributes.size() == 1) {
						if (attribute.elementSize == 1) {
							ProgressiveLoader::transformAttributeRange<uint8_t, float>(source, target, chunkSize, start, end, targetByteOffset);
						} else if (attribute.elementSize == 2) {
							ProgressiveLoader::transformAttributeRange<uint16_t, float>(source, target, chunkSize, start, end, targetByteOffset);
						} else if (attribute.elementSize == 4) {
							ProgressiveLoader::transformAttributeRange<uint32_t, float>(source, target, chunkSize, start, end, targetByteOffset);
						}
					} else if (attributes.size() == 2) {
						if (attribute.elementSize == 1) {
							ProgressiveLoader::transformAttributeRange<uint8_t, uint16_t>(source, target, chunkSize, start, end, targetByteOffset);
						} else if (attribute.elementSize == 2) {
							ProgressiveLoader::transformAttributeRange<uint16_t, uint16_t>(source, target, chunkSize, start, end, targetByteOffset);
						} else if (attribute.elementSize == 4) {
							ProgressiveLoader::transformAttributeRange<uint32_t, uint16_t>(source, target, chunkSize, start, end, targetByteOffset);
						}
					} else if (attributes.size() > 2) {
						if (attribute.elementSize == 1) {
							ProgressiveLoader::transformAttributeRange<uint8_t, uint8_t>(source, target, chunkSize, start, end, targetByteOffset);
						} else if (attribute.elementSize == 2) {
							ProgressiveLoader::transformAttributeRange<uint16_t, uint8_t>(source, target, chunkSize, start, end, targetByteOffset);
						} else if (attribute.elementSize == 4) {
							ProgressiveLoader::transformAttributeRange<uint32_t, uint8_t>(source, target, chunkSize, start, end, targetByteOffset);
						}
					}
				}

				vector<int> values;
				for (int abc = 0; abc < 100; abc++) {
					values.push_back(tu8[abc]);
				}


				targetByteOffset += packing;
			}

			int offset = pointsUploaded;

			schedule([data, target, offset, chunkSize]() {
				loader->uploadChunkAttribute(target, offset, chunkSize);

				free(data);
				});

			pointsUploaded += chunkSize;

		}

		//mtx->unlock();
		//delete mtx;

	};

	thread t1(setAttributeTask);
	//thread t2(setAttributeTask);
	//thread t3(setAttributeTask);

	t1.detach();
	//t2.detach();
	//t3.detach();
}



//V8Helper::_instance->registerFunction("setAttribute", [](const FunctionCallbackInfo<Value>& args) {
//	if (args.Length() != 1) {
//		V8Helper::_instance->throwException("setAttribute requires 1 arguments");
//		return;
//	}
//
//	if (loader == nullptr) {
//		return;
//	}
//
//	Isolate* isolate = Isolate::GetCurrent();
//
//	String::Utf8Value nameUTF8(args[0]);
//	string name = *nameUTF8;
//
//	double scale = args[1]->NumberValue();
//	double offset = args[2]->NumberValue();
//
//	static atomic<int> pointsUploaded = 0;
//	static atomic<int> chunkIndex = 0;
//
//	pointsUploaded = 0;
//	chunkIndex = 0;
//
//	struct RequestedAttribute {
//		string name;
//		double scale;
//		double offset;
//	};
//
//	auto obj = args[0]->ToObject(isolate);
//	auto length = obj->Get(String::NewFromUtf8(isolate, "length"))->Uint32Value();
//	auto array = Local<Array>::Cast(args[0]);
//
//	vector<RequestedAttribute> requestedAttributes;
//
//	for (int i = 0; i < length; i++) {
//		auto obji = array->Get(i)->ToObject(isolate);
//		auto strName = String::NewFromUtf8(isolate, "name", NewStringType::kNormal).ToLocalChecked();
//		auto strScale = String::NewFromUtf8(isolate, "scale", NewStringType::kNormal).ToLocalChecked();
//		auto strOffset = String::NewFromUtf8(isolate, "offset", NewStringType::kNormal).ToLocalChecked();
//
//		Local<Value> bla = obji->Get(strName);
//		String::Utf8Value utf8Name(isolate, bla);
//
//		string name = *utf8Name;
//		double scale = obji->Get(strScale)->NumberValue();
//		double offset = obji->Get(strOffset)->NumberValue();
//
//		int asd = 10;
//
//		RequestedAttribute a{ name, scale, offset };
//		requestedAttributes.emplace_back(a);
//	}
//
//
//	// TODO baaad
//	mutex* mtx = new mutex();
//
//
//	auto setAttributeTask = [requestedAttributes, mtx]() {
//		auto lasloader = loader->loader;
//
//		auto findAttribute = [lasloader](string name, Points* chunk) {
//
//			auto attributes = chunk->attributes;
//
//			auto it = std::find_if(attributes.begin(), attributes.end(), [name](Attribute& a) {
//				return a.name == name;
//			});
//
//			if (it == attributes.end()) {
//				return attributes[0];
//			} else {
//				return *it;
//			}
//		};
//
//		//			for (auto chunk : loader->chunks) {
//
//
//					//int index = chunkIndex;
//
//		while (true) {
//
//			mtx->lock();
//
//			if (chunkIndex >= loader->chunks.size()) {
//				break;
//			}
//
//			auto chunk = loader->chunks[chunkIndex];
//			chunkIndex++;
//			mtx->unlock();
//
//			int chunkSize = chunk->size;
//			void* data = malloc(chunkSize * 4);
//			uint32_t* target = reinterpret_cast<uint32_t*>(data);
//			uint8_t* tu8 = reinterpret_cast<uint8_t*>(target);
//			uint16_t* tu16 = reinterpret_cast<uint16_t*>(target);
//
//			int targetByteOffset = 0;
//			int packing = 4;
//			if (requestedAttributes.size() == 2) {
//				packing = 2;
//			}if (requestedAttributes.size() > 2) {
//				packing = 1;
//			}
//
//			for (RequestedAttribute requestedAttribute : requestedAttributes) {
//
//				string name = requestedAttribute.name;
//				double scale = requestedAttribute.scale;
//				double offset = requestedAttribute.offset;
//
//				auto attribute = findAttribute(requestedAttribute.name, chunk);
//
//				auto source = attribute.data->data;
//
//				if (requestedAttributes.size() == 1) {
//					if (attribute.elementSize == 1) {
//						ProgressiveLoader::transformAttribute<uint8_t, float>(source, target, chunkSize, scale, offset, targetByteOffset);
//					} else if (attribute.elementSize == 2) {
//						ProgressiveLoader::transformAttribute<uint16_t, float>(source, target, chunkSize, scale, offset, targetByteOffset);
//					} else if (attribute.elementSize == 4) {
//						ProgressiveLoader::transformAttribute<uint32_t, float>(source, target, chunkSize, scale, offset, targetByteOffset);
//					}
//				} else if (requestedAttributes.size() == 2) {
//					if (attribute.elementSize == 1) {
//						ProgressiveLoader::transformAttribute<uint8_t, uint16_t>(source, target, chunkSize, scale, offset, targetByteOffset);
//					} else if (attribute.elementSize == 2) {
//						ProgressiveLoader::transformAttribute<uint16_t, uint16_t>(source, target, chunkSize, scale, offset, targetByteOffset);
//					} else if (attribute.elementSize == 4) {
//						ProgressiveLoader::transformAttribute<uint32_t, uint16_t>(source, target, chunkSize, scale, offset, targetByteOffset);
//					}
//				} else if (requestedAttributes.size() > 2) {
//					if (attribute.elementSize == 1) {
//						ProgressiveLoader::transformAttribute<uint8_t, uint8_t>(source, target, chunkSize, scale, offset, targetByteOffset);
//					} else if (attribute.elementSize == 2) {
//						ProgressiveLoader::transformAttribute<uint16_t, uint8_t>(source, target, chunkSize, scale, offset, targetByteOffset);
//					} else if (attribute.elementSize == 4) {
//						ProgressiveLoader::transformAttribute<uint32_t, uint8_t>(source, target, chunkSize, scale, offset, targetByteOffset);
//					}
//				}
//
//				vector<int> values;
//				for (int abc = 0; abc < 100; abc++) {
//					values.push_back(tu8[abc]);
//				}
//
//
//				targetByteOffset += packing;
//			}
//
//			int offset = pointsUploaded;
//
//			schedule([data, target, offset, chunkSize]() {
//				loader->uploadChunkAttribute(target, offset, chunkSize);
//
//				free(data);
//			});
//
//			pointsUploaded += chunkSize;
//
//		}
//
//		//mtx->unlock();
//		//delete mtx;
//
//	};
//
//	thread t1(setAttributeTask);
//	//thread t2(setAttributeTask);
//	//thread t3(setAttributeTask);
//
//	t1.detach();
//	//t2.detach();
//	//t3.detach();
//
//
//
//
//});