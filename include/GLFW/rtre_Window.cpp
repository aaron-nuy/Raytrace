#pragma once
#include "rtre_Window.h"

rtre::Window::Window() 
	:
	m_Window(glfwCreateWindow(300, 300, "Window", NULL, NULL))
{
}

rtre::Window::Window(int width, int height, const std::string& title, WMonitor primaryMonitor)
	:
	m_Window(glfwCreateWindow(width, height, title.c_str(), primaryMonitor, NULL))
	
{
	if (primaryMonitor == NULL) {
		m_isFullscreen = 0;
	}
	else {
		m_isFullscreen = 1;
	}
}

rtre::Window::Window(WSize size, const std::string& title, WMonitor primaryMonitor)
	:
	m_Window(glfwCreateWindow(size.width, size.height, title.c_str(), primaryMonitor, NULL))
{
	if (primaryMonitor == NULL) {
		m_isFullscreen = 0;
	}
	else {
		m_isFullscreen = 1;
	}
}



rtre::Window::~Window()
{
	glfwDestroyWindow(m_Window);
}

void rtre::Window::destroyWindow()
{
	glfwDestroyWindow(m_Window);
}


int rtre::Window::init()
{
	return glfwInit();
}

void rtre::Window::terminate()
{
	glfwTerminate();
}

void rtre::Window::initHint(int hint, int value)
{
	glfwInitHint(hint, value);
}

void rtre::Window::pollEvents()
{
	glfwPollEvents();
}

rtre::WMonitor rtre::Window::getPrimaryMonitor()
{
	return glfwGetPrimaryMonitor();
}

void rtre::Window::setTitle(const std::string& title)
{
	glfwSetWindowTitle(m_Window, title.c_str());
}

int rtre::Window::isFullScreen()
{
	return m_isFullscreen;
}

void rtre::Window::setFullScreen()
{
	const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
	glfwSetWindowMonitor(m_Window, glfwGetPrimaryMonitor(), 0, 0, mode->width, mode->height, GLFW_DONT_CARE);
	m_isFullscreen = 1;
}

void rtre::Window::setWindowed(int width, int height, int positionX,int positionY)
{
	glfwSetWindowMonitor(m_Window, NULL, positionX, positionY, width, height, GLFW_DONT_CARE);
	m_isFullscreen = 0;
}

void rtre::Window::setWindowed(WSize size, WCoords coords)
{
	glfwSetWindowMonitor(m_Window, NULL, coords.x, coords.y, size.width, size.height, GLFW_DONT_CARE);
	m_isFullscreen = 0;
}

void rtre::Window::makeContextCurrent()
{
	glfwMakeContextCurrent(m_Window);
}

void rtre::Window::detachContext()
{
	glfwMakeContextCurrent(NULL);
}

void rtre::Window::swapInterval(int value)
{
	glfwSwapInterval(value);
}

void rtre::Window::swapBuffers()
{
	glfwSwapBuffers(m_Window);
}

double rtre::Window::getTime()
{
	return glfwGetTime();
}

int rtre::Window::shouldClose()
{
	return glfwWindowShouldClose(m_Window);
}

void rtre::Window::setShouldClose(int value)
{
	glfwSetWindowShouldClose(m_Window, value);
}

int rtre::Window::isKeyPressed(int key)
{
	return glfwGetKey(m_Window,key) == GLFW_PRESS;
}

int rtre::Window::isKeyReleased(int key)
{
	return glfwGetKey(m_Window, key) == GLFW_RELEASE;;
}



int rtre::Window::isMousePressed(int button)
{
	return glfwGetMouseButton(m_Window, button) == GLFW_PRESS;
}

int rtre::Window::isMouseReleased(int button)
{
	return glfwGetMouseButton(m_Window, button) == GLFW_RELEASE;
}

rtre::WSize rtre::Window::getWindowSize()
{
	int width, height;
	glfwGetWindowSize(m_Window, &width, &height);
	return WSize(width,height);
}

void rtre::Window::setWindowSize(rtre::WSize size)
{
	glfwSetWindowSize(m_Window, size.width, size.height);
}

void rtre::Window::setWindowSize(int width, int height)
{
	glfwSetWindowSize(m_Window, width, height);
}

rtre::WCoords rtre::Window::getCursorPosition()
{
	double x, y;
	glfwGetCursorPos(m_Window,&x,&y);
	return WCoords(x, y);
}

void rtre::Window::setCursorPosition(double x, double y)
{
	glfwSetCursorPos(m_Window, x, y);
}

void rtre::Window::setCursorPosition(rtre::WCoords coords)
{
	glfwSetCursorPos(m_Window, coords.x, coords.y);
}

void rtre::Window::setInputMode(int mode, int value)
{
	glfwSetInputMode(m_Window, mode, value);
}

GLFWkeyfun rtre::Window::setKeyCallback(GLFWkeyfun callback_handler)
{
	return glfwSetKeyCallback(m_Window, callback_handler);
}

GLFWmousebuttonfun rtre::Window::setMouseButtonCallback(GLFWmousebuttonfun callback_handler)
{
	return glfwSetMouseButtonCallback(m_Window, callback_handler);
}

rtre::WCoords rtre::Window::getWindowPosition()
{
	int x, y;
	glfwGetWindowPos(m_Window, &x, &y);
	return WCoords(x,y);
}

void rtre::Window::setWindowPosition(rtre::WCoords coords)
{
	glfwSetWindowPos(m_Window, coords.x, coords.y);
}

void rtre::Window::setWindowPosition(int x, int y)
{
	glfwSetWindowPos(m_Window, x, y);
}

rtre::WCoords::WCoords(double x, double y)
{
	this->x = x;
	this->y = y;
}


rtre::WSize::WSize(int width, int height)
{
	this->height = height;
	this->width = width;
}