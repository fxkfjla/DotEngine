// #define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
// #include <vulkan/vulkan.h> 

int main()
{
	if(!glfwInit())
		return -1;

	// GLFWwindow* window = glfwCreateWindow(640, 480, "DotEngine", nullptr, nullptr);

	// glfwMakeContextCurrent(window);
	// while(!glfwWindowShouldClose(window))
	// {
	// 	// glClear(GL_COLOR_BUFFER_BIT);
	// 	glfwSwapBuffers(window);
	// 	glfwPollEvents();
	// }

	glfwTerminate();

	return 0;
}