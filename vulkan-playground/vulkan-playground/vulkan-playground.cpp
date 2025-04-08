#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>

// Vulkan 기본 인스턴스 : 앱구성에 필요한 정보를 담는다
VkInstance s_Instance = VK_NULL_HANDLE;

// Vulkan Physical Device : 실제 GPU 기기를 저장한다
VkPhysicalDevice s_PhysicalDevice = VK_NULL_HANDLE;

int main() {
	if (!glfwInit()) {
		std::cerr << "Failed to initialize GLFW" << std::endl;
		return -1;
	}

	GLFWwindow* window = glfwCreateWindow(800, 600, "Hello Vulkan", NULL, NULL);
	if (!window) {
		std::cerr << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}

	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "Hello Vulkan";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "No Engine";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_0;

	VkInstanceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;

	if (vkCreateInstance(&createInfo, nullptr, &s_Instance) != VK_SUCCESS) {
		std::cerr << "Failed to create Vulkan instance" << std::endl;
		return -1;
	}

	// 물리 디바이스 선택
	uint32_t physicalDeviceCount = 0;
	vkEnumeratePhysicalDevices(s_Instance, &physicalDeviceCount, nullptr); // 먼저 갯수를 가져온다
	if (physicalDeviceCount == 0) {
		std::cerr << "Failed to find G^PUs with Vulkan support!" << std::endl;
		return -1;
	}

	std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
	vkEnumeratePhysicalDevices(s_Instance, &physicalDeviceCount, physicalDevices.data());

	// Physical Device list 출력하여 확인하기
	for (int i = 0; i < physicalDeviceCount; i++) {
		VkPhysicalDevice& physicalDevice = physicalDevices[i];
		VkPhysicalDeviceProperties physicalDeviceProperties;
		vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);
		std::cout << "[" << i << "]=======[Physical Device] : " << physicalDeviceProperties.deviceName << "=======" << std::endl;
		std::cout << "Driver Version : " << physicalDeviceProperties.driverVersion << std::endl;
		std::cout << "API Version : " << physicalDeviceProperties.apiVersion << std::endl;
		std::cout << std::endl;
	}

	// Physical Device List 중에 하나 선택하기
	int selectedPhysicalDeviceIndex = -1;
	while (true) {
		std::cout << "Select Physical Device Index [0 - " << physicalDeviceCount - 1 << "] : ";
		std::cin >> selectedPhysicalDeviceIndex;
		if (0 <= selectedPhysicalDeviceIndex < physicalDeviceCount) {
			// 선택된 Physical Device 정보 출력
			VkPhysicalDevice& physicalDevice = physicalDevices[selectedPhysicalDeviceIndex];
			VkPhysicalDeviceProperties physicalDeviceProperties;
			vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);
			std::cout << "Selected Physical Device Info [" << selectedPhysicalDeviceIndex << "]" << std::endl;
			std::cout << "Name : " << physicalDeviceProperties.deviceName << std::endl;
			std::cout << "Driver Version : " << physicalDeviceProperties.driverVersion << std::endl;
			std::cout << "API Version : " << physicalDeviceProperties.apiVersion << std::endl;
			std::cout << std::endl;
			break;
		}
		else {
			std::cerr << "Selected Wrong Index, Out of range - ";
		}
	}

	s_PhysicalDevice = physicalDevices[selectedPhysicalDeviceIndex];

	return 0;
}