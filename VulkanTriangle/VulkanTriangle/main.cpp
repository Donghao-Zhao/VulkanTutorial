/* Author Donghao Zhao */

//
//
#define GLFW_INCLUDE_VULKAN

//
//
#include <GLFW/glfw3.h>

//
//
#include <glm/glm.hpp>

// Input and output library, necessary for input and output operation
// 输入输出流库
#include <iostream>

// File stream library, necessary for file operation
// 文件流库
#include <fstream>

// Standard exception library, necessary for reporting and propagating errors
// 标准异常库
#include <stdexcept>

// Algorithm library, necessary for std::clamp
// 算法库，std::clamp 方法需要
#include <algorithm>

// Vector Container library
// vector 容器库
#include <vector>

// C string library
// C string 库
#include <cstring>

// C standard library, necessary for EXIT_SUCCESS and EXIT_FAILURE macro
// C 标准库，EXIT_SUCCESS and EXIT_FAILURE 宏需要
#include <cstdlib>

// C standard integer, necessary for uint32_t
// C 标准整型库，uint32_t 需要
#include <cstdint>

// Determine the minimum and maximum of data types, and necessary for std::numeric_limits
// 用以确定数据类型的范围，std::numeric_limits 需要
#include <limits>

// Array library
// array 库
#include <array>

// ?
// ？
#include <optional>

// Set library
// set 库
#include <set>

// Map library
// map 库
#include <map>

// The width of the window
// 窗口的宽
const uint32_t WIDTH = 800;
// The height of the window
// 窗口的高
const uint32_t HEIGHT = 600;

// 可以同时并行处理的帧数
const int MAX_FRAMES_IN_FLIGHT = 2;

// The validation layers that we would like to enable
// 要启用的校验层
const std::vector<const char*> validationLayers = {
	// All useful standard validation layers
	// 所有的标准校验层
	"VK_LAYER_KHRONOS_validation"
};

// The device extensions we desired
// 所需的设备扩展
const std::vector<const char*> deviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

// Enable validation with respect to the NDEBUG C++ standard macro
// 根据 C++ NDEBUG 宏定义来确定是否启用校验层
#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

// Helper function to create the debugMessenger object
// 创建 debugMessenger 对象
VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
	// An extension function will not be automatically loaded, so use vkGetInstanceProcAddr to look up its address, and create a proxy function to do the labor
	// Vulkan 扩展中的函数不会被 Vulkan 库自动加载，所以需要借助 vkGetInstanceProcAddr 找到函数的地址，然后创建代理函数来载入创建函数 vkCreateDebugUtilsMessengerEXT
	// Prefix PFN: Pointer to FuNction
		// VkResult (*func) (VkInstance_T*, const VkDebugUtilsMessengerCreateInfoEXT*, const VkAllocationCallbacks*, VkDebugUtilsMessengerEXT_T**)
		//  = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	
	// If the func above is not a null pointer
	// 如果 func 不是空指针
	if (func != nullptr) {
		// Use the proxy function to create the debugMessenger object
		// 使用代理函数创建 debugMessenger 对象
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	} else {
		// Return the error code that the extension is not present
		// 返回扩展不在场的错误码
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

// Helper function to destroy the debugMessenger object
// 摧毁 debugMessenger 对象
void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
	// Get the address of the vkDestroyDebugUtilsMessengerEXT extension function
	// 找到 vkDestroyDebugUtilsMessengerEXT 扩展函数的地址
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	// If the func above is not a null pointer
	// 如果 func 不是空指针
	if (func != nullptr) {
		// Use the proxy function to destroy the debugMessenger object
		// 使用代理函数摧毁 debugMessenger 对象
		func(instance, debugMessenger, pAllocator);
	}
}

// 
// 队列族结构体
struct QueueFamilyIndices {
	// 支持绘制指令的队列族
	std::optional<uint32_t> graphicsFamily;
	// 支持表现的队列族
	std::optional<uint32_t> presentFamily;

	bool isComplete() {
		return graphicsFamily.has_value() && presentFamily.has_value();
	}
};

// 交换链细节信息
struct SwapChainSupportDetails {
	// 基础表面特性（交换链最小最大图像数量，最小最大图像宽高）
	VkSurfaceCapabilitiesKHR capabilities;
	// 表面格式（像素格式，颜色空间）
	std::vector<VkSurfaceFormatKHR> formats;
	// 可用的呈现模式
	std::vector<VkPresentModeKHR> presentModes;
};

// The vertex structure
// 顶点数据结构体
struct Vertex {
	// The coordinate of the vertex, 2 dimensional data
	// 顶点坐标 2维
	glm::vec2 pos;
	// The RGB color of attached with the vertex
	// 顶点颜色 RGB
	glm::vec3 color;

	/* Configure from where to acquire the vertex information */
	// 配置从何处获取顶点信息
	static VkVertexInputBindingDescription getBindingDescription() {
		// Create the instance regarding the description to bind the vertex
		// 创建实例
		VkVertexInputBindingDescription bindingDescription{};
		// Set the index of the binding in the array of binding to 0
		// 设置在 binding 数组中的索引
		bindingDescription.binding = 0;
		// Set the stride of the vertex data to the size of it
		// 设置每个顶点数据的步长，即每个顶点数据所占的字节数
		bindingDescription.stride = sizeof(Vertex);
		// Set the input rate to per vertex
		// 设置输入速率为每顶点一次
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		// Return the instance
		// 返回实例
		return bindingDescription;										
	}

	/* Configure how to extract attribute from the input data */
	/* 配置如何从顶点数据中提取特征 */
	static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions() {
		// Construct an array with size 2 regarding to the VkVertexInputAttributeDescription
		// 创建能放下 2 个 VkVertexInputAttributeDescription 的顶点特征描述数组
		std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions{};

		/* Coordinate data of the vertex */
		/* 位置坐标数据 */

		// The coordinate data sit on the binding 0
		// 位置坐标数据在第 0 个 binding
		attributeDescriptions[0].binding = 0;
		// At location 0
		// 在位置 0
		attributeDescriptions[0].location = 0;
		// The format of the data is 2 of 32bit signed float number
		// 数据类型是 2 个 32bit 有符号浮点数
		attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
		// The offset of the coordinate data of the vertex is the size of the member variable: pos
		// 位置坐标数据占用的空间
		attributeDescriptions[0].offset = offsetof(Vertex, pos);

		/* Color data of the vertex */
		/* 颜色数据 */

		// The coordinate data sit on the binding 0
		// 颜色数据在第 0 个 binding
		attributeDescriptions[1].binding = 0;
		// At location 1
		// 在位置 1
		attributeDescriptions[1].location = 1;
		// The format of the data is 3 of 32bit signed float number
		// 数据类型是 3 个 32bit 有符号浮点数
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		// The offset of the color data of the vertex is the size of the member variable: color
		// 颜色数据占用的空间
		attributeDescriptions[1].offset = offsetof(Vertex, color);

		// Return the attributeDescriptions instance
		// 返回配置好的顶点特征描述实例
		return attributeDescriptions;
	}
};

