/* Author Donghao Zhao */

// Seems here for surface creation
// 貌似是用来创建窗口表面的
#define GLFW_INCLUDE_VULKAN

// Seems here for surface creation
// 貌似是用来创建窗口表面的
#include <GLFW/glfw3.h>

// Seems here for surface creation
// 貌似是用来创建窗口表面的
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

// To support std::optional<uint32_t>
// 支持 std::optional<uint32_t>
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

// Allow the rendering of one frame to not interfere with the recording of the next
// 允许这帧的渲染不干扰下帧的录制
const int MAX_FRAMES_IN_FLIGHT = 2;

/* The validation layers that we would like to enable */
/* 要启用的校验层 */
const std::vector<const char*> validationLayers = {
	// All useful standard validation layers
	// 所有的标准校验层
	"VK_LAYER_KHRONOS_validation"
};

/* The device extensions we desired */
/* 所需的设备扩展 */
const std::vector<const char*> deviceExtensions = {
	// Swap chain extension
	// 交换链扩展
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

/* Enable validation with respect to the NDEBUG C++ standard macro */
/* 根据 C++ NDEBUG 宏定义来确定是否启用校验层 */
#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

/* Helper function to create the debugMessenger object */
/* 创建 debugMessenger 对象 */
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

/* Helper function to destroy the debugMessenger object */
/* 摧毁 debugMessenger 对象 */
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


/* The queue family that the physical device should support */
/* 物理设备需要支持的队列族 */
// Each family of queues allows only a subset of commands
// 每一个队列族只允许一部分命令
struct QueueFamilyIndices {
	// Any value of uint32_t could in theory be a valid queue family index including 0, so we use std::optional
	// std::optional is a wrapper that contains no value until you assign something to it
	// std::optional 是一个 wrapper，直到赋值之后才真的有值
	// Queue family to support graphics commands
	// 支持图形指令的队列族
	std::optional<uint32_t> graphicsFamily;
	// Queue family to support present the image to the surface
	// 支持呈现的队列族
	std::optional<uint32_t> presentFamily;
	
	// Check if <uint32_t> container really has a value or not
	// 检查 <uint32_t> 容器里的条目是否真的有值
	bool isComplete() {
		return graphicsFamily.has_value() && presentFamily.has_value();
	}
};

/* Details of swap chain support */
/* 需要支持的交换链细节信息 */
struct SwapChainSupportDetails {
	// Basic surface capabilities (min/max number of images in swap chain, min/max width and height of images)
	// 基础表面特性（交换链最小最大图像数量，最小最大图像宽高）
	VkSurfaceCapabilitiesKHR capabilities;
	// Surface formats (pixel format, color space)
	// 表面格式（像素格式，颜色空间）
	std::vector<VkSurfaceFormatKHR> formats;
	// Available presentation modes
	// 可用的呈现模式
	std::vector<VkPresentModeKHR> presentModes;
};

/* The vertex structure */
/* 顶点数据结构体 */
struct Vertex {
	// The coordinate of the vertex, 2 dimensional data
	// 顶点坐标 2维
	glm::vec2 pos;
	// The RGB color of attached with the vertex
	// 顶点颜色 RGB
	glm::vec3 color;

	/* Configure from where to acquire the vertex information */
	/* 配置从何处获取顶点信息 */
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
		// 位置坐标的偏移量为成员变量 pos 占用的空间
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
		// 颜色数据的偏移量为成员变量 color 所占用的空间
		attributeDescriptions[1].offset = offsetof(Vertex, color);

		// Return the attributeDescriptions instance
		// 返回配置好的顶点特征描述实例
		return attributeDescriptions;
	}
};

/* The vertex data */
/* 顶点数据 */
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

