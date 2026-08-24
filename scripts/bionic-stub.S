// bionic-stub.S — provides symbols missing from the Android bionic libc
// (as targeted by the NDK's api-24) that V8/ICU/abseil reference via
// rusty_v8 when cross-compiled for aarch64-linux-android.
//
// Written in assembly to avoid C header conflicts:
//   - __clear_cache is a clang builtin -> conflicting types if defined in C
//   - bionic's stdlib.h already declares strtof_l/strtod_l (with locale_t),
//     but the symbols are missing from the linked libc at api-24
//
// strtof_l/strtod_l: aarch64 calling convention passes nptr/endptr/loc in
// x0/x1/x2; strtof/strtod take x0/x1 and ignore the rest, so a tail jump is
// correct. Android has a single C locale anyway.
//
// __clear_cache: standard icache maintenance loop (64-byte lines) followed by
// dsb ish + isb, mirroring compiler-rt's aarch64 implementation.

    .text
    .align 2
    .global __clear_cache
    .type __clear_cache, %function
__clear_cache:
    // x0 = begin, x1 = end
    sub     x1, x1, #1
    // align begin down to 64 bytes
    lsr     x2, x0, #6
    lsl     x2, x2, #6
1:
    ic      ivau, x2
    add     x2, x2, #64
    cmp     x2, x1
    bls     1b
    dsb     ish
    isb
    ret
    .size __clear_cache, .-__clear_cache

    .global strtof_l
    .type strtof_l, %function
strtof_l:
    b       strtof
    .size strtof_l, .-strtof_l

    .global strtod_l
    .type strtod_l, %function
strtod_l:
    b       strtod
    .size strtod_l, .-strtod_l

    .section .note.GNU-stack,"",%progbits
