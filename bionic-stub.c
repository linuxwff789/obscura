// Symbols that V8/ICU/abseil reference but bionic does not export at the API
// level obscura targets (24). Linked into the final Android binaries only.
//
// The bionic headers provide static-inline fallbacks for the *_l functions at
// this API level, so the real symbols are emitted through asm aliases instead
// of colliding with the inline definitions.
#include <stddef.h>
#include <stdlib.h>

// bionic has no libc __clear_cache symbol. Do NOT call
// __builtin___clear_cache here: on aarch64 bionic that builtin lowers to a
// call to __clear_cache itself, i.e. infinite recursion (observed as a 100%
// CPU hang during the first baseline JIT compile). Use the compiler-rt
// aarch64 sequence instead.
void __clear_cache(void* begin, void* end) {
  char* b = (char*)begin;
  char* e = (char*)end;
  for (char* p = b; p < e; p += 64) {
    __asm__ __volatile__("dc cvau, %0" ::"r"(p) : "memory");
  }
  __asm__ __volatile__("dsb ish" ::: "memory");
  for (char* p = b; p < e; p += 64) {
    __asm__ __volatile__("ic ivau, %0" ::"r"(p) : "memory");
  }
  __asm__ __volatile__("dsb ish" ::: "memory");
  __asm__ __volatile__("isb" ::: "memory");
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

// Android's bionic linker requires the TLS segment to be at least 64-byte
// aligned on arm64, while V8's static objects only request 8. One over-aligned
// thread-local forces the linker to raise the segment alignment; --undefined
// keeps it alive through --gc-sections.
__thread char __obscura_tls_align[64] __attribute__((used, aligned(64)));
