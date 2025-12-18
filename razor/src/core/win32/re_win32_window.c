#if RE_PLATFORM == RE_PLATFORM_WINDOWS

#include <re_debug.h>
#include "./re_win32.h"

typedef struct re_Window_T {
    HWND h_window;

    HICON h_small_icon;
    HICON h_big_icon;

    DWORD prev_style;
    WINDOWPLACEMENT prev_place;

    re_WindowCloseCallback onClose;

    uint32_t width;
    uint32_t height;

    re_WindowFlag flags;
} re_Window_T;

#ifdef RE_VULKAN_AVAILABLE

#include "../re_vulkan_window.h"
#include <vulkan/vulkan_win32.h>

#define __RE_VULKAN_WINDOW_EXTENSION_COUNT 2u
static const char* __RE_VULKAN_WINDOW_EXTENSIONS[__RE_VULKAN_WINDOW_EXTENSION_COUNT] = {
    VK_KHR_SURFACE_EXTENSION_NAME,
    VK_KHR_WIN32_SURFACE_EXTENSION_NAME
};

#endif

// *=================================================
// *
// * __re_processWindowEvents
// *
// *=================================================

LRESULT CALLBACK __re_processWin32WindowEvents(
    HWND h_window,
    UINT u_message,
    WPARAM w_param,
    LPARAM l_param
) {
    switch (u_message) {
        case WM_DESTROY: {
            re_Window window = (re_Window)GetWindowLongPtr(h_window, GWLP_USERDATA);

            if (window->onClose != RE_NULL_HANDLE) {
                window->onClose();
            }

            PostQuitMessage(0);
            return 0;
        }

        case WM_NCCREATE: {
            CREATESTRUCT* create_info = (CREATESTRUCT*)(l_param);
            re_Window window = (re_Window)(create_info->lpCreateParams);

            SetWindowLongPtr(h_window, GWLP_USERDATA, (LONG_PTR)(window));

            break;
        }

        case WM_PAINT: {
            PAINTSTRUCT paint = {0};

            HDC hdc = BeginPaint(h_window, &paint);
            FillRect(hdc, &paint.rcPaint, (HBRUSH)(COLOR_WINDOW + 1));
            EndPaint(h_window, &paint);

            return 0;
        }

        case WM_SIZE: {
            re_Window window = (re_Window)GetWindowLongPtr(h_window, GWLP_USERDATA);
            window->width = (uint32_t)(LOWORD(l_param));
            window->height = (uint32_t)(HIWORD(l_param));

            return 0;
        }
    }

    return DefWindowProc(h_window, u_message, w_param, l_param);
}

#ifdef RE_VULKAN_AVAILABLE

// *=================================================
// *
// * __re_getVulkanWindowExtensions
// *
// *=================================================

const char** __re_getVulkanWindowExtensions(uint32_t* extension_count) {
    *extension_count = __RE_VULKAN_WINDOW_EXTENSION_COUNT;
    return __RE_VULKAN_WINDOW_EXTENSIONS;
}

// *=================================================
// *
// * __re_createVulkanSurface
// *
// *=================================================

VkSurfaceKHR __re_createVulkanSurface(
    const re_Window window,
    const VkInstance instance,
    const VkAllocationCallbacks* allocator
) {
    VkWin32SurfaceCreateInfoKHR surface_create_info = {0};
    surface_create_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    surface_create_info.hinstance = h_instance;
    surface_create_info.hwnd = window->h_window;

    VkSurfaceKHR surface = VK_NULL_HANDLE;
    const VkResult surface_create_result = vkCreateWin32SurfaceKHR(
        instance,
        &surface_create_info,
        allocator,
        &surface
    );

    re_assert(surface_create_result == VK_SUCCESS, "Failed to create Vulkan Win32 window surface!");

    return surface;
}

// *=================================================
// *
// * __re_getVulkanExtent
// *
// *=================================================

