#pragma once

/**
 * @file
 *
 * Span data structure which represents a continuous range of elements in memory.
 */

/// Iterates over the elements of a span.
/// @param elem_ty Type of the elements in the span.
/// @param elem Pointer to the element in the span.
/// @param span Expression evaluating to a span.
#define SPAN_FOREACH(elem_ty, elem, span) \
    for (elem_ty* elem = (span).elems; elem != (span).elems + (span).elem_count; ++elem)

/// Iterates over the elements of a span in reverse order.
/// @see SPAN_FOREACH.
#define SPAN_REV_FOREACH(elem_ty, elem, span) \
    for (elem_ty* elem = (span).elems + (span).elem_count; elem-- != (span).elems; )

/// Declares a span.
/// @param elem_ty Type of the elements in the span.
#define SPAN_DECL(name, elem_ty) \
    struct name { \
        elem_ty* elems; \
        size_t elem_count; \
    };

/// Iterates over the elements of a constant span.
/// @see SPAN_FOREACH.
#define CONST_SPAN_FOREACH(elem_ty, elem, span) \
    for (elem_ty const* elem = (span).elems; elem != (span).elems + (span).elem_count; ++elem)

/// Iterates over the elements of a constant span in reverse order.
/// @see CONST_SPAN_FOREACH.
#define CONST_SPAN_REV_FOREACH(elem_ty, elem, span) \
    for (elem_ty const* elem = (span).elems + (span).elem_count; elem-- != (span).elems; )

/// Declares a constant span.
/// @see SPAN_DECL.
#define CONST_SPAN_DECL(name, elem_ty) \
    struct name { \
        elem_ty const* elems; \
        size_t elem_count; \
    };
