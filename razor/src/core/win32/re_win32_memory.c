#if RE_PLATFORM == RE_PLATFORM_WINDOWS

#include <re_debug.h>
#include "./re_win32.h"

// *=================================================
// *
// * re_malloc
// *
// *=================================================

void* re_malloc(const size_t size) {
    re_assert(size > 0, "Cannot allocate 0 bytes!");

    return HeapAlloc(process_heap, 0, size);
}

// *=================================================
// *
// * re_calloc
// *
// *=================================================

void* re_calloc(const size_t element_count, const size_t element_size) {
    const size_t total_size = element_count * element_size;
    re_assert(total_size > 0, "Cannot contiguous allocate 0 bytes!");

    return HeapAlloc(process_heap, HEAP_ZERO_MEMORY, total_size);
}

// *=================================================
// *
// * re_realloc
// *
// *=================================================

void* re_realloc(void* src, const size_t new_size) {
    re_assert(src != NULL, "Cannot reallocate from NULL pointer!");
    re_assert(new_size > 0, "Cannot reallocate into 0 bytes!");

    return HeapReAlloc(process_heap, 0, src, new_size);
}

// *=================================================
// *
// * re_free
// *
// *=================================================

void re_free(void* src) {
    re_assert(src != NULL, "Cannot free NULL pointer!");

    BOOL ok = HeapFree(process_heap, 0, src);
    re_assert(ok, "Attempted to free invalid memory!");
}

// *=================================================
// *
// * re_mallocAlign
// *
// *=================================================

void* re_mallocAlign(const size_t size, const size_t alignment) {
    re_assert(size > 0, "Cannot allocate 0 bytes!");
    re_assert((alignment & (alignment - 1)) == 0, "Alignment must be power-of-two!");

    return _aligned_malloc(size, alignment);
}

// *=================================================
// *
// * re_callocAlign
// *
// *=================================================

void* re_callocAlign(const size_t element_count, const size_t element_size, const size_t alignment) {
    const size_t total_size = element_count * element_size;
    void* ptr = re_mallocAlign(total_size, alignment);

    if (ptr == RE_NULL_HANDLE) {
        return RE_NULL_HANDLE;
    }

    memset(ptr, 0, total_size);
    return ptr;
}

// *=================================================
// *
// * re_freeAlign
// *
// *=================================================

void re_freeAlign(void* src) {
    re_assert(src != NULL, "Cannot free NULL pointer!");
    _aligned_free(src);
}

#endif