#include <razor.h>

#if RE_PLATFORM == RE_PLATFORM_WINDOWS

#include <Windows.h>

// *=================================================
// *
// * re_malloc
// *
// *=================================================

void* re_malloc(re_size size) {
    re_assert(size > 0, "Cannot allocate 0 bytes!");

    void* mem;
    mem = HeapAlloc(GetProcessHeap(), 0, size);

    re_assert(mem != NULL, "Failed to allocate memory!");
    return mem;
}

// *=================================================
// *
// * re_calloc
// *
// *=================================================

void* re_calloc(re_size element_count, re_size element_size) {
    re_size total_size = element_count * element_size;
    re_assert(total_size > 0, "Cannot contiguous allocate 0 bytes!");

    void* mem;
    mem = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, total_size);

    re_assert(mem != NULL, "Failed to contiguous allocate memory!");
    return mem;
}

// *=================================================
// *
// * re_realloc
// *
// *=================================================

void* re_realloc(void* src, re_size new_size) {
    re_assert(src != NULL, "Cannot reallocate from NULL pointer!");
    re_assert(new_size > 0, "Cannot reallocate into 0 bytes!");

    void* mem;
    mem = HeapReAlloc(GetProcessHeap(), 0, src, new_size);

    re_assert(mem != NULL, "Failed to reallocate memory!");
    return mem;
}

// *=================================================
// *
// * re_free
// *
// *=================================================

void re_free(void* src) {
    re_assert(src != NULL, "Cannot free NULL pointer!");

    BOOL result = HeapFree(GetProcessHeap(), 0, src);
    re_assert(result == TRUE, "Failed to free memory!");
}

#endif