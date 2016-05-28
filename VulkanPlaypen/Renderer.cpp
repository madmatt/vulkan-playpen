#include "Renderer.h"

#include <assert.h>
#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <vector>

#ifdef _WIN32
#include <Windows.h>
#endif

Renderer::Renderer()
{
	_SetupDebug();
	_InitInstance();
	_InitDebug();
	_InitDevice();
}


Renderer::~Renderer()
{
	_DeInitDevice();
	_DeInitDebug();
	_DeInitInstance();
}

void Renderer::_InitInstance()
{
	VkApplicationInfo app_info {};
	app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	app_info.apiVersion = VK_MAKE_VERSION( 1, 0, 3 );
	app_info.applicationVersion = VK_MAKE_VERSION( 0, 1, 0 );
	app_info.pApplicationName = "Vulkan Playpen";

	VkInstanceCreateInfo instance_info {};
	instance_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instance_info.pApplicationInfo = &app_info;
	instance_info.enabledLayerCount = _instance_layers.size();
	instance_info.ppEnabledLayerNames = _instance_layers.data();
	instance_info.enabledExtensionCount = _instance_extensions.size();
	instance_info.ppEnabledExtensionNames = _instance_extensions.data();
	instance_info.pNext = &debug_callback_create_info;

	auto err = vkCreateInstance( &instance_info, nullptr, &_instance );

	if( err != VK_SUCCESS ) {
		assert( 1 && "vkCreateInstance failed" );
		std::exit( -1 );
	}
}

void Renderer::_DeInitInstance()
{
	vkDestroyInstance( _instance, nullptr );
	_instance = nullptr;
}

void Renderer::_InitDevice()
{
	_InitPhysicalDevice();
	_InitGraphicsFamilyIndex();
	_ListValidationLayers();


	float queue_priorities[] { 1.0f };

	VkDeviceQueueCreateInfo device_queue_create_info {};
	device_queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	device_queue_create_info.queueFamilyIndex = _graphics_family_index;
	device_queue_create_info.queueCount = 1;
	device_queue_create_info.pQueuePriorities = queue_priorities;

	VkDeviceCreateInfo device_info {};
	device_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	device_info.queueCreateInfoCount = 1;
	device_info.pQueueCreateInfos = &device_queue_create_info;
	device_info.enabledLayerCount = _device_layers.size();
	device_info.ppEnabledLayerNames = _device_layers.data();
	device_info.enabledExtensionCount = _device_extensions.size();
	device_info.ppEnabledExtensionNames = _device_extensions.data();

	auto err = vkCreateDevice( _gpu, &device_info, nullptr, &_device );

	if( err != VK_SUCCESS ) {
		assert( 1 && "vkCreateDevice failed" );
		std::exit( -1 );
	}


}

void Renderer::_DeInitDevice()
{
	vkDestroyDevice( _device, nullptr );
	_device = nullptr;
}

void Renderer::_InitPhysicalDevice()
{
	uint32_t gpu_count = 0;
	vkEnumeratePhysicalDevices( _instance, &gpu_count, nullptr );

	std::vector<VkPhysicalDevice> gpu_list( gpu_count );
	vkEnumeratePhysicalDevices( _instance, &gpu_count, gpu_list.data() );

	_gpu = gpu_list[0];
	_InitGpuProperties();
}

void Renderer::_InitGpuProperties()
{
	vkGetPhysicalDeviceProperties( _gpu, &_gpu_properties );
}

void Renderer::_InitGraphicsFamilyIndex()
{
	uint32_t family_count = 0;
	vkGetPhysicalDeviceQueueFamilyProperties( _gpu, &family_count, nullptr );

	std::vector<VkQueueFamilyProperties> family_property_list( family_count );
	vkGetPhysicalDeviceQueueFamilyProperties( _gpu, &family_count, family_property_list.data() );

	bool found = false;
	for( uint32_t i = 0; i < family_count; i++ ) {
		if( family_property_list[i].queueFlags & VK_QUEUE_GRAPHICS_BIT ) {
			found = true;
			_graphics_family_index = i;
		}
	}

	if( !found ) {
		assert( 1 && "vkGetPhysicalDeviceQueueFamilyProperties found no queue family with VK_QUEUE_GRAPHICS_BIT set" );
		std::exit( -1 );
	}
}

