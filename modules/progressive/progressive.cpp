

#include "modules/progressive/progressive.h"

#include <iostream>

using std::cout;
using std::endl;

static double startUpload = 0;
static double endUpload = 0;
static bool loadingLAS = false;

//ProgressiveLoader* loader = nullptr;


void uploadHook(LoadData loadData) {
	auto start = now();
	//cout << "chunks.size(): " << loader->loader->chunks.size() << endl;

	loadData.loader->uploadNextAvailableChunk();
	loadData.loader->uploadNextAvailableChunk();
	loadData.loader->uploadNextAvailableChunk();
	loadData.loader->uploadNextAvailableChunk();
	loadData.loader->uploadNextAvailableChunk();

	schedule([loadData]() {

		if (!loadData.loader->isDone()) {
			uploadHook(loadData);
		} else {
			endUpload = now();
			double duration = endUpload - startUpload;
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

LoadData loadLasProgressive(string file) {

	ProgressiveLoader* loader = new ProgressiveLoader(file);
	LoadData load;
	//load.tStart = now();

	load.loader = loader;

	uploadHook(load);

	return load;
}
//
//V8Helper::_instance->registerFunction("loadLASProgressive", [](const FunctionCallbackInfo<Value>& args) {
//	if (args.Length() != 1) {
//		V8Helper::_instance->throwException("loadLASProgressive requires 1 arguments");
//		return;
//	}
//
//	String::Utf8Value fileUTF8(args[0]);
//	string file = *fileUTF8;
//
//	startUpload = now();
//	loadingLAS = true;
//
//
//	loader = new ProgressiveLoader(file);
//	auto duration = now() - startUpload;
//	cout << "loader created after " << duration << "s" << endl;
//
//	auto isolate = Isolate::GetCurrent();
//	Local<ObjectTemplate> lasTempl = ObjectTemplate::New(isolate);
//	auto objLAS = lasTempl->NewInstance();
//
//	auto lNumPoints = v8::Integer::New(isolate, loader->loader->header.numPoints);
//
//	auto lHandles = Array::New(isolate, loader->ssVertexBuffers.size());
//	for (int i = 0; i < loader->ssVertexBuffers.size(); i++) {
//		auto lHandle = v8::Integer::New(isolate, loader->ssVertexBuffers[i]);
//		lHandles->Set(i, lHandle);
//	}
//	objLAS->Set(String::NewFromUtf8(isolate, "handles"), lHandles);
//	objLAS->Set(String::NewFromUtf8(isolate, "numPoints"), lNumPoints);
//
//	{
//		Local<ObjectTemplate> boxTempl = ObjectTemplate::New(isolate);
//		auto objBox = lasTempl->NewInstance();
//
//		auto& header = loader->loader->header;
//
//		auto lMin = Array::New(isolate, 3);
//		lMin->Set(0, v8::Number::New(isolate, header.minX));
//		lMin->Set(1, v8::Number::New(isolate, header.minY));
//		lMin->Set(2, v8::Number::New(isolate, header.minZ));
//
//		auto lMax = Array::New(isolate, 3);
//		lMax->Set(0, v8::Number::New(isolate, header.maxX));
//		lMax->Set(1, v8::Number::New(isolate, header.maxY));
//		lMax->Set(2, v8::Number::New(isolate, header.maxZ));
//
//		objBox->Set(String::NewFromUtf8(isolate, "min"), lMin);
//		objBox->Set(String::NewFromUtf8(isolate, "max"), lMax);
//
//		objLAS->Set(String::NewFromUtf8(isolate, "boundingBox"), objBox);
//	}
//
//
//	auto pObjLAS = Persistent<Object, CopyablePersistentTraits<Object>>(isolate, objLAS);
//
//	duration = now() - startUpload;
//	//cout << "uploadHook after " << duration << "s" << endl;
//
//	uploadHook(loader, pObjLAS);
//
//	duration = now() - startUpload;
//	//cout << "returning value after " << duration << "s" << endl;
//
//	args.GetReturnValue().Set(objLAS);
//});




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