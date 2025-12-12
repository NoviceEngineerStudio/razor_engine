#if RE_PLATFORM == RE_PLATFORM_WINDOWS

#include <re_debug.h>
#include "./re_win32.h"

HANDLE process_heap = RE_NULL_HANDLE;
HINSTANCE h_instance = RE_NULL_HANDLE;

// *=================================================
// *
// * re_coreInit
// *
// *=================================================

void re_coreInit() {
    static bool has_init = false;

    if (has_init) {
        return;
    }

    has_init = true;

    process_heap = GetProcessHeap();
    re_assert(process_heap != RE_NULL_HANDLE, "HELL");
    h_instance = GetModuleHandle(RE_NULL_HANDLE);

    WNDCLASS window_class = {0};
    window_class.hInstance = h_instance;
    window_class.lpfnWndProc = __re_processWin32WindowEvents;
    window_class.hCursor = LoadCursor(RE_NULL_HANDLE, IDC_ARROW);
    window_class.lpszClassName = __RE_WIN32_WINDOW_CLASS_NAME;
    window_class.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

    const ATOM register_result = RegisterClass(&window_class);
    re_assert(register_result, "Failed to register Win32 window class!");
}

#endif