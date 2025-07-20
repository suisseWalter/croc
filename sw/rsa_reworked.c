/* rng_crypto_perf.c
 *
 * C99 demo for FRNG+SW RNG performance measurement on RISC-V IBEX
 * Measures cycle count for single RNG generation calls
 * Uses custom uart and printf from print.h
 *
 * Author: Modified for RISC-V IBEX
 */

 #include "uart.h"
 #include "print.h"  // provides printf
 
 // -----------------------------------------------------------------------------
 // RISC-V cycle counter functions
 // -----------------------------------------------------------------------------
 
 // Read RISC-V cycle counter (mcycle CSR)
 static inline uint64_t get_cycle_count(void) {
     uint32_t cycles_low, cycles_high1, cycles_high2;
     
     // Read cycle counter atomically (handle 32-bit reads of 64-bit counter)
     do {
         __asm__ volatile ("csrr %0, mcycleh" : "=r" (cycles_high1));
         __asm__ volatile ("csrr %0, mcycle"  : "=r" (cycles_low));
         __asm__ volatile ("csrr %0, mcycleh" : "=r" (cycles_high2));
     } while (cycles_high1 != cycles_high2);
     
     return ((uint64_t)cycles_high1 << 32) | cycles_low;
 }
 
 // -----------------------------------------------------------------------------
 // FRNG hardware RNG
 // -----------------------------------------------------------------------------
 
 #ifndef USER_FRNG_BASE_ADDR
 #define USER_FRNG_BASE_ADDR 0x40000000  // Default base address
 #endif
 
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
 // Performance measurement functions
 // -----------------------------------------------------------------------------
 
 uint32_t measure_single_rng_call(void) {
     uint64_t start_cycles, end_cycles;
     uint32_t random_value;
     
     // Measure cycles for a single RNG call
     start_cycles = get_cycle_count();
     random_value = get_random32();
     end_cycles = get_cycle_count();
     
     // Prevent compiler optimization by using the value
     (void)random_value;
     
     return (uint32_t)(end_cycles - start_cycles);
 }
 
 // -----------------------------------------------------------------------------
 // RSA constants and minimal arithmetic
 // -----------------------------------------------------------------------------
 #define RSA_N 3233u  // 61*53
 #define RSA_E 17u
 #define RSA_D 2753u
 
 /**
  * Modular exponentiation: base^exp mod RSA_N
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
     
     // Generate blinding factor r in [2, N-1]
     uint32_t r;
     do { r = get_random32() % (RSA_N - 2) + 2; } while (r >= RSA_N);
     
     uint32_t re = modexp(r, RSA_E);
     printf("r=%x, r^e=%x\n", r, re);
     uart_write_flush();
     
     // Blind: m * re mod N
     uint32_t mb = (m * re) % RSA_N;
     printf("m'=%x\n", mb);
     uart_write_flush();
     
     // Decrypt: m'^d mod N
     uint32_t ms = modexp(mb, RSA_D);
     printf("dec=%x\n", ms);
     uart_write_flush();
     
     // Unblind
     int32_t invr = modinv((int32_t)r);
     uint32_t mout = (uint32_t)((ms * (uint32_t)invr) % RSA_N);
     printf("out=%x '%c%c'\n", mout, (char)(mout >> 8), (char)mout);
     uart_write_flush();
 }
 
 int main(void) {
     uart_init();
     
     printf("RNG+RSA Demo\n");
     
     // Test hardware RNG
     rng_init(1);
     uint32_t hw_cycles = measure_single_rng_call();
     printf("HW: %u cycles\n", hw_cycles);
     uart_write_flush();
     
     // Test software RNG  
     rng_init(0);
     uint32_t sw_cycles = measure_single_rng_call();
     printf("SW: %u cycles\n", sw_cycles);
     uart_write_flush();
     
     // Switch back to hardware for RSA demo
     rng_init(1);
     example_encrypt("Hi");
     
     while (1) {}
 }