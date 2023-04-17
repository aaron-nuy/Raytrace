#pragma once
#include "glfw3.h"
#include <string>

#define ar_KeyHandler(HandlerName) void HandlerName (GLFWwindow* window, int key, int scancode, int action, int mods)
#define ar_IsKeyPressed(clicked) (key==clicked && action == GLFW_PRESS)
#define ar_IsKeyReleased(clicked) (key==clicked && action == GLFW_RELEASE)
#define ar_IsKeyRepeated(clicked) (key==clicked && action == GLFW_REPEAT)

#define ar_MouseButtonHandler(HandlerName) void HandlerName (GLFWwindow* window, int button, int action, int mods)
#define ar_IsMouseBtnPressed(clicked) (button==clicked && action == GLFW_PRESS)
#define ar_IsMouseBtnReleased(clicked) (button==clicked && action == GLFW_RELEASE)


namespace rtre {
	
	struct WSize {
		int height;
		int width;

		WSize(int width, int height);
	};
	struct WCoords {
		double x;
		double y;

		WCoords(double x, double y);

	};

	typedef GLFWmonitor* WMonitor;

	class Window {

		GLFWwindow* m_Window;
		int m_isFullscreen;
	public:


		Window();
		Window(Window& ) = delete;
		/*
			if primary monitor is not specified, it initializes to null
			if primary monitor is specified with getPrimaryMonitor, window launchs in fullscreen mode
		*/
		Window(int width, int height, const std::string& title, WMonitor primaryMonitor = NULL);
		/*
		if primaryMonir is not specified, it initializes to null
		if primary monitor is specified with getPrimaryMonitor, window launchs in fullscreen mode
		*/
		Window(WSize size, const std::string& title, WMonitor primaryMonitor = NULL);
		~Window();

		GLFWwindow* getWindow() { return m_Window; }

		void destroyWindow();

		static int init();
		static void terminate();
		static void initHint(int hint, int value);
		static void pollEvents();

		static WMonitor getPrimaryMonitor();

		void makeContextCurrent();
		void detachContext();

		void swapInterval(int value);
		void swapBuffers();

		double getTime();

		int shouldClose();
		void setShouldClose(int value);

		int isKeyPressed(int key);
		int isKeyReleased(int key);
		int isMousePressed(int button);
		int isMouseReleased(int button);

		int isClickingLeft() {
			return (glfwGetInputMode(m_Window, GLFW_CURSOR) == GLFW_CURSOR_NORMAL) && isMousePressed(GLFW_MOUSE_BUTTON_1);
		}


		GLFWkeyfun setKeyCallback(GLFWkeyfun callback_handler);
		GLFWmousebuttonfun setMouseButtonCallback(GLFWmousebuttonfun callback_handler);

		WSize getWindowSize();
		void setWindowSize(WSize size);
		void setWindowSize(int width, int height);

		WCoords getCursorPosition();
		void setCursorPosition(double x, double y);
		void setCursorPosition(WCoords coords);

		WCoords getWindowPosition();
		void setWindowPosition(WCoords coords);
		void setWindowPosition(int x, int y);

		void setInputMode(int mode, int value);
		void setTitle(const std::string& title);

		int isFullScreen();
		void setFullScreen();
		void setWindowed(int height, int width, int positionX, int positionY);
		void setWindowed(WSize size, WCoords coords);
	};
}