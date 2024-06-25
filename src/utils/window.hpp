#ifndef WINDOW 
#define WINDOW

#include "glad/glad.h"
#include <GLFW/glfw3.h>
#include <iostream>

enum class WindowCallbacks
{
    FrameBufferSize,
    CursorPos,
    Scroll
};

class Window
{
private:
    GLFWwindow* window;
    int windowWidth;
    int windowHeight;
    GLFWmonitor* currentMonitor;
    int shouldClose;

public:
    Window(int width, int height, const char* title, GLFWmonitor* monitor = nullptr);
    int getWindowCloseState();
    void makeWindowContextCurrent();
    GLFWwindow* getWindow();
	void createViewport(int width, int height);
};
#endif // !DEBUG
