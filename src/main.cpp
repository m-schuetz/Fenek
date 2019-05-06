
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


using std::unordered_map;
using std::vector;
using std::cout;
using std::endl;
using std::chrono::high_resolution_clock;
using std::chrono::duration;
using std::chrono::duration_cast;

namespace fs = std::experimental::filesystem;

static long long start_time = std::chrono::high_resolution_clock::now().time_since_epoch().count();

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

		int numPoints = 70'000'000;
		int bytesPerPoint = 16;

		updateBuffer.size = numPoints * bytesPerPoint;

		glCreateBuffers(1, &updateBuffer.handle);

		//{// map buffer method
		//	GLbitfield storageFlags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
		//	glNamedBufferStorage(updateBuffer.handle, updateBuffer.size, nullptr, storageFlags);

		//	GLbitfield mapFlags = GL_MAP_WRITE_BIT | GL_MAP_FLUSH_EXPLICIT_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
		//	updateBuffer.mapPtr = glMapNamedBufferRange(updateBuffer.handle, 0, updateBuffer.size, mapFlags);
		//}

		{ // bufferData method
			
			glNamedBufferData(updateBuffer.handle, updateBuffer.size, nullptr, GL_DYNAMIC_DRAW);

			updateBuffer.data = malloc(10'000'000 * 16);
		}
		

	}

	V8Helper::_instance->registerFunction("test", [](const FunctionCallbackInfo<Value>& args) {
		if (args.Length() != 0) {
			V8Helper::_instance->throwException("test requires 0 arguments");
			return;
		}

		cout << "test start" << endl;

		thread t([]() {
			float* fptr = reinterpret_cast<float*>(updateBuffer.data);
			uint8_t* u8ptr = reinterpret_cast<uint8_t*>(updateBuffer.data);
			int byteOffset = 0;

			int bytePerPoint = 16;
			int numUpdatedPoints = 1'000'000;

			std::random_device rd;
			std::mt19937 mt(rd());
			std::uniform_real_distribution<float> dist(-1.0, 1.0);

			for (int i = 0; i < numUpdatedPoints; i++) {

				fptr[4 * byteOffset + 4 * i + 0] = dist(mt);
				fptr[4 * byteOffset + 4 * i + 1] = dist(mt);
				fptr[4 * byteOffset + 4 * i + 2] = dist(mt);

				u8ptr[16 * byteOffset + 16 * i + 12] = 255 * (0.5 * dist(mt) + 0.5);
				u8ptr[16 * byteOffset + 16 * i + 13] = 255 * (0.5 * dist(mt) + 0.5);
				u8ptr[16 * byteOffset + 16 * i + 14] = 255 * (0.5 * dist(mt) + 0.5);
				u8ptr[16 * byteOffset + 16 * i + 15] = 255;
			}



			//fptr[offset + 0] = 1.0;
			//fptr[offset + 1] = 2.0;
			//fptr[offset + 2] = 4.0;

			//int size = 3 * sizeof(float);
			int size = numUpdatedPoints * 16;

			cout << "test points updated pinned" << endl;

			schedule([byteOffset, size]() {
				cout << "test flusing" << endl;
				//glFlushMappedNamedBufferRange(updateBuffer.handle, byteOffset, size);
				glNamedBufferSubData(updateBuffer.handle, 0, size, updateBuffer.data);
				cout << "test flushed" << endl;
			});

		});
		t.detach();


		args.GetReturnValue().Set(updateBuffer.handle);
	});

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