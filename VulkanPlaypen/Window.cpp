#include <assert.h>
#include <cstdlib>

#include "Window.h"
#include "Renderer.h"

Window::Window( Renderer* r, uint32_t size_x, uint32_t size_y, std::string name )
{
	_renderer = r;
	_surface_size_x = size_x;
	_surface_size_y = size_y;
	_window_name = name;
	
	_InitOSWindow();
	_InitSurface();
}


Window::~Window()
{
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