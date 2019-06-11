
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

using namespace LASLoaderThreaded;

namespace fs = std::experimental::filesystem;

static long long start_time = std::chrono::high_resolution_clock::now().time_since_epoch().count();

int numPointsUploaded = 0;

ProgressiveLoader* loader = nullptr;

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
	loader->uploadNextAvailableChunk();
	loader->uploadNextAvailableChunk();
	loader->uploadNextAvailableChunk();
	loader->uploadNextAvailableChunk();
	loader->uploadNextAvailableChunk();
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

	V8Helper::_instance->registerFunction("setAttribute", [](const FunctionCallbackInfo<Value>& args) {
		if (args.Length() != 3) {
			V8Helper::_instance->throwException("setAttribute requires 1 arguments");
			return;
		}

		if (loader == nullptr) {
			return;
		}

		String::Utf8Value nameUTF8(args[0]);
		string name = *nameUTF8;

		double scale = args[1]->NumberValue();
		double offset = args[2]->NumberValue();

		static atomic<int> pointsUploaded = 0;
		static atomic<int> chunkIndex = 0;

		pointsUploaded = 0;
		chunkIndex = 0;
	

		// TODO baaad
		mutex* mtx = new mutex();
		

		auto setAttributeTask = [name, scale, offset, mtx]() {
			auto lasloader = loader->loader;

			auto findAttribute = [lasloader](string name, Points* chunk) {

				auto attributes = chunk->attributes;

				auto it = std::find_if(attributes.begin(), attributes.end(), [name](Attribute& a) {
					return a.name == name;
					});

				if (it == attributes.end()) {
					return attributes[0];
				}
				else {
					return *it;
				}
			};

//			for (auto chunk : loader->chunks) {

			
			//int index = chunkIndex;

			while(true){

				mtx->lock();

				if (chunkIndex >= loader->chunks.size()) {
					break;
				}

				auto chunk = loader->chunks[chunkIndex];
				chunkIndex++;
				mtx->unlock();

				auto attribute = findAttribute(name, chunk);

				int chunkSize = chunk->size;
				void* data = malloc(chunkSize * 16);
				XYZRGBA* target = reinterpret_cast<XYZRGBA*>(data);

				auto source = attribute.data->data;

				for (int i = 0; i < chunkSize; i++) {

					target[i] = chunk->xyzrgba[i];

					int32_t* targeti32 = reinterpret_cast<int32_t*>(&target[i].r);

					if (attribute.bytes == 1) {
						int32_t val = reinterpret_cast<uint8_t*>(source)[i];

						targeti32[0] = val;
					}
					else if (attribute.bytes == 2) {
						int32_t val = reinterpret_cast<int16_t*>(source)[i];

						targeti32[0] = val;
					}
					else if (attribute.bytes == 4) {
						int32_t val = reinterpret_cast<uint32_t*>(source)[i];

						targeti32[0] = val;
					}
				}

				int offset = pointsUploaded;

				schedule([data, target, offset, chunkSize]() {
					loader->uploadChunk(target, offset, chunkSize);

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

		


	});


	V8Helper::_instance->registerFunction("loadLASProgressive", [](const FunctionCallbackInfo<Value>& args) {
		if (args.Length() != 1) {
			V8Helper::_instance->throwException("loadLASProgressive requires 1 arguments");
			return;
		}

		String::Utf8Value fileUTF8(args[0]);
		string file = *fileUTF8;

		startUpload = now();

		loader = new ProgressiveLoader(file);
		//ProgressiveLoader* loader = new ProgressiveLoader(file);

		auto isolate = Isolate::GetCurrent();
		Local<ObjectTemplate> lasTempl = ObjectTemplate::New(isolate);
		auto objLAS = lasTempl->NewInstance();

		auto lHandle0 = v8::Integer::New(isolate, loader->ssVertexBuffers[0]);
		auto lHandle1 = v8::Integer::New(isolate, loader->ssVertexBuffers[1]);
		auto lNumPoints = v8::Integer::New(isolate, 0);

		auto lHandles = Array::New(isolate, loader->ssVertexBuffers.size());
		for (int i = 0; i < loader->ssVertexBuffers.size(); i++) {
			auto lHandle = v8::Integer::New(isolate, loader->ssVertexBuffers[i]);
			lHandles->Set(i, lHandle);
		}
		objLAS->Set(String::NewFromUtf8(isolate, "handles"), lHandles);
		//objLAS->Set(String::NewFromUtf8(isolate, "handle0"), lHandle0);
		//objLAS->Set(String::NewFromUtf8(isolate, "handle1"), lHandle1);
		objLAS->Set(String::NewFromUtf8(isolate, "numPoints"), lNumPoints);

		auto pObjLAS = v8::Persistent<Object, v8::CopyablePersistentTraits<v8::Object>>(isolate, objLAS);

		uploadHook(loader, pObjLAS);

		args.GetReturnValue().Set(objLAS);
	});

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