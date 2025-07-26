#include "uart.h"
#include "print.h"
#include "timer.h"
#include "util.h"

// Minimal TinyMT32 for fair comparison
typedef struct {
    uint32_t s[4];
    uint32_t m1, m2, tm;
} rng_t;

void init_rng(rng_t *r) {
    r->s[0] = 0x7B78CF44; r->s[1] = 0x8F7011EE;
    r->s[2] = 0xFC78FF1F; r->s[3] = 0x3793FDFF;
    r->m1 = 0x8F7011EE; r->m2 = 0xFC78FF1F; r->tm = 0x3793FDFF;
}

uint32_t gen_rng(rng_t *r) {
    uint32_t y = r->s[3];
    uint32_t x = (r->s[0] & 0x7fffffff) ^ r->s[1] ^ r->s[2];
    x ^= x << 1; y ^= (y >> 1) ^ x;
    r->s[0] = r->s[1]; r->s[1] = r->s[2];
    r->s[2] = x ^ (y << 10); r->s[3] = y;
    if (y & 1) { r->s[1] ^= r->m1; r->s[2] ^= r->m2; }
    uint32_t t = r->s[3] ^ (r->s[0] + (r->s[2] >> 8));
    return (t & 1) ? t ^ r->tm : t;
}

uint32_t work(uint32_t n) {
    uint32_t x = 0;
    for (uint32_t i = 0; i < n; i++) x += i;
    return x;
}

int main() {
    uart_init();
    printf("RNG Speed Test\n");
    uart_write_flush();
    
    uint32_t start, end, cycles;
    const uint32_t N = 50; // Reduced sample size
    
    // Test HW speed
    start = get_mcycle();
    for (uint32_t i = 0; i < N; i++) {
        volatile uint32_t r = *reg32(USER_FRNG_BASE_ADDR, 0);
    }
    end = get_mcycle();
    cycles = end - start;
    printf("HW: 0x%x cyc, 0x%x per read\n", cycles, cycles/N);
    uart_write_flush();
    
    // Test SW speed
    rng_t rng;
    init_rng(&rng);
    start = get_mcycle();
    for (uint32_t i = 0; i < N; i++) {
        volatile uint32_t r = gen_rng(&rng);
    }
    end = get_mcycle();
    uint32_t sw_cycles = end - start;
    printf("SW: 0x%x cyc, 0x%x per call\n", sw_cycles, sw_cycles/N);
    uart_write_flush();
    
    printf("HW is 0x%x times faster\n", sw_cycles/cycles);
    uart_write_flush();
    
    // Test with work
    printf("Testing with CPU work...\n");
    uart_write_flush();
    
    uint32_t result = 0;
    start = get_mcycle();
    for (uint32_t i = 0; i < N; i++) {
        volatile uint32_t r = *reg32(USER_FRNG_BASE_ADDR, 0);
        result += work(20);
    }
    end = get_mcycle();
    uint32_t hw_work = end - start;
    
    init_rng(&rng);
    result = 0;
    start = get_mcycle();
    for (uint32_t i = 0; i < N; i++) {
        volatile uint32_t r = gen_rng(&rng);
        result += work(20);
    }
    end = get_mcycle();
    uint32_t sw_work = end - start;
    
    printf("HW+work: 0x%x cyc\n", hw_work);
    printf("SW+work: 0x%x cyc\n", sw_work);
    printf("Speedup: 0x%x\n", sw_work/hw_work);
    uart_write_flush();
    
    return 1;
}