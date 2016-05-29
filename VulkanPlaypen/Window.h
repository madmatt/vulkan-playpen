#pragma once

#include <string>
#include "Renderer.h"

class Window
{

public:
	Window(Renderer* r, uint32_t size_x, uint32_t size_y, std::string name);
	~Window();

	void Close();
	bool Update();

private:
	bool _window_should_run = true;

	uint32_t _surface_size_x = 512;
	uint32_t _surface_size_y = 512;
	std::string _window_name;

	Renderer* _renderer = nullptr;

#if VK_USE_PLATFORM_WIN32_KHR
	HINSTANCE _win32_instance = NULL;
	HWND _win32_window = NULL;
	std::string _win32_class_name;
	static uint64_t _win32_class_id_counter;
#endif

	void _InitOSWindow();
	void _DeInitOSWindow();
	void _UpdateOSWindow();
	void _InitOSSurface();

};