VkExtent2D __re_getVulkanExtent(const re_Window window) {
    VkExtent2D extent;

    extent.width = window->width;
    extent.height = window->height;

    return extent;
}

#endif

// *=================================================
// *
// * re_createWindow
// *
// *=================================================

re_Window re_createWindow(const re_WindowCreateInfo* create_info) {
    re_assert(create_info != RE_NULL_HANDLE, "Attempting to create window with NULL create info!");

    const re_WindowFlag flags = create_info->flags;
    const bool is_fullscreen  = (flags & RE_WINDOW_FULLSCREEN) != 0;
    const bool is_resizeable  = (flags & RE_WINDOW_RESIZEABLE) != 0;
    const bool is_borderless  = (flags & RE_WINDOW_BORDERLESS) != 0;
    const bool is_hidden      = (flags & RE_WINDOW_HIDDEN) != 0;
    const bool is_topmost     = (flags & RE_WINDOW_TOPMOST) != 0;
    const bool is_no_maximize = (flags & RE_WINDOW_NO_MAXIMIZE) != 0;
    const bool is_no_minimize = (flags & RE_WINDOW_NO_MINIMIZE) != 0;

    re_Window window = (re_Window)re_calloc(1, sizeof(re_Window_T));

    DWORD style = 0;
    DWORD ex_style = 0;

    if (is_borderless) {
        style |= WS_POPUP;
    }
    else {
        style = WS_OVERLAPPEDWINDOW;

        if (!is_resizeable) {
            style &= ~(WS_THICKFRAME | WS_MAXIMIZEBOX);
        }

        if (is_no_minimize) {
            style &= ~WS_MINIMIZEBOX;
        }

        if (is_no_maximize) {
            style &= ~WS_MAXIMIZEBOX;
        }
    }

    if (is_topmost) {
        ex_style |= WS_EX_TOPMOST;
    }

    HWND h_window = CreateWindowEx(
        ex_style,
        __RE_WIN32_WINDOW_CLASS_NAME,
        create_info->title,
        style,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        (LONG)(create_info->width),
        (LONG)(create_info->height),
        RE_NULL_HANDLE,
        RE_NULL_HANDLE,
        h_instance,
        (void*)(window)
    );

    re_assert(h_window != RE_NULL_HANDLE, "Failed to create Win32 window handle!");
    window->h_window = h_window;

    if (create_info->icon_path != RE_NULL_HANDLE) {
        window->h_small_icon = (HICON)LoadImage(
            RE_NULL_HANDLE,
            create_info->icon_path,
            IMAGE_ICON,
            __RE_WIN32_WINDOW_SMALL_ICON_SIZE,
            __RE_WIN32_WINDOW_SMALL_ICON_SIZE,
            LR_LOADFROMFILE
        );

        window->h_big_icon = (HICON)LoadImage(
            RE_NULL_HANDLE,
            create_info->icon_path,
            IMAGE_ICON,
            __RE_WIN32_WINDOW_BIG_ICON_SIZE,
            __RE_WIN32_WINDOW_BIG_ICON_SIZE,
            LR_LOADFROMFILE
        );

        if (window->h_small_icon == RE_NULL_HANDLE) {
            re_logError("Failed to set Win32 window's small icon: %s", create_info->icon_path);
        }
        else {
            SendMessage(h_window, WM_SETICON, ICON_SMALL, (LPARAM)window->h_small_icon);
        }

        if (window->h_big_icon == RE_NULL_HANDLE) {
            re_logError("Failed to set Win32 window's big icon: %s", create_info->icon_path);
        }
        else {
            SendMessage(h_window, WM_SETICON, ICON_BIG, (LPARAM)window->h_big_icon);
        }
    }

    if (!is_hidden) {
        ShowWindow(h_window, SW_SHOW);
    }

    if (is_fullscreen) {
        re_setWindowFullscreen(window, true);
    }

    window->width = create_info->width;
    window->height = create_info->height;
    window->flags = flags;

    return window;
}

