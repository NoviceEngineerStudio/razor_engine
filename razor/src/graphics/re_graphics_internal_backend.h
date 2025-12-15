#ifndef __RAZOR_GRAPHICS_INTERNAL_BACKEND_HEADER_FILE
#define __RAZOR_GRAPHICS_INTERNAL_BACKEND_HEADER_FILE

#include <re_graphics.h>

typedef void*(*re_CreateInternalGraphicsContext)(const re_GraphicsContextCreateInfo* create_info);
typedef void(*re_DestroyInternalGraphicsContext)(void** context);

typedef struct re_GraphicsInternalBackend {
    re_CreateInternalGraphicsContext createContext;
    re_DestroyInternalGraphicsContext destroyContext;
} re_GraphicsInternalBackend;

extern re_GraphicsInternalBackend RE_GRAPHICS_INTERNAL_BACKEND;

#endif