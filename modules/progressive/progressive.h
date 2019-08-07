
#pragma once

#include "v8.h"

#include "modules/progressive/ProgressiveLoader.h"
#include "modules/progressive/ProgressiveBINLoader.h"

using v8::CopyablePersistentTraits;
using v8::Persistent;
using v8::Object;
using v8::Array;
using v8::Isolate;
using v8::ObjectTemplate;
using v8::Local;


class LoadData {
public:

	ProgressiveLoader* loader = nullptr;
	double tStartUpload = 0;
	double tEndUpload = 0;

	LoadData() {

	}

};


class BinLoadData {
public:

	ProgressiveBINLoader* loader = nullptr;
	double tStartUpload = 0;
	double tEndUpload = 0;

	BinLoadData() {

	}

};


void uploadHook(shared_ptr<LoadData> loadData);

shared_ptr<LoadData> loadLasProgressive(string file);


void binaryUploadHook(BinLoadData* loader);

shared_ptr<BinLoadData> loadBinProgressive(string file);

