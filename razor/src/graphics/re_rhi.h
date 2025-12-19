#ifndef __RAZOR_GRAPHICS_RENDER_HARWARE_INTERFACE_HEADER_FILE
#define __RAZOR_GRAPHICS_RENDER_HARWARE_INTERFACE_HEADER_FILE

#include <re_graphics.h>

typedef void*(*re_CreateGraphicsBackendContextFn)(const re_GraphicsInstanceCreateInfo* create_info);
typedef void(*re_DestroyGraphicsBackendContextFn)(void** context);

typedef struct re_RHIVirtualTable {
    re_CreateGraphicsBackendContextFn createInternalGraphicsContext;
    re_DestroyGraphicsBackendContextFn destroyInternalGraphicsContext;
} re_RHIVirtualTable;

extern re_RHIVirtualTable RE_GRAPHICS_RHI;

#endif