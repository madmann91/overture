#pragma once

/// @file
///
/// Symbol visibility control for data structure instantiation.

/// Macro that constrains the visibility of a symbol. Data structures that accept a visibility
/// parameter can be instantiated publicly by using such code:
///
///     // module.c
///     DATA_STRUCTURE_IMPL(..., PUBLIC)
///     // module.h
///     DATA_STRUCTURE_DECL(..., PUBLIC)
///
/// Or privately, with the following code:
///
///     // module.c
///     DATA_STRUCTURE_DEFINE(..., PRIVATE)
///
/// @param vis Visibility of the symbol, which can be either `PUBLIC` or `PRIVATE`.
#define VISIBILITY(vis) vis##_VISIBILITY

#define PUBLIC_VISIBILITY
#define PRIVATE_VISIBILITY static inline
