//
//#include <iostream>
//#include <chrono>
//#include <regex>
//#include <unordered_map>
//#include <vector>
//#include <filesystem>
//#include <cstdlib>
//#include <iomanip>
//#include <string>
//#include <sstream>
//
//#include "libplatform/libplatform.h"
//#include "v8.h"
//
//using std::unordered_map;
//using std::vector;
//using std::cout;
//using std::endl;
//using std::chrono::high_resolution_clock;
//using std::chrono::duration;
//using std::chrono::duration_cast;
//using std::string;
//using std::regex;
//using std::smatch;
//using std::regex_match;
//using std::stringstream;
//using std::ssub_match;
//
//using v8::Isolate;
//using v8::ArrayBuffer;
//using v8::Local;
//using v8::HandleScope;
//using v8::ObjectTemplate;
//using v8::FunctionTemplate;
//using v8::V8;
//using v8::Platform;
//using v8::Handle;
//using v8::Context;
//using v8::String;
//using v8::Value;
//using v8::FunctionCallbackInfo;
//using v8::PropertyCallbackInfo;
//using v8::NewStringType;
//using v8::Script;
//using v8::Persistent;
//using v8::Object;
//using v8::External;
//using v8::EscapableHandleScope;
//using v8::Array;
//using v8::Uint8Array;
//using v8::Float32Array;
//using v8::Float64Array;
//using v8::TypedArray;
////using v8::NamedPropertyGetterCallback;
//using v8::Promise;
//using v8::CopyablePersistentTraits;
//using v8::Function;
//using v8::Number;
//using v8::TryCatch;
//
//namespace fs = std::experimental::filesystem;
//
//class ArrayBufferAllocator : public v8::ArrayBuffer::Allocator {
//public:
//	virtual void* Allocate(size_t length) {
//		void* data = AllocateUninitialized(length);
//		return data == NULL ? data : memset(data, 0, length);
//	}
//	virtual void* AllocateUninitialized(size_t length) { return malloc(length); }
//	virtual void Free(void* data, size_t) { free(data); }
//};
//
//struct Scope {
//	Isolate::Scope iscope;
//	HandleScope hscope;
//
//	Scope(Isolate *isolate) : iscope(isolate), hscope(isolate) {
//
//	}
//};
//
//int getErrorLine(string traceStr) {
//
//	std::istringstream ss(traceStr.c_str());
//
//	regex pattern(".*at .*:(\\d*):.\\d*");
//	smatch result;
//
//	string line;
//	while (std::getline(ss, line, '\n')) {
//		if (regex_match(line, result, pattern)) {
//			ssub_match match_line = result[1];
//			int line = std::stoi(match_line);
//
//			return line - 1;
//		} else {
//
//		}
//	}
//
//	return -1;
//}
//
//string getNumberedLines(string str, int first, int last) {
//
//	std::istringstream ss(str.c_str());
//
//	string result = "";
//
//	int i = 0;
//	string line;
//	while (getline(ss, line, '\n')) {
//
//		if (i >= first && i <= last) {
//			result = result + std::to_string(i + 1) + ":" + line + "\n";
//		}
//
//		i++;
//	}
//
//	return result;
//}
//
//class V8Hlpr {
//
//public:
//
//	Isolate * isolate = nullptr;
//	Scope *scope = nullptr;
//	Local<ObjectTemplate> global;
//	Local<Context> context;
//	Context::Scope *context_scope = nullptr;
//
//	V8Hlpr() {
//		V8::InitializeICU();
//		V8::InitializeExternalStartupData("");
//
//		std::unique_ptr<v8::Platform> platform = v8::platform::NewDefaultPlatform();
//		v8::V8::InitializePlatform(platform.get());
//		V8::Initialize();
//
//		// Create a new Isolate and make it the current one.
//		//ArrayBufferAllocator allocator;
//		Isolate::CreateParams create_params;
//		create_params.array_buffer_allocator = v8::ArrayBuffer::Allocator::NewDefaultAllocator();
//		isolate = Isolate::New(create_params);
//
//		//scope = new Scope(isolate);
//
//		//Local<ObjectTemplate> globalTemplate = ObjectTemplate::New();
//		//context = Context::New(isolate, NULL, globalTemplate);
//
//		global = ObjectTemplate::New(isolate);
//
//		context = Context::New(isolate, NULL, global);
//	}
//};
//
//int main() {
//
//
//	V8Hlpr *helper = new V8Hlpr();
//
//	Context::Scope context_scope(helper->context);
//
//	//
//	//
//	Local<ObjectTemplate> tpl = ObjectTemplate::New(helper->isolate);
//	tpl->SetInternalFieldCount(1);
//	Local<Object> obj = tpl->NewInstance(helper->context).ToLocalChecked();
//
//	for (int i = 0; i < 1'000; i++) {
//
//		{
//			auto lValue = v8::Integer::New(helper->isolate, i * 2);
//			string name = "val_" + std::to_string(i);
//
//			obj->Set(String::NewFromUtf8(helper->isolate, name.c_str()), lValue);
//		}
//
//
//		{
//			auto name = String::NewFromUtf8(helper->isolate, ("fun_" + std::to_string(i)).c_str());
//
//			auto function = Function::New(helper->context, [](const FunctionCallbackInfo<Value>& args) {
//				cout << "fun_" << endl;
//				return;
//			}).ToLocalChecked();
//
//			obj->Set(name, function);
//		}
//
//
//	}
//
//	
//
//	helper->context->Global()->Set(
//		String::NewFromUtf8(helper->isolate, "gl"),
//		obj
//	);
//
//
//
//
//
//	//
//	//
//	//
//	std::string command;
//	command += "1 + 2";
//	//command += "gl";
//	//command += "gl.val_2 + gl.val_4";
//	//command += "gl.fun_100()";
//
//	auto source = String::NewFromUtf8(helper->isolate, command.c_str(),
//		NewStringType::kNormal).ToLocalChecked();
//	auto script = Script::Compile(helper->context, source);
//	if (script.IsEmpty()) {
//		cout << "failed to compile script" << endl;
//		cout << command << endl;
//		return 1;
//	}
//
//	auto lscript = script.ToLocalChecked();
//
//	TryCatch trycatch(helper->isolate);
//
//	auto result = lscript->Run(helper->context);
//
//	if (result.IsEmpty()) {
//		cout << "failed to run script" << endl;
//
//		String::Utf8Value exception_str(helper->isolate, trycatch.Exception());
//
//		auto stack_trace_string = trycatch.StackTrace(helper->context).ToLocalChecked();
//		v8::String::Utf8Value trace_str(helper->isolate, stack_trace_string);
//
//		std::string exceptionStr = *exception_str;
//		std::string traceStr = *trace_str;
//
//		cout << "====" << endl;
//		//cout << exceptionStr << endl;
//		//cout << "====" << endl;
//
//		cout << traceStr << endl;
//		cout << "====" << endl;
//
//		int line = getErrorLine(traceStr);
//
//		if (line >= 0) {
//			std::string lines = getNumberedLines(command, line - 2, line + 2);
//			cout << lines;
//		} else {
//			cout << ":(" << endl;
//		}
//
//		cout << "====" << endl;
//	} else {
//		auto lresult = result.ToLocalChecked();
//
//		String::Utf8Value utf8(helper->isolate, lresult);
//
//		if (*utf8 != nullptr) {
//			cout << std::string(*utf8) << endl;
//		}
//	}
//
//
//
//
//	return 0;
//}