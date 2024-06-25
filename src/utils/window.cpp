#include "window.hpp"

Window::Window(int width, int height, const char* title, GLFWmonitor* monitor) : windowWidth(width), windowHeight(height), currentMonitor(monitor)
{
    std::cout << "Creating window...\n";
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    this->window = glfwCreateWindow(width, height, title, monitor, nullptr); 
    if (this->window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
    }
    std::cout << "Created window!: " << this->window << "\n";
}

void Window::makeWindowContextCurrent()
{
    glfwMakeContextCurrent(this->window);    
}

GLFWwindow* Window::getWindow()
{
    return this->window;    
}

int Window::getWindowCloseState()
{
    shouldClose = glfwWindowShouldClose(this->window);
    return this->shouldClose;
}

void Window::createViewport(int width, int height)
{
    std::cout << "Creating viewport...\n";
    makeWindowContextCurrent();
    glViewport(0,0, width, height);
    std::cout << "Created viewport!\n";
}
