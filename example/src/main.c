#define RE_LOGGER_ENABLED
#define RE_ASSERT_ENABLED
#include <razor.h>

static bool is_running = false;

void quit_app() {
    is_running = false;
}

int main(void) {
    re_InitParams init_params = {0};
    init_params.core.app_name = "Example Project v0.0.1";
    init_params.core.app_major_version = 0u;
    init_params.core.app_minor_version = 0u;
    init_params.core.app_patch_version = 1u;

    re_init(&init_params);

    re_WindowCreateInfo window_create_info = {0};
    window_create_info.title = init_params.core.app_name;
    window_create_info.width = 800;
    window_create_info.height = 400;
    window_create_info.flags = RE_WINDOW_RESIZEABLE;

    re_Window window = re_createWindow(&window_create_info);
    re_setWindowCloseCallback(window, quit_app);

    re_GraphicsContextCreateInfo graphics_ctx_create_info = {0};
    graphics_ctx_create_info.window = window;
    graphics_ctx_create_info.profile = RE_RENDERER_STANDARD;

    re_GraphicsContext graphics_ctx = re_createGraphicsContext(&graphics_ctx_create_info);

    is_running = true;
    while (is_running) {
        re_pollEvents(window);
    }

    re_destroyGraphicsContext(&graphics_ctx);
    re_destroyWindow(&window);

    return 0;
}