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
	appInfo.pApplicationName = "Hello Vulkan RT";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "No Engine";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_3; // <- Vulkan 1.1 이상의 기능을 쓰려면 api 버전을 명확하게 명시해줘야함.

	// 디버깅 용도로, 더 진행하기전에 레이어를 추가해야함
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	std::cout << "Available Vulkan Layers:\n";
	for (const auto& layer : availableLayers) {
		std::cout << "  " << layer.layerName << std::endl;
	}
	std::cout << std::endl;

	const std::vector<const char*> validationLayers = {
		"VK_LAYER_KHRONOS_validation"
	};

	const std::vector<const char*> requiredInstanceExtensions = {
		VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME, // Instance 확장
		VK_EXT_DEBUG_UTILS_EXTENSION_NAME
	};

	// Ray Tracing을 위해 확장기능을 추가해야함
	uint32_t availableInstanceExtensionCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &availableInstanceExtensionCount, nullptr);

	std::vector<VkExtensionProperties> availableInstanceExtensions(availableInstanceExtensionCount);
	vkEnumerateInstanceExtensionProperties(nullptr, &availableInstanceExtensionCount, availableInstanceExtensions.data());

	for (const auto& requiredInstanceExtension : requiredInstanceExtensions)
	{
		bool found = false;
		for (const auto& availableInstanceExtension : availableInstanceExtensions) {
			if (strcmp(availableInstanceExtension.extensionName, requiredInstanceExtension) == 0) {
				std::cout << requiredInstanceExtension << " ... [OK]" << std::endl;
				found = true;
				break;
			}
		}
		if (!found) {
			std::cerr << requiredInstanceExtension << "... Missing instance extension" << std::endl;
			return -1;
		}
	}
	std::cout << std::endl;

	VkInstanceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;

	// 확장 기능 등록 (디버깅을 위한) - 레이어
	createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
	createInfo.ppEnabledLayerNames = validationLayers.data();

	// 확장 기능 등록 - extensions
	createInfo.enabledExtensionCount = static_cast<uint32_t>(requiredInstanceExtensions.size());
	createInfo.ppEnabledExtensionNames = requiredInstanceExtensions.data();

	if (vkCreateInstance(&createInfo, nullptr, &s_Instance) != VK_SUCCESS) {
		std::cerr << "Failed to create Vulkan instance" << std::endl;
		return -1;
	}

	// 생성된 Instance를 기반으로 Debug callback 생성
	VkDebugUtilsMessengerEXT debugMessenger;

	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
	debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	debugCreateInfo.messageSeverity =
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	debugCreateInfo.messageType =
		VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	debugCreateInfo.pfnUserCallback = [](
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData) -> VkBool32 {
			std::cerr << "Validation Layer: " << pCallbackData->pMessage << std::endl << std::endl;
			return VK_FALSE;
		};

	// 확장 함수 가져오기 (vkCreateDebugUtilsMessenger)
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(s_Instance, "vkCreateDebugUtilsMessengerEXT");

	if (func != nullptr) {
		func(s_Instance, &debugCreateInfo, nullptr, &debugMessenger);
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
		std::cout << std::endl;
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

	// RayCasting을 위해서 확장기능을 추가해야함
	std::vector<const char*> requiredDeviceExtensions = {
		VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME,
		VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME,
		VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME,
		VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME,
		VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME,
		VK_KHR_SPIRV_1_4_EXTENSION_NAME,
		VK_KHR_SHADER_FLOAT_CONTROLS_EXTENSION_NAME,
		VK_KHR_DEVICE_GROUP_EXTENSION_NAME,                     // 추가!
		VK_KHR_MAINTENANCE3_EXTENSION_NAME                      // 추가!
	};

	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(s_PhysicalDevice, nullptr, &extensionCount, nullptr);

	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(s_PhysicalDevice, nullptr, &extensionCount, availableExtensions.data());

	for (const char* required : requiredDeviceExtensions) {
		bool found = false;
		for (const auto& ext : availableExtensions) {
			if (strcmp(required, ext.extensionName) == 0) {
				std::cout << required << " ... [OK]" << std::endl;
				found = true;
				break;
			}
		}
		if (!found) {
			std::cerr << required << " ... Missing required device extension" << std::endl;
			return -1;
		}
	}

	std::cout << std::endl;

	// 논리 Device 생성을 위한 Ray Tracing 관련 기능 구조체 셋업
	VkPhysicalDeviceAccelerationStructureFeaturesKHR accelStructFeatures{};
	accelStructFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR;
	accelStructFeatures.accelerationStructure = VK_TRUE;
	accelStructFeatures.pNext = nullptr;

	VkPhysicalDeviceRayTracingPipelineFeaturesKHR rtPipelineFeatures{};
	rtPipelineFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR;
	rtPipelineFeatures.rayTracingPipeline = VK_TRUE;
	rtPipelineFeatures.pNext = &accelStructFeatures; // 다양한 확장 기능은 pNext를 통해 확장합니다.

	VkPhysicalDeviceBufferDeviceAddressFeatures bufferDeviceAddressFeatures{};
	bufferDeviceAddressFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES;
	bufferDeviceAddressFeatures.bufferDeviceAddress = VK_TRUE;
	bufferDeviceAddressFeatures.pNext = &rtPipelineFeatures; // 다양한 확장 기능은 pNext를 통해 확장합니다.

	VkPhysicalDeviceVulkan11Features vulkan11Features{};
	vulkan11Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES;
	vulkan11Features.pNext = &bufferDeviceAddressFeatures;

	VkPhysicalDeviceFeatures2 deviceFeature2{};
	deviceFeature2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
	deviceFeature2.pNext = &vulkan11Features;

	// 최종 device 생성
	VkDeviceCreateInfo deviceCreateInfo{};
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	
	// Queue 생성
	deviceCreateInfo.queueCreateInfoCount = 1;
	deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;

	// 확장기능 
	deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(requiredDeviceExtensions.size());
	deviceCreateInfo.ppEnabledExtensionNames = requiredDeviceExtensions.data();

	// 구조체 확장
	deviceCreateInfo.pNext = &deviceFeature2; // 다양한 확장 기능은 pNext를 통해 확장합니다.

	if (vkCreateDevice(s_PhysicalDevice, &deviceCreateInfo, nullptr, &s_Device) != VK_SUCCESS) {
		std::cerr << "Failed to create logical device!" << std::endl;
		return -1;
	}

	// 위 과정을 통해 Raytracing용 구조체 확장 및 / 확장 기능 사용여부를 제공한뒤 Device를 생성해야
	// vkCmdTraceRaysKHR() 와 같은 명령어를 쓸 수 있습니다.

	std::cout << "Logical device created successfully!" << std::endl;
	return 0;
}