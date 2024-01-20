#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <fstream>
#include <stdexcept>
#include <algorithm>	// Necessary for std::clamp
#include <vector>
#include <cstring>
#include <cstdlib>
#include <cstdint>		// Necessary for uint32_t
#include <limits>		// Necessary for std::numeric_limits
#include <optional>
#include <set>

//窗口宽高
const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

//可以同时并行处理的帧数
const int MAX_FRAMES_IN_FLIGHT = 2;

//要启用的校验层列表
const std::vector<const char*> validationLayers = {
	"VK_LAYER_KHRONOS_validation"
};

//所需的设备扩展列表
const std::vector<const char*> deviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

//创建 VkDebugUtilsMessengerEXT 对象，但创建函数不会被 Vulkan 库自动加载，所以需要手动加载，创建代理函数来载入创建函数
VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	if (func != nullptr) {
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	} else {
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

//创建 DestroyDebugUtilsMessengerEXT 的代理函数
void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func != nullptr) {
		func(instance, debugMessenger, pAllocator);
	}
}

//队列族结构体
struct QueueFamilyIndices {
	//支持绘制指令的队列族
	std::optional<uint32_t> graphicsFamily;
	//支持表现的队列族
	std::optional<uint32_t> presentFamily;

	bool isComplete() {
		return graphicsFamily.has_value() && presentFamily.has_value();
	}
};

//交换链细节信息
struct SwapChainSupportDetails {
	//基础表面特性（交换链最小最大图像数量，最小最大图像宽高）
	VkSurfaceCapabilitiesKHR capabilities;
	//表面格式（像素格式，颜色空间）
	std::vector<VkSurfaceFormatKHR> formats;
	//可用的呈现模式
	std::vector<VkPresentModeKHR> presentModes;
};

class HelloTriangleApplication {
public:
	void run() {

		//初始化 GLFW
		initWindow();
		//初始化 Vulkan 对象
		initVulkan();
		//直到窗口关闭终止
		mainLoop();
		//资源清理
		cleanup();
	}

private:
	//窗口句柄
	GLFWwindow* window;

	//实例句柄
	VkInstance instance;
	//存储回调函数信息
	VkDebugUtilsMessengerEXT debugMessenger;
	//窗口表面
	VkSurfaceKHR surface;

	//存储显卡信息，该对象会在 VkInstance 清楚时自动清除自己
	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
	//逻辑设备，作为与物理设备交互的接口
	VkDevice device;

	//存储逻辑设备的队列句柄，会随着逻辑设备的清楚而自动清楚
	VkQueue graphicsQueue;
	//呈现队列句柄
	VkQueue presentQueue;

	//存储交换链
	VkSwapchainKHR swapChain;
	//存储图像句柄
	std::vector<VkImage> swapChainImages;
	//存储交换链图像格式
	VkFormat swapChainImageFormat;
	//存储交换链图像范围
	VkExtent2D swapChainExtent;
	//存储图像视图
	std::vector<VkImageView> swapChainImageViews;
	//存储所有帧缓冲对象
	std::vector<VkFramebuffer> swapChainFramebuffers;

	//存储渲染流程
	VkRenderPass renderPass;
	//存储 Layout
	VkPipelineLayout pipelineLayout;
	//存储管线对象
	VkPipeline graphicsPipeline;

	//指令池，管理指令缓冲对象使用的内存并负责指令缓冲对象的分配
	VkCommandPool commandPool;
	//存储指令缓冲对象
	std::vector<VkCommandBuffer> commandBuffers;

	//图像被获取，可以开始渲染的信号量
	std::vector<VkSemaphore> imageAvailableSemaphores;
	//渲染已经结果，可以开始呈现的信号量
	std::vector<VkSemaphore> renderFinishedSemaphores;
	//为每一帧创建栅栏，来进行 CPU 和 GPU 之间的同步
	std::vector<VkFence> inFlightFences;
	//追踪当前渲染的是哪一帧
	uint32_t currentFrame = 0;

	//标记窗口是否发生改变
	bool framebufferResized = false;

	void initWindow() {
		//初始化 GLFW 窗口
		glfwInit();

		//初始化 GLFW 库，GLFW_NO_API 显式地阻止自动创建 OpenGL 上下文
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

		//禁止窗口大小改变
		//glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

		//存储创建的窗口句柄
		window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
		//将 this 指针存储在 GLFW 窗口相关的数据中
		glfwSetWindowUserPointer(window, this);
		//设置处理窗口大小改变的回调函数
		glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
	}

