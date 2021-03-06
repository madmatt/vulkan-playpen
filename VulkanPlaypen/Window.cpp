#include <assert.h>
#include <cstdlib>

#include "Window.h"
#include "Renderer.h"
#include "RendererUtils.h"

Window::Window( Renderer* r, uint32_t size_x, uint32_t size_y, std::string name )
{
	_renderer = r;
	_surface_size_x = size_x;
	_surface_size_y = size_y;
	_window_name = name;
	
	_InitOSWindow();
	_InitSurface();
	_InitSwapchain();
}


Window::~Window()
{
	_DeInitSwapchain();
	_DeInitSurface();
	_DeInitOSWindow();
}

void Window::Close()
{
	_window_should_run = false;
}

bool Window::Update()
{
	_UpdateOSWindow();
	return _window_should_run;
}

void Window::_InitSurface()
{
	_InitOSSurface();

	VkBool32 WSI_supported = false;
	vkGetPhysicalDeviceSurfaceSupportKHR( _renderer->getPhysicalDevice(), _renderer->getGraphicsFamilyIndex(), _surface, &WSI_supported );
	if( !WSI_supported ) {
		assert( 0 && "WSI not supported on this platform" );
		std::exit( -1 );
	}

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR( _renderer->getPhysicalDevice(), _surface, &_surface_capabilities );

	if( _surface_capabilities.currentExtent.width < UINT32_MAX ) {
		_surface_size_x = _surface_capabilities.currentExtent.width;
		_surface_size_y = _surface_capabilities.currentExtent.height;
	}

	// Get surface formats that the surface supports
	{
		uint32_t surface_format_count = 0;
		vkGetPhysicalDeviceSurfaceFormatsKHR( _renderer->getPhysicalDevice(), _surface, &surface_format_count, nullptr );
		
		if( surface_format_count == 0 ) {
			assert( 0 && "Failed to get any surface formats" );
			std::exit( -1 );
		}

		std::vector<VkSurfaceFormatKHR> surface_formats( surface_format_count );
		vkGetPhysicalDeviceSurfaceFormatsKHR( _renderer->getPhysicalDevice(), _surface, &surface_format_count, surface_formats.data() );

		if( surface_formats[0].format == VK_FORMAT_UNDEFINED ) {
			_surface_format.format = VK_FORMAT_B8G8R8A8_UNORM;
			_surface_format.colorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
		} else {
			_surface_format = surface_formats[0];
		}
	}
}

void Window::_DeInitSurface()
{
	vkDestroySurfaceKHR( _renderer->getInstance(), _surface, nullptr );
}

void Window::_InitSwapchain()
{
	if( _swapchain_image_count > _surface_capabilities.maxImageCount ) {
		_swapchain_image_count = _surface_capabilities.maxImageCount;
	}

	if( _swapchain_image_count < _surface_capabilities.minImageCount + 1 ) {
		_swapchain_image_count = _surface_capabilities.minImageCount + 1;
	}

	// Pick the best presentation mode. FIFO is always available, but we want to use Mailbox if it's available on this system
	VkPresentModeKHR present_mode = VK_PRESENT_MODE_FIFO_KHR;
	{	
		uint32_t present_mode_count = 0;
		vkResultErrorCheck( vkGetPhysicalDeviceSurfacePresentModesKHR( _renderer->getPhysicalDevice(), _surface, &present_mode_count, nullptr ) );
		
		std::vector<VkPresentModeKHR> present_mode_list( present_mode_count );
		vkResultErrorCheck( vkGetPhysicalDeviceSurfacePresentModesKHR( _renderer->getPhysicalDevice(), _surface, &present_mode_count, present_mode_list.data() ) );

		for( auto m : present_mode_list ) {
			if( m == VK_PRESENT_MODE_MAILBOX_KHR ) {
				present_mode = m;
			}
		}
	}

	VkSwapchainCreateInfoKHR create_info = {};
	create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	create_info.surface = _surface;
	create_info.minImageCount = _swapchain_image_count;
	create_info.imageFormat = _surface_format.format;
	create_info.imageColorSpace = _surface_format.colorSpace;
	create_info.imageExtent.width = _surface_size_x;
	create_info.imageExtent.height = _surface_size_y;
	create_info.imageArrayLayers = 1;
	create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	create_info.queueFamilyIndexCount = 0;
	create_info.pQueueFamilyIndices = nullptr;
	create_info.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	create_info.presentMode = present_mode;
	create_info.clipped = VK_TRUE;
	create_info.oldSwapchain = VK_NULL_HANDLE;
	
	// Create the swapchain
	vkResultErrorCheck( vkCreateSwapchainKHR( _renderer->getDevice(), &create_info, nullptr, &_swapchain ) );

	// Ensure we've create a swapchain with a valid number of items (& store back to _swapchain_image_count)
	vkResultErrorCheck( vkGetSwapchainImagesKHR( _renderer->getDevice(), _swapchain, &_swapchain_image_count, nullptr ) );
}

void Window::_DeInitSwapchain()
{
	vkDestroySwapchainKHR( _renderer->getDevice(), _swapchain, nullptr );
}
