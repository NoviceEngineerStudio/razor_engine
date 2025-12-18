#ifndef __RAZOR_GRAPHICS_HEADER_FILE
#define __RAZOR_GRAPHICS_HEADER_FILE

#ifdef __cplusplus
    extern "C" {
#endif

#include "./re_core.h"

// *=================================================
// *
// * Module Initialization
// *
// *=================================================

/// @brief Initialize the graphics engine module.
RE_API void re_graphicsInit();

// *=================================================
// *
// * Backend Graphics Context
// *
// *=================================================

#define RE_MAX_RENDER_PASS_ATTACHMENTS 8
#define RE_MAX_RENDER_PASS_SUBPASSES 4
#define RE_MAX_RENDER_PASS_DEPENDENCIES 16

typedef struct re_GraphicsContext_T re_GraphicsContext_T;
typedef re_GraphicsContext_T* re_GraphicsContext;

typedef enum re_RenderProfile {
    RE_RENDERER_SIMPLE,
    RE_RENDERER_STANDARD,
    RE_RENDERER_HEAVY
} re_RenderProfile;

typedef enum re_AttachmentType {
    RE_ATTACHMENT_TYPE_COLOR,
    RE_ATTACHMENT_TYPE_DEPTH,
    RE_ATTACHMENT_TYPE_RESOLVE
} re_AttachmentType;

typedef enum re_LoadOp {
    RE_LOAD_OP_CLEAR,
    RE_LOAD_OP_LOAD,
    RE_LOAD_OP_DONT_CARE
} re_LoadOp;

typedef enum re_StoreOp {
    RE_STORE_OP_STORE,
    RE_STORE_OP_DONT_CARE
} re_StoreOp;

typedef enum re_SampleCount {
    RE_SAMPLE_COUNT_1_BIT,
    RE_SAMPLE_COUNT_2_BIT,
    RE_SAMPLE_COUNT_4_BIT,
    RE_SAMPLE_COUNT_8_BIT,
    RE_SAMPLE_COUNT_16_BIT,
    RE_SAMPLE_COUNT_32_BIT,
    RE_SAMPLE_COUNT_64_BIT
} re_SampleCount;

typedef struct re_AttachmentDescription {
    re_AttachmentType type;
    re_LoadOp load_op;
    re_StoreOp store_op;
    re_LoadOp stencil_load_op;
    re_StoreOp stencil_store_op;
    re_SampleCount samples;
    bool is_presentable;
    bool uses_stencil;
} re_AttachmentDescription;

typedef struct re_SubpassDescription {
    uint32_t color_attachment_count;
    uint32_t color_attachments[RE_MAX_RENDER_PASS_ATTACHMENTS];

    uint32_t depth_attachment;
    bool has_depth_attachment;
} re_SubpassDescription;

typedef struct re_DependencyDescription {
    // TODO: Fill this out!
} re_DependencyDescription;


typedef struct re_RenderPassDescription {
    uint32_t attachment_count;
    re_AttachmentDescription attachments[RE_MAX_RENDER_PASS_ATTACHMENTS];

    uint32_t subpass_count;
    re_SubpassDescription subpasses[RE_MAX_RENDER_PASS_SUBPASSES];

    uint32_t dependency_count;
    re_DependencyDescription dependencies[RE_MAX_RENDER_PASS_DEPENDENCIES];
} re_RenderPassDescription;

typedef struct re_GraphicsContextCreateInfo {
    re_Window window;
    re_RenderPassDescription render_pass_description;
    re_RenderProfile profile;
    bool vsync_enabled;
} re_GraphicsContextCreateInfo;

/// @brief Creates a new backend graphics context.
/// @param create_info The backend graphics context's creation parameters.
/// @return A new backend graphics context.
RE_API re_GraphicsContext re_createGraphicsContext(const re_GraphicsContextCreateInfo* create_info);

/// @brief Destroy a backend graphics context.
/// @param context The graphics context to destroy. 
RE_API void re_destroyGraphicsContext(re_GraphicsContext* context);

// *=================================================

#ifdef __cplusplus
    }
#endif

#endif