#pragma once

#include "mem.h"
#include "hash.h"

#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>
#include <assert.h>

/// @file
///
/// Strings and string views. Strings are manually allocated and freed, while string views represent
/// lightweight references to a string in memory.

/// Constructs a string view from the given C string.
#define STR_VIEW(x) ((struct str_view) { .data = (x), .length = strlen((x)) })

/// View into a character string. May or may not be zero-terminated.
struct str_view {
    const char* data;
    size_t length;
};

/// Dynamically-allocated string. May or may not be zero-terminated.
struct str {
    char* data;
    size_t length;
    size_t capacity;
};

[[nodiscard]] static inline bool str_view_is_equal(const struct str_view* str_view, const struct str_view* other) {
    return
        str_view->length == other->length &&
        !memcmp(str_view->data, other->data, str_view->length);
}

[[nodiscard]] static inline uint32_t str_view_hash(uint32_t h, const struct str_view* str_view) {
    for (size_t i = 0; i < str_view->length; ++i)
        h = hash_uint8(h, str_view->data[i]);
    return h;
}

[[nodiscard]] static inline struct str str_create(void) {
    return (struct str) {};
}

/// Shrinks a string view by the given number of characters on the left and on the right.
[[nodiscard]] static inline struct str_view str_view_shrink(struct str_view str_view, size_t left, size_t right) {
    assert(str_view.length >= left + right);
    return (struct str_view) { .data = str_view.data + left, .length = str_view.length - left - right };
}

[[nodiscard]] static inline struct str str_copy(struct str_view view) {
    char* copy = xmalloc(view.length);
    xmemcpy(copy, view.data, view.length);
    return (struct str) {
        .data = copy,
        .length = view.length,
        .capacity = view.length
    };
}

[[nodiscard]] static inline struct str_view str_to_view(const struct str* str) {
    return (struct str_view) {
        .data = str->data,
        .length = str->length
    };
}

static inline void str_grow(struct str* str, size_t added_bytes) {
    if (str->length + added_bytes > str->capacity) {
        str->capacity += str->capacity >> 1;
        if (str->length + added_bytes > str->capacity)
            str->capacity = str->length + added_bytes;
        str->data = xrealloc(str->data, str->capacity);
    }
}

static inline void str_push(struct str* str, char c) {
    str_grow(str, 1);
    str->data[str->length++] = c;
}

static inline void str_append(struct str* str, struct str_view view) {
    str_grow(str, view.length);
    xmemcpy(str->data + str->length, view.data, view.length);
    str->length += view.length;
}

static inline void str_clear(struct str* str) {
    str->length = 0;
}

static inline void str_destroy(struct str* str) {
    free(str->data);
}

/// Makes sure the given string is zero-terminated. Returns a valid C-string that points to it.
/// This function can be called several times, and will only append a terminator character once.
static inline const char* str_terminate(struct str* str) {
    if (str->length == 0 || str->data[str->length - 1] != 0)
        str_push(str, '\0');
    return str->data;
}

/// Appends formatted text at the end of the given string.
[[gnu::format(printf, 2, 3)]]
static inline void str_printf(struct str* str, const char* fmt, ...) {
    size_t remaining_size = str->capacity - str->length;

    va_list args;
    va_start(args, fmt);
    int req_size = vsnprintf(str->data + str->length, remaining_size, fmt, args);
    va_end(args);

    if ((size_t)req_size >= remaining_size) {
        str_grow(str, req_size + 1);
        remaining_size = str->capacity - str->length;

        va_start(args, fmt);
        req_size = vsnprintf(str->data + str->length, remaining_size, fmt, args);
        va_end(args);

        assert((size_t)req_size < remaining_size);
        str->length += req_size;
    }
}
