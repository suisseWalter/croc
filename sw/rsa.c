/* rng_crypto.c
 *
 * C99 demo for FRNG+SW RNG and toy 32-bit RSA with blinding
 * Uses custom uart and printf from print.h
 * Reduced multiplication/division footprint where possible.
 *
 * Author: ChatGPT
 * 
 * Use this command to compile and run it. (start from the verilator directory
 * cd ../sw && make bin/rsa.elf && riscv64-unknown-elf-objcopy -O verilog bin/rsa.elf bin/rsa.hex && cd ../verilator && verilator --binary -j 0 -Wno-fatal -Wno-style --timing --autoflush --trace --trace-structs -CFLAGS "-O0" --top tb_croc_soc -f croc.f && ./obj_dir/Vtb_croc_soc +binary="../sw/bin/rsa.hex"
 * 
 */

#include "uart.h"
#include "print.h"  // provides printf

// -----------------------------------------------------------------------------
// FRNG hardware RNG
// -----------------------------------------------------------------------------

static inline uint32_t frng_read(uint32_t idx) {
    return *(volatile uint32_t *)(USER_FRNG_BASE_ADDR + (idx & 0xFFu)*4);
}
static uint32_t (*get_random32)(void) = 0;
static uint32_t hw_random32(void) {
    static uint32_t idx;
    uint32_t v = frng_read(idx);
    idx++;
    return v;
}

// -----------------------------------------------------------------------------
// Software fallback RNG (xorshift32)
// -----------------------------------------------------------------------------
static uint32_t sw_state;
static uint32_t sw_random32(void) {
    uint32_t x = sw_state;
    x ^= x << 13;
    x ^= x << 13;
    x ^= x << 13;
    x ^= x << 13;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    return sw_state = x;
}

/**
 * Initialize RNG. If hw_ok non-zero, use hardware; otherwise seed and use software.
 */
void rng_init(int hw_ok) {
    if (hw_ok) {
        get_random32 = hw_random32;
    } else {
        // Seed SW xorshift from FRNG
        uint32_t seed = 0xA5A5A5Au;
        for (int i = 0; i < 4; i++) seed ^= 0x1234567u;
        sw_state = seed ? seed : 0x5A5A5A5Au;
        get_random32 = sw_random32;
    }
}

// -----------------------------------------------------------------------------
// RSA constants and minimal arithmetic
// -----------------------------------------------------------------------------
#define RSA_N 3233u  // 61*53
#define RSA_E 17u
#define RSA_D 2753u

/**
 * Modular exponentiation: base^exp mod RSA_N
 * Avoids 64-bit division; uses only 32-bit arithmetic with reduce via remainder.
 */
static uint32_t modexp(uint32_t base, uint32_t exp) {
    uint32_t result = 1;
    base %= RSA_N;
    while (exp) {
        if (exp & 1u) {
            uint32_t prod = result * base;
            result = prod - (prod / RSA_N) * RSA_N;
        }
        uint32_t sq = base * base;
        base = sq - (sq / RSA_N) * RSA_N;
        exp >>= 1;
    }
    return result;
}

/** Extended Euclidean algorithm for mod inverse */
static int32_t modinv(int32_t a) {
    int32_t t = 0, newt = 1;
    int32_t r = RSA_N, newr = a;
    while (newr) {
        int32_t q = r / newr;
        int32_t tmp = newt;
        newt = t - q * newt;
        t = tmp;
        tmp = newr;
        newr = r - q * newr;
        r = tmp;
    }
    return (r == 1) ? (t < 0 ? t + RSA_N : t) : -1;
}

/**
 * Demo: encrypt two-character message with RSA blinding
 */
void example_encrypt(const char *msg) {
    uint16_t m = ((uint16_t)msg[0] << 8) | (uint8_t)msg[1];
    printf("Plain: %c%c -> %x\n", msg[0], msg[1], m);
    uart_write_flush();
    // generate blinding factor r in [2, N-1]
    uint32_t r;
    do { r = get_random32() % (RSA_N - 2) + 2; } while (r >= RSA_N);
    /* do { r = get_random32() % (RSA_N - 2) + 2; } while (r >= RSA_N);
    do { r = get_random32() % (RSA_N - 2) + 2; } while (r >= RSA_N);
    do { r = get_random32() % (RSA_N - 2) + 2; } while (r >= RSA_N);
    do { r = get_random32() % (RSA_N - 2) + 2; } while (r >= RSA_N);
    do { r = get_random32() % (RSA_N - 2) + 2; } while (r >= RSA_N); */
    uint32_t re = modexp(r, RSA_E);
    printf("r=%x, r^e=%x\n", r, re);
    uart_write_flush();
    // blind: m * re mod N
    uint32_t mb = (m * re) % RSA_N;
    printf("m'=%x\n", mb);
    uart_write_flush();
    // decrypt: m'^d mod N
    uint32_t ms = modexp(mb, RSA_D);
    printf("dec=%x\n", ms);
    uart_write_flush();
    // unblind
    int32_t invr = modinv((int32_t)r);
    uint32_t mout = (uint32_t)((ms * (uint32_t)invr) % RSA_N);
    printf("out=%x '%c%c'\n", mout, (char)(mout >> 8), (char)mout);
    uart_write_flush();
}

int main(void) {
    uart_init();
    rng_init(1);
    printf("RNG+RSA Demo\n");
    printf("1000 hardware random numbers:\n");
    uart_write_flush();
    for (int i = 0; i < 1001; i++) {
        uint32_t r = get_random32();
    }
    printf("Done.\n");
    uart_write_flush();
    rng_init(0);
    printf("1000 software random numbers:\n");
    uart_write_flush();
    for (int i = 0; i < 1001; i++) {
        uint32_t r = get_random32();
    }
    rng_init(1);
    printf("Done.\n");
    uart_write_flush();
    example_encrypt("Hi");
    uart_write_flush();
    printf("Testing complete.\n");
    uart_write_flush();
    return 0;
}
