
#include <iostream>
#include <chrono>
#include <unordered_map>
#include <vector>
#include <filesystem>
#include <cstdlib>
#include <iomanip>
#include <random>

#include "GL\glew.h"
#include "GLFW\glfw3.h"

#include "LASLoader.h"
#include "ProgressiveLoader.h"


using std::unordered_map;
using std::vector;
using std::cout;
using std::endl;
using std::chrono::high_resolution_clock;
using std::chrono::duration;
using std::chrono::duration_cast;

namespace fs = std::experimental::filesystem;

static long long start_time = std::chrono::high_resolution_clock::now().time_since_epoch().count();

int numPointsUploaded = 0;

double now() {
	auto now = std::chrono::high_resolution_clock::now();
	long long nanosSinceStart = now.time_since_epoch().count() - start_time;

	double secondsSinceStart = double(nanosSinceStart) / 1'000'000'000;

	return secondsSinceStart;
}

#include "Application.h"
#include "V8Helper.h"
#include "utils.h"
#include "Shader.h"

struct GLUpdateBuffer{
	void* mapPtr = nullptr;
	uint32_t size = 0;
	GLuint handle = 0;
	void* data = nullptr;
};

static GLUpdateBuffer updateBuffer = GLUpdateBuffer();

static void APIENTRY debugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam) {


	if (
		severity == GL_DEBUG_SEVERITY_NOTIFICATION 
		|| severity == GL_DEBUG_SEVERITY_LOW 
		|| severity == GL_DEBUG_SEVERITY_MEDIUM
		) {
		return;
	}

	cout << message << endl;
}


void error_callback(int error, const char* description){
	fprintf(stderr, "Error: %s\n", description);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods){

	cout << "key: " << key << ", scancode: " << scancode << ", action: " << action << ", mods: " << mods << endl;

	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, GLFW_TRUE);
	}

	KeyEvent data = { key, scancode, action, mods };

	Application::instance()->dispatchKeyEvent(data);
}

static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos){
	//cout << "xpos: " << xpos << ", ypos: " << ypos << endl;
	MouseMoveEvent data = { xpos, ypos };

	Application::instance()->dispatchMouseMoveEvent(data);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset){
	MouseScrollEvent data = { xoffset, yoffset };

	Application::instance()->dispatchMouseScrollEvent(data);
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods){
	//if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
	//	popup_menu();

	MouseButtonEvent data = { button, action, mods };

	if (action == GLFW_PRESS) {
		Application::instance()->dispatchMouseDownEvent(data);
	} else if(action == GLFW_RELEASE) {
		Application::instance()->dispatchMouseUpEvent(data);
	}
}

//string ObjectToString(v8::Isolate* isolate, Local<Value> value) {
//	String::Utf8Value utf8_value(isolate, value);
//	return string(*utf8_value);
//}

double startUpload = 0.0;
double endUpload = 0.0;


void uploadHook(ProgressiveLoader* loader, v8::Persistent<Object, v8::CopyablePersistentTraits<v8::Object>> pObjLAS) {

	
	//cout << "chunks.size(): " << loader->loader->chunks.size() << endl;

	loader->uploadNextAvailableChunk();
	loader->uploadNextAvailableChunk();
	loader->uploadNextAvailableChunk();


	auto isolate = Isolate::GetCurrent();
	Local<Object> objLAS = Local<Object>::New(isolate, pObjLAS);

	auto lNumPoints = v8::Integer::New(isolate, loader->pointsUploaded);
	objLAS->Set(String::NewFromUtf8(isolate, "numPoints"), lNumPoints);

	schedule([loader, pObjLAS]() {

		if (!loader->isDone()) {
			uploadHook(loader, pObjLAS);
		} else {
			endUpload = now();
			double duration = endUpload - startUpload;
			cout << "upload duration: " << duration << "s" << endl;
		}
	});
};