// *=================================================
// *
// * re_destroyWindow
// *
// *=================================================

void re_destroyWindow(re_Window* window) {
    re_assert(window != RE_NULL_HANDLE, "Attempting to destroy NULL window!");

    re_Window window_data = *window;
    re_assert(window_data != RE_NULL_HANDLE, "Attempting to destroy NULL window!");

    if (window_data->h_small_icon != RE_NULL_HANDLE) {
        DestroyIcon(window_data->h_small_icon);
    }

    if (window_data->h_big_icon != RE_NULL_HANDLE) {
        DestroyIcon(window_data->h_big_icon);
    }

    DestroyWindow(window_data->h_window);

    re_free(window_data);
    *window = RE_NULL_HANDLE;
}

// *=================================================
// *
// * re_getWindowSize
// *
// *=================================================

void re_getWindowSize(const re_Window window, uint32_t* width, uint32_t* height) {
    re_assert(window != RE_NULL_HANDLE, "Attempting to set fullscreen mode of NULL window!");

    *width = window->width;
    *height = window->height;
}

// *=================================================
// *
// * re_isWindowFullscreen
// *
// *=================================================

bool re_isWindowFullscreen(const re_Window window) {
    re_assert(window != RE_NULL_HANDLE, "Attempting to set fullscreen mode of NULL window!");

    return (window->flags & RE_WINDOW_FULLSCREEN) != 0;
}

// *=================================================
// *
// * re_setWindowFullscreen
// *
// *=================================================

void re_setWindowFullscreen(re_Window window, const bool fullscreen_enabled) {
    re_assert(window != RE_NULL_HANDLE, "Attempting to set fullscreen mode of NULL window!");

    if (re_isWindowFullscreen(window) == fullscreen_enabled) {
        return;
    }

    window->flags ^= RE_WINDOW_FULLSCREEN;
    
    HWND h_window = window->h_window;

    if (fullscreen_enabled) {
        DWORD style = GetWindowLong(h_window, GWL_STYLE);
        window->prev_style = style;

        GetWindowPlacement(h_window, &window->prev_place);

        SetWindowLong(h_window, GWL_STYLE, style & ~(WS_OVERLAPPEDWINDOW));

        HMONITOR h_monitor = MonitorFromWindow(h_window, MONITOR_DEFAULTTOPRIMARY);
        MONITORINFO monitor_info = { sizeof(MONITORINFO) };

        if (GetMonitorInfo(h_monitor, &monitor_info)) {
            RECT monitor_rect = monitor_info.rcMonitor;

            SetWindowPos(
                h_window,
                HWND_TOP,
                monitor_rect.left,
                monitor_rect.top,
                monitor_rect.right - monitor_rect.left,
                monitor_rect.bottom - monitor_rect.top,
                SWP_NOOWNERZORDER | SWP_FRAMECHANGED
            );
        }
    }
    else {
        SetWindowLong(h_window, GWL_STYLE, window->prev_style);
        SetWindowPlacement(h_window, &window->prev_place);
        SetWindowPos(h_window, RE_NULL_HANDLE, 0, 0, 0, 0, (
            SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER |
            SWP_NOOWNERZORDER | SWP_FRAMECHANGED
        ));
    }
}

// *=================================================
// *
// * re_setWindowCloseCallback
// *
// *=================================================

void re_setWindowCloseCallback(re_Window window, const re_WindowCloseCallback callback) {
    re_assert(window != RE_NULL_HANDLE, "Attempting to set fullscreen mode of NULL window!");

    window->onClose = callback;
}

// *=================================================
// *
// * re_pollEvents
// *
// *=================================================

void re_pollEvents(re_Window window) {
    re_assert(window != RE_NULL_HANDLE, "Attempting to set fullscreen mode of NULL window!");

    MSG message;
    while (PeekMessage(&message, RE_NULL_HANDLE, 0, 0, PM_REMOVE)) {
        TranslateMessage(&message);
        DispatchMessage(&message);
    }
}

#endif