#include <razor.h>

#if RE_PLATFORM == RE_PLATFORM_WINDOWS

#include <Windows.h>

typedef struct re_Window_T {
    HWND h_window;
    HINSTANCE h_instance;

    HICON h_small_icon;
    HICON h_large_icon;

    void(*closeCallback)();

    re_u32 width;
    re_u32 height;
    re_bool is_fullscreen;
} re_Window_T;

#pragma region Private Window Methods

// *=================================================
// *
// * Window Event Processing
// *
// *=================================================

static LRESULT CALLBACK re_processWindowEvents(
    HWND h_window,
    UINT u_message,
    WPARAM w_param,
    LPARAM l_param
) {
    switch (u_message) {
        case WM_DESTROY: {
            re_Window window = (re_Window)GetWindowLongPtr(h_window, GWLP_USERDATA);

            if (window->closeCallback != NULL) {
                window->closeCallback();
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
            window->width = (re_u32)(LOWORD(l_param));
            window->height = (re_u32)(HIWORD(l_param));

            return 0;
        }
    }

    return DefWindowProc(h_window, u_message, w_param, l_param);
}

#pragma endregion
#pragma region Public Window Methods

// *=================================================
// *
// * re_createWindow
// *
// *=================================================

#define __RE_WINDOW_CLASS_NAME__ "RE_WindowClass"
#define __RE_WINDOW_SMALL_ICON_SIZE__ 16u
#define __RE_WINDOW_LARGE_ICON_SIZE__ 32u

re_Window re_createWindow(const re_WindowCreateInfo* create_info) {
    re_assert(create_info != NULL, "Attempting to create window with NULL create info!");

    re_Window window = (re_Window)re_malloc(sizeof(re_Window_T));
    
    HINSTANCE h_instance = GetModuleHandle(NULL);
    window->h_instance = h_instance;

    static re_bool class_registered = re_false;
    if (!class_registered) {
        WNDCLASS window_class = {0};

        window_class.hInstance = h_instance;
        window_class.lpfnWndProc = re_processWindowEvents;
        window_class.hCursor = LoadCursor(NULL, IDC_ARROW);
        window_class.lpszClassName = __RE_WINDOW_CLASS_NAME__;
        window_class.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

        ATOM register_result = RegisterClass(&window_class);
        re_assert(register_result, "Failed to register Win32 window class!");

        class_registered = re_true;
    }

    DWORD style = WS_OVERLAPPEDWINDOW;
    DWORD ex_style = 0;

    HWND h_window = CreateWindowExA(
        ex_style,
        __RE_WINDOW_CLASS_NAME__,
        create_info->title,
        style,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        (LONG)(create_info->width),
        (LONG)(create_info->height),
        NULL,
        NULL,
        h_instance,
        (void*)(window)
    );

    re_assert(h_window != NULL, "Failed to create Win32 window handle!");
    window->h_window = h_window;

    if (create_info->icon_path != NULL) {
        window->h_small_icon = (HICON)(LoadImageA(
            NULL,
            create_info->icon_path,
            IMAGE_ICON,
            __RE_WINDOW_SMALL_ICON_SIZE__,
            __RE_WINDOW_SMALL_ICON_SIZE__,
            LR_LOADFROMFILE
        ));

        if (window->h_small_icon == NULL) {
            re_logError("Failed to set Win32 window's small icon: %s", create_info->icon_path);
        }
        else {
            SendMessage(h_window, WM_SETICON, ICON_SMALL, (LPARAM)window->h_small_icon);
        }

        window->h_large_icon = (HICON)(LoadImageA(
            NULL,
            create_info->icon_path,
            IMAGE_ICON,
            __RE_WINDOW_LARGE_ICON_SIZE__,
            __RE_WINDOW_LARGE_ICON_SIZE__,
            LR_LOADFROMFILE
        ));

        if (window->h_large_icon == NULL) {
            re_logError("Failed to set Win32 window's large icon: %s", create_info->icon_path);
        }
        else {
            SendMessage(h_window, WM_SETICON, ICON_BIG, (LPARAM)window->h_large_icon);
        }
    }

    ShowWindow(h_window, SW_SHOW);

    window->width = create_info->width;
    window->height = create_info->height;
    window->closeCallback = create_info->closeCallback;

    window->is_fullscreen = re_false;
    if (create_info->fullscreen_enabled) {
        re_setWindowFullscreen(window, re_true);
        
    }

    return window;
}

// *=================================================
// *
// * re_destroyWindow
// *
// *=================================================

void re_destroyWindow(re_Window* window) {
    re_assert(window != NULL, "Attempting to destroy NULL window!");

    re_Window window_data = *window;
    re_assert(window_data != NULL, "Attempting to destroy NULL window!");

    DestroyIcon(window_data->h_small_icon);
    DestroyIcon(window_data->h_large_icon);

    DestroyWindow(window_data->h_window);

    re_free(window_data);
    *window = NULL;
}

// *=================================================
// *
// * re_getWindowSize
// *
// *=================================================

void re_getWindowSize(const re_Window window, re_u32* width, re_u32* height) {
    re_assert(window != NULL, "Attempting to get window size of NULL window!");
    re_assert(width != NULL && height != NULL, "Attempting to assign window size to NULL pointers!");

    *width = window->width;
    *height = window->height;
}

// *=================================================
// *
// * re_isWindowFullscreen
// *
// *=================================================

re_bool re_isWindowFullscreen(const re_Window window) {
    re_assert(window != NULL, "Attempting to check window fullscreen mode of NULL window!");

    return window->is_fullscreen;
}

// *=================================================
// *
// * re_setWindowFullscreen
// *
// *=================================================

void re_setWindowFullscreen(re_Window window, const re_bool fullscreen_enabled) {
    re_assert(window != NULL, "Attempting to set fullscreen mode of NULL window!");

    if (window->is_fullscreen == fullscreen_enabled) {
        return;
    }

    window->is_fullscreen = fullscreen_enabled;

    static WINDOWPLACEMENT prev_place = { sizeof(WINDOWPLACEMENT) };

    HWND h_window = window->h_window;
    DWORD style = GetWindowLong(h_window, GWL_STYLE);

    if (fullscreen_enabled) {
        GetWindowPlacement(h_window, &prev_place);
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
        SetWindowLong(h_window, GWL_STYLE, style | WS_OVERLAPPEDWINDOW);
        SetWindowPlacement(h_window, &prev_place);
        SetWindowPos(h_window, NULL, 0, 0, 0, 0, (
            SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER |
            SWP_NOOWNERZORDER | SWP_FRAMECHANGED
        ));
    }
}

// *=================================================
// *
// * re_pollEvents
// *
// *=================================================

void re_pollEvents(re_Window window) {
    MSG message;
    while (PeekMessage(&message, NULL, 0, 0, PM_REMOVE)) {
        TranslateMessage(&message);
        DispatchMessage(&message);
    }
}

#pragma endregion
#pragma region Vulkan Window Methods

#include "./re_vulkan_window.h"
#include <vulkan/vulkan_win32.h>

// *=================================================
// *
// * re_getVulkanWindowExtensions
// *
// *=================================================

#define __RE_VULKAN_WINDOW_EXTENSION_COUNT__ 2u

static const char* __RE_VULKAN_WINDOW_EXTENSIONS__[__RE_VULKAN_WINDOW_EXTENSION_COUNT__] = {
    VK_KHR_SURFACE_EXTENSION_NAME,
    VK_KHR_WIN32_SURFACE_EXTENSION_NAME
};

const char** re_getVulkanWindowExtensions(re_u32* extension_count) {
    *extension_count = __RE_VULKAN_WINDOW_EXTENSION_COUNT__;
    return __RE_VULKAN_WINDOW_EXTENSIONS__;
}

// *=================================================
// *
// * re_createVulkanSurface
// *
// *=================================================

VkSurfaceKHR re_createVulkanSurface(
    const re_Window window,
    const VkInstance instance,
    const VkAllocationCallbacks* allocator
) {
    VkWin32SurfaceCreateInfoKHR surface_create_info = {0};
    surface_create_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    surface_create_info.hinstance = window->h_instance;
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
// * re_getVulkanExtent
// *
// *=================================================

VkExtent2D re_getVulkanExtent(const re_Window window) {
    VkExtent2D extent;

    extent.width = window->width;
    extent.height = window->height;

    return extent;
}

#pragma endregion

#endif