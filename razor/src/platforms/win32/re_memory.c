#include <razor.h>

#if RE_PLATFORM == RE_PLATFORM_WINDOWS

#include <Windows.h>

// *=================================================
// *
// * re_malloc
// *
// *=================================================

void* re_malloc(const size_t size) {
    re_assert(size > 0, "Cannot allocate 0 bytes!");

    return HeapAlloc(GetProcessHeap(), 0, size);
}

// *=================================================
// *
// * re_calloc
// *
// *=================================================

void* re_calloc(const size_t element_count, const size_t element_size) {
    const size_t total_size = element_count * element_size;
    re_assert(total_size > 0, "Cannot contiguous allocate 0 bytes!");

    return HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, total_size);
}

// *=================================================
// *
// * re_realloc
// *
// *=================================================

void* re_realloc(void* src, const size_t new_size) {
    re_assert(src != RE_NULL_HANDLE, "Cannot reallocate from NULL pointer!");
    re_assert(new_size > 0, "Cannot reallocate into 0 bytes!");

    return HeapReAlloc(GetProcessHeap(), 0, src, new_size);
}

// *=================================================
// *
// * re_free
// *
// *=================================================

void re_free(void* src) {
    re_assert(src != RE_NULL_HANDLE, "Cannot free NULL pointer!");

    HeapFree(GetProcessHeap(), 0, src);
}

#endif