VKAPI_ATTR VkBool32 VKAPI_CALL
VulkanDebugCallback( VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT object_type, uint64_t src_obj, size_t location, int32_t msg_code, const char* layer_prefix, const char* msg, void* user_data )
{
	char* type = "";

	if( flags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT ) {
		type = "INFO";
	}
	else if( flags & VK_DEBUG_REPORT_WARNING_BIT_EXT ) {
		type = "WARN";
	}
	else if( flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT ) {
		type = "PERF";
	}
	else if( flags & VK_DEBUG_REPORT_ERROR_BIT_EXT ) {
		type = "ERR";
	} else if (flags & VK_DEBUG_REPORT_DEBUG_BIT_EXT ) {
		type = "DEBUG";
	}
	else {
		type = "????";
	}

	std::cout << "[" << std::left << std::setw(6) << type << " @ " << layer_prefix << "] " << msg << std::endl;

#ifdef _WIN32
	if( flags & VK_DEBUG_REPORT_ERROR_BIT_EXT ) {
		char buff[1024];
		sprintf_s( buff, "[%s @ %s] %s", type, layer_prefix, msg );

		MessageBox( NULL, buff, "Vulkan fatal", 0 );
	}
#endif

	return false;
}

void Renderer::_SetupDebug()
{
	debug_callback_create_info.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
	debug_callback_create_info.pfnCallback = VulkanDebugCallback;
	debug_callback_create_info.flags = VK_DEBUG_REPORT_INFORMATION_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT | VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_DEBUG_BIT_EXT | VK_DEBUG_REPORT_FLAG_BITS_MAX_ENUM_EXT | 0;

	_instance_layers.push_back( "VK_LAYER_LUNARG_image" );
	_instance_layers.push_back( "VK_LAYER_LUNARG_object_tracker" );

	_instance_extensions.push_back( VK_EXT_DEBUG_REPORT_EXTENSION_NAME );

	_device_layers.push_back( "VK_LAYER_LUNARG_image" );
	_device_layers.push_back( "VK_LAYER_LUNARG_object_tracker" );
}

void Renderer::_InitDebug()
{
	fvkCreateDebugReportCallbackEXT = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr( _instance, "vkCreateDebugReportCallbackEXT" );
	fvkDestroyDebugReportCallbackEXT = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr( _instance, "vkDestroyDebugReportCallbackEXT" );

	if( fvkCreateDebugReportCallbackEXT == nullptr || fvkDestroyDebugReportCallbackEXT == nullptr ) {
		assert( 1 && "fvkCreateDebugReportCallbackEXT or fvkDestroyDebugReportCallbackEXT not set correctly" );
		std::exit( -1 );
	}

	
	

	fvkCreateDebugReportCallbackEXT( _instance, &debug_callback_create_info, nullptr, &_debug_report );
}

void Renderer::_DeInitDebug()
{
	fvkDestroyDebugReportCallbackEXT( _instance, _debug_report, nullptr );
	_debug_report = 0;
}

void Renderer::_ListValidationLayers()
{
	// Instance Layer debug list
	{
		uint32_t layer_count = 0;
		vkEnumerateInstanceLayerProperties( &layer_count, nullptr );
		std::vector<VkLayerProperties> layer_property_list( layer_count );
		vkEnumerateInstanceLayerProperties( &layer_count, layer_property_list.data() );

		std::cout << "Instance Layers: " << layer_property_list.size() << " item(s) \n";

		for( auto &i : layer_property_list ) {
			std::cout << " " << std::left << std::setw( 40 ) << i.layerName << " | " << std::setw( 100 ) << i.description << std::endl;
		}

		std::cout << std::endl << std::endl;
	}

	// Device layer debug list
	{
		uint32_t layer_count = 0;
		vkEnumerateDeviceLayerProperties( _gpu, &layer_count, nullptr );
		std::vector<VkLayerProperties> layer_property_list( layer_count );
		vkEnumerateDeviceLayerProperties( _gpu, &layer_count, layer_property_list.data() );

		std::cout << "Device Layers: " << layer_property_list.size() << " item(s) \n";

		for( auto &i : layer_property_list ) {
			std::cout << " " << std::left << std::setw( 40 ) << i.layerName << " | " << std::setw( 100 ) << i.description << std::endl;
		}

		std::cout << std::endl << std::endl;
	}
}