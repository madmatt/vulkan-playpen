#pragma once

#include <vulkan/vulkan.h>

class Renderer
{
public:
	Renderer();
	~Renderer();

private:
	void _InitInstance();
	void _DeInitInstance();

	void _InitDevice();
	void _DeInitDevice();

	void _InitGpu();

	void _InitGpuProperties();

	void _InitGraphicsFamilyIndex();

	VkInstance					_instance				= nullptr;

	VkPhysicalDevice			_gpu					= nullptr;
	VkPhysicalDeviceProperties	_gpu_properties			{};
	VkDevice					_device					= nullptr;

	uint32_t					_graphics_family_index	= 0;
};

