#include <GLFW/glfw3.h>

class TestTriangle
{
public:
    TestTriangle() = default;
    void run();
private:
    void initWindow();
    void initVulkan();
    void mainLoop();
    void cleanup();
private:
    GLFWwindow* window;
    const int width = 640;
    const int height = 480;
};