/* The HelloTriangleApplication class */
/* HelloTriangleApplication 类 */
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

	// Window surface is an abstract type of surface to present rendered images to
	// 窗口表面是一个用来呈现图片的抽象的类型
	VkSurfaceKHR surface;

	// Physical device object, destroy itself when VkInstance is destroyed
	// 物理显卡对象。会在 VkInstance 被摧毁时自动摧毁自己
	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
	// Logical device, as the interface to the physical device
	// 逻辑设备，作为与物理设备交互的接口
	VkDevice device;

	// The graphics queue handles
	// 图形队列的句柄
	VkQueue graphicsQueue;
	// The presentation queue handles
	// 呈现队列的句柄
	VkQueue presentQueue;

	// The swap chain handler
	// 交换链句柄
	VkSwapchainKHR swapChain;
	// The images in the swap chain
	// 交换链图像句柄
	std::vector<VkImage> swapChainImages;
	// The format of the image in swap chain
	// 交换链图像格式
	VkFormat swapChainImageFormat;
	// The extent of the swap chain
	// 交换链图像范围
	VkExtent2D swapChainExtent;
	// The image view container
	// 图像视图容器
	std::vector<VkImageView> swapChainImageViews;
	// The frame buffer container
	// 所有帧缓冲对象
	std::vector<VkFramebuffer> swapChainFramebuffers;

	// The render pass
	// 渲染通道
	VkRenderPass renderPass;
	// The pipeline layout
	// 图形管线的布局
	VkPipelineLayout pipelineLayout;
	// The graphics pipeline
	// 图形管线对象
	VkPipeline graphicsPipeline;

	// The command pool, command pools manage the memory that is used to store the buffers and command buffers are allocated from them
	// 指令池，管理用于存储指令缓冲的内存，指令缓冲对象从这些内存分配而来
	VkCommandPool commandPool;

	// The vertex buffer
	// 顶点数组
	VkBuffer vertexBuffer;
	// The memory allocated to the vertex buffer
	// 分配给顶点数组的内存
	VkDeviceMemory vertexBufferMemory;

	// The command buffer
	// 指令缓冲
	std::vector<VkCommandBuffer> commandBuffers;

	// Create the image available semaphores, indicating the image is captured and ready to render
	// 图像已被获取，可以开始渲染的信号量
	std::vector<VkSemaphore> imageAvailableSemaphores;

	// Create the render finished semaphores, indicating the render is complete and ready to present
	// 图像已被渲染完成，可以开始呈现的信号量
	std::vector<VkSemaphore> renderFinishedSemaphores;

	// Create the in flight fences, for synchronization between CPU and GPU, make sure only one frame is rendering at a time
	// 为每一帧创建屏障，来进行 CPU 和 GPU 之间的同步
	std::vector<VkFence> inFlightFences;

	// Trace the current frame for rendering
	// 追踪当前渲染的是哪一帧
	uint32_t currentFrame = 0;

	// Flag for if the fram buffer is resized
	// 标记窗口是否发生改变
	bool framebufferResized = false;

	/* Initialize the window */
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

		// Store the this pointer in the data of the GLFW window
		// 将 this 指针存储在 GLFW 窗口相关的数据中
		glfwSetWindowUserPointer(window, this);

		// Set the callback function to deal with the resize of the window
		// 设置处理窗口大小改变的回调函数
		glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
	}

	/* Function that deals with the resize of the window */
	/* 处理窗口大小改变的事件的函数 */
	//		The reason that we're creating a static function as a callback is because GLFW does not know how to properly call a member function
	//			with the right this pointer to our HelloTriangleApplication instance.
	//		我们创建一个静态函数作为回调的原因是，在我们的 HelloTriangleApplication 实例中，GLFW 不知道如何正确地通过 this 指针调用成员函数
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
		// Setup debug messenger
		// 初始化调试信使
		setupDebugMessenger();
		/* Create window surface */
		/* 创建窗口表面 */
		createSurface();
		// Select a graphics card in the system that support the features we need
		// 从系统中选出一个支持我们想要的特性的显卡
		pickPhysicalDevice();
		// Set up a logical device to interface with the physical device
		// 创建逻辑设备，作为用来与物理设备进行交互的接口
		createLogicalDevice();
		// Create swap chain, a queue of images that are waiting to be presented to the screen
		// 创建交换链，交换链是一组正在等待被送往显示器显示的图片
		createSwapChain();
		// Create image view, which describes how to access the image and which part of the image to access
		// 创建图像视图，它描述了如何访问图像以及要访问图像的哪一部分
		createImageViews();
		// Create render pass, which specify the framebuffer attachments that will be used while rendering
		// 创建渲染通道，设置用于渲染的帧缓冲附着
		createRenderPass();
		// Create a graphics pipeline
		// 创建图形管线
		createGraphicsPipeline();
		// Create frame buffer
		// 创建帧缓冲
		createFramebuffers();
		// Create command pool
		// 创建指令缓冲
		createCommandPool();
		// Create vertex buffer
		// 创建顶点数组
		createVertexBuffer();
		// Create command buffer
		// 创建指令缓冲
		createCommandBuffers();
		// Create synchronization object
		// 创建同步对象
		createSyncObjects();
	}

	/* The main loop */
	/* 主循环 */
	void mainLoop() {
		// If the window should be closed?
		// 检测窗口是否需要关闭
		while (!glfwWindowShouldClose(window)) {
			// Loop and check for events like pressing the X button until the window has been closed by the user
			// 循环检测事件并处理，如是否按下了关闭按键等
			glfwPollEvents();
			
			// Draw a frame
			// 绘制一帧
			drawFrame();
		}

		// Wait for the logical device to finish operations before exiting mainLoop and destroying the window,
		//		because all of the operations in drawFrame are asynchronous
		// 等待逻辑设备操作结束执行时才销毁窗口，因为 drawFrame 中的所有操作都是异步的
		vkDeviceWaitIdle(device);
	}

	/* Destroy the resources related with swap chain */
	/* 摧毁与交换链相关的资源 */
	void cleanupSwapChain() {
		// Destroy all frame buffers
		// 摧毁所有帧缓冲
		for (auto framebuffer : swapChainFramebuffers) {
			vkDestroyFramebuffer(device, framebuffer, nullptr);
		}
		
		// Destroy all image views
		// 摧毁所有图像视图
		for (auto imageView : swapChainImageViews) {
			vkDestroyImageView(device, imageView, nullptr);
		}

		// Destroy the swap chain
		// 摧毁交换链
		vkDestroySwapchainKHR(device, swapChain, nullptr);
	}

	/* Destroy the resources requested before the end of the program */
	/* 程序结束前摧毁创建的资源 */
	void cleanup() {
		// Destroy the resources related with swap chain
		// 摧毁与交换链相关的资源
		cleanupSwapChain();

		// Destroy the graphics pipeline
		// 摧毁图形管线
		vkDestroyPipeline(device, graphicsPipeline, nullptr);
		// Destroy the pipeline layout
		// 摧毁管线布局
		vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
		// Destroy the render pass
		// 摧毁渲染通道
		vkDestroyRenderPass(device, renderPass, nullptr);

		// Destroy the vertex buffer
		// 摧毁顶点数组
		vkDestroyBuffer(device, vertexBuffer, nullptr);
		// Free the memory allocated to the vertex buffer
		// 释放分配给顶点数组的存储空间
		vkFreeMemory(device, vertexBufferMemory, nullptr);

		// Destroy the synchronization object
		// 摧毁信号量
		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			// Destroy the image available semaphores
			// 摧毁图像是否准备好的信号量
			vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
			// Destroy the render finished semaphores
			// 摧毁渲染是否结束的信号量
			vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
			// Destroy the in flight fences
			// 摧毁正在绘制屏障
			vkDestroyFence(device, inFlightFences[i], nullptr);
		}

		// Destroy the command pool
		// 摧毁指令池
		vkDestroyCommandPool(device, commandPool, nullptr);

		// Destroy the logical device
		// 摧毁逻辑设备
		vkDestroyDevice(device, nullptr);

		// If validation layer is enabled, destroy the VkDebugUtilsMessengerEXT
		// 如果启用了校验层，摧毁 VkDebugUtilsMessengerEXT 对象
		if (enableValidationLayers) {
			DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
		}

		// Destroy window surface
		// 摧毁窗口表面
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

	/* Create window surface */
	/* 重建交换链 */
	void recreateSwapChain() {
		// While the window is minimized, always acquire the current extent of the frame buffer
		// 在最小化状态时，一直获取当前的窗口的帧缓冲的大小
		int width = 0, height = 0;
		glfwGetFramebufferSize(window, &width, &height);
		while (width == 0 || height == 0) {
			glfwGetFramebufferSize(window, &width, &height);
			// Monitor the event continuously
			// 监测某些事件
			glfwWaitEvents();
		}

		// Wait for the logical device to be idle, prevent recreating the swap chain when the resources are in use
		// 等待设备处于空闲状态，避免在对象使用过程中将其清除重建
		vkDeviceWaitIdle(device);

		// Destroy the resources related with swap chain
		// 摧毁与交换链相关的资源
		cleanupSwapChain();

		// Create swap chain
		// 创建交换链
		createSwapChain();
		// Create image views
		// 创建图像视图
		createImageViews();
		// Create frame buffer
		// 创建帧缓冲
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
		// 构造一个持有应用信息的实例
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
		// 构造一个持有创建VulkanInstance所需信息的实例
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
		// The enabled extension amount is the size of the extension container
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
			// The enabled validation layer amount is the size of the validation layer container
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
			// If the validation layer is not enabled, then the amount of the validation layer will be 0
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

	/* Construct the information for creating the debug messenger */
	/* 构造调试信使的创建信息 */
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

	/* Create window surface */
	/* 创建窗口表面 */
	void createSurface() {
		// The following function has a different implementation for each platform
		// 下面这个函数是跨平台的
		if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
			throw std::runtime_error("failed to create window surface!");
		}
	}

	/* Setup debug messenger */
	/* 初始化调试信使 */
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

	/* Select a graphics card in the system that support the features we need */
	/* 从系统中选出一个支持我们想要的特性的物理显卡（在这里我们选第一个） */
	void pickPhysicalDevice() {
		// The amount of the physical device
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
		// 创建一个持有 VkPhysicalDevice 对象的容器，大小为 deviceCount
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
	//	// 构造一个持有物理设备的基本属性的实例（如基础的设备属性，如名称、类型、支持的 Vulkan 版本）
	//	VkPhysicalDeviceProperties deviceProperties;
	//	// Construct a instance that holds information on the features of physical device
	//	// 构造一个持有物理设备的功能特性的实例（如纹理压缩，64 位浮点和多视口渲染等特性）
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

	/* Create the logical device */
	/* 创建逻辑设备 */
	void createLogicalDevice() {
		// Search for and return the queue family index we desired
		// 查找并返回需求的队列族的索引
		QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

		// Create a container used to store all the queue creation information
		// 创建用来存储所有的队列创建信息的容器
		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		// Create a set for queue family, and initialize with the value(index) of graphicsFamily and presentFamily
		// 将图形队列族和呈现队列族的索引放入一个集合中
		std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };

		// Assign priorities to queues to influence the scheduling of command buffer execution
		// 通过赋予队列优先级的方式来影响命令缓冲的执行顺序
		float queuePriority = 1.0f;
		for (uint32_t queueFamily : uniqueQueueFamilies) {
			// Construct a instance that describes the number of queues we want for a single queue family
			// 创建一个实例，用来描述针对一个队列族所需队列的数量
			VkDeviceQueueCreateInfo queueCreateInfo{};
			// Explicitly specify the structure type
			// 明确指定结构的类型
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			// The queue family index desired is the one we chose
			// 队列族的索引是我们选择的那个可以满足需求的索引
			queueCreateInfo.queueFamilyIndex = queueFamily;
			// The queue amount is 1
			// 队列数目为 1
			queueCreateInfo.queueCount = 1;
			// The priority of the queue
			// 队列的优先级
			queueCreateInfo.pQueuePriorities = &queuePriority;
			// Push back the information regarding the creation of the queue
			// 将每个队列的创建信息放入队列族容器中
			queueCreateInfos.push_back(queueCreateInfo);
		}

		// Specify the device features we need, empty for now
		// 指定应用程序使用的设备特性，目前为空
		VkPhysicalDeviceFeatures deviceFeatures{};

		// Information to create a logical device
		// 用于创建逻辑设备的信息
		VkDeviceCreateInfo createInfo{};
		// Explicitly specify the structure type
		// 明确指定结构的类型
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

		// Specify the amount of the creation information of the queue
		// 指定需要创建多少个队列
		createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
		// Specify the information for the creation of the queue
		// 指定用于创建队列的信息
		createInfo.pQueueCreateInfos = queueCreateInfos.data();
		// Specify the device features we need
		// 指定我们需要的设备特性
		createInfo.pEnabledFeatures = &deviceFeatures;

		// The enabled extension amount is the size of the deviceExtensions container
		// 启用的设备扩展的个数是 deviceExtensions 的大小
		createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
		// The name of the enabled extension is the data field of the deviceExtensions
		// 启用的设备扩展的名字是 deviceEntensions 的 data 部分
		createInfo.ppEnabledExtensionNames = deviceExtensions.data();

		// Set validation layers for device as same as the instances is supported by Vulkan
		// 现在 Vulkan 支持设置 设备和实例使用相同的校验层
		if (enableValidationLayers) {
			// Set the amount of the validation layers to be enable
			// 设置需要被启用的校验层的个数
			createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			// Set the name of the validation layer
			// 设置校验层的名字
			createInfo.ppEnabledLayerNames = validationLayers.data();
		} else {
			createInfo.enabledLayerCount = 0;
		}

		// Use the information above to create the logical device
		// 使用以上信息创建逻辑设备
		if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) {
			throw std::runtime_error("failed to create logical device!");
		}

		// Retrieving the graphics queue handles
		// 取回图形队列的句柄
		vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
		// Retrieving the present queue handles
		// 取回呈现队列的句柄
		vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
	}


	/* Create swap chain (a queue of images that are waiting to be presented to the screen) */
	/* 创建交换链（一个等待呈现到屏幕上的图像队列） */
	void createSwapChain() {
		// Acquire:
		// 获取：
		//		basic surface capabilities (min/max number of images in swap chain, min/max width and height of images)
		//		基本表面功能（交换链中图像的最小/最大数量、图像的最小/最大宽度和高度）
		//		surface formats (pixel format, color space)
		//		表面格式（像素格式、色域）
		//		available presentation modes
		//		可用的呈现模式
		SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);

		// Choose the Surface format (color depth, color space)
		// 选择交换链的表面格式（色深、色域）
		VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
		// Choose the presentation mode (conditions for "swapping" images to the screen)
		// 选择交换链的呈现模式（将图像“交换”到屏幕的条件）
		VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
		// Choose the swap extent (resolution of images in swap chain)
		// 选择交换链的画幅范围（交换链中图像的分辨率）
		VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

		// Simply sticking image amount to the minimum means that we may sometimes have to wait on the driver to complete internal operations before we can acquire another image to render to.
		//		Therefore it is recommended to request at least one more image than the minimum. (Don't understand right now)
		// 简单地把这个设置为最小值，意味着我们有时可能必须等待驱动程序完成内部操作，然后才能获取另 1 个图像进行渲染。因此，建议至少多请求 1 张图像（目前不理解）
		uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
		// If the imageCount we set exceeds the maximum of the image amount supported by the swap chain, cap the imageCount to the maximum
		// 如果 imageCount 的值超过了交换链所支持的最大值，则将 imageCount 设为那个最大值
		// maxImageCount = 0 means no maximum limitation is set
		// maxImageCount = 0 意味着没有最大值的限制
		if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
			imageCount = swapChainSupport.capabilities.maxImageCount;
		}

		// Construct the instance that hold information regarding how to allocate the memory
		// 构造一个持有 交换链的创建 信息的实例
		VkSwapchainCreateInfoKHR createInfo{};
		// Explicitly specify the structure type
		// 明确指定结构的类型
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		// Specify the image from the swap chain will be present to which surface
		// 明确交换链中的图片会被呈现到哪个表面
		createInfo.surface = surface;

		// Specify the minimum amount of the image of the swap chain
		// 明确交换链的最小的图片个数
		createInfo.minImageCount = imageCount;
		// Specify the image format (color depth) of the swap chain
		// 指定交换链的图片格式为表面的格式
		createInfo.imageFormat = surfaceFormat.format;
		// Specify the image color space of the swap chain (color depth, color space)
		// 指定交换链的图片色域为表面的色域
		createInfo.imageColorSpace = surfaceFormat.colorSpace;
		// Specify the image extent (resolution) of the swap chain
		// 指定交换链的画幅为我们设置的画幅
		createInfo.imageExtent = extent;
		// Specify the amount of layers each image consists of, always 1 unless we are developing a stereoscopic 3D application
		// 层次指定每个图像包含的层数，通常为1，除非我们正在开发立体3D应用程序（VR）
		createInfo.imageArrayLayers = 1;
		
		// Specify what kind of operations we'll use the images in the swap chain for, for now is just render directly to them, which means that they're used as color attachment
		// 指定我们将在交换链中使用图像的操作类型，目前只是直接渲染到它们，这意味着它们被用作颜色附件
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		// It is also possible that you'll render images to a separate image first to perform operations like post-processing.
		// In that case you may use a value like VK_IMAGE_USAGE_TRANSFER_DST_BIT instead and use a memory operation to transfer the rendered image to a swap chain image.
		// TODO:也有可能先将图像渲染到单独的图像中，以执行后处理等操作。在这种情况下，可以使用类似 VK_IMAGE_USAGE_TRANSFER_DST_BIT 的值，并使用内存操作将渲染的图像传输到交换链图像。

		/* Specify how to handle swap chain images that will be used across multiple queue families */
		/* 指定多个队列族如何共用交换链图像 */
		// Search for and return the queue family index we desired
		// 查找并返回需求的队列族的索引
		QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
		// Create a set for queue family, and initialize with the value(index) of graphicsFamily and presentFamily
		// 将图形队列族和呈现队列族的索引放入一个集合中
		uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

		// If the graphics queue family and the presentation queue family are not the same, then
		//		drawing on the images in the swap chain from the graphics queue and then submitting them on the presentation queue
		// 如果图形队列和呈现队列不再同一个队列，则在交换链的图形队列上进行图像的绘制，然后将图像提交给呈现队列来显示
		if (indices.graphicsFamily != indices.presentFamily) {
			// Images can be used across multiple queue families without explicit ownership transfers.
			// 图像可以在多个队列族使用，不需要显示地改变所有权
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			// Specify the number of queue family index that the image will be shared with
			// 指定共享所有权的队列族的数目
			createInfo.queueFamilyIndexCount = 2;
			// Specify the queue family indices that the image will be shared with
			// 指定共享所有权的队列族
			createInfo.pQueueFamilyIndices = queueFamilyIndices;
		} else {
			// An image is owned by one queue family at a time and ownership must be explicitly transferred before using it in another queue family.
			// This option offers the best performance, which is the case on most hardware
			// 一张图像同时只能被一个队列族拥有，显示改变所有权才能换族。这一模式性能最佳，多数硬件使用这种模式
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		}

		// Specify a certain transform should be applied to images in the swap chain, like a 90 degree clockwise rotation, right now no transformation is wanted
		// 对交换链中图像指定一个固定的变换操作，如顺时针旋转 90 度，currentTransform 为不进行变换操作
		createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
		// Specify if the alpha channel should be used for blending with other windows in the window system, simply ignore the alpha channel for now
		// 指定 alpha 通道是否被用来和窗口系统中其他窗口混合，目前忽略 alpha 通道，即设置为不透明
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		// Set the presentation mode (FIFO)
		// 设置呈现模式
		createInfo.presentMode = presentMode;
		// Specify that we don't care about the color of pixels that are obscured, for example because another window is in front of them. Enabling clipping will get the best performance.
		// 设置不关心被窗口系统其他窗口遮挡的像素颜色，比如有另一个窗口在它们前面的情况。启用剪辑将获得最佳性能。
		createInfo.clipped = VK_TRUE;

		// Use the information above to create the swap chain
		// 使用以上信息创建交换链
		if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) != VK_SUCCESS) {
			throw std::runtime_error("failed to create swap chain!");
		}

		/* Retrieve the swap chain images */
		/* 取回交换链中的图像 */
		// Acquire the amount of the swap chain images
		// 获取交换链中的图像的数目
		vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
		// Resize the amount of the swapChainImages container to imageCount
		// 将 swapChainImages 容器的大小调整为 imageCount
		swapChainImages.resize(imageCount);
		// Acquire the swap chain images, store them in the swapChainImages container
		// 获取交换链中的图像，保存在 swapChainImages 中
		vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());

		// Store the format and extent we've chosen for the swap chain images in member variables, will need them in other place
		// 存储交换链的图像格式和画幅，别的地方会用到
		swapChainImageFormat = surfaceFormat.format;
		swapChainExtent = extent;
	}

	/* Create image views */
	/* 创建图像视图 */
	void createImageViews() {
		// Resize the container to fit all of the image views we'll be creating
		// 调整容器大小以适应我们将创建的所有图像视图
		swapChainImageViews.resize(swapChainImages.size());

		// Iterate over all of the swap chain images to create the image views
		// 遍历交换链所有图像来创建图像视图
		for (size_t i = 0; i < swapChainImages.size(); i++) {
			// Construct a instance that hold information about creating the image views
			// 填写图像视图创建信息结构体
			VkImageViewCreateInfo createInfo{};
			// Explicitly specify the structure type
			// 明确指定结构的类型
			createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			// The source image of the image view will be the image in the swap chain
			// v图像视图的源图像为交换链中的图像
			createInfo.image = swapChainImages[i];
			// Specify that the image will be treated as a 2D texture
			// 指定图像数据的解释方式，指定图像被看作是 2 维纹理
			createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			// Specify the format of the image view is the format of the image in the swap chain
			// 指定图像视图中的图像的格式为交换链中的图像的格式
			createInfo.format = swapChainImageFormat;
			// Specify the mapping of the color channel to be default
			// 设置图像颜色通道的映射为默认映射
			createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
			
			// Images will be used as color targets
			// 图像被用作渲染目标
			createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			// No mipmapping levels
			// 没有 mipmap
			createInfo.subresourceRange.baseMipLevel = 0;
			createInfo.subresourceRange.levelCount = 1;
			// No multiple layers
			// 没有多个图层
			createInfo.subresourceRange.baseArrayLayer = 0;
			createInfo.subresourceRange.layerCount = 1;

			// Create the image view based on the above information
			// 基于以上信息创建图像视图
			if (vkCreateImageView(device, &createInfo, nullptr, &swapChainImageViews[i]) != VK_SUCCESS) {
				throw std::runtime_error("failed to create image views!");
			}
		}
	}

	/* Create render pass, which specify the framebuffer attachments that will be used while rendering */
	/* 创建渲染通道，设置用于渲染的帧缓冲附着 */
	//		RenderPass本质是定义一个完整的渲染流程以及使用的所有资源的描述
	//		We'll have just a single color buffer attachment represented by one of the images from the swap chain
	//		我们将只有一个颜色缓冲附着，由交换链中的一个图像代表
	void createRenderPass() {
		// Construct the instance that hold information regarding the color attachment
		// 构造一个持有 颜色附着 信息的实例
		VkAttachmentDescription colorAttachment{};
		// The format of the color attachment should match the format of the swap chain images
		// 颜色附着的格式需要与交换链中的图像的格式想匹配
		colorAttachment.format = swapChainImageFormat;
		// No multisampling, stick to 1 sample
		// 没有使用多重采样，所以样本量仍然为 1
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		// 用于指定渲染前后对附着中的数据进行的操作（对颜色和深度缓冲起效），load 为前
		// Before rendering: the data in the attachment will be cleared to constant values
		// 渲染前，使用常量值来清除附着内容
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		// After rendering: Rendered contents will be stored in memory and can be read later
		// 渲染后，被渲染的内容会被存储起来，以便之后读取
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		
		// The configuration for stencil data is irrelevant for now
		// 以下为模板数据的配置，目前不关心
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		/* Create attachment reference */
		/* 创建颜色附着的引用 */

		// Construct the instance that hold information regarding the reference of the color attachment
		// 构造一个持有 附着引用 信息的实例
		VkAttachmentReference colorAttachmentRef{};
		// Specify the index to refer the color attachment 
		// 指定要引用的附着的索引
		colorAttachmentRef.attachment = 0;
		// Specify the layout we would like the attachment to have during a subpass that uses this reference: color buffer with optimal performance
		// 指定进行子流程时引用的附着使用的布局方式：颜色缓冲并且有最佳性能
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		/* Create subpass */
		/* 创建渲染子流程 */

		// Construct the instance that hold information regarding the subpass of render pass
		// 构造一个持有 渲染子流程 信息的实例
		VkSubpassDescription subpass{};
		// Specify that this is a graphics subpass
		// 指定这是一个图形渲染的子流程
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		// Specify the amount of the color reference
		// 指定引用的颜色附着的数目
		subpass.colorAttachmentCount = 1;
		// Specify the color reference
		// 指定引用的颜色附着
		subpass.pColorAttachments = &colorAttachmentRef;
		// The index of the attachment in this array is directly referenced from the fragment shader, i.e. outColor
		// 此数组中附件的索引是直接从片段着色器引用的，即 outColor

		/* Specify the dependency of the subpass */
		/* 配置子流程依赖 */
		//		Image layout transitions are controlled by subpass dependencies, which specify memory and execution dependencies between subpasses
		//		图像布局转换由子进程依赖项控制，这些依赖项指定子进程之间的内存和执行的依赖项

		//		Configure built in dependencies that take care of the transition at the start of the render pass and at the end of the render pass
		//		配置在渲染过程开始和结束时负责转换的内置依赖项
		
		// Construct the instance that hold information of the dependency of the subpass
		// 构造一个持有子流程的依赖信息的实例
		VkSubpassDependency dependency{};
		// Specify the source of the dependency, which is the implicit subpass before the render pass
		// 指定依赖提供者，为隐晦的渲染流程开始前的子流程的索引
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		// Specify the destination of the dependency: the index 0 refers to our subpass, which is the first and only one
		// 指定依赖消费者为索引 0 的子流程：之前创建的 subpass，也即第一个也是唯一一个
		dependency.dstSubpass = 0;
		// Specify to wait on the color attachment output stage, wait for the swap chain to finish reading from the image
		// 指定需要等待颜色附着输出这一管线阶段
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		// Specify to wait on the color attachment stage itself
		// 指定需要等待颜色附着输出阶段本身
		dependency.srcAccessMask = 0;
		// Specify the waiter stage, which is the color attachment output stage
		// 指定需要等待颜色附着输出这一管线阶段
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		// Specify the waiter operation, which is the color attachment write operation
		// 指定需要等待颜色附着输出阶段中的写操作
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		/* Create render pass */
		/* 创建渲染通道 */
		// Construct the instance that hold information of the render pass
		// 构造一个持有 渲染通道 信息的实例
		VkRenderPassCreateInfo renderPassInfo{};
		// Explicitly specify the structure type
		// 明确指定结构的类型
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		// Specify the amount of the color attachment
		// 指定颜色附着的数目
		renderPassInfo.attachmentCount = 1;
		// Specify the color attachment
		// 指定颜色附着
		renderPassInfo.pAttachments = &colorAttachment;
		// Specify the amount of the subpass
		// 指定渲染子阶段的数目
		renderPassInfo.subpassCount = 1;
		// Specify the subpass
		// 指定渲染子阶段
		renderPassInfo.pSubpasses = &subpass;
		// Specify the amount of the dependency of the subpass
		// 指定子流程依赖的数目
		renderPassInfo.dependencyCount = 1;
		// Specify the dependency of the subpass
		// 指定子流程依赖
		renderPassInfo.pDependencies = &dependency;

		// Create the render pass based on the information above
		// 基于以上信息创建渲染通道
		if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
			throw std::runtime_error("failed to create render pass!");
		}
	}

	/* Create graphcis pipeline */
	/* 创建图形管线 */
	void createGraphicsPipeline() {
		// Read the vertex shader, SPIR-V byte code compiled by GLSL compiler
		// 读入顶点着色器的程序（由 GLSL 编译生成的 SPIR-V 字节码）
		auto vertShaderCode = readFile(".\\shaders\\vert.spv");
		// Read the fragment shader, SPIR-V byte code compiled by GLSL compiler
		// 读入片元着色器的程序（由 GLSL 编译生成的 SPIR-V 字节码）
		auto fragShaderCode = readFile(".\\shaders\\frag.spv");

		// Create the shader module using the vertex shader code
		// 使用顶点着色代码创建顶点着色器模块
		VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
		// Create the fragment module using the vertex shader code
		// 使用片元着色代码创建片元着色器模块
		VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);

		// Construct the instance that hold information on the creation of the vertex shader stage
		// 构造持有着色器阶段创建信息结构体
		VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
		// Explicitly specify the structure type
		// 明确指定结构的类型
		vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		// Explicitly specify the stage of the shader to be a vertex stage
		// 明确着色器的阶段为顶点阶段
		vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		// Specify the shader module to the stage: vertex shader
		// 指定需要使用的着色器模块：顶点着色器
		vertShaderStageInfo.module = vertShaderModule;
		// Specify the function name to be called
		// 指定需要调用的函数名
		vertShaderStageInfo.pName = "main";

		// Construct the instance that hold information on the creation of the vertex shader stage
		// 构造持有着色器阶段创建信息结构体
		VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
		// Explicitly specify the structure type
		// 明确指定结构的类型
		fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		// Explicitly specify the stage of the shader to be a fragment stage
		// 明确着色器的阶段为片元阶段
		fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		// Specify the shader module to the stage: fragment shader
		// 指定需要使用的着色器模块：片元着色器
		fragShaderStageInfo.module = fragShaderModule;
		// Specify the function name to be called
		// 指定需要调用的函数名
		fragShaderStageInfo.pName = "main";

		// Put the two shader modules into an array for future reference
		// 将 2 个着色器阶段放入 shaderStages 的数组中，后面会用到
		VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

		/** Configured the fixed funtion stage **/
		/** 设置固定功能的阶段 **/

		/* Configure the information of the input */
		/* 配置输入的信息 */

		// Specify the format of the vertex data that will be passed to the vertex shader
		// 用来描述传递给顶点着色器的顶点数据格式
		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		// Explicitly specify the structure type
		// 明确指定结构的类型
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

		// Acquire the binding information of the vertex data
		// 获取顶点数据的绑定信息
		//		Bindings: spacing between data and whether the data is per-vertex or per-instance (see instancing)
		//		绑定：数据之间的间距以及数据是按顶点还是按实例
		auto bindingDescription = Vertex::getBindingDescription();
		// Acquire the attribute information of the vertex data
		// 获取顶点数据中的属性
		//		Attribute descriptions: type of the attributes passed to the vertex shader, which binding to load them from and at which offset
		//		属性描述：传递到顶点着色器的属性类型，从哪个绑定加载这些属性以及偏移量
		auto attributeDescriptions = Vertex::getAttributeDescriptions();

		// The amount of the binding
		// 输入顶点的绑定个数为 1
		vertexInputInfo.vertexBindingDescriptionCount = 1;
		// The amount of the vertex attribute description is 2, i.e. vertex coordinate and vertex color
		// 输入顶点的属性的大小为相应的大小为 2：顶点坐标和顶点颜色
		vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
		// Reference the binding description of the vertex data as the bingding description of the input
		// 引用顶点数据绑定信息的描述，作为输入的绑定信息
		vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
		// The attribute description of the vertex is the attribute description of the input
		// 将顶点的属性作为输入的属性	
		vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

		/* Configure the information of the input assembly stage */
		/* 配置输入装配阶段的信息 */

		// Construct the instance that hold information on the creation of the input assembly stage
		// 构造持有输入装配阶段的创建信息的结构体
		VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
		// Explicitly specify the structure type
		// 明确指定结构的类型
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		// Specify the topology of the input to be triangle
		// 将输入的拓扑指定为三角形
		//		VK_PRIMITIVE_TOPOLOGY_POINT_LIST: points from vertices
		//		VK_PRIMITIVE_TOPOLOGY_POINT_LIST: 由顶点构成的点
		//		VK_PRIMITIVE_TOPOLOGY_LINE_LIST: line from every 2 vertices without reuse
		//		VK_PRIMITIVE_TOPOLOGY_LINE_LIST: 由 2 个顶点构成的线，不重复使用
		//		VK_PRIMITIVE_TOPOLOGY_LINE_STRIP: the end vertex of every line is used as start vertex for the next line
		//		VK_PRIMITIVE_TOPOLOGY_LINE_STRIP: 每条线的结束顶点用作下一条线的开始顶点
		//		VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST: triangle from every 3 vertices without reuse
		//		VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST: 由 3 个顶点构成的三角形，不重复使用
		//		VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP: the second and third vertex of every triangle are used as first two vertices of the next triangle
		//		VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP: 每个三角形的第二个和第三个顶点用作下一个三角形的前两个顶点
		inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		// Set the primitive restart to be disabled
		// 不启用几何图元重启
		// TODO: (Don't know right now) Element buffer allows you to perform optimizations like reusing vertices. If you set the primitiveRestartEnable member to VK_TRUE,
		//		then it's possible to break up lines and triangles in the _STRIP topology modes by using a special index of 0xFFFF or 0xFFFFFFFF
		//		元素缓冲区允许您执行优化，例如重用顶点。如果将 primitiveRestartEnable 成员设置为 VK_TRUE，则可以使用 0xFFFF 或 0xFFFFFFFF 的特殊索引来分解 _STRIP 拓扑模式中的直线和三角形
		inputAssembly.primitiveRestartEnable = VK_FALSE;

		// Construct the instance that hold information on the creation of the viewport stage
		// 构造持有视口阶段的创建信息的结构体
		VkPipelineViewportStateCreateInfo viewportState{};
		// Explicitly specify the structure type
		// 明确指定结构的类型
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		// Specify the amount of viewport at the pipeline creation time
		// 在创建管线的时候指定视口的数目为 1
		viewportState.viewportCount = 1;
		// Specify the amount of scissor at the pipeline creation time
		// 在创建管线的时候指定裁剪矩形的数目为 1
		viewportState.scissorCount = 1;

		/* Configure the information of the rasterization stage */
		/* 配置光栅化阶段的信息 */

		// Construct the instance that hold information on the creation of the rasterization stage
		// 构造持有光栅化阶段的创建信息的结构体
		VkPipelineRasterizationStateCreateInfo rasterizer{};
		// Explicitly specify the structure type
		// 明确指定结构的类型
		rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		// Disable the depth clamp function, so as to discard the fragments that are beyond the near and far planes
		// 关闭 depth clamp 功能，即丢弃近平面和远平面外的片段
		//		Depth clamp is useful in some special cases like shadow maps. Using depth clamp requires enabling a GPU feature.
		//		Depth clamp 在某些特殊情况下很有用，如阴影贴图。使用 depth clamp （深度钳？）需要启用GPU功能。
		rasterizer.depthClampEnable = VK_FALSE;
		// Allow all geometry passes through the rasterizer stage, set to VK_TRUE then geometry never pass
		// 允许所有几何图元通过光栅化，若设置为 VK_TRUE 则不允许所有几何通过
		rasterizer.rasterizerDiscardEnable = VK_FALSE;
		// Specify how fragments are generated for geometry
		// 指定几何图元生成片段（像素）的方式，整个多边形包括内部
		//		VK_POLYGON_MODE_FILL: fill the area of the polygon with fragments
		//		VK_POLYGON_MODE_FILL: 用片元填充多边形区域
		//		VK_POLYGON_MODE_LINE: polygon edges are drawn as lines
		//		VK_POLYGON_MODE_LINE: 多边形边绘制为直线
		//		VK_POLYGON_MODE_POINT: polygon vertices are drawn as points
		//		VK_POLYGON_MODE_POINT: 多边形顶点绘制为点

		//		Using any mode other than fill requires enabling a GPU feature
		//		使用填充以外的任何模式都需要启用GPU功能
		rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
		
		// Specify the thickness of lines in terms of number of fragments
		// 根据片元的数量指定线条的厚度
		//		Any line thicker than 1.0f requires you to enable the wideLines GPU feature
		//		线宽大于 1 则需要启用 GPU 的 wideLines 特性
		rasterizer.lineWidth = 1.0f;
		// Specify the type of face culling to the back face
		// 指定剔除的面为背面
		rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
		// Specify the vertex order for faces to be considered front-facing
		// 指定顺时针的定点顺序为正面
		rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
		// Disable depth bias
		// 关闭 depth bias 功能
		// The rasterizer can alter the depth values by adding a constant value or biasing them based on a fragment's slope. This is sometimes used for shadow mapping
		// 添加一个常量值或者一个基于片段所处线段斜率值得到的变量值到深度上，这个功能有时候对阴影贴图有用
		rasterizer.depthBiasEnable = VK_FALSE;

		// Construct the instance that hold information on the creation of the multisampling stage
		// 构造持有多重采样阶段的创建信息的结构体
		//		Multisampling combines the fragment shader results of multiple polygons that rasterize to the same pixel
		//		多重采样将光栅化到同一个像素的多个多边形的片段着色器的结果组合起来
		VkPipelineMultisampleStateCreateInfo multisampling{};
		// Explicitly specify the structure type
		// 明确指定结构的类型
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		// Disable multisampling. Multisampling need GPU features to support
		// 不启用多重采样，若要启用多重采样需要启用 GPU 的特性
		multisampling.sampleShadingEnable = VK_FALSE;
		// Specify the sampling for rasterization to be 1 sample per pixel
		// 设置光栅化的采样方式为每像素 1 个采样
		multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

		/* Just jump the depth and stencil testing for simplicity */
		/* 为了避免示例过于复杂，先跳过深度和模板测试阶段 */
		
		/* Color blending */
		/* 颜色混合 */
		//		After a fragment shader has returned a color, it needs to be combined with the color that is already in the framebuffer
		//		片段着色器返回颜色后，需要将其与帧缓冲区中已存在的颜色组合
		
		/* Configure the per attached framebuffer color blending settings */
		/* 设置对每一个帧缓冲的颜色混合参数 */
		//		The first way of color blending: mix the old and new value to produce a final color in a per framebuffer order (disabled in here)
		//		第一种混合方式：对每个绑定的帧缓冲进行单独的颜色混合配置（不使用这种方式，所以先关闭）
		//			If blendEnable is set to VK_FALSE, then the new color from the fragment shader is passed through unmodified.
		//			如果blendEnable设置为VK_FALSE，则片段着色器中的新颜色将不经修改地通过。
		//			Otherwise, the two mixing operations are performed to compute a new color.
		//			否则，执行两个混合操作以计算新的颜色。
		//			The resulting color is AND'd with the colorWriteMask to determine which channels are actually passed through.
		//			生成的颜色与colorWriteMask进行AND运算，以确定实际通过的通道。

		// Construct the instance that hold the color blending information per attached framebuffer
		// 构造一个持有对于每一个帧缓冲的颜色混合附件的结构体
		VkPipelineColorBlendAttachmentState colorBlendAttachment{};
		// Specify the information that will participate in the color blending
		// 指名需要参与颜色混合的通道有哪些
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		// Disable the color blending for per framebuffer, so as we don't use the first method
		// 关闭对于每个帧缓冲的颜色混合，不使用第一种颜色混合方法
		colorBlendAttachment.blendEnable = VK_FALSE;

		/* Configure the global color blending settings */
		/* 设置全局颜色混合参数 */

		// Construct the instance that hold information on the creation of the color blending stage
		// 构造持有颜色混合阶段的创建信息的结构体
		VkPipelineColorBlendStateCreateInfo colorBlending{};
		// Explicitly specify the structure type
		// 明确指定结构的类型
		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		
		// The first way of color blending: combine the old and new value using a bitwise operation, disabled now
		// 第二种混合方式：使用位运算组合旧值和新值来混合颜色，这里关闭，开启的话会禁用第一种
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.logicOp = VK_LOGIC_OP_COPY; //  Optional
		// TODO:Specify the amount of color blending attachment for the color blending
		// 指名颜色混合所要使用的颜色混合附件的数目（目前还不是很清晰）
		colorBlending.attachmentCount = 1;
		// TODO:Specify the color blending attachment for the color blending
		// 指名颜色混合所要使用的颜色混合附件
		colorBlending.pAttachments = &colorBlendAttachment;
		colorBlending.blendConstants[0] = 0.0f; //  Optional
		colorBlending.blendConstants[1] = 0.0f; //  Optional
		colorBlending.blendConstants[2] = 0.0f; //  Optional
		colorBlending.blendConstants[3] = 0.0f; //  Optional

		/* Configure dynamic states */
		/* 配置动态阶段 */

		// Specify the dynamic state, which can be changed without recreating the pipeline at draw time
		// 指定动态阶段，动态阶段在绘制时无需重新创建管线即可更改
		std::vector<VkDynamicState> dynamicStates = {
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_SCISSOR
		};
		// Construct the instance that holds information about creating dynamic states
		// 构造一个持有动态阶段的创建信息的实例
		VkPipelineDynamicStateCreateInfo dynamicState{};
		// Explicitly specify the structure type
		// 明确指定结构的类型
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		// Specify the amount of the dynamics state
		// 指定动态阶段的数目
		dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
		// Specify the dynamics state
		// 指定动态阶段
		dynamicState.pDynamicStates = dynamicStates.data();

		/* Create the pipeline layout */
		/* 创建管线布局 */
		// To support uniform values in shaders, commonly used to pass the transformation matrix to the vertex shader, or to create texture samplers in the fragment shader
		// TODO: 支持着色器中的 uniform 值，通常用于将变换矩阵传递到顶点着色器，或在片段着色器中创建纹理采样器，虽然现在不用，但是也要定义

		// TODO: passing dynamic values to shaders?
		// TODO：使用 layout 定义着色器的 uniform？
		// Construct the instance that holds information about creating pipeline layout
		// 构造一个持有管线布局的创建信息的实例
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		// Explicitly specify the structure type
		// 明确指定结构的类型
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 0; // Optional
		pipelineLayoutInfo.pSetLayouts = nullptr; // Optional
		pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
		pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

		// Create the pipeline layout based on the information above
		// 使用以上信息创建管线的布局
		if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create pipeline layout!");
		}

		/* Create the graphics pipeline */
		/* 创建图形管线 */

		// Construct the instance that holds information about creating graphics pipeline
		// 构造一个持有图形管线的创建信息的实例
		VkGraphicsPipelineCreateInfo pipelineInfo{};
		// Explicitly specify the structure type
		// 明确指定结构的类型
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		// Specify the amount of the shader stage in the graphics pipeline
		// 指定图形管线中的着色器阶段的数目
		pipelineInfo.stageCount = 2;
		// Specify the shader stages in the graphics pipeline
		// 指定图形管线中的着色器阶段
		pipelineInfo.pStages = shaderStages;
		// Specify the vertex input information in the graphics pipeline
		// 指定图形管线中的顶点输入的信息
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		// Specify the input assembly stage in the graphics pipeline
		// 指定图形管线中的输入装配阶段
		pipelineInfo.pInputAssemblyState = &inputAssembly;
		// Specify the viewport stage in the graphics pipeline
		// 指定图形管线中的视角阶段
		pipelineInfo.pViewportState = &viewportState;
		// Specify the rasterization stage in the graphics pipeline
		// 指定图形管线中的光栅化阶段
		pipelineInfo.pRasterizationState = &rasterizer;
		// Specify the multisampling stage in the graphics pipeline
		// 指定图形管线中的多重采样阶段
		pipelineInfo.pMultisampleState = &multisampling;
		// Specify the color blending stage in the graphics pipeline
		// 指定图形管线中的颜色混合阶段
		pipelineInfo.pColorBlendState = &colorBlending;
		// Specify the dynamic state stage in the graphics pipeline
		// 指定图形管线中动态阶段
		pipelineInfo.pDynamicState = &dynamicState;
		// Specify the pipeline layout in the graphics pipeline
		// 指定图形管线中管线布局
		pipelineInfo.layout = pipelineLayout;
		// Specify the render pass in the graphics pipeline
		// 指定图形管线中渲染通道
		pipelineInfo.renderPass = renderPass;
		// Specify the amount of the subpass in the graphics pipeline
		// 指定图形管线中的渲染子阶段的数目
		pipelineInfo.subpass = 0;

		// Specify NOT to create a new graphics pipeline by deriving from an existing pipeline
		// 用于以一个创建好的图形管线为基础创建一个新的图形管线
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; //  Optional

		// Create the graphics pipeline based on the information above
		// 使用以上信息创建图形管线
		if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS) {
			throw std::runtime_error("failed to create graphics pipeline!");
		}

		// Destroy the vertex shader module
		// 摧毁顶点着色器模块
		vkDestroyShaderModule(device, fragShaderModule, nullptr);
		// Destroy the fragment shader module
		// 摧毁片元着色器模块
		vkDestroyShaderModule(device, vertShaderModule, nullptr);
	}

	/* Create frame buffer */
	/* 创建帧缓冲 */
	void createFramebuffers() {
		// Resize the size of the frame buffer in swap chain to the size of the image views
		// 将交换链中的帧缓冲的容器大小改为图像视图的容器大小
		swapChainFramebuffers.resize(swapChainImageViews.size());
		
		// Iterate through the image views and create framebuffers from them
		// 遍历每个图像视图并从中创建帧缓冲
		for (size_t i = 0; i < swapChainImageViews.size(); i++) {
			VkImageView attachments[] = {
				swapChainImageViews[i]
			};

			// Construct the instance that holds information about creating frame buffer
			// 构造一个持有帧缓冲的创建信息的实例
			VkFramebufferCreateInfo framebufferInfo{};
			// Explicitly specify the structure type
			// 明确指定结构的类型
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			// Specify the render pass that the framebuffer needs to be compatible with
			// 指定帧缓冲需要兼容的渲染通道
			framebufferInfo.renderPass = renderPass;
			// Specify the amount of the VkImageView objects that should be bound to the respective attachment
			// 指定需要被绑定到对应的颜色附着的图像视图的数目
			framebufferInfo.attachmentCount = 1;
			// Specify the VkImageView objects that should be bound to the respective attachment
			// 指定需要被绑定到对应的颜色附着的图像视图
			framebufferInfo.pAttachments = attachments;
			// Specify the width of the frame buffer be the width of the extent of the swap chain
			// 指定帧缓冲的宽度为交换链的画幅的宽度
			framebufferInfo.width = swapChainExtent.width;
			// Specify the height of the frame buffer be the width of the extent of the swap chain
			// 指定帧缓冲的高度为交换链的画幅的高度
			framebufferInfo.height = swapChainExtent.height;
			// Specify the layer of the frame buffer to be 1
			// 帧缓冲图像层数
			framebufferInfo.layers = 1;

			// Create the frame buffer based on the information above
			// 使用以上信息创建帧缓冲
			if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS) {
				throw std::runtime_error("failed to create framebuffer!");
			}
		}
	}

	/* Create command pool */
	/* 创建指令缓冲 */
	void createCommandPool() {
		// Search for and return the queue family index we desired
		// 查找并返回需求的队列族的索引
		QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice);

		// Construct the instance that holds information about creating command pool
		// 构造一个持有指令池的创建信息的实例
		VkCommandPoolCreateInfo poolInfo{};
		// Explicitly specify the structure type
		// 明确指定结构的类型
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		// Allow command buffers to be rerecorded individually, without this flag they all have to be reset together
		// 允许命令缓冲单独重新记录命令，如果没有此标志，则必须将所有命令缓冲一起重置
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		// Chosen the graphics queue family because we are recording commands for drawing
		// 选择将绘制指令提交给图形队列族，因为我们正在录制绘图命令
		poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

		// Create command pool based on the information above
		// 使用以上信息创建指令缓冲
		if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
			throw std::runtime_error("failed to create command pool!");
		}
	}

	/* Create vertex buffer */
	/* 创建顶点数组 */
	void createVertexBuffer() {
		// Construct the instance that holds information of creating buffer
		// 构造一个持有 buffer 创建信息的实例
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
		// 构造一个持有内存需求的实例
		VkMemoryRequirements memRequirements;
		// Query the desired properties of the vertex buffer
		// 询问顶点数据 buffer 对于内存的需求
		vkGetBufferMemoryRequirements(device, vertexBuffer, &memRequirements);

		// Construct the instance that hold information regarding how to allocate the memory
		// 构造一个持有内存分配信息的实例
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

	/* Find the memory type desired */
	/* 寻找存储的类型 */
	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
		// Construct a instance to store memory properties information, including memory type and memory heap
		// 构造一个持有 memory 属性信息的实例（包含存储类型和存储堆）
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

	/* Create command buffers */
	/* 创建指令缓冲 */
	void createCommandBuffers() {
		// Resize the size of the commandBuffers to 2
		// 将指令缓冲容器的大小设置为 2
		commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

		// Construct a instance that hold information on the allocation of the command buffer
		// 构造一个持有 命令缓冲 的分配信息的实例
		VkCommandBufferAllocateInfo allocInfo{};
		// Explicitly specify the structure type
		// 明确指定结构的类型
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		// The command pool that the command buffer allocated from
		// 使用的指令池和需要分配的指令缓冲个数
		allocInfo.commandPool = commandPool;
		// Set the level to be primary: Can be submitted to a queue for execution, but cannot be called from other command buffers
		// 分配的级别设置为主要，可被提交到队列执行，不能被其他指令缓冲对象调用
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		// Specify the amount of the command buffer to be the size of the commandBuffers, which is 2
		// 指定命令缓冲的大小为 commandBuffers 的大小，也即 2
		allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();

		// Allocate command buffers based on the information above
		// 基于以上信息分配命令缓冲
		if (vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate command buffers!");
		}
	}

	/* Record command to the command buffer */
	/* 记录指令到指令缓冲 */
	void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
		// Construct a instance that hold information on how to start the command buffer
		// 构造一个持有命令缓冲的启动信息的实例
		VkCommandBufferBeginInfo beginInfo{};
		// Explicitly specify the structure type
		// 明确指定结构的类型
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		
		// TODO：可以使得在上一帧还未结束渲染时，提交下一帧的渲染指令

		// Begin recording command to the command buffer
		// 开始向指令缓冲中记录指令
		if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
			throw std::runtime_error("failed to begin recording command buffer!");
		}

		/* Starting a render pass */
		/* 开始渲染通路 */
 
		// Construct a instance that hold information on the allocation of the command buffer
		// 构造一个持有渲染通路的启动信息的实例
		VkRenderPassBeginInfo renderPassInfo{};
		// Explicitly specify the structure type
		// 明确指定结构的类型
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		// Specify the render pass
		// 指定渲染通路
		renderPassInfo.renderPass = renderPass;
		// Specify the frame buffer according to the image index
		// 指定帧缓存，参考图像的索引
		renderPassInfo.framebuffer = swapChainFramebuffers[imageIndex];
		// Specify the offset of the render area
		// 指定渲染区域的偏移量
		renderPassInfo.renderArea.offset = { 0, 0 };
		// Specify the extent of the render area
		// 指定渲染区域的画幅
		renderPassInfo.renderArea.extent = swapChainExtent;

		// Specify the color that is used to clear the image data
		// 设置清除颜色
		VkClearValue clearColor = { {{0.0f, 0.0f, 0.0f, 1.0f}} };
		// Specify the amount of the clear color
		// 指定用于清除的颜色的数目
		renderPassInfo.clearValueCount = 1;
		// Specify the vaule of the clear color
		// 指定用于清除的颜色
		renderPassInfo.pClearValues = &clearColor;
		
		// Begin the render pass
		// 渲染通路开始
		//		VK_SUBPASS_CONTENTS_INLINE: the render pass commands will be embedded in the primary command buffer itself and no secondary command buffers will be executed
		//		VK_SUBPASS_CONTENTS_INLINE：渲染通路的命令将嵌入主命令缓冲本身，不会执行任何辅助命令缓冲中的指令
		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
			
			// Bind the graphics pipeline to the command buffer
			// 将这个指令缓冲绑定到已创建那个图形管线
			vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

			/* Configure the information about viewport */
			/* 配置视口的信息 */

			// A viewport basically describes the region of the framebuffer that the output will be rendered to
			// 视口基本上描述了 输出图像将会被渲染到帧缓冲的哪个区域
			VkViewport viewport{};
			// Configure the coordinate of the viewport
			// 配置视口的坐标
			viewport.x = 0.0f;
			viewport.y = 0.0f;
			// Configure the width and height of the viewport to be the width and height of the swap chain
			// 配置视口的宽高为交换链的宽高
			viewport.width = static_cast<float>(swapChainExtent.width);
			viewport.height = static_cast<float>(swapChainExtent.height);
			// Configure the range of depth values to use for the framebuffer to the standard values
			// 配置帧缓冲区使用的深度值范围为标准值
			viewport.minDepth = 0.0f;
			viewport.maxDepth = 1.0f;
			// Set the viewport in the command buffer
			// 在命令缓冲中设置视口
			vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

			// Specify the information about scissor
			// 定义裁剪矩形的信息
			VkRect2D scissor{};
			// Specify the offset coordinate of the scissor to be 0
			// 配置裁剪矩形的偏移量均为 0
			scissor.offset = { 0, 0 };
			// Specify the extent of the scissor to be the extent of the swap chain
			// 配置裁剪矩形的画幅为交换链的画幅
			scissor.extent = swapChainExtent;
			// Set the scissor in the command buffer
			// 在命令缓冲中设置裁剪矩形
			vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

			// Specify the buffers to be bind for vertex buffer
			// 需要绑定的 buffer 包括哪些
			VkBuffer vertexBuffers[] = { vertexBuffer };
			// Specify the offset for the buffer with the same index
			// 需要绑定的缓冲所对应的偏移量
			VkDeviceSize offsets[] = { 0 };
			// Bind the vertexBuffers to the binding: bind 1 binding at binding 0, the buffer is vertexBuffers, and the offsets of the vertexBuffers
			// 将 vertexBuffers 绑定到 binding：从第 0 个 binding 位置起，绑定 1 个 binding，绑定的 buffer 是 vertexbuffers，从 vertexbuffer 的 offsets 的偏移量开始读
			vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

			// Issue the draw command
			// 提交绘制操作到指令缓冲
			vkCmdDraw(commandBuffer, static_cast<uint32_t>(vertices.size()), 1, 0, 0);
		
		// End the render pass
		// 渲染通路结束
		vkCmdEndRenderPass(commandBuffer);

		// End recording command to the command buffer
		// 结束指令到指令缓冲
		if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
			throw std::runtime_error("failed to record command buffer!");
		}
	}

	/* Create synchronization object */
	/* 创建同步对象 */
	void createSyncObjects() {
		// Resize the image available semaphores contrainer to the number of the maximum frames in flight
		// 将图像是否准备好的信号量的大小调整为最大正在绘制的帧数的大小
		imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		// Resize the render finished semaphores contrainer to the number of the maximum frames in flight
		// 将渲染是否结束的信号量的大小调整为最大正在绘制的帧数的大小
		renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		// Resize the in flight fences contrainer to the number of the maximum frames in flight
		// 将正在绘制屏障的大小调整为最大正在绘制的帧数的大小
		inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

		// Construct a instance that hold information on creating semaphore
		// 构造一个持有信号量创建信息的实例
		VkSemaphoreCreateInfo semaphoreInfo{};
		// Explicitly specify the structure type
		// 明确指定结构的类型
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		// Construct a instance that hold information on creating fence
		// 构造一个持有屏障的创建信息的实例
		VkFenceCreateInfo fenceInfo{};
		// Explicitly specify the structure type
		// 明确指定结构的类型
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		// Set the fence to be signaled to prevent the deadlock
		// 设置初始状态为已发出信号来避免死锁
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		// For each frame in flight
		// 对于每一个正在绘制的帧
		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			// Create semaphores and fence based on the information above
			// 使用以上信息创建信号量和屏障
			if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
				vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
				vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) {
				throw std::runtime_error("failed to create synchronization objects for a frame!");
			}
		}
	}

	/* Draw a frame */
	/* 绘制一帧 */
	void drawFrame() {
		// Wait until the previous frame has finished, so that the command buffer and semaphores are available to use for the current frame
		// 等待当前帧所使用的指令缓冲和信号量结束执行
		vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

		// Acquire the image from the swap chain, disable the timeout
		// 从交换链获取图像，禁用超时
		uint32_t imageIndex;
		VkResult result = vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

		// If the swap chain is out of date
		// 如果交换链过期
		if (result == VK_ERROR_OUT_OF_DATE_KHR) {
			// Recreate the swap chain
			// 重建交换链
			recreateSwapChain();
			return;
		} else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
			throw std::runtime_error("failed to acquire swap chain image!");
		}

		// Reset the fence to the unsignaled state for the current frame, only reset the fence if we are submitting work
		// 重置屏障至初始状态，只有当我们提交指令时才重置屏障
		vkResetFences(device, 1, &inFlightFences[currentFrame]);

		// Reset the command buffer
		// 重置指令缓冲
		vkResetCommandBuffer(commandBuffers[currentFrame], /*VkCommandBufferResetFlagBits*/ 0);
		// Record command to the command buffer
		// 记录指令到指令缓冲
		recordCommandBuffer(commandBuffers[currentFrame], imageIndex);

		/* Specify the submit information */
		/* 配置提交信息 */

		// Construct a instance that hold information on the submission of the command buffer
		// 构造一个持有提交指令缓冲信息的实例
		VkSubmitInfo submitInfo{};
		// Explicitly specify the structure type
		// 明确指定结构的类型
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		// 指定队列开始执行前需要等待的信号量，以及需要等待的管线阶段

		// Specify we should wait the imageAvailableSemaphores semaphores before execution begins
		// 指定指令开始执行前需要等待 imageAvailableSemaphores 信号量
		VkSemaphore waitSemaphores[] = { imageAvailableSemaphores[currentFrame] };
		// Specify we should wait the stage of the graphics pipeline that writes to the color attachment before execution begins
		// 指定指令开始执行前，需要等待写入颜色附件的图形管道的阶段
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		// Specify the amount of the semaphores to wait on before execution begins
		// 指定指令开始执行前，需要等待的信号量的数目
		submitInfo.waitSemaphoreCount = 1;
		// Specify which semaphores to wait on before execution begins
		// 指定指令开始执行前，需要等待哪个信号量
		submitInfo.pWaitSemaphores = waitSemaphores;
		// Specify which stages of the pipeline to wait on before execution begins
		// 指定指令开始执行前，需要等待图形管线中的哪个阶段
		submitInfo.pWaitDstStageMask = waitStages;

		// Specify the amount of the command buffers that will be submitted
		// 指定实际被提交执行的指令缓冲对象的数目
		submitInfo.commandBufferCount = 1;
		// Specify the command buffers that will be submitted
		// 指定实际被提交执行的指令缓冲对象
		submitInfo.pCommandBuffers = &commandBuffers[currentFrame];

		// Specify the semaphores, renderFinishedSemaphores, need to be signaled after the execution of the command buffer
		// 指定在指令缓冲中的指令执行完成后需要被示意发出本帧的渲染已经完成的信号
		VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[currentFrame] };
		// Specify the amount of the semaphores need to be signaled after the execution of the command buffer
		// 指定在指令缓冲中的指令执行完成后需要被示意的信号量的数目
		submitInfo.signalSemaphoreCount = 1;
		// Specify the semaphores need to be signaled after the execution of the command buffer
		// 指定在指令缓冲中的指令执行完成后需要被示意的信号量
		submitInfo.pSignalSemaphores = signalSemaphores;

		// Submit the command buffer to the graphics queue, and signal the inFlightFences after the execution of the command buffer
		// 提交指令缓冲给图形指令队列，并在指令缓冲执行结束后对 inFlightFences 屏障对象示意
		if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS) {
			throw std::runtime_error("failed to submit draw command buffer!");
		}

		/* Configure the presentation information */
		/* 配置呈现信息 */

		// Construct a instance that hold information on presentation
		// 构造一个持有呈现信息的实例
		VkPresentInfoKHR presentInfo{};
		// Explicitly specify the structure type
		// 明确指定结构的类型
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

		// Specify the amount of the semaphores to wait before presentation
		// 指定开始呈现操作时需要等待的信号量的数目
		presentInfo.waitSemaphoreCount = 1;
		// Specify the semaphores to wait to be the renderFinishedSemaphores, because we want to wait on the command buffer to finish execution
		// 指定开始呈现操作时需要等待的信号量为 renderFinishedSemaphores，因为我们想要等待指令缓冲结束执行
		presentInfo.pWaitSemaphores = signalSemaphores;

		// Specify the swap chains to present inages to
		// 指定呈现图像的交换链
		VkSwapchainKHR swapChains[] = { swapChain };
		// Specify the amount of the swap chains to be 1
		// 指定交换链的数目
		presentInfo.swapchainCount = 1;
		// Specify the swap chains
		// 指定交换链
		presentInfo.pSwapchains = swapChains;

		// Specify the index of the image for each swap chain
		// 指定图像在交换链中的索引
		presentInfo.pImageIndices = &imageIndex;

		// Submit the request to present an image to the swap chain
		// 提交将交换链中的图像进行呈现的请求
		result = vkQueuePresentKHR(presentQueue, &presentInfo);

		// If (the swap chain is out of date) || (the swap chain is not optimal for the current situation) || (the frame buffer is resized)
		// 当(交换链过期) || (表面属性已经不能准确匹配) || (帧缓存被改变了大小)
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized) {
			// Reset the flag that the frame buffer is resized
			// 重置帧缓存被改变了大小的标志位
			framebufferResized = false;
			// Recreate the swap chain
			// 重建交换链
			recreateSwapChain();
		} else if (result != VK_SUCCESS) {
			throw std::runtime_error("failed to present swap chain image!");
		}

		// Update the currentFrame
		// 更新 currentFrame
		currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
	}

	/* Create the shader modules */
	/* 创建着色器模块 */
	VkShaderModule createShaderModule(const std::vector<char>& code) {
		// Construct the instance that holds information about the creation of the shader module
		// 构造一个持有 着色器模块 的创建信息实例
		VkShaderModuleCreateInfo createInfo{};
		// Explicitly specify the structure type
		// 明确指定结构的类型
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		// Specify the size of the shader code
		// 指定着色器代码的大小
		createInfo.codeSize = code.size();
		// Specify the pointer of the code. The bytecode pointer is a uint32_t pointer rather than a char pointer, so a reinterpreted cast is performed
		// 指定着色器代码的地址，需要的字节码的类型是 uint32_t * 而不是 char * ，所以需要将存储字节码的数组指针的类型转换下
		createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

		// Create shader module based on the information above
		// 使用以上信息创建着色器模组
		VkShaderModule shaderModule;
		if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
			throw std::runtime_error("failed to create shader module!");
		}

		// Return the shader module created
		// 返回创建的着色器模组
		return shaderModule;
	}

	/* Choose the proper surface format of the swap chain from the available surface formats */
	/* 从可用的交换链的表面格式中，选择需要使用的表面格式 */
	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {

		// For each surface format in the available formats, check and return the one that supports the format and color space we desired
		// 对于每一个可用的表面格式，检测并返回 像素格式和色域 符合我们的需求的表面格式
		for (const auto& availableFormat : availableFormats) {
			if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
				return availableFormat;
			}
		}
		
		// If there were no surface format that could meet our desire, simply choose the first one regretly (or rate them)
		// 如果不能在列表中找到我们想要的格式，直接选第一个（也可以打分找出最合适的格式）
		return availableFormats[0];
	}

	/* Choose the presentation mode of the swap chain */
	/* 选择交换链的呈现模式 */
	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
		for (const auto& availablePresentMode : availablePresentModes) {
			// Choose triple buffering if it is available (not availble for my integrated GPU, but available for my dedicated GPU)
			// 如果交换链支持三重缓冲，则选择三重缓冲（我的核显不支持三重缓冲，但是我的独显支持)（这里可以选独显演示）
			// Triple buffering renders frames as fast as possible while still avoiding tearing, resulting in fewer latency issues than standard vertical sync
			// 三重缓冲综合表现最佳，既避免了撕裂现象，同时相较于普通垂直同步有较低的延迟
			if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
				return availablePresentMode;
			}
		}

		// VK_PRESENT_MODE_IMMEDIATE_KHR:		single buffering
		// VK_PRESENT_MODE_FIFO_KHR:			FIFO queue (double buffering), VSync
		// VK_PRESENT_MODE_FIFO_RELAXED_KHR:	FIFO queue, but the image is transferred right away when it finally arrives, if the queue was empty at the last vertical blank
		// VK_PRESENT_MODE_MAILBOX_KHR:			triple buffering

		// Only the VK_PRESENT_MODE_FIFO_KHR mode is guaranteed to be available, so if triple buffering is not available, use FIFO mode, i.e. standard VSync
		// 只有 FIFO 模式会确保被所有设备支持，没有三重缓冲就使用 FIFO 模式
		// TODO: 貌似很多驱动程序对 FIFO 支持不够好，所以应该使用立即显示模式
		return VK_PRESENT_MODE_FIFO_KHR;
	}

	/* Choose the swap extent (The resolution of images in swap chain) */
	/* 选择交换范围（即交换链中图像的分辨率） */
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
		// Some window managers do allow us to differ the resolution of the swap chain images from the resolution of the window, this is indicated by
		//		setting the width and height in currentExtent to a special value: the maximum value of uint32_t
		// 一些窗口管理器确实允许我们将交换链图像的分辨率与窗口的分辨率不同，这是通过将 currentExtent 中的宽度和高度设置为一个特殊值来表示的：uint32_t的最大值
		// If the current surface doesn't support setting customized extent, return the current extent
		// 如果当前表面不允许自定义画幅，返回表面自带的画幅
		if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
			return capabilities.currentExtent;
		} else {
			/* Set the resolution to customized resolution if allowed */
			/* 设置为当前帧缓冲的实际大小 */
			// Acquire the current extent of the frame buffer
			// 获取当前的窗口的帧缓冲的大小
			int width, height;
			glfwGetFramebufferSize(window, &width, &height);

			// Put the width and height above in a structure
			// 将窗口宽高放入一个结构体中
			VkExtent2D actualExtent = {
				static_cast<uint32_t>(width),
				static_cast<uint32_t>(height)
			};
			// Set the extent while bounding the values of width and height between the allowed minimum and maximum extents that are supported by the implementation
			// 设置自定义的画幅范围，并确保这个范围不会过大或过小
			actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
			actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

			// Return the actual extent
			// 返回实际的画幅
			return actualExtent;
		}
	}

	/* Populate the SwapChainSupportDetails struct */
	/* 填写 SwapChainSupportDetails 结构体 */
	SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device) {
		// Construct a instance to store the details of the swap chain
		// 构造一个持有交换链细节信息的实例
		SwapChainSupportDetails details;

		// Acquire the window surface available
		// 获取可用的窗口表面特性
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

		// Get the amount of the format that window surface supported by the physical device
		// 获取物理设备的窗口表面所支持的格式的数目
		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

		// If the formatCount is not 0
		// 如果 formatCount 不是 0
		if (formatCount != 0) {
			// Resize the size of the format to formatCount
			// 将 format 的大小改为 formatCount
			details.formats.resize(formatCount);
			// Get the format that window surface supported by the physical device
			// 获取物理设备的窗口表面所支持的格式
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
		}
		
		// Get the amount of the presentation mode that window surface supported by the surface of the physical device
		// 获取物理设备的所支持的表面的呈现格式的数目
		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

		// If the presentModeCount is not 0
		// 如果 presentModeCount 不是 0
		if (presentModeCount != 0) {
			// Resize the size of the presentModes to presentModeCount
			// 将 presentModes 的大小改为 presentModeCount
			details.presentModes.resize(presentModeCount);
			// Get the presention mode that window surface supported by the surface of the physical device
			// 获取物理设备的所支持的表面的呈现格式
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
		}
		// Return the populated SwapChainSupportDetails
		// 返回已经填满交换链信息的实例
		return details;
	}

	/* Check if the physical device is suitable for our desire */
	/* 检查获取的物理设备是否满足我们的需求 */
	bool isDeviceSuitable(VkPhysicalDevice device) {
		// Search for and return the queue family index we desired
		// 查找并返回需求的队列族的索引
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
			// Swap chain is adequate if the format and presentMode exist
			// 如果交换链的格式和呈现模式存在，则交换链是合格的
			swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
		}

		// Return true if (the queue index of the graphics family and the present family exist) && (the extensions are all supported) && (the swap chain is adequate)
		// 返回值为真如果（图形队列族和呈现队列族的索引存在）&&（所有的设备扩展都支持）&&（交换链的能力合格）
		return indices.isComplete() && extensionsSupported && swapChainAdequate;
	}

	/* Search for and return the queue family index we desired */
	/* 查找并返回需求的队列族的索引 */
	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device) {
		// Construct the instance that hold information of queue family index
		// 构造一个持有 队列族索引 的实例
		QueueFamilyIndices indices;
		
		// Get the amount of the queueFamily that the physical device support
		// 获取物理设备所支持的队列族的个数
		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

		// Create a container to store the properties of the queue family, with the size queueFamilyCount
		// 基于物理设备所支持的队列族的个数，创建存储队列族的容器
		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);

		// Get the queue family that the physical device support, and store them in the container
		// 获取物理设备所支持的队列族，并将他们保存在 queueFamilies 容器中
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

		// 队列族包含很多信息，如支持的操作类型、该队列族可创建的队列个数
		// Iterate through the queueFamilies to find out a queue family index we desired
		// 遍历 queueFamilies 来找到满足需求的队列族
		int i = 0;
		for (const auto& queueFamily : queueFamilies) {
			// Find the queue family index whose queue flag has the VK_QUEUE_GRAPHICS_BIT
			// 寻找队列标志位包含 VK_QUEUE_GRAPHICS_BIT 的队列族
			if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
				indices.graphicsFamily = i;
			}

			// Check if the physical device support presentation
			// 检查设备是否支持呈现
			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

			// If the presenttion support is supported, give the index value to the presentFamily field
			// 如果该队列族支持呈现功能，则将索引值赋给 presentFamily
			if (presentSupport) {
				indices.presentFamily = i;
			}

			// If a physical device satisfy the two desires above, break the loop
			// 如果物理设备满足上面的需求，跳出循环
			if (indices.isComplete()) {
				break;
			}

			// Increase the iterator
			// 增加迭代器的值
			i++;
		}

		// Return the index
		// 返回索引
		return indices;
	}

	/* Check if the device extension is supported */
	/* 检测是否支持所需的设备扩展 */
	bool checkDeviceExtensionSupport(VkPhysicalDevice device) {
		// Get the amount of the device extension supported
		// 获取可用的设备扩展的个数
		uint32_t extensionCount;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

		// Create a container to store the properties of the device extension, with the size of extensionCount
		// 基于可用的设备扩展的个数，创建存储设备扩展的容器
		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		// Get the device extension that the device support, and store them in the container
		// 获取设备所支持的所有可用的设备扩展，并将他们保存在 availableExtensions 容器中
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

		// Initialize a set of strings with the deviceExtensions as the content
		// 使用所需设备扩展的内容创建一个几何
		std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

		// For the each extensions available, earse the same name in the requiredExtension set
		// 对于每一个可用的扩展，将所需扩展容器中的同名项剔除
		for (const auto& extension : availableExtensions) {
			requiredExtensions.erase(extension.extensionName);
		}

		// If the set is empty, that means the available extensions can cover the desired extensions
		// 如果集合空了，说明可用的扩展可以覆盖我们的需求
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

	/* Check if the validation layers that we would like to use are all supported */
	/* 检查配置的校验层是否全被支持 */
	bool checkValidationLayerSupport() {
		// Initialize the counter
		// 初始化计数器
		uint32_t layerCount;
		// Get the amount of the validation layers available
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

	/* Helper function to read file content */
	/* 用于读入二进制文件内容的辅助函数 */
	static std::vector<char> readFile(const std::string& filename) {
		// Use C++ standard file input stream to read the content of the target file
		// 使用 C++ 标准文件输入流读入目标文件的内容
		// ate: Start reading at the end of the file
		// ate：从文件尾部开始读取
		// binary: Read the file as binary file (avoid text transformations)
		// binary：以二进制形式读取文件（避免文字转换）
		std::ifstream file(filename, std::ios::ate | std::ios::binary);

		// Throw a runtime error if the file can't be opened
		// 如果文件没有被成功打开，抛出一个运行时错误
		if (!file.is_open()) {
			throw std::runtime_error("failed to open file!");
		}

		// Use the read position to determine the size of the file, as we are starting to read at the end of the file
		// 使用读取位置来确定文件的大小，因为我们设置从文件末尾开始读取
		size_t fileSize = (size_t)file.tellg();
		// Create a container based on the size of the file to store the content of the file
		// 根据文件的大小创建一个容器以存储文件的内容
		std::vector<char> buffer(fileSize);

		// Jump to the start of the file
		// 跳到文件头部来读取整个文件
		file.seekg(0);
		// Really reading the content of the file. the size of the reading is fileSize, and the content will be stored in the buffer container
		// 真的开始读文件内容，大小为 fileSize，内容会被存储在 buffer 容器中
		file.read(buffer.data(), fileSize);

		// Close the file
		// 关闭文件
		file.close();

		// Return the content of the file
		// 返回读入的文件内容
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

/* The main function */
/* 主函数 */
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
