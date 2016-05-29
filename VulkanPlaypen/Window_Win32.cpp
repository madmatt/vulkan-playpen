#include "BUILD_OPTIONS.h"
#include "Platform.h"
#include "Window.h"

#include <assert.h>

#if VK_USE_PLATFORM_WIN32_KHR

LRESULT CALLBACK WindowsEventHandler( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	Window* window = reinterpret_cast<Window*>( GetWindowLongPtrW( hWnd, GWLP_USERDATA ) );

	switch( uMsg ) {
		case WM_CLOSE:
			window->Close();
			return 0;

		case WM_SIZE:
			// This is called if the window size needs to be changed (e.g. by dragging the window edges)
			// We ignore this, because we'll disable window resizing for now
			break;

		default:
			break;
	}

	return DefWindowProc( hWnd, uMsg, wParam, lParam );
}

uint64_t Window::_win32_class_id_counter = 0;

void Window::_InitOSWindow()
{
	WNDCLASSEX win_class {};
	assert( _surface_size_x > 0 );
	assert( _surface_size_y > 0 );

	_win32_instance = GetModuleHandle( nullptr );
	_win32_class_name = _window_name + "_" + std::to_string( _win32_class_id_counter );
	_win32_class_id_counter++;

	// Init the window class struct
	win_class.cbSize = sizeof( WNDCLASSEX );
	win_class.style = CS_HREDRAW | CS_VREDRAW;
	win_class.lpfnWndProc = WindowsEventHandler;
	win_class.cbClsExtra = 0;
	win_class.cbWndExtra = 0;
	win_class.hInstance = _win32_instance;
	win_class.hIcon = LoadIcon( NULL, IDI_APPLICATION );
	win_class.hIconSm = LoadIcon( NULL, IDI_WINLOGO );
	win_class.hCursor = LoadCursor( NULL, IDC_ARROW );
	win_class.hbrBackground = (HBRUSH)GetStockObject( WHITE_BRUSH );
	win_class.lpszMenuName = NULL;
	win_class.lpszClassName = _win32_class_name.c_str();
	
	// Register window class
	if( !RegisterClassEx( &win_class ) ) {
		assert( 0 && "Can't create a window to draw into" );
		fflush( stdout );
		std::exit( -1 );
	}

	DWORD ex_style = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
	DWORD style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;

	// Create window with the registered class
	RECT wr = { 0, 0, LONG( _surface_size_x ), LONG( _surface_size_y ) };
	AdjustWindowRectEx( &wr, style, false, ex_style );
	_win32_window = CreateWindowEx( 0, _win32_class_name.c_str(), _window_name.c_str(), style, CW_USEDEFAULT, CW_USEDEFAULT, ( wr.right - wr.left ), ( wr.bottom - wr.top ), NULL, NULL, _win32_instance, NULL );

	if( !_win32_window ) {
		assert( 0 && "Can't CreateWindowEx()" );
		fflush( stdout );
		std::exit( -1 );
	}

	SetWindowLongPtr( _win32_window, GWLP_USERDATA, ( LONG_PTR )this );
	ShowWindow( _win32_window, SW_SHOW );
	SetForegroundWindow( _win32_window );
	SetFocus( _win32_window );
}

void Window::_DeInitOSWindow()
{
	DestroyWindow( _win32_window );
	UnregisterClass( _win32_class_name.c_str(), _win32_instance );
}

void Window::_UpdateOSWindow()
{
	MSG msg;
	if( PeekMessage( &msg, _win32_window, 0, 0, PM_REMOVE ) ) {
		TranslateMessage( &msg );
		DispatchMessage( &msg );
	}
}

#endif