int main() {

	cout << std::setprecision(3) << std::fixed;
	cout << "<main> " << "(" << now() << ")" << endl;

	//{
	//	cout << "building js package" << endl;
	//
	//	std::system("cd ../../ & rollup -c");
	//}
	//cout << "<built> " << "(" << now() << ")" << endl;

	glfwSetErrorCallback(error_callback);

	if (!glfwInit()) {
		// Initialization failed
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	//glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);

	int numMonitors;
	GLFWmonitor** monitors = glfwGetMonitors(&numMonitors);

	GLFWwindow* window = nullptr;

	cout << "<20> " << "(" << now() << ")" << endl;

	cout << "<create windows>" << endl;
	if (numMonitors > 1) {
		const GLFWvidmode * modeLeft = glfwGetVideoMode(monitors[0]);
		const GLFWvidmode * modeRight = glfwGetVideoMode(monitors[1]);

		window = glfwCreateWindow(modeRight->width, modeRight->height - 300, "Simple example", nullptr, nullptr);

		if (!window) {
			glfwTerminate();
			exit(EXIT_FAILURE);
		}

		glfwSetWindowPos(window, modeLeft->width, 0);
	} else {
		const GLFWvidmode * mode = glfwGetVideoMode(monitors[0]);

		window = glfwCreateWindow(mode->width / 2, mode->height / 2, "Simple example", nullptr, nullptr);

		if (!window) {
			glfwTerminate();
			exit(EXIT_FAILURE);
		}

		glfwSetWindowPos(window, mode->width / 2, 2 * mode->height / 3);
	}

	cout << "<windows created> " << "(" << now() << ")" << endl;

	cout << "<set input callbacks>" << endl;
	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window, cursor_position_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);

	glfwMakeContextCurrent(window);
	glfwSwapInterval(0);

	GLenum err = glewInit();
	if (GLEW_OK != err) {
		/* Problem: glewInit failed, something is seriously wrong. */
		fprintf(stderr, "glew error: %s\n", glewGetErrorString(err));
	}

	cout << "<glewInit done> " << "(" << now() << ")" << endl;

	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);
	glDebugMessageCallback(debugCallback, NULL);

	high_resolution_clock::time_point start = high_resolution_clock::now();
	high_resolution_clock::time_point previous = start;

	int fpsCounter = 0;
	high_resolution_clock::time_point lastFPSTime = start;


	V8Helper::instance()->window = window;
	V8Helper::instance()->setupV8();

	cout << "<V8 has been set up> " << "(" << now() << ")" << endl;

	{
		cout << "<run start.js>" << endl;
		string code = loadFileAsString("../../src_js/start.js");
		V8Helper::instance()->runScript(code);
	}

	cout << "<start.js was executed> " << "(" << now() << ")" << endl;

	auto updateJS = V8Helper::instance()->compileScript("update();");
	auto renderJS = V8Helper::instance()->compileScript("render();");

	{

		int numPoints = 30'000'000;
		int bytesPerPoint = 16;

		updateBuffer.size = numPoints * bytesPerPoint;

		glCreateBuffers(1, &updateBuffer.handle);

		{// map buffer method, see https://www.slideshare.net/CassEveritt/approaching-zero-driver-overhead/85
			GLbitfield mapFlags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
			GLbitfield storageFlags = mapFlags | GL_DYNAMIC_STORAGE_BIT;

			glNamedBufferStorage(updateBuffer.handle, updateBuffer.size, nullptr, storageFlags);

			updateBuffer.mapPtr = glMapNamedBufferRange(updateBuffer.handle, 0, updateBuffer.size, mapFlags);
		}

		//{ // bufferData method
		//	
		//	glNamedBufferData(updateBuffer.handle, updateBuffer.size, nullptr, GL_DYNAMIC_DRAW);

		//	updateBuffer.data = malloc(10'000'000 * 16);
		//}
		

	}


	V8Helper::_instance->registerFunction("test", [](const FunctionCallbackInfo<Value>& args) {
		if (args.Length() != 1) {
			V8Helper::_instance->throwException("test requires 0 arguments");
			return;
		}

		cout << "test start" << endl;

		String::Utf8Value fileUTF8(args[0]);

		string file = *fileUTF8;

		LASLoaderThreaded::LASLoader* loader = new LASLoaderThreaded::LASLoader(file);

		thread t([loader]() {
			uint8_t *u8ptr = reinterpret_cast<uint8_t*>(updateBuffer.mapPtr);
			float* fptr = reinterpret_cast<float*>(updateBuffer.mapPtr);
			int byteOffset = 0;
			int bytePerPoint = 16;

			while (!loader->allChunksServed()) {

				if (loader->hasChunkAvailable()) {
					auto chunk = loader->getNextChunk();

					//cout << "new chunk! write it to pinned buffer" << endl;

					for (int i = 0; i < chunk->size; i++) {

						float *xyz = reinterpret_cast<float*>(u8ptr + byteOffset);
						uint8_t *rgba = reinterpret_cast<uint8_t*>(u8ptr + byteOffset + 12);

						xyz[0] = chunk->position[3 * i + 0];
						xyz[1] = chunk->position[3 * i + 1];
						xyz[2] = chunk->position[3 * i + 2];

						rgba[0] = chunk->rgba[4 * i + 0];
						rgba[1] = chunk->rgba[4 * i + 1];
						rgba[2] = chunk->rgba[4 * i + 2];
						rgba[3] = chunk->rgba[4 * i + 3];

						byteOffset += bytePerPoint;
					}

					delete chunk;

					//cout << "done writing to pinned buffer" << endl;
				}


				std::this_thread::sleep_for(std::chrono::milliseconds(5));
			}

			
			//cout << "test points updated pinned" << endl;

		});
		t.detach();


		args.GetReturnValue().Set(updateBuffer.handle);
	});

	V8Helper::_instance->registerFunction("loadLASProgressive", [](const FunctionCallbackInfo<Value>& args) {
		if (args.Length() != 1) {
			V8Helper::_instance->throwException("loadLASProgressive requires 1 arguments");
			return;
		}

		String::Utf8Value fileUTF8(args[0]);
		string file = *fileUTF8;

		startUpload = now();

		ProgressiveLoader* loader = new ProgressiveLoader(file);

		auto isolate = Isolate::GetCurrent();
		Local<ObjectTemplate> lasTempl = ObjectTemplate::New(isolate);
		auto objLAS = lasTempl->NewInstance();

		auto lHandle = v8::Integer::New(isolate, loader->ssVertexBuffer);
		auto lNumPoints = v8::Integer::New(isolate, 0);

		objLAS->Set(String::NewFromUtf8(isolate, "handle"), lHandle);
		objLAS->Set(String::NewFromUtf8(isolate, "numPoints"), lNumPoints);

		auto pObjLAS = v8::Persistent<Object, v8::CopyablePersistentTraits<v8::Object>>(isolate, objLAS);

		//function<void(void)> uploadHook = [uploadHook, loader, pObjLAS]() {
		//	loader->uploadNextAvailableChunk();
		//
		//	auto isolate = Isolate::GetCurrent();
		//	Local<Object> objLAS = Local<Object>::New(isolate, pObjLAS);
		//
		//	auto lNumPoints = v8::Integer::New(isolate, loader->pointsUploaded);
		//	objLAS->Set(String::NewFromUtf8(isolate, "numPoints"), lNumPoints);
		//
		//	schedule(uploadHook);
		//};

		//schedule(uploadHook);

		uploadHook(loader, pObjLAS);

		args.GetReturnValue().Set(objLAS);
	});


	V8Helper::_instance->registerFunction("loadLASTest", [](const FunctionCallbackInfo<Value>& args) {
		if (args.Length() != 1) {
			V8Helper::_instance->throwException("loadLASTest requires 1 arguments");
			return;
		}

		cout << "test start" << endl;
		double start = now();

		String::Utf8Value fileUTF8(args[0]);

		string file = *fileUTF8;

		LASLoaderThreaded::LASLoader* loader = new LASLoaderThreaded::LASLoader(file);

		int bytePerPoint = 16;
		
		GLuint handle = 0;
		glCreateBuffers(1, &handle);
		
		uint32_t size = loader->header.numPoints * bytePerPoint;
		//GLbitfield usage = GL_STATIC_DRAW;
		GLbitfield usage = GL_STREAM_DRAW;
		glNamedBufferData(handle, size, nullptr, usage);

		auto isolate = Isolate::GetCurrent();
		Local<ObjectTemplate> lasTempl = ObjectTemplate::New(isolate);
		auto objLAS = lasTempl->NewInstance();
		
		auto lHandle = v8::Integer::New(isolate, handle);
		auto lNumPoints = v8::Integer::New(isolate, 0);
		
		objLAS->Set(String::NewFromUtf8(isolate, "handle"), lHandle);
		objLAS->Set(String::NewFromUtf8(isolate, "numPoints"), lNumPoints);
		
		auto pObjLAS = v8::Persistent<Object, v8::CopyablePersistentTraits<v8::Object>>(isolate, objLAS);

		thread t([handle, loader, bytePerPoint, start, pObjLAS]() {
		
			int targetOffset = 0;
		
			int ci = 0;
		
			while (!loader->allChunksServed()) {
		
				if (loader->hasChunkAvailable()) {
					auto start01 = now();
					auto chunk = loader->getNextChunk();

					//cout << "num processed chunks: " << loader->chunks.size() << endl;
		
					//cout << "processing chunk " << ci << endl;
					//cout << "blabla chunk " << chunk->size << endl;
					//cout << "chunk available!" << endl;
		
					//vector<uint8_t> data(bytePerPoint * chunk->size);
					uint8_t* data = new uint8_t[bytePerPoint * chunk->size];
					
					int byteOffset = 0;

					
					for (int i = 0; i < chunk->size; i++) {
					
						float *xyz = reinterpret_cast<float*>(data + byteOffset);
						uint8_t *rgba = reinterpret_cast<uint8_t*>(data + byteOffset + 12);
					
						xyz[0] = chunk->position[3 * i + 0];
						xyz[1] = chunk->position[3 * i + 1];
						xyz[2] = chunk->position[3 * i + 2];
					
						rgba[0] = chunk->rgba[4 * i + 0];
						rgba[1] = chunk->rgba[4 * i + 1];
						rgba[2] = chunk->rgba[4 * i + 2];
						rgba[3] = chunk->rgba[4 * i + 3];
					
						byteOffset += bytePerPoint;
					}
					
					int size = chunk->size * bytePerPoint;
					int chunkSize = chunk->size;
					
					bool isLastOne = loader->allChunksServed();

					
		
					schedule([handle, targetOffset, size, chunkSize, data, ci, start, isLastOne, pObjLAS]() {
					
						auto isolate = Isolate::GetCurrent();
						Local<Object> objLAS = Local<Object>::New(isolate, pObjLAS);
						
						glNamedBufferSubData(handle, targetOffset, size, data);

						delete[] data;
					
						numPointsUploaded += chunkSize;
					
						auto lNumPoints = v8::Integer::New(isolate, numPointsUploaded);
					
						objLAS->Set(String::NewFromUtf8(isolate, "numPoints"), lNumPoints);
						//cout << "numPointsUploaded: " << numPointsUploaded << endl;
						
						if (isLastOne) {
							double end = now();
							double duration = end - start;
							cout << "total load to GPU upload duration: " << duration << endl;
						}
					});

					auto end01 = now();
					int duration = int((end01 - start01) * 1000.0);

					//cout << "to gpu buffer duration: " << duration << endl;
		
					targetOffset += chunk->size * bytePerPoint;
					ci++;
		
					delete chunk;

					
		
					std::this_thread::sleep_for(std::chrono::milliseconds(1));
				}
		
		
				
			}
		
			
		
			
		
		});
		t.detach();

		args.GetReturnValue().Set(objLAS);
	});


	//V8Helper::_instance->registerFunction("test", [](const FunctionCallbackInfo<Value>& args) {
	//	if (args.Length() != 1) {
	//		V8Helper::_instance->throwException("test requires 0 arguments");
	//		return;
	//	}

	//	cout << "test start" << endl;

	//	String::Utf8Value fileUTF8(args[0]);

	//	string file = *fileUTF8;

	//	LASLoaderThreaded::LASLoader* loader = new LASLoaderThreaded::LASLoader(file);

	//	thread t([]() {
	//		float* fptr = reinterpret_cast<float*>(updateBuffer.mapPtr);
	//		uint8_t* u8ptr = reinterpret_cast<uint8_t*>(updateBuffer.mapPtr);
	//		int byteOffset = 0;

	//		int bytePerPoint = 16;
	//		int numUpdatedPoints = 5'000'000;

	//		std::random_device rd;
	//		std::mt19937 mt(rd());
	//		std::uniform_real_distribution<float> dist(-1.0, 1.0);
	//		std::uniform_real_distribution<float> dist01(-1.0, 1.0);

	//		float t = now();

	//		for (int i = 0; i < numUpdatedPoints; i++) {
	//			fptr[4 * byteOffset + 4 * i + 0] = dist(mt);
	//			fptr[4 * byteOffset + 4 * i + 1] = 0.1 * dist(mt) + 2.0 * (sin(t) + 1);
	//			fptr[4 * byteOffset + 4 * i + 2] = dist(mt);

	//			u8ptr[16 * byteOffset + 16 * i + 12] = dist01(mt) * 255;
	//			u8ptr[16 * byteOffset + 16 * i + 13] = dist01(mt) * 255;
	//			u8ptr[16 * byteOffset + 16 * i + 14] = 0;
	//			u8ptr[16 * byteOffset + 16 * i + 15] = 255;
	//		}

	//		//for (int j = 0; j < 10'000; j++) {
	//		while(true){

	//			float t = now();

	//			float h = sin(t);

	//			for (int i = 0; i < 20'000'000; i += 100) {
	//				fptr[4 * byteOffset + 4 * i + 1] = 2.0 * h + 1;
	//			}

	//			auto duration = now() - t;
	//			cout << "duration: " << duration << "s" << endl;

	//			std::this_thread::sleep_for(std::chrono::milliseconds(1));

	//		}

	//		int size = numUpdatedPoints * 16;

	//		cout << "test points updated pinned" << endl;
	//		
	//	});
	//	t.detach();

	//	
	//	args.GetReturnValue().Set(updateBuffer.handle);
	//});


	//V8Helper::_instance->registerFunction("test", [](const FunctionCallbackInfo<Value>& args) {
	//	if (args.Length() != 0) {
	//		V8Helper::_instance->throwException("test requires 0 arguments");
	//		return;
	//	}

	//	cout << "test start" << endl;

	//	thread t([]() {
	//		float* fptr = reinterpret_cast<float*>(updateBuffer.data);
	//		uint8_t* u8ptr = reinterpret_cast<uint8_t*>(updateBuffer.data);
	//		int byteOffset = 0;

	//		int bytePerPoint = 16;
	//		int numUpdatedPoints = 1'000'000;

	//		std::random_device rd;
	//		std::mt19937 mt(rd());
	//		std::uniform_real_distribution<float> dist(-1.0, 1.0);

	//		for (int i = 0; i < numUpdatedPoints; i++) {

	//			fptr[4 * byteOffset + 4 * i + 0] = dist(mt);
	//			fptr[4 * byteOffset + 4 * i + 1] = dist(mt);
	//			fptr[4 * byteOffset + 4 * i + 2] = dist(mt);

	//			u8ptr[16 * byteOffset + 16 * i + 12] = 255 * (0.5 * dist(mt) + 0.5);
	//			u8ptr[16 * byteOffset + 16 * i + 13] = 255 * (0.5 * dist(mt) + 0.5);
	//			u8ptr[16 * byteOffset + 16 * i + 14] = 255 * (0.5 * dist(mt) + 0.5);
	//			u8ptr[16 * byteOffset + 16 * i + 15] = 255;
	//		}



	//		//fptr[offset + 0] = 1.0;
	//		//fptr[offset + 1] = 2.0;
	//		//fptr[offset + 2] = 4.0;

	//		//int size = 3 * sizeof(float);
	//		int size = numUpdatedPoints * 16;

	//		cout << "test points updated pinned" << endl;

	//		schedule([byteOffset, size]() {
	//			cout << "test flusing" << endl;
	//			//glFlushMappedNamedBufferRange(updateBuffer.handle, byteOffset, size);
	//			glNamedBufferSubData(updateBuffer.handle, 0, size, updateBuffer.data);
	//			cout << "test flushed" << endl;
	//		});

	//	});
	//	t.detach();


	//	args.GetReturnValue().Set(updateBuffer.handle);
	//});

	//V8Helper::_instance->registerFunction("test", [](const FunctionCallbackInfo<Value>& args) {
	//	if (args.Length() != 0) {
	//		V8Helper::_instance->throwException("test requires 0 arguments");
	//		return;
	//	}

	//	cout << "test start" << endl;

	//	thread t([]() {
	//		float* fptr = reinterpret_cast<float*>(updateBuffer.mapPtr);
	//		uint8_t* u8ptr = reinterpret_cast<uint8_t*>(updateBuffer.mapPtr);
	//		int byteOffset = 0;

	//		int bytePerPoint = 16;
	//		int numUpdatedPoints = 1'000'000;

	//		std::random_device rd;
	//		std::mt19937 mt(rd());
	//		std::uniform_real_distribution<double> dist(-1.0, 1.0);

	//		for (int i = 0; i < numUpdatedPoints; i++) {
	//			fptr[4 * byteOffset + 4 * i + 0] = dist(mt);
	//			fptr[4 * byteOffset + 4 * i + 1] = dist(mt);
	//			fptr[4 * byteOffset + 4 * i + 2] = dist(mt);

	//			u8ptr[16 * byteOffset + 16 * i + 12] = 255;
	//			u8ptr[16 * byteOffset + 16 * i + 13] = 255;
	//			u8ptr[16 * byteOffset + 16 * i + 14] = 0;
	//			u8ptr[16 * byteOffset + 16 * i + 15] = 255;
	//		}



	//		//fptr[offset + 0] = 1.0;
	//		//fptr[offset + 1] = 2.0;
	//		//fptr[offset + 2] = 4.0;

	//		//int size = 3 * sizeof(float);
	//		int size = numUpdatedPoints * 16;

	//		cout << "test points updated pinned" << endl;

	//		schedule([byteOffset, size]() {
	//			cout << "test flusing" << endl;
	//			glFlushMappedNamedBufferRange(updateBuffer.handle, byteOffset, size);
	//			cout << "test flushed" << endl;
	//		});
	//		
	//	});
	//	t.detach();

	//	
	//	args.GetReturnValue().Set(updateBuffer.handle);
	//});

	cout << "<entering first render loop> " << "(" << now() << ")" << endl;

	while (!glfwWindowShouldClose(window)){

		// ----------------
		// TIME
		// ----------------

		high_resolution_clock::time_point now = high_resolution_clock::now();
		double nanosecondsSinceLastFrame = double((now - previous).count());
		double nanosecondsSinceLastFPSMeasure = double((now - lastFPSTime).count());

		double timeSinceLastFrame = nanosecondsSinceLastFrame / 1'000'000'000;
		double timeSinceLastFPSMeasure = nanosecondsSinceLastFPSMeasure / 1'000'000'000;

		previous = now;

		if(timeSinceLastFPSMeasure >= 1.0){
			double fps = double(fpsCounter) / timeSinceLastFPSMeasure;
			stringstream ssFPS; 
			ssFPS << fps;
			

			V8Helper::instance()->debugValue["FPS"] = ssFPS.str();
			
			// ----------------
			// PRINT MESSAGES
			// ----------------
			if (Application::instance()->reportState) {
				cout << "============" << endl;
				for (auto &entry : V8Helper::instance()->debugValue) {
					cout << entry.first << ": " << entry.second << endl;
				}
				cout << "== end of frame ==" << endl;
			}

			lastFPSTime = now;
			fpsCounter = 0;
		}
		V8Helper::instance()->timeSinceLastFrame = timeSinceLastFrame;

		// ----------------

		EventQueue::instance->process();

		if (timeSinceLastFrame > 0.016) {
			cout << "too slow! time since last frame: " << int(timeSinceLastFrame * 1000.0) << "ms" << endl;
		}

		// ----------------
		// RENDER WITH JAVASCRIPT
		// ----------------
		
		//Application::instance()->lockScreenCapture();
		updateJS->Run(V8Helper::instance()->context);
		renderJS->Run(V8Helper::instance()->context);
		//Application::instance()->unlockScreenCapture();


		// ----------------
		// swap and events
		// ----------------
		glfwSwapBuffers(window);
		glfwPollEvents();

		fpsCounter++;

	}

	glfwDestroyWindow(window);
	glfwTerminate();
	exit(EXIT_SUCCESS);

	return 0;
}