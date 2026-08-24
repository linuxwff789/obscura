// bionic-stub.c — provides symbols missing from Android bionic that
// V8/ICU/abseil (via rusty_v8) reference when cross-compiled for android.
// Mirrors what Termux's libandroid-stub package does for deno.
#include <stdlib.h>

// bionic has no __clear_cache libc symbol; Rust codegen references it
// directly. Route through the compiler builtin (correct on aarch64).
void __clear_cache(char* begin, char* end) {
  __builtin___clear_cache(begin, end);
}

// bionic lacks locale-aware *_l variants (glibc-only). Fall back to the
// non-locale versions (Android has a single C locale anyway).
float strtof_l(const char* restrict nptr, char** restrict endptr, void* restrict loc) {
  (void)loc;
  return strtof(nptr, endptr);
}

double strtod_l(const char* restrict nptr, char** restrict endptr, void* restrict loc) {
  (void)loc;
  return strtod(nptr, endptr);
}
