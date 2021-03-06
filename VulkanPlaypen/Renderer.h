#pragma once

#include "Platform.h"
#include "Window.h"

#include <cstdlib>
#include <vector>

class Renderer
{
public:
	Renderer();
	~Renderer();

	Window* OpenWindow( uint32_t size_x, uint32_t size_y, std::string windowName );

	bool Run();

	const VkInstance getInstance() const;
	const VkPhysicalDevice getPhysicalDevice() const;
	const VkDevice getDevice() const;
	const VkQueue getQueue() const;
	const uint32_t getGraphicsFamilyIndex() const;
	const VkPhysicalDeviceProperties& getPhysicalDeviceProperties() const;

private:
	void _SetupLayersAndExtensions();
	
	void _InitInstance();
	void _DeInitInstance();

	void _InitDevice();
	void _DeInitDevice();

	void _InitQueue();

	void _InitPhysicalDevice();

	void _InitGpuProperties();

	void _InitGraphicsFamilyIndex();

	void _SetupDebug();
	void _InitDebug();
	void _DeInitDebug();

	void _ListValidationLayers();

	VkInstance _instance = VK_NULL_HANDLE;

	VkPhysicalDevice _gpu = VK_NULL_HANDLE;
	VkPhysicalDeviceProperties _gpu_properties {};

	VkDevice _device = VK_NULL_HANDLE;

	VkQueue _queue = VK_NULL_HANDLE;

	uint32_t _graphics_family_index = 0;

	Window* _window = nullptr;

	std::vector<const char*> _instance_layers;
	std::vector<const char*> _instance_extensions;

	std::vector<const char*> _device_layers;
	std::vector<const char*> _device_extensions;

	PFN_vkCreateDebugReportCallbackEXT fvkCreateDebugReportCallbackEXT = nullptr;
	PFN_vkDestroyDebugReportCallbackEXT fvkDestroyDebugReportCallbackEXT = nullptr;

	VkDebugReportCallbackEXT _debug_report = VK_NULL_HANDLE;

	VkDebugReportCallbackCreateInfoEXT debug_callback_create_info = {};
};

VKAPI_ATTR VkBool32 VKAPI_CALL VulkanDebugCallback( VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT object_type, uint64_t src_obj, size_t location, int32_t msg_code, const char * layer_prefix, const char * msg, void * user_data );
