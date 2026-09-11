/*
 * buffer_bounds_safety.h -- portability macros for optional -fbounds-safety
 *
 * Copyright (c) 2026 Jeff Bindel <jeff@incrediblybased.co>
 * License: BSD 3-clause (same as lighttpd)
 *
 * When LI_SUPPORT_FBOUNDS_SAFETY is defined (typically via
 * -DLI_SUPPORT_FBOUNDS_SAFETY and a Clang toolchain that implements
 * -fbounds-safety), these macros expand to Clang bounds annotations.
 * Otherwise they expand to nothing so default builds are unchanged.
 *
 * Pattern matches libwebp / libpng / giflib / lz4 / zstd / libzip / bzip2
 * inert-macro -fbounds-safety adoption: annotations are inert unless
 * explicitly enabled.
 */

#ifndef INCLUDED_BUFFER_BOUNDS_SAFETY_H
#define INCLUDED_BUFFER_BOUNDS_SAFETY_H

#ifdef LI_SUPPORT_FBOUNDS_SAFETY

#  include <ptrcheck.h>
/* Non-ABI-breaking sized-by annotations for byte buffers whose companion
 * field / argument is a capacity in bytes (e.g. buffer.size).
 * Prefer LI_SIZED_BY for buffers that are non-NULL when live; use
 * *_OR_NULL when the pointer may be NULL while the companion size is zero
 * (buffer.ptr may be NULL in the empty / unset state).
 */
#  define LI_SIZED_BY(n) __sized_by(n)
#  define LI_SIZED_BY_OR_NULL(n) __sized_by_or_null(n)
#  define LI_COUNTED_BY(n) __counted_by(n)
#  define LI_COUNTED_BY_OR_NULL(n) __counted_by_or_null(n)

#else /* !LI_SUPPORT_FBOUNDS_SAFETY */

#  define LI_SIZED_BY(n)
#  define LI_SIZED_BY_OR_NULL(n)
#  define LI_COUNTED_BY(n)
#  define LI_COUNTED_BY_OR_NULL(n)

#endif /* LI_SUPPORT_FBOUNDS_SAFETY */

#endif /* INCLUDED_BUFFER_BOUNDS_SAFETY_H */
