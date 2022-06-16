#define GLFW_INCLUDE_VULKAN
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
// #include <vulkan/vulkan.h> 

int main()
{
	if(!glfwInit())
		return -1;

	uint32_t extensionCout = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCout, nullptr);
	// GLFWwindow* window = glfwCreateWindow(640, 480, "DotEngine", nullptr, nullptr);
	while(true);
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
