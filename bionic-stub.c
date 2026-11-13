// Symbols that V8/ICU/abseil reference but bionic does not export at the
// API level obscura targets (24). Linked into the final Android binaries only.
//
// Mirrors what Termux's libandroid-stub / the older build-android workflow
// provided, plus aligned_alloc (bionic gained it in API 28).
#include <stddef.h>
#include <stdlib.h>

// bionic has no __clear_cache libc symbol; Rust codegen references it.
void __clear_cache(char* begin, char* end) {
  __builtin___clear_cache(begin, end);
}

// bionic has a single C locale, so the locale-aware variants fall back to the
// plain conversion routines.
float strtof_l(const char* restrict nptr, char** restrict endptr,
               void* restrict loc) {
  (void)loc;
  return strtof(nptr, endptr);
}

double strtod_l(const char* restrict nptr, char** restrict endptr,
                void* restrict loc) {
  (void)loc;
  return strtod(nptr, endptr);
}

// __cxa_thread_atexit_impl is missing on some older API levels; V8's
// thread-local destructors only need the Rust-side registration, so a no-op
// would be wrong. aligned_alloc is the one we can safely emulate.
void* aligned_alloc(size_t alignment, size_t size) {
  void* ptr = NULL;
  if (posix_memalign(&ptr, alignment, size) != 0) {
    return NULL;
  }
  return ptr;
}
