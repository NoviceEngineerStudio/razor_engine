#define RE_LOGGER_ENABLED
#define RE_ASSERT_ENABLED
#include <razor.h>

static re_bool run_application = re_false;

void closeApp() {
    run_application = re_false;
}

int main(void) {
    re_logInfo("Starting example project...");

    re_WindowCreateInfo window_create_info = {0};
    window_create_info.title = "Razor Engine Example Project";
    window_create_info.width = 1280;
    window_create_info.height = 720;
    window_create_info.closeCallback = closeApp;

    re_Window window = re_createWindow(&window_create_info);

    re_GraphicsBackendCreateInfo graphics_backend_create_info = {0};
    graphics_backend_create_info.window = window;

    re_GraphicsBackend graphics_backend = re_createGraphicsBackend(&graphics_backend_create_info);

    run_application = re_true;
    while (run_application) {
        re_pollEvents(window);
    }

    graphics_backend->destroy(&graphics_backend);
    re_destroyWindow(&window);

    re_logSuccess("Example project has successfully exited!");
    return 0;
}