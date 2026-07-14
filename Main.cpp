#include "Simulator.h"

int main()
{
    rtre::Window::init();
    rtre::Window::initHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    rtre::Window::initHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    rtre::Window::initHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    rtre::Window window(800, 480, "Engine");
    window.makeContextCurrent();

    Simulator simulator;
    simulator.init(window);
    simulator.run();

    rtre::Window::terminate();

    return 0;
}
