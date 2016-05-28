#include "Renderer.h"

#include <assert.h>
#include <cstdlib>
#include <vector>


Renderer::Renderer()
{
	_InitInstance();
	_InitDevice();
}


Renderer::~Renderer()
{
	_DeInitDevice();
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
