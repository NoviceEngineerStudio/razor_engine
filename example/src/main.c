#define RE_LOGGER_ENABLED
#define RE_ASSERT_ENABLED
#include <razor.h>

static bool is_running = false;

void quit_app() {
    is_running = false;
}

int main(void) {
    re_init();

    re_WindowCreateInfo window_create_info = {0};
    window_create_info.title = "Example Project";
    window_create_info.width = 800;
    window_create_info.height = 400;
    window_create_info.flags = RE_WINDOW_RESIZEABLE;

    re_Window window = re_createWindow(&window_create_info);
    re_setWindowCloseCallback(window, quit_app);

    is_running = true;
    while (is_running) {
        re_pollEvents(window);
    }

    return 0;
}