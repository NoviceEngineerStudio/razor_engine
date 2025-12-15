#include <re_utils.h>

bool re_isStrEqual(const char* str_a, const char* str_b) {
    if (str_a == str_b) {
        return true;
    }

    if (str_a == RE_NULL_HANDLE || str_b == RE_NULL_HANDLE) {
        return false;
    }

    while (*str_a != '\0' && *str_b != '\0') {
        if (*str_a != *str_b) {
            return false;
        }

        ++str_a;
        ++str_b;
    }

    return *str_a == *str_b;
}