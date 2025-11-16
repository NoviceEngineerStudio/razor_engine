#include <razor.h>

// *=================================================
// *
// * Numeric Comparisons
// *
// *=================================================

#define __RE_IMPLEMENT_NUMERIC_COMPARISONS__(Type, Suffix) \
Type re_min##Suffix(Type a, Type b) { return (a < b) ? a : b; } \
Type re_max##Suffix(Type a, Type b) { return (a > b) ? a : b; } \
Type re_clamp##Suffix(Type value, Type lower, Type upper) { \
    return  re_max##Suffix(lower, re_min##Suffix(upper, value)); \
}

__RE_IMPLEMENT_NUMERIC_COMPARISONS__(re_i8, I8);
__RE_IMPLEMENT_NUMERIC_COMPARISONS__(re_i16, I16);
__RE_IMPLEMENT_NUMERIC_COMPARISONS__(re_i32, I32);

__RE_IMPLEMENT_NUMERIC_COMPARISONS__(re_u8, U8);
__RE_IMPLEMENT_NUMERIC_COMPARISONS__(re_u16, U16);
__RE_IMPLEMENT_NUMERIC_COMPARISONS__(re_u32, U32);

__RE_IMPLEMENT_NUMERIC_COMPARISONS__(re_f32, F32);

__RE_IMPLEMENT_NUMERIC_COMPARISONS__(re_size, Size);

#ifdef RE_X64
    __RE_IMPLEMENT_NUMERIC_COMPARISONS__(re_i64, I64);
    __RE_IMPLEMENT_NUMERIC_COMPARISONS__(re_u64, U64);
    __RE_IMPLEMENT_NUMERIC_COMPARISONS__(re_f64, F64);
#endif

// *=================================================
// *
// * re_getStrLen
// *
// *=================================================

re_u32 re_getStrLen(const char* str) {
    if (str == NULL) {
        return 0;
    }

    re_u32 length = 0;
    while (*str++ != '\0') {
        ++length;
    }

    return length;
}

// *=================================================
// *
// * re_isStrEqual
// *
// *=================================================

re_bool re_isStrEqual(const char* str_a, const char* str_b) {
    if (str_a == str_b) {
        return re_true;
    }

    if (str_a == NULL || str_b == NULL) {
        return re_false;
    }

    while (*str_a != '\0' && *str_b != '\0') {
        if (*str_a != *str_b) {
            return re_false;
        }

        ++str_a;
        ++str_b;
    }

    return *str_a == *str_b;
}