	//静态函数才能用作回调函数
	static void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
		auto app = reinterpret_cast<HelloTriangleApplication*>(glfwGetWindowUserPointer(window));
		app->framebufferResized = true;
	}


	void initVulkan() {
		createInstance();
		setupDebugMessenger();
		createSurface();
		pickPhysicalDevice();
		createLogicalDevice();
		createSwapChain();
		createImageViews();
		createRenderPass();
		createGraphicsPipeline();
		createFramebuffers();
		createCommandPool();
		createCommandBuffers();
		createSyncObjects();
	}

	void mainLoop() {
		//窗口不关闭
		while (!glfwWindowShouldClose(window)) {
			//执行事件处理
			glfwPollEvents();
			//绘制三角形
			drawFrame();
		}

		//等待逻辑设备操作结束执行时才销毁窗口
		vkDeviceWaitIdle(device);
	}

	void cleanupSwapChain() {
		//消除帧缓冲
		for (size_t i = 0; i < swapChainFramebuffers.size(); i++) {
			vkDestroyFramebuffer(device, swapChainFramebuffers[i], nullptr);
		}
		
		//清楚图像视图
		for (size_t i = 0; i < swapChainImageViews.size(); i++) {
			vkDestroyImageView(device, swapChainImageViews[i], nullptr);
		}

		//清除交换链
		vkDestroySwapchainKHR(device, swapChain, nullptr);
	}

	void cleanup() {
		cleanupSwapChain();

		vkDestroyPipeline(device, graphicsPipeline, nullptr);
		vkDestroyPipelineLayout(device, pipelineLayout, nullptr);

		vkDestroyRenderPass(device, renderPass, nullptr);
		
		//消除信号量
		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
			vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
			vkDestroyFence(device, inFlightFences[i], nullptr);
		}

		//消除指令池
		vkDestroyCommandPool(device, commandPool, nullptr);

		//清理逻辑设备
		vkDestroyDevice(device, nullptr);

		//清理 VkDebugUtilsMessengerEXT 对象
		if (enableValidationLayers) {
			DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
		}

		//消除表面
		vkDestroySurfaceKHR(instance, surface, nullptr);
		//清除实例
		vkDestroyInstance(instance, nullptr);
		
		//清除窗口
		glfwDestroyWindow(window);
		
		// GLFW 停止
		glfwTerminate();
	}

	//重建交换链
	void recreateSwapChain() {
		//最小化时，停止渲染
		int width = 0, height = 0;
		glfwGetFramebufferSize(window, &width, &height);
		while (width == 0 || height == 0) {
			glfwGetFramebufferSize(window, &width, &height);
			glfwWaitEvents();
		}

		//等待设备处于空闲状态，避免在对象使用过程中将其清除重建
		vkDeviceWaitIdle(device);


		cleanupSwapChain();

		createSwapChain();
		createImageViews();
		createFramebuffers();
	}

	//创建实例
	void createInstance() {
		//检查校验层
		if (enableValidationLayers && !checkValidationLayerSupport()) {
			throw std::runtime_error("validation layers requested, but not available!");
		}

		//应用程序信息
		VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "Hello Triangle";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "No Engine";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_0;

		//所需全局扩展和校验层的结构体
		VkInstanceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;
		
		//全局扩展
		auto extensions = getRequiredExtensions();
		createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
		createInfo.ppEnabledExtensionNames = extensions.data();

		//创建回调函数创建信息指针作为 p next
		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
		//全局校验层
		if (enableValidationLayers) {
			createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			createInfo.ppEnabledLayerNames = validationLayers.data();

			populateDebugMessengerCreateInfo(debugCreateInfo);
			createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
		} else {
			createInfo.enabledLayerCount = 0;

			createInfo.pNext = nullptr;
		}

		//创建 Vulkan 实例
		if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
			throw std::runtime_error("failed to create instance!");
		}
	}

	/*
	//获取扩展信息
	//获取扩展数量
	uint32_t extensionCount = 0;
	vkEnumerateinstanceExtensionProperties(nullptr, &extensionCount, nullptr);
	//存储扩展信息
	std::vector<VkExtensionProperties> Extensions(extensionCount);
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, Extensions.data());
	std::cout << "available extensions:" << std::endl;

	for (const auto& extension : Extensions)
	{
		std::cout << "\t" << extension.extensionName << std::endl;
	}
	*/

	//生成回调函数的创建信息
	void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
		createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		createInfo.pfnUserCallback = debugCallback;
	}

	// 窗口表面
	void createSurface() {
		//下面这个函数是跨平台的
		if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
			throw std::runtime_error("failed to create window surface!");
		}
	}
	//初始化回调函数
	void setupDebugMessenger() {
		if (!enableValidationLayers) return;

		VkDebugUtilsMessengerCreateInfoEXT createInfo;
		populateDebugMessengerCreateInfo(createInfo);

		if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
			throw std::runtime_error("failed to set up debug messenger!");
		}
	}

	//接受调试信息的回调函数
	/*
		mseeageSeverity 指定消息级别（可能是诊断信息、资源创建之类的信息、警告信息、不合法和可能造成崩溃的信息）
		messageType 消息类型（与规范性能无关，违反规范或发生可能的错误，进行了可能影响 Vulkan 性能的行为
		pCallbackData 指向一个结构体的指针，结构体包含：以 null 结尾包含调试信息的字符串，存储有和消息相关的 Vulkan 对象句柄的数组、数组中的对象个数
		pUserData 设置回调函数时，传递的数据指针
	*/

	//物理设备和队列族
	//选择一个物理设备，选第一个满足需求的设备
	void pickPhysicalDevice() {
		//请求显卡数量
		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

		if (deviceCount == 0) {
			throw std::runtime_error("failed to find GPUs with Vulkan support!");
		}
		
		//存储 VkPhysicalDevice 对象
		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

		//检查设备，并选择使用第一个满足需求的设备
		for (const auto& device : devices) {
			if (isDeviceSuitable(device)) {
				physicalDevice = device;
				break;
			}
		}

		if (physicalDevice == VK_NULL_HANDLE) {
			throw std::runtime_error("failed to find a suitable GPU!");
		}

		//给设备按照特性加权打分，分高者任之
		//std::multimap<int, VkPhysicalDevice> candidates;
		// for (const auto& device : device) {
		//	int score = rateDeviceSuitability(device);
		//	candidates.insert(std::make_pair(score, device));
		//} else {
		//	throw std::runtime_error("failed to find a suitable GPU!")
		//}
	}

	//逻辑设备和队列
	void createLogicalDevice() {
		QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

		//创建所有使用的队列族
		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		//创建每一个不同的队列族
		std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };

		float queuePriority = 1.0f;
		for (uint32_t queueFamily : uniqueQueueFamilies) {
			//该结构体描述了针对一个队列族所需队列数量
			VkDeviceQueueCreateInfo queueCreateInfo{};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.queueFamilyIndex = queueFamily;
			queueCreateInfo.queueCount = 1;
			//优先级控制指令缓冲的执行顺序
			queueCreateInfo.pQueuePriorities = &queuePriority;
			queueCreateInfos.push_back(queueCreateInfo);
		}

		//指定应用程序使用的设备特性
		VkPhysicalDeviceFeatures deviceFeatures{};

		//创建逻辑设备
		VkDeviceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

		createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
		createInfo.pQueueCreateInfos = queueCreateInfos.data();

		createInfo.pEnabledFeatures = &deviceFeatures;

		//启用交换链扩展
		createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
		createInfo.ppEnabledExtensionNames = deviceExtensions.data();

		//设备和实例使用相同的校验层，不需要额外的扩展支持
		if (enableValidationLayers) {
			createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			createInfo.ppEnabledLayerNames = validationLayers.data();
		} else {
			createInfo.enabledLayerCount = 0;
		}

		//之前把启用交换链扩展这两行代码放在 vkCreateDevice 下面了，导致我的虚拟设备没有开交换链扩展，所以后面创建交换链扩展时一直创建失败
		if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) {
			throw std::runtime_error("failed to create logical device!");
		}

		//获取指定队列族的队列句柄
		vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
		vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
	}

	//交换链
	//创建交换链
	void createSwapChain() {
		SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);

		VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
		VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
		VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

		//设置交换链中图像个数，至少设置为最小个数 +1 来实现三重缓冲
		uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
		//maxImageCount = 0 意味着只要内存满足，可以使用任意数量的图像
		if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
			imageCount = swapChainSupport.capabilities.maxImageCount;
		}

		//填写创建信息结构体
		VkSwapchainCreateInfoKHR createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = surface;

		createInfo.minImageCount = imageCount;
		createInfo.imageFormat = surfaceFormat.format;
		createInfo.imageColorSpace = surfaceFormat.colorSpace;
		createInfo.imageExtent = extent;
		//指定每个图像包含的层次，通常是 1，VR 相关程序会使用更多的层次
		createInfo.imageArrayLayers = 1;
		//指定我们将在图像上进行怎样的操作，下面就仅仅是绘制操作（还可以设置为可以后期处理的操作）
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		//指定多个队列族使用交换链图像的方式
		QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
		uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

		//图形队列和呈现队列不再同一个队列
		//通过图形队列在交换链图像上进行绘制，将图像提交给呈现队列来显示
		if (indices.graphicsFamily != indices.presentFamily) {
			//图像可以在多个队列族使用，不需要显示地改变所有权
			//协同模式
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			//指定共享所有全的队列族
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices = queueFamilyIndices;
		} else {
			//一张图像同时只能被一个队列族拥有，显示改变所有权才能换族，这一模式性能最佳
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		}

		//对交换链中图像指定一个固定的交换操作，currentTransform 为不操作
		createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
		//指定 alpha 通道是否被用来和窗口系统中其他窗口混合，下面将忽略 alpha 通道，设置为不透明
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		//设置呈现模式
		createInfo.presentMode = presentMode;
		//表示不关心被窗口系统其他窗口遮挡的像素颜色
		createInfo.clipped = VK_TRUE;

		//创建交换链
		if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) != VK_SUCCESS) {
			throw std::runtime_error("failed to create swap chain!");
		}

		//获取交换链图像句柄
		vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
		swapChainImages.resize(imageCount);
		vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());

		//存储交换链的图像格式和范围
		swapChainImageFormat = surfaceFormat.format;
		swapChainExtent = extent;
	}

	//图像视图
	void createImageViews() {
		//分配空间
		swapChainImageViews.resize(swapChainImages.size());

		//遍历交换链所有图像来创建图像视图
		for (size_t i = 0; i < swapChainImages.size(); i++) {
			//填写图像视图创建信息结构体
			VkImageViewCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			createInfo.image = swapChainImages[i];
			//指定图像数据的解释方式，viewType 指定图像被看作是几维纹理
			createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			createInfo.format = swapChainImageFormat;
			//components 用于进行图像颜色通道的映射，此处使用默认映射
			createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
			//subresourceRange 用于指定图像的用途和图像哪一部分可以被访问，此处图像被用作渲染目标，只存在一个图层
			createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			createInfo.subresourceRange.baseMipLevel = 0;
			createInfo.subresourceRange.levelCount = 1;
			createInfo.subresourceRange.baseArrayLayer = 0;
			createInfo.subresourceRange.layerCount = 1;

			//创建图像视图
			if (vkCreateImageView(device, &createInfo, nullptr, &swapChainImageViews[i]) != VK_SUCCESS) {
				throw std::runtime_error("failed to create image views!");
			}
		}
	}

	//渲染流程
	//设置用于渲染的帧缓冲附着
	void createRenderPass() {
		VkAttachmentDescription colorAttachment{};
		colorAttachment.format = swapChainImageFormat;
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		//用于指定渲染前后对附着中的数据进行的操作（对颜色和深度缓冲起效），load 为前
		//使用一个常量值来清除附着内容，这里会在每次渲染前用黑色清除帧缓冲
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		//渲染后的内容会被存储起来，以便之后读取
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		//下面两个对模板缓冲起效，但不用，所以不关心 //TODO:
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		//使渲染后的图像可以被交换链呈现
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		//子流程和附着引用
		VkAttachmentReference colorAttachmentRef{};
		//指定要引用的附着索引
		colorAttachmentRef.attachment = 0;
		//指定进行子流程时引用的附着使用的布局方式
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		//描述子流程
		VkSubpassDescription subpass{};
		//指定这是一个图形渲染的子流程
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		//指定引用的颜色附着
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;

		//配置子流程依赖
		VkSubpassDependency dependency{};
		//src 指的渲染流程开始前的子流程，dst 表示渲染阶段结束后的子流程
		//指定被依赖的子流程的索引
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		//依赖被依赖的子流程的索引
		dependency.dstSubpass = 0;
		//需要等待颜色附着输出这一管线阶段
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		//子流程将进行的操作类型
		dependency.srcAccessMask = 0;
		//指定需要等待的管线阶段
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		//子流程将进行的操作类型
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		//渲染流程
		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = 1;
		renderPassInfo.pAttachments = &colorAttachment;
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;

		if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
			throw std::runtime_error("failed to create render pass!");
		}
	}

	//图形管线
	void createGraphicsPipeline() {
		auto vertShaderCode = readFile(".\\shaders\\vert.spv");
		auto fragShaderCode = readFile(".\\shaders\\frag.spv");

		//着色器模块只在管线创建时需要
		VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
		VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);

		//填写指定着色器阶段创建信息结构体
		VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
		vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertShaderStageInfo.module = vertShaderModule;
		//pname 用于指定阶段调用的着色器函数
		vertShaderStageInfo.pName = "main";

		//填写指定着色器阶段创建信息结构体
		VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
		fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragShaderStageInfo.module = fragShaderModule;
		//pname 用于指定阶段调用的着色器函数
		fragShaderStageInfo.pName = "main";

		VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };
		//下面来设置固定功能

		//用来描述传递给顶点着色器的顶点数据格式，因为目前是硬编码顶点数据，所以其实不用描述，赋值 0
		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

		//两个 Descriptions 用于描述顶点数据组织信息的结构体数据
		vertexInputInfo.vertexBindingDescriptionCount = 0;
		vertexInputInfo.vertexAttributeDescriptionCount = 0;

		//输入装配
		//下面这个结构体描述：顶点数据定义哪种类型的几何图元，以及是否启用几何图元重启
		VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		//每三个顶点构成一个三角形图元
		inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssembly.primitiveRestartEnable = VK_FALSE;

		//定义视口
		
		//定义裁剪矩形

		//组合视口和裁剪矩形
		VkPipelineViewportStateCreateInfo viewportState{};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.scissorCount = 1;

		//光栅化
		VkPipelineRasterizationStateCreateInfo rasterizer{};
		rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		//丢弃近平面和远平面外的片段
		rasterizer.depthClampEnable = VK_FALSE;
		//允许所有几何图元通过光栅化
		rasterizer.rasterizerDiscardEnable = VK_FALSE;
		//指定几何图元生成片段（像素）的方式，整个多边形包括内部，其他 mode 需要启用 GPU 特性 
		rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
		//线宽大于 1 需要启用 GPU 特性
		rasterizer.lineWidth = 1.0f;
		//剔除背面
		rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
		//顺时针的定点顺序是正面
		rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
		//添加一个常量值或者一个基于片段所处线段斜率值得到的变量值到深度上。对阴影贴图有用，这里关闭
		rasterizer.depthBiasEnable = VK_FALSE;

		//多重采样，组合多个不同多边形产生的片段的颜色来决定最终的像素颜色，代价较小，但需要启用 GPU 特性，暂时关闭
		VkPipelineMultisampleStateCreateInfo multisampling{};
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

		//深度和模板测试先不管
		
		//颜色混合

		//第一种混合方式：对每个绑定的帧缓冲进行单独的颜色混合配置，先关闭
		VkPipelineColorBlendAttachmentState colorBlendAttachment{};
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_FALSE;

		//可以设置全局混合常量
		VkPipelineColorBlendStateCreateInfo colorBlending{};
		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		
		//第二种混合方式：使用位运算组合旧值和新值来混合颜色，这里关闭，开启的话会禁用第一种
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &colorBlendAttachment;
		colorBlending.blendConstants[0] = 0.0f; // Optional
		colorBlending.blendConstants[1] = 0.0f; // Optional
		colorBlending.blendConstants[2] = 0.0f; // Optional
		colorBlending.blendConstants[3] = 0.0f; // Optional

		//动态状态
		//填写以下结构体指定要动态修改的状态
		std::vector<VkDynamicState> dynamicStates = {
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_SCISSOR
		};
		VkPipelineDynamicStateCreateInfo dynamicState{};
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
		dynamicState.pDynamicStates = dynamicStates.data();

		//管线布局
		//使用 layout 定义着色器的 uniform，虽然现在不用，但是也要定义
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 0; // Optional
		pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional

		if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create pipeline layout!");
		}

		//创建管线对象
		VkGraphicsPipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = 2;
		pipelineInfo.pStages = shaderStages;
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &inputAssembly;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizer;
		pipelineInfo.pMultisampleState = &multisampling;
		pipelineInfo.pColorBlendState = &colorBlending;
		pipelineInfo.pDynamicState = &dynamicState;
		pipelineInfo.layout = pipelineLayout;
		pipelineInfo.renderPass = renderPass;
		pipelineInfo.subpass = 0;

		//用于以一个创建好的图形管线为基础创建一个新的图形管线
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional

		//创建管线对象
		if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS) {
			throw std::runtime_error("failed to create graphics pipeline!");
		}

		//清除 shaderModule
		vkDestroyShaderModule(device, fragShaderModule, nullptr);
		vkDestroyShaderModule(device, vertShaderModule, nullptr);
	}

	//帧缓冲
	void createFramebuffers() {
		swapChainFramebuffers.resize(swapChainImageViews.size());
		//为交换链的每一个图像视图对象创建帧缓冲

		for (size_t i = 0; i < swapChainImageViews.size(); i++) {
			VkImageView attachments[] = {
				swapChainImageViews[i]
			};

			VkFramebufferCreateInfo framebufferInfo{};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			//指定帧缓冲需要兼容的渲染流程对象
			framebufferInfo.renderPass = renderPass;
			//指定附着
			framebufferInfo.attachmentCount = 1;
			framebufferInfo.pAttachments = attachments;
			framebufferInfo.width = swapChainExtent.width;
			framebufferInfo.height = swapChainExtent.height;
			//帧缓冲图像层数
			framebufferInfo.layers = 1;

			if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS) {
				throw std::runtime_error("failed to create framebuffer!");
			}
		}
	}
	
	//指令缓冲
	void createCommandPool() {
		QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice);

		VkCommandPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		//绘制指令可以被提交给图形操作的队列
		poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

		if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
			throw std::runtime_error("failed to create command pool!");
		}
	}

	//分配指令缓冲
	void createCommandBuffers() {
		//绘制操作是在帧缓冲上进行的
		commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		//使用的指令池和需要分配的指令缓冲个数
		allocInfo.commandPool = commandPool;
		//主要缓冲指令对象，可被提交到队列执行，不能被其他指令缓冲对象调用
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();

		if (vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate command buffers!");
		}
	}

	//记录指令到指令缓冲
	void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		
		//可以使得在上一帧还未结束渲染时，提交下一帧的渲染指令 //TODO：

		if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
			throw std::runtime_error("failed to begin recording command buffer!");
		}

		//开始渲染流程
		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = renderPass;
		renderPassInfo.framebuffer = swapChainFramebuffers[imageIndex];
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = swapChainExtent;

		//设置清除颜色
		VkClearValue clearColor = { {{0.0f, 0.0f, 0.0f, 1.0f}} };
		renderPassInfo.clearValueCount = 1;
		renderPassInfo.pClearValues = &clearColor;
		
		//VK_SUBPASS_CONTENTS_INLINE 指的是指令只在主要指令缓冲，没有辅助指令缓冲
		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
			
			//绑定图形管线
			vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

			//定义视口
			VkViewport viewport{};
			viewport.x = 0.0f;
			viewport.y = 0.0f;
			viewport.width = static_cast<float>(swapChainExtent.width);
			viewport.height = static_cast<float>(swapChainExtent.height);
			viewport.minDepth = 0.0f;
			viewport.maxDepth = 1.0f;
			vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

			//定义裁剪矩形
			VkRect2D scissor{};
			scissor.offset = { 0, 0 };
			scissor.extent = swapChainExtent;
			//TODO；？？
			vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

			//绘制三角形，提交绘制操作到指令缓冲
			vkCmdDraw(commandBuffer, 3, 1, 0, 0);
		
		//结束渲染流程
		vkCmdEndRenderPass(commandBuffer);

		//结束指令到指令缓冲
		if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
			throw std::runtime_error("failed to record command buffer!");
		}
	}

	//创建同步对象
	void createSyncObjects() {
		imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

		VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		//设置初始状态为已发出信号
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
				vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
				vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) {
				throw std::runtime_error("failed to create synchronization objects for a frame!");
			}
		}
	}

	void drawFrame() {
		//等待当前帧所使用的指令缓冲结束执行
		vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

		//从交换链获取图像
		uint32_t imageIndex;
		//第三个参数是图像获取的超时时间，无符号 64 位最大整数表示禁用获取超时
		VkResult result = vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

		//如果交换链过期就重建
		if (result == VK_ERROR_OUT_OF_DATE_KHR) {
			recreateSwapChain();
			return;
		} else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
			throw std::runtime_error("failed to acquire swap chain image!");
		}

		//重置栅栏
		vkResetFences(device, 1, &inFlightFences[currentFrame]);

		vkResetCommandBuffer(commandBuffers[currentFrame], /*VkCommandBufferResetFlagBits*/ 0);
		recordCommandBuffer(commandBuffers[currentFrame], imageIndex);

		//提交指令缓冲
		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		//指定队列开始执行前需要等待的信号量，以及需要等待的管线阶段
		VkSemaphore waitSemaphores[] = { imageAvailableSemaphores[currentFrame] };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;

		//指定实际被提交执行的指令缓冲对象
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffers[currentFrame];

		//只当在指令缓冲执行结束后发出信号的信号量对象
		VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[currentFrame] };
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		//提交指令缓冲给图形指令队列，第四个参数指定缓冲执行结束后需要发起的信号栅栏对象
		if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS) {
			throw std::runtime_error("failed to submit draw command buffer!");
		}

		//配置呈现信息
		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

		//指定开始呈现操作时需要等待的信号量
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;

		//指定呈现图像的交换链，以及图像在交换链中的索引
		VkSwapchainKHR swapChains[] = { swapChain };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;

		presentInfo.pImageIndices = &imageIndex;

		//请求交换链进行图像呈现操作
		result = vkQueuePresentKHR(presentQueue, &presentInfo);

		//当交换链过期或者表面属性已经不能准确匹配时
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized) {
			framebufferResized = false;
			recreateSwapChain();
		} else if (result != VK_SUCCESS) {
			throw std::runtime_error("failed to present swap chain image!");
		}

		//更新 currentFrame
		currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
	}

	//将着色器字节码在管线上使用，需要 VkShaderModule 对象
	VkShaderModule createShaderModule(const std::vector<char>& code) {
		VkShaderModuleCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = code.size();
		//需要将存储字节码的数组指针转换下
		createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

		//创建 VkShaderModule 对象
		VkShaderModule shaderModule;
		if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
			throw std::runtime_error("failed to create shader module!");
		}

		return shaderModule;
	}

	//选择合适的表面格式
	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
		
		////如果表面没有自己的首选格式
		// if (availableFormats.size() == 1 && availableFormats[0].format == VK_FORMAT_UNDEFINED) {
		////返回我们设定的格式	
		// return { VK_FORMAT_B8G8R8A8_UNORM, VK_COLORSPACE_SRGB_NONLINEAR_KHR };
		// }

		//如果 Vulkan 返回了一个格式列表，返回指的是 availableFormats
		//检查我们想要设定的格式是否存在于这个列表中
		for (const auto& availableFormat : availableFormats) {
			if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
				return availableFormat;
			}
		}
		
		//如果不能在列表中找到我们想要的格式，可以打分找出最合适的格式也可以直接选第一个
		return availableFormats[0];
	}

	//选择最佳的可用呈现模式
	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
		////一般都有 FIFO 模式
		//VkPresentModeKHR bestMode = VK_PRESENT_MODE_FIFO_KHR;
		for (const auto& availablePresentMode : availablePresentModes) {
			//三重缓冲综合表现最佳，既避免了撕裂现象，同时有较低的延迟
			if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
				return availablePresentMode;
			}
		}

		//但很多驱动程序对 FIFO 支持不够好，所以应该使用立即显示模式
		//没有三重缓冲就返回该模式
		//TODO: 以上注释需要进一步修改

		return VK_PRESENT_MODE_FIFO_KHR;
	}

	//选择交换范围（即交换链中图像的分辨率）
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
		//不允许自己选择交换范围
		if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
			return capabilities.currentExtent;
		} else {
			//设置为当前帧缓冲的实际大小
			int width, height;
			glfwGetFramebufferSize(window, &width, &height);

			//允许自己选择对于窗口最合适的交换范围
			VkExtent2D actualExtent = {
				static_cast<uint32_t>(width),
				static_cast<uint32_t>(height)
			};

			actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
			actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

			return actualExtent;
		}
	}

	//用于填写上面的结构体
	SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device) {
		SwapChainSupportDetails details;

		//查询基础表面特性
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

		//查询表面支持的格式
		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

		if (formatCount != 0) {
			details.formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
		}
		
		//查询支持的呈现格式
		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

		if (presentModeCount != 0) {
			details.presentModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
		}

		return details;
	}

	//检查获取的设备是否满足我们的需求
	bool isDeviceSuitable(VkPhysicalDevice device) {
		////基础的设备属性，如名称、类型、支持的 Vulkan 版本
		//VkPhysicalDeviceProperties deviceProperties,
		////纹理压缩、64位浮点和多视口渲染等特性
		//VkPhysicalDeviceFeatures deviceFeatures;
		//vkGetPhysicalDeviceProperties(device, &deviceProperties);
		//vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
		//
		//return deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU && deviceFeatures.geometryShader;

		QueueFamilyIndices indices = findQueueFamilies(device);

		//检测所需设备扩展是否可行
		bool extensionsSupported = checkDeviceExtensionSupport(device);

		//检测交换链的能力是否满足需求
		bool swapChainAdequate = false;
		if (extensionsSupported) {
			SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
			swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
		}

		return indices.isComplete() && extensionsSupported && swapChainAdequate;
	}

	/*
	* int rateDeviceSuitability(VkPhysicalDevice device) {
	* //基础的设备属性，如名称、类型、支持的 Vulkan 版本
	* VkPhysicalDeviceProperties deviceProperties;
	* //纹理压缩，64 位浮点和多视口渲染等特性
	* VkPhysicalDeviceFeatures deviceFeatures;
	* vkGetPhysicalDeviceProperties(device, &deviceProperties);
	* vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
	* int score = 0;
	* 
	* //独显加分
	* if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
	*	score += 1000;
	* }
	* //纹理最大尺寸加分
	* score += deviceProperties.limits.maxImageDimension2D;
	* if (!deviceFeatures.geometryShader)
		return 0;
	* return score;
	*/

	//查找并返回需求的队列族的索引
	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device) {
		QueueFamilyIndices indices;
		
		//获取设备队列族个数
		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

		//存储队列族的 Properties
		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

		//队列族包含很多信息，如支持的操作类型、该队列族可创建的队列个数
		//找到一个支持的队列族
		int i = 0;
		for (const auto& queueFamily : queueFamilies) {
			//支持绘制指令的队列族索引
			if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
				indices.graphicsFamily = i;
			}

			//支持表现的队列族索引
			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

			if (presentSupport) {
				indices.presentFamily = i;
			}

			if (indices.isComplete()) {
				break;
			}

			i++;
		}

		return indices;
	}

	//检测所需的设备扩展
	bool checkDeviceExtensionSupport(VkPhysicalDevice device) {
		uint32_t extensionCount;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

		std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

		//枚举所有可用的扩展，从我们需要的集合中剔除，如果集合空了，说明可用的扩展可以覆盖我们的需求
		for (const auto& extension : availableExtensions) {
			requiredExtensions.erase(extension.extensionName);
		}

		return requiredExtensions.empty();
	}

	//根据是否启用校验层， 返回所需的扩展列表
	std::vector<const char*> getRequiredExtensions() {
		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions;
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

		if (enableValidationLayers) {
			extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		}

		return extensions;
	}

	//检查配置的校验层是否被支持
	bool checkValidationLayerSupport() {
		//获取所有可用的校验层列表
		uint32_t layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

		std::vector<VkLayerProperties> availableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

		//检查是否所有 validationLayers 列表中的校验层可以在 availableLayers 中找到
		for (const char* layerName : validationLayers) {
			bool layerFound = false;

			for (const auto& layerProperties : availableLayers) {
				if (strcmp(layerName, layerProperties.layerName) == 0) {
					layerFound = true;
					break;
				}
			}

			if (!layerFound) {
				return false;
			}
		}

		return true;
	}

	//载入二进制文件的辅助函数
	static std::vector<char> readFile(const std::string& filename) {
		// ate 从文件尾部开始读取，binary 以二进制形式读取文件
		std::ifstream file(filename, std::ios::ate | std::ios::binary);

		if (!file.is_open()) {
			throw std::runtime_error("failed to open file!");
		}

		//使用 ate 可以根据读取位置确定文件大小，分配空间
		size_t fileSize = (size_t)file.tellg();
		//std::cout << filename << "   " << fileSize << std::endl;
		std::vector<char> buffer(fileSize);

		//跳到文件头部来读取整个文件
		file.seekg(0);
		file.read(buffer.data(), fileSize);

		file.close();

		return buffer;
	}

	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
		std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

		return VK_FALSE;
	}
};

int main() {
	HelloTriangleApplication app;

	try {
		app.run();
	} catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
