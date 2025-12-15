#include <re_graphics.h>

#include <re_debug.h>
#include "./re_graphics_context.h"
#include "./re_graphics_internal_backend.h"

// *=================================================
// *
// * re_createGraphicsContext
// *
// *=================================================

re_GraphicsContext re_createGraphicsContext(const re_GraphicsContextCreateInfo* create_info) {
    re_GraphicsContext context = (re_GraphicsContext)re_calloc(1, sizeof(re_GraphicsContext_T));

    context->backend_context = RE_GRAPHICS_INTERNAL_BACKEND.createContext(create_info);

    return context;
}

// *=================================================
// *
// * re_destroyGraphicsContext
// *
// *=================================================

void re_destroyGraphicsContext(re_GraphicsContext* context) {
    re_assert(context != RE_NULL_HANDLE, "Attempting to destroy a NULL graphics context!");

    re_GraphicsContext context_data = *context;
    re_assert(context_data != RE_NULL_HANDLE, "Attempting to destroy a NULL graphics context!");

    RE_GRAPHICS_INTERNAL_BACKEND.destroyContext(&context_data->backend_context);

    re_free(context_data);
    *context = RE_NULL_HANDLE;
}