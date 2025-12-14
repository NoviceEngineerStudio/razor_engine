#ifndef __RAZOR_CORE_WIN32_HEADER_FILE
#define __RAZOR_CORE_WIN32_HEADER_FILE

#include <re_core.h>
#include <Windows.h>

#define __RE_WIN32_WINDOW_CLASS_NAME "RE_Win32WindowClass"
#define __RE_WIN32_WINDOW_SMALL_ICON_SIZE 16u
#define __RE_WIN32_WINDOW_BIG_ICON_SIZE 32u

extern HANDLE process_heap;
extern HINSTANCE h_instance;

/// @brief Initialize the Win32 core module values.
void __re_initCoreWin32();

/// @brief Processes an internal window event.
/// @param h_window The window receiving the event.
/// @param u_message The window message tag.
/// @param w_param The event's word parameter.
/// @param l_param The event's long parameter.
/// @return The result of the event's processing.
LRESULT CALLBACK __re_processWin32WindowEvents(
    HWND h_window,
    UINT u_message,
    WPARAM w_param,
    LPARAM l_param
);

#endif