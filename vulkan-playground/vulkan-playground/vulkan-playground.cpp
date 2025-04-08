#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>

// Vulkan 기본 인스턴스 : 앱구성에 필요한 정보를 담는다
VkInstance s_Instance = VK_NULL_HANDLE;

// Vulkan Physical Device : 실제 GPU 기기를 저장한다
VkPhysicalDevice s_PhysicalDevice = VK_NULL_HANDLE;

// Queue Family Index : Graphic 작업용 Queue Family의 Index를 저장해둡니다. 
int s_GraphicsQueueFamilyIndex = -1;

// Vulkan Logical Device : 논리 Device 저장한뒤 사용
VkDevice s_Device = VK_NULL_HANDLE;

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
		std::cout << "Driver Version : " <<
			VK_VERSION_MAJOR(physicalDeviceProperties.driverVersion) << "." <<
			VK_VERSION_MINOR(physicalDeviceProperties.driverVersion) << "." <<
			VK_VERSION_PATCH(physicalDeviceProperties.driverVersion) << std::endl;
		std::cout << "API Version : " <<
			VK_VERSION_MAJOR(physicalDeviceProperties.apiVersion) << "." <<
			VK_VERSION_MINOR(physicalDeviceProperties.apiVersion) << "." <<
			VK_VERSION_PATCH(physicalDeviceProperties.apiVersion) << std::endl;
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
			std::cout << "Driver Version : " <<
				VK_VERSION_MAJOR(physicalDeviceProperties.driverVersion) << "." <<
				VK_VERSION_MINOR(physicalDeviceProperties.driverVersion) << "." <<
				VK_VERSION_PATCH(physicalDeviceProperties.driverVersion) << std::endl;
			std::cout << "API Version : " <<
				VK_VERSION_MAJOR(physicalDeviceProperties.apiVersion) << "." <<
				VK_VERSION_MINOR(physicalDeviceProperties.apiVersion) << "." <<
				VK_VERSION_PATCH(physicalDeviceProperties.apiVersion) << std::endl;
			std::cout << std::endl;
			break;
		}
		else {
			std::cerr << "Selected Wrong Index, Out of range - ";
		}
	}

	// 앞으로 쓸 Physical Device 선택 및 저장
	s_PhysicalDevice = physicalDevices[selectedPhysicalDeviceIndex];

	// Graphic 작업용 Queue Family Index 저장하기
	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(s_PhysicalDevice, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(s_PhysicalDevice, &queueFamilyCount, queueFamilies.data());

	int graphicsFamilyIndex = -1;
	for (int i = 0; i < queueFamilies.size(); i++) {
		// VK_QUEUE_GRAPHICS_BIT 플레그를 가진 queue를 찾아 GraphicsFamilyIndex로 저장해둡니다.
		if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			graphicsFamilyIndex = i;
			break;
		}
	}

	// 만약 graphics 관련된 Family index를 찾지 못했다면, 프로그램을 종료시킵니다.
	if (graphicsFamilyIndex == -1) {
		std::cerr << "Failed to find a graphics queue family!" << std::endl;
		return -1;
	}

	s_GraphicsQueueFamilyIndex = graphicsFamilyIndex;

	// 논리 Device (Device라고 정의) 생성
	float queuePriority = 1.0f;
	VkDeviceQueueCreateInfo queueCreateInfo{};
	queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queueCreateInfo.queueFamilyIndex = s_GraphicsQueueFamilyIndex;
	queueCreateInfo.queueCount = 1;
	queueCreateInfo.pQueuePriorities = &queuePriority;

	VkDeviceCreateInfo deviceCreateInfo{};
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.queueCreateInfoCount = 1;
	deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;

	if (vkCreateDevice(s_PhysicalDevice, &deviceCreateInfo, nullptr, &s_Device) != VK_SUCCESS) {
		std::cerr << "Failed to create logical device!" << std::endl;
		return -1;
	}

	std::cout << "Logical device created successfully!" << std::endl;
	return 0;
}