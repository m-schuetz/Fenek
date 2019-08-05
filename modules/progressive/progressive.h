
#pragma once

#include "v8.h"

#include "modules/progressive/ProgressiveLoader.h"

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

	LoadData() {

	}

};



void uploadHook(LoadData loadData);

LoadData loadLasProgressive(string file);