// The vertex data
// 顶点数据
const std::vector<Vertex> vertices = {
	// The first vertex data
	// 第一个顶点数据
	{{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
	// The second vertex data
	// 第二个顶点数据
	{{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
	// The third vertex data
	// 第三个顶点数据
	{{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}			
};

// The HelloTriangleApplication class
// HelloTriangleApplication 类
class HelloTriangleApplication {
public:
	void run() {
		// Initialize the window
		// 初始化窗口
		initWindow();
		// Initialize all Vulkan objects
		// 初始化 Vulkan 的所有对象
		initVulkan();
		// The main loop, until the exit of the window
		// 主循环（直到窗口关闭终止）
		mainLoop();
		// Resource cleanup
		// 资源清理
		cleanup();
	}

private:
	// Create a GLFW window instance
	// 创建 1 个 GLFW 窗口实例
	GLFWwindow* window;

	// Create a VkInstance instance
	// 创建 1 个 VkInstance 实例
	VkInstance instance;
	
	// Create a debugMessenger handle, EXT stands for extension
	// 创建调试信使句柄，EXT 是扩展的意思
	VkDebugUtilsMessengerEXT debugMessenger;

	// 
	// 窗口表面
	VkSurfaceKHR surface;

	// Physical device object, destroy itself when VkInstance is destroyed
	// 物理显卡对象。会在 VkInstance 被摧毁时自动摧毁自己
	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
	// 
	// 逻辑设备，作为与物理设备交互的接口
	VkDevice device;

	// 存储逻辑设备的队列句柄，会随着逻辑设备的清楚而自动清楚
	VkQueue graphicsQueue;
	// 呈现队列句柄
	VkQueue presentQueue;

	// 存储交换链
	VkSwapchainKHR swapChain;
	// 存储图像句柄
	std::vector<VkImage> swapChainImages;
	// 存储交换链图像格式
	VkFormat swapChainImageFormat;
	// 存储交换链图像范围
	VkExtent2D swapChainExtent;
	// 存储图像视图
	std::vector<VkImageView> swapChainImageViews;
	// 存储所有帧缓冲对象
	std::vector<VkFramebuffer> swapChainFramebuffers;

	// 存储渲染流程
	VkRenderPass renderPass;
	// 存储 Layout
	VkPipelineLayout pipelineLayout;
	// 存储管线对象
	VkPipeline graphicsPipeline;

	// 指令池，管理指令缓冲对象使用的内存并负责指令缓冲对象的分配
	VkCommandPool commandPool;

	// 
	VkBuffer vertexBuffer;
	// 
	VkDeviceMemory vertexBufferMemory;

	// 存储指令缓冲对象
	std::vector<VkCommandBuffer> commandBuffers;

	// 图像被获取，可以开始渲染的信号量
	std::vector<VkSemaphore> imageAvailableSemaphores;

	// 渲染已经结果，可以开始呈现的信号量
	std::vector<VkSemaphore> renderFinishedSemaphores;

	// 为每一帧创建栅栏，来进行 CPU 和 GPU 之间的同步
	std::vector<VkFence> inFlightFences;

	// 追踪当前渲染的是哪一帧
	uint32_t currentFrame = 0;

	// 标记窗口是否发生改变
	bool framebufferResized = false;

	/* 初始化窗口 */
	void initWindow() {
		// Initialize the GLFW library
		// 初始化 GLFW (Graphics Library FrameWork) 库
		glfwInit();

		// Tell GLFW library explicitly not to create an OpenGL context 
		// 使用 GLFW_NO_API 显式地阻止 GLFW 自动创建 OpenGL 上下文
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

		// Disable the resize of the window
		// 禁止窗口大小改变
		// glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

		// specify the width, height, title, and the monitor to open the window 
		// 存储创建的窗口句柄（窗口宽、窗口高、窗口名称、显示器序号）
		window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);

		// 
		// 将 this 指针存储在 GLFW 窗口相关的数据中
		glfwSetWindowUserPointer(window, this);

		// 
		// 设置处理窗口大小改变的回调函数
		glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
	}

	// 
	// 静态函数才能用作回调函数
	static void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
		auto app = reinterpret_cast<HelloTriangleApplication*>(glfwGetWindowUserPointer(window));
		app->framebufferResized = true;
	}

	/* Initialize all Vulkan objects */
	/* 初始化 Vulkan 的所有对象 */
	void initVulkan() {
		// The instance is the connection between the application and the Vulkan library, creating it involves specifying some details about the application to the driver
		// 创建 Vulkan 实例，Vulkan 实例是应用和 Vulkan 库的桥梁，创建实例会涉及到应用程序到驱动的一些细节
		createInstance();
		// // Setup debug messenger
		// 初始化调试信使
		setupDebugMessenger();
		// 
		// 
		createSurface();
		// Select a graphics card in the system that support the features we need
		// 从系统中选出一个支持我们想要的特性的显卡
		pickPhysicalDevice();
		createLogicalDevice();
		createSwapChain();
		createImageViews();
		createRenderPass();
		createGraphicsPipeline();
		createFramebuffers();
		createCommandPool();
		createVertexBuffer();
		createCommandBuffers();
		createSyncObjects();
	}

	void mainLoop() {
		// 窗口不关闭
		while (!glfwWindowShouldClose(window)) {
			// Loop and check for events like pressing the X button until the window has been closed by the user
			// 循环检测事件并处理，如是否按下了关闭按键等
			glfwPollEvents();
			
			// 
			// 绘制一帧
			drawFrame();
		}

		// 等待逻辑设备操作结束执行时才销毁窗口
		vkDeviceWaitIdle(device);
	}

	void cleanupSwapChain() {
		// 消除帧缓冲
		for (size_t i = 0; i < swapChainFramebuffers.size(); i++) {
			vkDestroyFramebuffer(device, swapChainFramebuffers[i], nullptr);
		}
		
		// 清楚图像视图
		for (size_t i = 0; i < swapChainImageViews.size(); i++) {
			vkDestroyImageView(device, swapChainImageViews[i], nullptr);
		}

		// 清除交换链
		vkDestroySwapchainKHR(device, swapChain, nullptr);
	}

	void cleanup() {
		cleanupSwapChain();

		vkDestroyPipeline(device, graphicsPipeline, nullptr);
		vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
		vkDestroyRenderPass(device, renderPass, nullptr);

		vkDestroyBuffer(device, vertexBuffer, nullptr);
		vkFreeMemory(device, vertexBufferMemory, nullptr);

		// 摧毁信号量
		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
			vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
			vkDestroyFence(device, inFlightFences[i], nullptr);
		}

		// 摧毁指令池
		vkDestroyCommandPool(device, commandPool, nullptr);

		// 摧毁逻辑设备
		vkDestroyDevice(device, nullptr);

		// If validation layer is enabled, destroy the VkDebugUtilsMessengerEXT
		// 如果启用了校验层，摧毁 VkDebugUtilsMessengerEXT 对象
		if (enableValidationLayers) {
			DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
		}

		// 
		// 摧毁表面
		vkDestroySurfaceKHR(instance, surface, nullptr);
		
		// Destroy the instance we created
		// 摧毁创建的实例
		vkDestroyInstance(instance, nullptr);
		
		// Destroy the window we created
		// 摧毁已创建的窗口
		glfwDestroyWindow(window);
		
		// Terminate GLFW
		// 终止 GLFW
		glfwTerminate();
	}

	// 
	// 重建交换链
	void recreateSwapChain() {
		// 
		// 最小化时，停止渲染
		int width = 0, height = 0;
		glfwGetFramebufferSize(window, &width, &height);
		while (width == 0 || height == 0) {
			glfwGetFramebufferSize(window, &width, &height);
			glfwWaitEvents();
		}

		//
		// 等待设备处于空闲状态，避免在对象使用过程中将其清除重建
		vkDeviceWaitIdle(device);


		//
		//
		cleanupSwapChain();

		createSwapChain();
		createImageViews();
		createFramebuffers();
	}

	/* Create the Vulkan instance */
	/* 创建 Vulkan 实例 */
	void createInstance() {
		// Check if validation layers are supported
		// 检查是否支持校验层
		if (enableValidationLayers && !checkValidationLayerSupport()) {
			throw std::runtime_error("validation layers requested, but not available!");
		}

		/* Fill in the information to create the application */
		/* 填入应用信息 */

		// Construct a instance that holds information on how to create appInfo
		// 构造一个用于存储应用信息的实例
		VkApplicationInfo appInfo{};
		// Explicitly specify the structure type (for the support of p->next)
		// 明确指定结构的类型
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		// Set the name of the application to "Hello Triangle"
		// 应用的名字为“Hello Triangle”
		appInfo.pApplicationName = "Hello Triangle";
		// Set the version of the application
		// 设置应用的版本
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		// Set the name of the engine to "No engine"
		// 设置引擎的名字为“No Engine”
		appInfo.pEngineName = "No Engine";
		// Set the version of the engine
		// 设置引擎的版本
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		// Set the version of the API
		// 设置 API 的版本
		appInfo.apiVersion = VK_API_VERSION_1_0;

		/* Fill in the information used to create the Vulkan instance */
		/* 填入用于创建 VkInstance 的信息 */

		// Construct a instance that holds information on how to create VkInstance
		// 构造一个用于存储创建VulkanInstance所需信息的实例
		VkInstanceCreateInfo createInfo{};

		// Explicitly specify the structure type
		// 明确指定结构的类型
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;

		// Refer the appInfo that we just created to fill in the VkInstance creation information
		// 引用刚刚创建的 appInfo 作为 VkInstance 创建的信息
		createInfo.pApplicationInfo = &appInfo;
		
		// Get the global extensions that is required by the instance
		// 获取实例所需的全局扩展
		auto extensions = getRequiredExtensions();
		// The enabled extension count is the size of the extension container
		// 启用的扩展的个数为 extension 容器的大小
		createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
		// The name of the enabled extension are the data part of the extension container
		// 启用的扩展名字为 extension 容器的 data 部分
		createInfo.ppEnabledExtensionNames = extensions.data();

		/* Set the global validation layers */
		/* 设置全局校验层 */
		
		// Before the creation of the debugMessenger in Vulkan instance, create another debug messenger to debug any issues in the vkCreateInstance and vkDestroyInstance calls
		// 在 debugMessenger 创建之前，创建另一个专门用于输出 Vulkan 实例的创建和摧毁过程的 debug 信息的 messenger
		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
		// If the validation layers are enabled
		// 如果校验层被启用了
		if (enableValidationLayers) {
			// The enabled validation layer count is the size of the validation layer container
			// 启用的校验层的个数为 validation layer 容器的大小
			createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			// The name of the enabled validation layer are the data part of the validation layer container
			// 启用的校验层名字为 validation layer 容器的 data 部分
			createInfo.ppEnabledLayerNames = validationLayers.data();

			// Construct the information for creating the debug messenger
			// 构造调试信使的创建信息
			populateDebugMessengerCreateInfo(debugCreateInfo);
			// TODO: p->next of the createInfo point to debugCreateInfo, which directly call its callback function while the creation of the instance, don't know why
			// TODO：p->next 指向 debugCreateInfo，看起来这样操作直接在创建 VkInstance 的过程中调用了 debugCreateInfo 的回调函数输出了调试信息，不知道为什么能这么实现
			createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
		} else {
			// If the validation layer is not enabled, then the count of the validation layer will be 0
			// 如果没有启用校验层，则校验层的个数为 0
			createInfo.enabledLayerCount = 0;
			// p->next will be null pointer
			// p->next 指向空指针
			createInfo.pNext = nullptr;
		}

		// Create the Vulkan instance with respect to the information above
		// 使用以上信息创建 Vulkan 实例
		if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
			throw std::runtime_error("failed to create instance!");
		}
	}

	/*
	// 获取扩展信息
	// 获取扩展数量
	uint32_t extensionCount = 0;
	vkEnumerateinstanceExtensionProperties(nullptr, &extensionCount, nullptr);
	// 存储扩展信息
	std::vector<VkExtensionProperties> Extensions(extensionCount);
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, Extensions.data());
	std::cout << "available extensions:" << std::endl;

	for (const auto& extension : Extensions)
	{
		std::cout << "\t" << extension.extensionName << std::endl;
	}
	*/

	// Construct the information for creating the debug messenger
	// 构造调试信使的创建信息
	void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
		// Empty the createInfo, don't know why for now
		// 清空 createInfo，目前还不知道为什么要清空
		createInfo = {};
		// Explicitly specify the structure type
		// 明确指定结构的类型
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		// Identify what severity level of the message will be delivered to the messenger: verbose, warning, error
		// 指定什么严重级别的消息会被该信使收到（可能是诊断信息、资源创建之类的信息、警告信息、不合法和可能造成崩溃的信息）
		createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		// Identify what type of the message will be delivered to the messenger: general, validation, performance
		// 消息类型（与规范性能无关，违反规范或发生可能的错误，进行了可能影响 Vulkan 性能的行为
		createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		// User defined callback function when the debug message arrived
		// 用户定义的收到信息后的回调函数
		createInfo.pfnUserCallback = debugCallback;
	}

	//  窗口表面
	void createSurface() {
		// 下面这个函数是跨平台的
		if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
			throw std::runtime_error("failed to create window surface!");
		}
	}

	// Setup debug messenger
	// 初始化调试信使
	void setupDebugMessenger() {
		// If the validation layers is NOT enabled, return immediately
		// 如果校验没有被启用，跳过此初始化
		if (!enableValidationLayers) return;

		// Construct a instance that holds information on how to create debugMessenger using debug utility extension
		// 构造一个描述如何使用 debug utility 创建 debugMessenger 的实例
		VkDebugUtilsMessengerCreateInfoEXT createInfo;
		// Fill in information to createInfo
		// 向 createInfo 中填入信息
		populateDebugMessengerCreateInfo(createInfo);

		// Create the debug messenger using the information above
		// 使用以上信息创建调试信息信使
		if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
			throw std::runtime_error("failed to set up debug messenger!");
		}
	}

	// Select a graphics card in the system that support the features we need
	// 从系统中选出一个支持我们想要的特性的物理显卡（在这里我们选第一个）
	void pickPhysicalDevice() {
		// The count of the physical device
		// 可用物理设备的数量
		uint32_t deviceCount = 0;
		// Acquire the physical graphics card that is available
		// 获取可用的物理设备
		vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

		// If no graphics card were found, throw an runtime error
		// 如果没有找到任何物理设备，抛出一个运行时错误
		if (deviceCount == 0) {
			throw std::runtime_error("failed to find GPUs with Vulkan support!");
		}

		// Create a VkPhysicalDevice container with the size of deviceCount
		// 创建一个用于存储 VkPhysicalDevice 对象的容器，大小为 deviceCount
		std::vector<VkPhysicalDevice> devices(deviceCount);
		// Acquire the physical devices that is available and store them in the container we created
		// 获取可用的物理设备，并将他们存放在刚刚创建的容器里
		vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

		// For each physical device, check the compatibility and simply select the first one that satisfy the desire
		// 对于每一个物理设备，检查并选择使用第一个满足需求的设备
		for (const auto& device : devices) {
			if (isDeviceSuitable(device)) {
				physicalDevice = device;
				break;
			}
		}

		// If no physical were found, throw an runtime error
		// 如果没有找到任何物理设备，抛出一个运行时错误
		if (physicalDevice == VK_NULL_HANDLE) {
			throw std::runtime_error("failed to find a suitable GPU!");
		}


		///* Alternative method to choose a physical device more subtly, rate the device and use the highest one */
		///* 可以使用另一种方式来以一种更精细的方式来选择设备，给每个设备打分然后用分数最高的那个设备 */
		//// Use an ordered map to automatically sort candidates by increasing score
		//// 使用无序 map 来自动按照升序排序所有设备
		//std::multimap<int, VkPhysicalDevice> candidates;

		//// Rate each physical device and insert the score pair into the multimap
		//// 对于每一个物理设备评分，并把分数和物理设备作为一个 pair 插入到 multimap 中
		//for (const auto& device : devices) {
		//	// Rate each physical device
		//	// 对于每一个物理设备评分
		//	int score = rateDeviceSuitability(device);
		//	// Insert the score pair into the multimap
		//	// 把分数和物理设备作为一个 pair 插入到 multimap 中
		//	candidates.insert(std::make_pair(score, device));
		//}

		//// Check if the best candidate is suitable at all
		//// 检擦是否存在最好的（分数最高的）设备，返回这个设备或抛出运行时 error
		//if (candidates.rbegin()->first > 0) {
		//	physicalDevice = candidates.rbegin()->second;
		//} else {
		//	throw std::runtime_error("failed to find a suitable GPU!");
		//}
	}

	//// Rate the device according to the properties, use the device that has the highest score
	//// 给物理设备按照其所支持的特性打分，分高者任之
	//int rateDeviceSuitability(VkPhysicalDevice device) {
	//	// Construct a instance that holds information on the properties of physical device
	//	// 构造一个用于存储物理设备的基本属性的实例（如基础的设备属性，如名称、类型、支持的 Vulkan 版本）
	//	VkPhysicalDeviceProperties deviceProperties;
	//	// Construct a instance that holds information on the features of physical device
	//	// 构造一个用于存储物理设备的功能特性的实例（如纹理压缩，64 位浮点和多视口渲染等特性）
	//	VkPhysicalDeviceFeatures deviceFeatures;
	//	// Get the properties supported of the physical device
	//	// 获取物理设备所支持的基本属性
	//	vkGetPhysicalDeviceProperties(device, &deviceProperties);
	//	// Get the features supported of the physical device
	//	// 获取物理设备所支持的功能特性
	//	vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
	//	int score = 0;

	//	// Discrete GPUs have a significant performance advantage
	//	// 独显会有很显著的性能提升，加大分
	//	if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
	//		score += 1000;
	//	}

	//	// Maximum possible size of textures affects graphics quality
	//	// 最大可支持的纹理尺寸加分
	//	score += deviceProperties.limits.maxImageDimension2D;

	//	// If application can't function without geometry shaders
	//	// 如果应用程序没有几何着色器，打分为 0
	//	if (!deviceFeatures.geometryShader) {
	//		return 0;
	//	}
	//	// Return the score of the physical device
	//	// 返回对于物理设备的评分
	//	return score;
	//}

	// 
	// 逻辑设备和队列
	void createLogicalDevice() {
		QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

		// 创建所有使用的队列族
		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		// 创建每一个不同的队列族
		std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };

		float queuePriority = 1.0f;
		for (uint32_t queueFamily : uniqueQueueFamilies) {
			// 该结构体描述了针对一个队列族所需队列数量
			VkDeviceQueueCreateInfo queueCreateInfo{};
			// Explicitly specify the structure type
			// 明确指定结构的类型
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.queueFamilyIndex = queueFamily;
			queueCreateInfo.queueCount = 1;
			// 优先级控制指令缓冲的执行顺序
			queueCreateInfo.pQueuePriorities = &queuePriority;
			queueCreateInfos.push_back(queueCreateInfo);
		}

		// 指定应用程序使用的设备特性
		VkPhysicalDeviceFeatures deviceFeatures{};

		// 创建逻辑设备
		VkDeviceCreateInfo createInfo{};
		// Explicitly specify the structure type
		// 明确指定结构的类型
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

		createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
		createInfo.pQueueCreateInfos = queueCreateInfos.data();

		createInfo.pEnabledFeatures = &deviceFeatures;

		// 启用交换链扩展
		createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
		createInfo.ppEnabledExtensionNames = deviceExtensions.data();

		// 设备和实例使用相同的校验层，不需要额外的扩展支持
		if (enableValidationLayers) {
			createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			createInfo.ppEnabledLayerNames = validationLayers.data();
		} else {
			createInfo.enabledLayerCount = 0;
		}

		// 之前把启用交换链扩展这两行代码放在 vkCreateDevice 下面了，导致我的虚拟设备没有开交换链扩展，所以后面创建交换链扩展时一直创建失败
		if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) {
			throw std::runtime_error("failed to create logical device!");
		}

		// 获取指定队列族的队列句柄
		vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
		vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
	}

	// 交换链
	// 创建交换链
	void createSwapChain() {
		SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);

		VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
		VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
		VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

		// 设置交换链中图像个数，至少设置为最小个数 +1 来实现三重缓冲
		uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
		// maxImageCount = 0 意味着只要内存满足，可以使用任意数量的图像
		if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
			imageCount = swapChainSupport.capabilities.maxImageCount;
		}

		// 填写创建信息结构体
		VkSwapchainCreateInfoKHR createInfo{};
		// Explicitly specify the structure type
		// 明确指定结构的类型
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = surface;

		createInfo.minImageCount = imageCount;
		createInfo.imageFormat = surfaceFormat.format;
		createInfo.imageColorSpace = surfaceFormat.colorSpace;
		createInfo.imageExtent = extent;
		// 指定每个图像包含的层次，通常是 1，VR 相关程序会使用更多的层次
		createInfo.imageArrayLayers = 1;
		// 指定我们将在图像上进行怎样的操作，下面就仅仅是绘制操作（还可以设置为可以后期处理的操作）
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		// 指定多个队列族使用交换链图像的方式
		QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
		uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

		// 图形队列和呈现队列不再同一个队列
		// 通过图形队列在交换链图像上进行绘制，将图像提交给呈现队列来显示
		if (indices.graphicsFamily != indices.presentFamily) {
			// 图像可以在多个队列族使用，不需要显示地改变所有权
			// 协同模式
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			// 指定共享所有全的队列族
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices = queueFamilyIndices;
		} else {
			// 一张图像同时只能被一个队列族拥有，显示改变所有权才能换族，这一模式性能最佳
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		}

		// 对交换链中图像指定一个固定的交换操作，currentTransform 为不操作
		createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
		// 指定 alpha 通道是否被用来和窗口系统中其他窗口混合，下面将忽略 alpha 通道，设置为不透明
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		// 设置呈现模式
		createInfo.presentMode = presentMode;
		// 表示不关心被窗口系统其他窗口遮挡的像素颜色
		createInfo.clipped = VK_TRUE;

		// 创建交换链
		if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) != VK_SUCCESS) {
			throw std::runtime_error("failed to create swap chain!");
		}

		// 获取交换链图像句柄
		vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
		swapChainImages.resize(imageCount);
		vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());

		// 存储交换链的图像格式和范围
		swapChainImageFormat = surfaceFormat.format;
		swapChainExtent = extent;
	}

	// 图像视图
	void createImageViews() {
		// 分配空间
		swapChainImageViews.resize(swapChainImages.size());

		// 遍历交换链所有图像来创建图像视图
		for (size_t i = 0; i < swapChainImages.size(); i++) {
			// 填写图像视图创建信息结构体
			VkImageViewCreateInfo createInfo{};
			// Explicitly specify the structure type
			// 明确指定结构的类型
			createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			createInfo.image = swapChainImages[i];
			// 指定图像数据的解释方式，viewType 指定图像被看作是几维纹理
			createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			createInfo.format = swapChainImageFormat;
			// components 用于进行图像颜色通道的映射，此处使用默认映射
			createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
			// subresourceRange 用于指定图像的用途和图像哪一部分可以被访问，此处图像被用作渲染目标，只存在一个图层
			createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			createInfo.subresourceRange.baseMipLevel = 0;
			createInfo.subresourceRange.levelCount = 1;
			createInfo.subresourceRange.baseArrayLayer = 0;
			createInfo.subresourceRange.layerCount = 1;

			// 创建图像视图
			if (vkCreateImageView(device, &createInfo, nullptr, &swapChainImageViews[i]) != VK_SUCCESS) {
				throw std::runtime_error("failed to create image views!");
			}
		}
	}

	// 渲染流程
	// 设置用于渲染的帧缓冲附着
	void createRenderPass() {
		VkAttachmentDescription colorAttachment{};
		colorAttachment.format = swapChainImageFormat;
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		// 用于指定渲染前后对附着中的数据进行的操作（对颜色和深度缓冲起效），load 为前
		// 使用一个常量值来清除附着内容，这里会在每次渲染前用黑色清除帧缓冲
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		// 渲染后的内容会被存储起来，以便之后读取
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		// 下面两个对模板缓冲起效，但不用，所以不关心 // TODO:
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		// 使渲染后的图像可以被交换链呈现
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		// 子流程和附着引用
		VkAttachmentReference colorAttachmentRef{};
		// 指定要引用的附着索引
		colorAttachmentRef.attachment = 0;
		// 指定进行子流程时引用的附着使用的布局方式
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		// 描述子流程
		VkSubpassDescription subpass{};
		// 指定这是一个图形渲染的子流程
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		// 指定引用的颜色附着
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;

		// 配置子流程依赖
		VkSubpassDependency dependency{};
		// src 指的渲染流程开始前的子流程，dst 表示渲染阶段结束后的子流程
		// 指定被依赖的子流程的索引
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		// 依赖被依赖的子流程的索引
		dependency.dstSubpass = 0;
		// 需要等待颜色附着输出这一管线阶段
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		// 子流程将进行的操作类型
		dependency.srcAccessMask = 0;
		// 指定需要等待的管线阶段
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		// 子流程将进行的操作类型
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		// 渲染流程
		VkRenderPassCreateInfo renderPassInfo{};
		// Explicitly specify the structure type
		// 明确指定结构的类型
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

	// 图形管线
	void createGraphicsPipeline() {
		auto vertShaderCode = readFile(".\\shaders\\vert.spv");
		auto fragShaderCode = readFile(".\\shaders\\frag.spv");

		// 着色器模块只在管线创建时需要
		VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
		VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);

		// 填写指定着色器阶段创建信息结构体
		VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
		// Explicitly specify the structure type
		// 明确指定结构的类型
		vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertShaderStageInfo.module = vertShaderModule;
		// pname 用于指定阶段调用的着色器函数
		vertShaderStageInfo.pName = "main";

		// 填写指定着色器阶段创建信息结构体
		VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
		// Explicitly specify the structure type
		// 明确指定结构的类型
		fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragShaderStageInfo.module = fragShaderModule;
		// pname 用于指定阶段调用的着色器函数
		fragShaderStageInfo.pName = "main";

		VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };
		// 下面来设置固定功能

		// 用来描述传递给顶点着色器的顶点数据格式，因为目前是硬编码顶点数据，所以其实不用描述，赋值 0
		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		// Explicitly specify the structure type
		// 明确指定结构的类型
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

		// 获取顶点数据的绑定信息
		auto bindingDescription = Vertex::getBindingDescription();
		// 获取顶点数据中的属性
		auto attributeDescriptions = Vertex::getAttributeDescriptions();

		/* Configure the information of the input */
		/* 配置输入的信息 */

		// The count of the binding
		// 输入顶点的绑定个数为 1
		vertexInputInfo.vertexBindingDescriptionCount = 1;
		// The count of the vertex attribute description is 2, i.e. vertex coordinate and vertex color
		// 输入顶点的属性的大小为相应的大小为 2：顶点坐标和顶点颜色
		vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
		// Reference the binding description of the vertex data as the bingding description of the input
		// 引用顶点数据绑定信息的描述，作为输入的绑定信息
		vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
		// The attribute description of the vertex is the attribute description of the input
		// 将顶点的属性作为输入的属性	
		vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();


		// 输入装配
		// 下面这个结构体描述：顶点数据定义哪种类型的几何图元，以及是否启用几何图元重启
		VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
		// Explicitly specify the structure type
		// 明确指定结构的类型
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		// 每三个顶点构成一个三角形图元
		inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssembly.primitiveRestartEnable = VK_FALSE;

		// 定义视口
		
		// 定义裁剪矩形

		// 组合视口和裁剪矩形
		VkPipelineViewportStateCreateInfo viewportState{};
		// Explicitly specify the structure type
		// 明确指定结构的类型
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.scissorCount = 1;

		// 光栅化
		VkPipelineRasterizationStateCreateInfo rasterizer{};
		// Explicitly specify the structure type
		// 明确指定结构的类型
		rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		// 丢弃近平面和远平面外的片段
		rasterizer.depthClampEnable = VK_FALSE;
		// 允许所有几何图元通过光栅化
		rasterizer.rasterizerDiscardEnable = VK_FALSE;
		// 指定几何图元生成片段（像素）的方式，整个多边形包括内部，其他 mode 需要启用 GPU 特性 
		rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
		// 线宽大于 1 需要启用 GPU 特性
		rasterizer.lineWidth = 1.0f;
		// 剔除背面
		rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
		// 顺时针的定点顺序是正面
		rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
		// 添加一个常量值或者一个基于片段所处线段斜率值得到的变量值到深度上。对阴影贴图有用，这里关闭
		rasterizer.depthBiasEnable = VK_FALSE;

		// 多重采样，组合多个不同多边形产生的片段的颜色来决定最终的像素颜色，代价较小，但需要启用 GPU 特性，暂时关闭
		VkPipelineMultisampleStateCreateInfo multisampling{};
		// Explicitly specify the structure type
		// 明确指定结构的类型
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

		// 深度和模板测试先不管
		
		// 颜色混合

		// 第一种混合方式：对每个绑定的帧缓冲进行单独的颜色混合配置，先关闭
		VkPipelineColorBlendAttachmentState colorBlendAttachment{};
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_FALSE;

		// 可以设置全局混合常量
		VkPipelineColorBlendStateCreateInfo colorBlending{};
		// Explicitly specify the structure type
		// 明确指定结构的类型
		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		
		// 第二种混合方式：使用位运算组合旧值和新值来混合颜色，这里关闭，开启的话会禁用第一种
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.logicOp = VK_LOGIC_OP_COPY; //  Optional
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &colorBlendAttachment;
		colorBlending.blendConstants[0] = 0.0f; //  Optional
		colorBlending.blendConstants[1] = 0.0f; //  Optional
		colorBlending.blendConstants[2] = 0.0f; //  Optional
		colorBlending.blendConstants[3] = 0.0f; //  Optional

		// 动态状态
		// 填写以下结构体指定要动态修改的状态
		std::vector<VkDynamicState> dynamicStates = {
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_SCISSOR
		};
		VkPipelineDynamicStateCreateInfo dynamicState{};
		// Explicitly specify the structure type
		// 明确指定结构的类型
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
		dynamicState.pDynamicStates = dynamicStates.data();

		// 管线布局
		// 使用 layout 定义着色器的 uniform，虽然现在不用，但是也要定义
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		// Explicitly specify the structure type
		// 明确指定结构的类型
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 0; //  Optional
		pipelineLayoutInfo.pushConstantRangeCount = 0; //  Optional

		if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create pipeline layout!");
		}

		// 创建管线对象
		VkGraphicsPipelineCreateInfo pipelineInfo{};
		// Explicitly specify the structure type
		// 明确指定结构的类型
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

		// 用于以一个创建好的图形管线为基础创建一个新的图形管线
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; //  Optional

		// 创建管线对象
		if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS) {
			throw std::runtime_error("failed to create graphics pipeline!");
		}

		// 清除 shaderModule
		vkDestroyShaderModule(device, fragShaderModule, nullptr);
		vkDestroyShaderModule(device, vertShaderModule, nullptr);
	}

	// 帧缓冲
	void createFramebuffers() {
		swapChainFramebuffers.resize(swapChainImageViews.size());
		// 为交换链的每一个图像视图对象创建帧缓冲

		for (size_t i = 0; i < swapChainImageViews.size(); i++) {
			VkImageView attachments[] = {
				swapChainImageViews[i]
			};

			VkFramebufferCreateInfo framebufferInfo{};
			// Explicitly specify the structure type
			// 明确指定结构的类型
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			// 指定帧缓冲需要兼容的渲染流程对象
			framebufferInfo.renderPass = renderPass;
			// 指定附着
			framebufferInfo.attachmentCount = 1;
			framebufferInfo.pAttachments = attachments;
			framebufferInfo.width = swapChainExtent.width;
			framebufferInfo.height = swapChainExtent.height;
			// 帧缓冲图像层数
			framebufferInfo.layers = 1;

			if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS) {
				throw std::runtime_error("failed to create framebuffer!");
			}
		}
	}
	
	// 指令缓冲
	void createCommandPool() {
		QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice);

		VkCommandPoolCreateInfo poolInfo{};
		// Explicitly specify the structure type
		// 明确指定结构的类型
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		// 绘制指令可以被提交给图形操作的队列
		poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

		if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
			throw std::runtime_error("failed to create command pool!");
		}
	}

	/* Create vertex buffer */
	/* 创建顶点数组 */
	void createVertexBuffer() {
		// Construct the instance that hold information of creating buffer
		// 构造一个用于存储 buffer 创建信息的实例
		VkBufferCreateInfo bufferInfo{};
		// Explicitly specify the structure type
		// 明确指定结构的类型
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		// Specify the size of the buffer
		// 指定要创建的 buffer 的大小
		bufferInfo.size = sizeof(vertices[0]) * vertices.size();
		// Specify the usage of the buffer: as a vertex buffer
		// 指定这个 buffer 会被用作顶点数据的 buffer
		bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
		// TODO: Specify the buffer will not share with other command queue
		// TODO：不将 buffer 共享给其他的 queue，因为目前只有 Graphics queue
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		// Use the information above to create the buffer
		// 使用上面的信息创建 buffer
		if (vkCreateBuffer(device, &bufferInfo, nullptr, &vertexBuffer) != VK_SUCCESS) {
			throw std::runtime_error("failed to create vertex buffer!");
		}

		/* 获取 buffer 对内存的需求，从而为刚刚创建的顶点数组分配内存 */
		// Construct the instance that hold information regarding the memory for creating the buffer 
		// 构造一个用于存储内存需求的实例
		VkMemoryRequirements memRequirements;
		// Query the desired properties of the vertex buffer
		// 询问顶点数据 buffer 对于内存的需求
		vkGetBufferMemoryRequirements(device, vertexBuffer, &memRequirements);

		// Construct the instance that hold information regarding how to allocate the memory
		// 构造一个用于存储内存分配信息的实例
		VkMemoryAllocateInfo allocInfo{};
		// Explicitly specify the structure type
		// 明确指定结构的类型
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		// The size of the allocation will be the size of the memory needed
		// 分配内存为顶点数组所需 buffer 的大小
		allocInfo.allocationSize = memRequirements.size;
		// Configure the memory type index to the desired memory type index we found
		// 配置存储属性的索引号为满足需求的索引号
		// VK_MEMORY_PROPERTY_HOST_COHERENT_BIT: indicate the memory heap is host coherent, which ensures that the mapped memory always matches the contents of the allocated memory,
		// 		and the driver will be aware of our writes to the buffer
		// 需要能映射到 CPU 的地址空间所以 CPU 可以直接写并且对 CPU 是有一致性的
		allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		// Using the information above to allocate memory
		// 按照如上配置的信息为顶点数组分配存储空间
		if (vkAllocateMemory(device, &allocInfo, nullptr, &vertexBufferMemory) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate vertex buffer memory!");
		}

		// Associate this memory with the buffer
		// 将分配的存储空间与顶点数组绑定在一起
		vkBindBufferMemory(device, vertexBuffer, vertexBufferMemory, 0);

		// Define a void type point to store the address
		// 定义一个空类型的指针用于寻址
		void* data;
		// Map the allocated memory to the data point above, so that we can use the data to address the memory
		// 将刚刚分配出来的顶点内存映射到 data 指针上
		vkMapMemory(device, vertexBufferMemory, 0, bufferInfo.size, 0, &data);
		// Copy the vertex data to the memory that the data pointer pointed
		// 将顶点数据拷贝到 data 所指向的内存中
		memcpy(data, vertices.data(), (size_t)bufferInfo.size);
		// Unmap the relationship between the data point with the allocated memory
		// 释放 data 对顶点内存的映射
		vkUnmapMemory(device, vertexBufferMemory);
	}

	// Find the memory type desired
	/* 寻找存储的类型 */
	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
		// Construct a instance to store memory properties information, including memory type and memory heap
		// 构造一个用于存储 memory 属性信息的实例（包含存储类型和存储堆）
		// The VkPhysicalDeviceMemoryProperties structure has two arrays: memoryTypes and memoryHeaps.
		// Memory heaps are distinct memory resources like dedicated VRAM and swap space in RAM for when VRAM runs out. The different types of memory exist within these heaps.
		VkPhysicalDeviceMemoryProperties memProperties;
		// Acquire the memory properties that the graphics card has
		// 获取显卡所支持的存储属性
		vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

		// For each memory property we acquired
		// 对于每一个获取到的存储属性
		for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
			// If (the memory type we desire is supported by the graphics card) && (the exist memory properties is supported by the memory type)
			// 如果（找到了我们想要的存储类型）并且（在属性列表中找到了想要寻找的存储属性）
			if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
				// Return the index of the memory property
				// 返回这个存储属性的索引号
				return i;
			}
		}
		// If the memory desired is not available, throw an error
		// 如果没找到想要的存储属性，就抛出一个异常
		throw std::runtime_error("failed to find suitable memory type!");

	}

	// 分配指令缓冲
	void createCommandBuffers() {
		// 绘制操作是在帧缓冲上进行的
		commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

		VkCommandBufferAllocateInfo allocInfo{};
		// Explicitly specify the structure type
		// 明确指定结构的类型
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		// 使用的指令池和需要分配的指令缓冲个数
		allocInfo.commandPool = commandPool;
		// 主要缓冲指令对象，可被提交到队列执行，不能被其他指令缓冲对象调用
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();

		if (vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate command buffers!");
		}
	}

	// 记录指令到指令缓冲
	void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
		VkCommandBufferBeginInfo beginInfo{};
		// Explicitly specify the structure type
		// 明确指定结构的类型
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		
		// 可以使得在上一帧还未结束渲染时，提交下一帧的渲染指令 // TODO：

		if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
			throw std::runtime_error("failed to begin recording command buffer!");
		}

		// 开始渲染流程
		VkRenderPassBeginInfo renderPassInfo{};
		// Explicitly specify the structure type
		// 明确指定结构的类型
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = renderPass;
		renderPassInfo.framebuffer = swapChainFramebuffers[imageIndex];
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = swapChainExtent;

		// 设置清除颜色
		VkClearValue clearColor = { {{0.0f, 0.0f, 0.0f, 1.0f}} };
		renderPassInfo.clearValueCount = 1;
		renderPassInfo.pClearValues = &clearColor;
		
		// VK_SUBPASS_CONTENTS_INLINE 指的是指令只在主要指令缓冲，没有辅助指令缓冲
		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
			
			// 将这个 command buffer 绑定到已创建那个图形管线
			vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

			// 定义视口
			VkViewport viewport{};
			viewport.x = 0.0f;
			viewport.y = 0.0f;
			viewport.width = static_cast<float>(swapChainExtent.width);
			viewport.height = static_cast<float>(swapChainExtent.height);
			viewport.minDepth = 0.0f;
			viewport.maxDepth = 1.0f;
			vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

			// 定义裁剪矩形
			VkRect2D scissor{};
			scissor.offset = { 0, 0 };
			scissor.extent = swapChainExtent;
			// TODO:？？
			vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

			// 需要绑定的 buffer 包括哪些
			VkBuffer vertexBuffers[] = { vertexBuffer };
			// 需要绑定的 buffer 所对应的 offset 有哪些
			VkDeviceSize offsets[] = { 0 };
			// 将 vertexbuffer 绑定到 binding：从第 0 个 binding 位置起，绑定 1 个 binding，绑定的 buffer 是 vertexbuffers，从 vertexbuffer 的 offsets 的位置开始读
			vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

			// 绘制三角形，提交绘制操作到指令缓冲
			vkCmdDraw(commandBuffer, static_cast<uint32_t>(vertices.size()), 1, 0, 0);
		
		// 结束渲染流程
		vkCmdEndRenderPass(commandBuffer);

		// 结束指令到指令缓冲
		if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
			throw std::runtime_error("failed to record command buffer!");
		}
	}

	// 创建同步对象
	void createSyncObjects() {
		imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

		// 
		// 
		VkSemaphoreCreateInfo semaphoreInfo{};
		// Explicitly specify the structure type
		// 明确指定结构的类型
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fenceInfo{};
		// Explicitly specify the structure type
		// 明确指定结构的类型
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		// 设置初始状态为已发出信号
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
		// 等待当前帧所使用的指令缓冲结束执行
		vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

		// 从交换链获取图像
		uint32_t imageIndex;
		// 第三个参数是图像获取的超时时间，无符号 64 位最大整数表示禁用获取超时
		VkResult result = vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

		// 如果交换链过期就重建
		if (result == VK_ERROR_OUT_OF_DATE_KHR) {
			recreateSwapChain();
			return;
		} else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
			throw std::runtime_error("failed to acquire swap chain image!");
		}

		// 重置栅栏
		vkResetFences(device, 1, &inFlightFences[currentFrame]);

		vkResetCommandBuffer(commandBuffers[currentFrame], /*VkCommandBufferResetFlagBits*/ 0);
		recordCommandBuffer(commandBuffers[currentFrame], imageIndex);

		// 提交指令缓冲
		VkSubmitInfo submitInfo{};
		// Explicitly specify the structure type
		// 明确指定结构的类型
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		// 指定队列开始执行前需要等待的信号量，以及需要等待的管线阶段
		VkSemaphore waitSemaphores[] = { imageAvailableSemaphores[currentFrame] };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;

		// 指定实际被提交执行的指令缓冲对象
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffers[currentFrame];

		// 只当在指令缓冲执行结束后发出信号的信号量对象
		VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[currentFrame] };
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		// 提交指令缓冲给图形指令队列，第四个参数指定缓冲执行结束后需要发起的信号栅栏对象
		if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS) {
			throw std::runtime_error("failed to submit draw command buffer!");
		}

		// 配置呈现信息
		VkPresentInfoKHR presentInfo{};
		// Explicitly specify the structure type
		// 明确指定结构的类型
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

		// 指定开始呈现操作时需要等待的信号量
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;

		// 指定呈现图像的交换链，以及图像在交换链中的索引
		VkSwapchainKHR swapChains[] = { swapChain };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;

		presentInfo.pImageIndices = &imageIndex;

		// 请求交换链进行图像呈现操作
		result = vkQueuePresentKHR(presentQueue, &presentInfo);

		// 当交换链过期或者表面属性已经不能准确匹配时
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized) {
			framebufferResized = false;
			recreateSwapChain();
		} else if (result != VK_SUCCESS) {
			throw std::runtime_error("failed to present swap chain image!");
		}

		// 更新 currentFrame
		currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
	}

	// 将着色器字节码在管线上使用，需要 VkShaderModule 对象
	VkShaderModule createShaderModule(const std::vector<char>& code) {
		VkShaderModuleCreateInfo createInfo{};
		// Explicitly specify the structure type
		// 明确指定结构的类型
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = code.size();
		// 需要将存储字节码的数组指针转换下
		createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

		// 创建 VkShaderModule 对象
		VkShaderModule shaderModule;
		if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
			throw std::runtime_error("failed to create shader module!");
		}

		return shaderModule;
	}

	// 选择合适的表面格式
	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
		
		// // 如果表面没有自己的首选格式
		//  if (availableFormats.size() == 1 && availableFormats[0].format == VK_FORMAT_UNDEFINED) {
		// // 返回我们设定的格式	
		//  return { VK_FORMAT_B8G8R8A8_UNORM, VK_COLORSPACE_SRGB_NONLINEAR_KHR };
		//  }

		// 如果 Vulkan 返回了一个格式列表，返回指的是 availableFormats
		// 检查我们想要设定的格式是否存在于这个列表中
		for (const auto& availableFormat : availableFormats) {
			if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
				return availableFormat;
			}
		}
		
		// 如果不能在列表中找到我们想要的格式，可以打分找出最合适的格式也可以直接选第一个
		return availableFormats[0];
	}

	// 选择最佳的可用呈现模式
	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
		// // 一般都有 FIFO 模式
		// VkPresentModeKHR bestMode = VK_PRESENT_MODE_FIFO_KHR;
		for (const auto& availablePresentMode : availablePresentModes) {
			// 三重缓冲综合表现最佳，既避免了撕裂现象，同时有较低的延迟
			if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
				return availablePresentMode;
			}
		}

		// 但很多驱动程序对 FIFO 支持不够好，所以应该使用立即显示模式
		// 没有三重缓冲就返回该模式
		// TODO: 以上注释需要进一步修改

		return VK_PRESENT_MODE_FIFO_KHR;
	}

	// 选择交换范围（即交换链中图像的分辨率）
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
		// 不允许自己选择交换范围
		if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
			return capabilities.currentExtent;
		} else {
			// 设置为当前帧缓冲的实际大小
			int width, height;
			glfwGetFramebufferSize(window, &width, &height);

			// 允许自己选择对于窗口最合适的交换范围
			VkExtent2D actualExtent = {
				static_cast<uint32_t>(width),
				static_cast<uint32_t>(height)
			};

			actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
			actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

			return actualExtent;
		}
	}

	// 用于填写上面的结构体
	SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device) {
		SwapChainSupportDetails details;

		// 查询基础表面特性
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

		// 查询表面支持的格式
		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

		if (formatCount != 0) {
			details.formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
		}
		
		// 查询支持的呈现格式
		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

		if (presentModeCount != 0) {
			details.presentModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
		}

		return details;
	}

	// Check if the physical device is suitable for our desire
	// 检查获取的物理设备是否满足我们的需求
	bool isDeviceSuitable(VkPhysicalDevice device) {
		// 
		// 
		QueueFamilyIndices indices = findQueueFamilies(device);

		// Check if the physical device can support the desired extension
		// 检测所需设备扩展是否可行
		bool extensionsSupported = checkDeviceExtensionSupport(device);

		// Initialize the flag indicating if the swap chain is adequate
		// 初始化交换链能力是否合格的标志位
		bool swapChainAdequate = false;
		// If the physical device support the extension we desired
		// 如果物理设备支持我们想要的扩展
		if (extensionsSupported) {
			// Check if the physical device can support the desired swap chain
			// 检测交换链的能力是否满足需求
			SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
			// 
			// 
			swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
		}

		// Return true if TODO, the extension, and the swap chain
		// 
		return indices.isComplete() && extensionsSupported && swapChainAdequate;
	}

	// 
	// 查找并返回需求的队列族的索引
	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device) {
		QueueFamilyIndices indices;
		
		// 获取设备队列族个数
		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

		// 存储队列族的 Properties
		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

		// 队列族包含很多信息，如支持的操作类型、该队列族可创建的队列个数
		// 找到一个支持的队列族
		int i = 0;
		for (const auto& queueFamily : queueFamilies) {
			// 支持绘制指令的队列族索引
			if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
				indices.graphicsFamily = i;
			}

			// 支持表现的队列族索引
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

	// 检测所需的设备扩展
	bool checkDeviceExtensionSupport(VkPhysicalDevice device) {
		uint32_t extensionCount;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

		std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

		// 枚举所有可用的扩展，从我们需要的集合中剔除，如果集合空了，说明可用的扩展可以覆盖我们的需求
		for (const auto& extension : availableExtensions) {
			requiredExtensions.erase(extension.extensionName);
		}

		return requiredExtensions.empty();
	}

	/* Get the required extension */
	/* 获取所需的扩展列表 */
	std::vector<const char*> getRequiredExtensions() {
		// Initialize the GLFW extension counter to 0
		// 初始化 GLFW 扩展计数器为 0
		uint32_t glfwExtensionCount = 0;
		// Initialize the GLFW extension
		// 初始化 GLFW extension
		const char** glfwExtensions;

		// Using a built-in function to acquire the global extension desired by the instance
		// 使用内置的函数，来获取实例所需的全局扩展
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
		// Put the GLFW extensions to the container
		// 将所有 GLFW 扩展放入容器中
		std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

		// If validation layer is enabled
		// 如果启用了校验层
		if (enableValidationLayers) {
			// Push back the debug utilities
			// 把校验层也放入容器中
			extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		}

		// Return the extension container
		// 返回 extension 容器
		return extensions;
	}

	// Check if the validation layers that we would like to use are all supported
	// 检查配置的校验层是否全被支持
	bool checkValidationLayerSupport() {
		// Initialize the counter
		// 初始化计数器
		uint32_t layerCount;
		// Get the count of the validation layers available
		// 获取可用的校验层的数目
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

		// Create a availableLayers container with the size regarding the layerCount
		// 创建大小为 layerCount 的容器
		std::vector<VkLayerProperties> availableLayers(layerCount);
		// Get the details of the validation layers available using the same function
		// 使用相同的函数获取校验层的细节，保存在创建的 availableLayers 容器中
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

		// Check if all the validation layers that we desired are avail in the available validation layers
		// 检查是否所有 validationLayers 列表中的校验层可以在 availableLayers 中找到
		for (const char* layerName : validationLayers) {
			// Initialize the FOUND flag with false
			// 将标志位设置为 false
			bool layerFound = false;

			// Iterate through all the validation layers available
			// 遍历所有的可用校验层
			for (const auto& layerProperties : availableLayers) {
				// Check if the regarding validation layers is available through string comparison
				// 通过字符串比较的方式查看校验层是否可用
				if (strcmp(layerName, layerProperties.layerName) == 0) {
					// If the validation layer has been found, break
					// 如果想要找的校验层找到了，break
					layerFound = true;
					break;
				}
			}
			// If a validation layer is NOT found, return false
			// 如果至少有一个校验层没有找到，返回 false
			if (!layerFound) {
				return false;
			}
		}
		// If all validation layers are found, return true
		// 如果所有的校验层都被找到了，返回 true
		return true;
	}

	// Helper function to read file content
	// 载入二进制文件的辅助函数
	static std::vector<char> readFile(const std::string& filename) {
		//  ate 从文件尾部开始读取，binary 以二进制形式读取文件
		std::ifstream file(filename, std::ios::ate | std::ios::binary);

		if (!file.is_open()) {
			throw std::runtime_error("failed to open file!");
		}

		// 使用 ate 可以根据读取位置确定文件大小，分配空间
		size_t fileSize = (size_t)file.tellg();
		// std::cout << filename << "   " << fileSize << std::endl;
		std::vector<char> buffer(fileSize);

		// 跳到文件头部来读取整个文件
		file.seekg(0);
		file.read(buffer.data(), fileSize);

		file.close();

		return buffer;
	}

	/* Debug callback function */
	/* 调试回调函数 */
	// Identify the severity of the message, the type of the message, and the details of the message itself
	// 指定信息的严重程度，信息的类型，和信息本体
		// pCallbackData: 指向一个结构体的指针，结构体包含：以 null 结尾包含调试信息的字符串，存储有和消息相关的 Vulkan 对象句柄的数组、数组中的对象个数
		// pUserData : 设置回调函数时，传递的数据指针
	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
		// Output the pMessage field of the pCallbackData
		// 输出 pCallbackData 的 pMessage 部分
		std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

		// Return VK_FALSE
		// 返回 VK_FALSE
		return VK_FALSE;
	}
};

int main() {
	// Construct the HelloTriangleApplication instance named app
	// 创建 HelloTriangleApplication 实例，命名为 app
	HelloTriangleApplication app;

	// 尝试运行上面这个实例，并捕捉标准异常
	// Try to run the app while catching the errors
	try {
		app.run();
	} catch (const std::exception& e) {
		// 如果有异常，就把异常的内容输出
		// If there were error, then we use output it through standard output
		std::cerr << e.what() << std::endl;
		// Return code of failure if there were failure
		// 有异常则返回值为失败
		return EXIT_FAILURE;
	}
	// Return code for peace out
	// 无异常则返回值为成功
	return EXIT_SUCCESS;
}
