// Symbols that V8/ICU/abseil reference but bionic does not export at the API
// level obscura targets (24). Linked into the final Android binaries only.
//
// The bionic headers provide static-inline fallbacks for the *_l functions at
// this API level, so the real symbols are emitted through asm aliases instead
// of colliding with the inline definitions.
#include <stddef.h>
#include <stdlib.h>

// bionic has no libc __clear_cache symbol; Rust/V8 codegen references it.
// Match the compiler builtin's signature to avoid the conflicting-type error.
void __clear_cache(void* begin, void* end) {
  __builtin___clear_cache((char*)begin, (char*)end);
}

// bionic has a single C locale, so the locale-aware variants fall back to the
// plain conversion routines.
float __obscura_strtof_l(const char* restrict nptr, char** restrict endptr,
                         void* restrict loc) __asm__("strtof_l");
float __obscura_strtof_l(const char* restrict nptr, char** restrict endptr,
                         void* restrict loc) {
  (void)loc;
  return strtof(nptr, endptr);
}

double __obscura_strtod_l(const char* restrict nptr, char** restrict endptr,
                          void* restrict loc) __asm__("strtod_l");
double __obscura_strtod_l(const char* restrict nptr, char** restrict endptr,
                          void* restrict loc) {
  (void)loc;
  return strtod(nptr, endptr);
}

// aligned_alloc only exists from API 28; emulate it for the API 24 target.
void* __obscura_aligned_alloc(size_t alignment, size_t size)
    __asm__("aligned_alloc");
void* __obscura_aligned_alloc(size_t alignment, size_t size) {
  void* ptr = NULL;
  if (posix_memalign(&ptr, alignment, size) != 0) {
    return NULL;
  }
  return ptr;
}
