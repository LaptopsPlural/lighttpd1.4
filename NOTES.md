# Google Patch Rewards — lighttpd local draft notes

**Date:** 2026-09-11 (America/Chicago)  
**Do not claim yet:** need upstream merge + ≥30 days, then https://bughunters.google.com/report/patch_rewards

## Chosen target + why

- **Project:** lighttpd (Tier-1 · High-profile web and mail servers — Google Patch Rewards memory-safety track)
- **Upstream:** https://git.lighttpd.net/lighttpd/lighttpd1.4 (canonical forge; GitHub mirror `lighttpd/lighttpd1.4` exists — prefer forge for PR/MR)
- **Local clone:** `/workspace/google-patch-lighttpd`
- **Branch:** `local/buffer-ptr-fbounds-safety` (from `master`)
- **Why this target:**
  1. Widely deployed HTTP server; internal `buffer` backs request/response string+binary paths on untrusted input.
  2. Clear, mergeable first-CL scope: **one** internal buffer+capacity pair — `buffer.ptr` ↔ `buffer.size` — textbook `__sized_by` / counted_by companion, not a whole-tree sweep.
  3. Same pattern as libpng / libwebp / giflib / lz4 / zstd / libzip / bzip2: **inert macros** when the flag is off; experimental Clang `-fbounds-safety` only when explicitly enabled.
  4. Stable layout preserved (no field reorder); annotation links `ptr` to its **capacity** (`size`).
  5. Not already done upstream (no `__sized_by` / `-fbounds-safety` in tree; no overlapping open annotation PRs on GitHub search 2026-09-11).
  6. AI CONTRIBUTING gate clear (no CONTRIBUTING.md; no AI ban in README / COPYING / INSTALL / AUTHORS / NEWS / `.github`).

**Why `buffer.ptr`/`size` over `used` or other containers:** `size` is the allocated capacity at `*ptr`; `used` is the live length (including terminating NUL) and is not the correct companion for an allocation bound. Other chunk/queue buffers are natural follow-ups.

**ABI / layout note:** Field order is preserved (`ptr` remains before `size`). `buffer_realloc` already assigns **capacity then pointer**. Header is internal (`noinst_HEADERS`); not installed public ABI.

## Security benefit

`buffer` is the core string/binary container used across request parsing, response generation, and config paths. Callers already track capacity in `size`, but the compiler cannot see that `ptr` is bounded by that field.

This draft:

1. Introduces `src/buffer_bounds_safety.h` with `LI_SIZED_BY` / `LI_SIZED_BY_OR_NULL` / `LI_COUNTED_BY*` (empty by default).
2. Annotates **only** `buffer.ptr` → `LI_SIZED_BY_OR_NULL(size)` (capacity bound; `ptr` may be NULL in the empty/unset state).
3. Keeps existing field order. Documents that `buffer_realloc` already assigns **capacity before pointer**.
4. Wires optional CMake `ENABLE_FBOUNDS_SAFETY` / meson `enable_fbounds_safety` / autotools `--enable-fbounds-safety` (default **OFF**) → `-DLI_SUPPORT_FBOUNDS_SAFETY` + `-fbounds-safety`.

**Default builds are unchanged:** macros expand to nothing; no new runtime checks without the experimental flag.

## Files changed

| File | Change |
|------|--------|
| `src/buffer_bounds_safety.h` | **New** — inert / Clang bounds macros |
| `src/buffer.h` | Include header; annotate `buffer.ptr` |
| `src/buffer.c` | Comment: capacity-first assign already present in `buffer_realloc` |
| `src/CMakeLists.txt` | `ENABLE_FBOUNDS_SAFETY` option OFF + apply flags when ON |
| `meson_options.txt` | `enable_fbounds_safety` option false |
| `src/meson.build` | Apply `-fbounds-safety` when option ON |
| `configure.ac` | `--enable-fbounds-safety` (default no) |
| `src/Makefile.am` | Add `buffer_bounds_safety.h` to `noinst_HEADERS` list |
| `NOTES.md` | This file |

## Verified locally 2026-09-11

| Check | Result |
|-------|--------|
| Default `ENABLE_FBOUNDS_SAFETY=OFF` cmake `-DWITH_PCRE2=OFF -DWITH_ZLIB=OFF` build + `ctest` (9/9) | **PASS** (gcc; `lighttpd` + unit/integration tests) |
| `ENABLE_FBOUNDS_SAFETY=ON` | **Not feasible on this box** — needs Clang with `-fbounds-safety` / `ptrcheck.h` |

## How to build / test

Default (macros inert — must stay green):

```sh
cmake -S . -B build -DENABLE_FBOUNDS_SAFETY=OFF
cmake --build build -j
ctest --test-dir build --output-on-failure
# or meson:
meson setup build -Denable_fbounds_safety=false
meson compile -C build
```

With experimental bounds-safety toolchain (maintainers / CI; **not** available on this box — no Clang/`ptrcheck.h`):

```sh
cmake -S . -B build-fbs -DENABLE_FBOUNDS_SAFETY=ON \
  -DCMAKE_C_COMPILER=<clang-with-fbounds-safety>
cmake --build build-fbs -j
# or meson:
meson setup build-fbs -Denable_fbounds_safety=true
meson compile -C build-fbs
# or autotools:
./configure --enable-fbounds-safety CC=<clang-with-fbounds-safety>
```

## Upstream submit plan

1. Open a focused **Gitea PR/MR** against canonical forge https://git.lighttpd.net/lighttpd/lighttpd1.4 branch **`master`** (prefer forge over GitHub mirror).
2. Proposed title: `buffer: add optional -fbounds-safety annotations for buffer.ptr`
3. Frame as secure-by-design / Safe Buffers-style systematization of the existing ptr+size pair; cite libwebp/libpng/lz4/zstd/libzip prior art and Google Patch Rewards memory-safety goals.
4. Emphasize: default build behavior unchanged; flag OFF; no PoC / no CVE claim; internal layout field order unchanged; header not installed.
5. Do **not** claim on https://bughunters.google.com/report/patch_rewards until **merge + ≥30 days**.

## Follow-ups (separate CLs)

- Annotate `used`-relative helpers only if a real `-fbounds-safety` build shows diagnostics worth fixing
- Other internal byte buffers on request/chunk paths (`chunk`, temporary CGI buffers, etc.) as diagnostics dictate

## AI gate

- **No `CONTRIBUTING.md`** in upstream tree.
- Searched README, COPYING, INSTALL, AUTHORS, NEWS, `.github/` — **no AI / LLM / Copilot ban**.
- **AI gate: CLEAR** (no ban found).

## Overlap check

- GitHub issue search `fbounds|counted_by|sized_by|bounds-safety` on `lighttpd/lighttpd1.4`: **0 hits** (2026-09-11).
- In-tree: no `__sized_by` / `-fbounds-safety` / `ptrcheck.h` prior to this draft.

## Status

**LOCAL DRAFT ONLY** — commit on `local/buffer-ptr-fbounds-safety`. Do **not** push/PR from this agent run. Still **no claim** until merge + ≥30 days unreverted.
