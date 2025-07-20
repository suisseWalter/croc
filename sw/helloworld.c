#include "uart.h"
#include "print.h"
#include "timer.h"
#include "gpio.h"
#include "util.h"

int main() {
    uart_init();
    printf("Hello World!\n");
    uart_write_flush();
    uart_write_flush();
    printf("Large-scale PRNG Test - 1500 values\n");
    uart_write_flush();
    
    // SET 1: 500 values with first seed set
    printf("=== SET 1 ===\n");
    uart_write_flush();
    
    // Seed set 1
    *reg32(USER_FRNG_BASE_ADDR, 0) = 0xDEADBEEF;
    *reg32(USER_FRNG_BASE_ADDR, 4) = 0xCAFEBABE;
    *reg32(USER_FRNG_BASE_ADDR, 8) = 0x12345678;
    *reg32(USER_FRNG_BASE_ADDR, 12) = 0x87654321;
    *reg32(USER_FRNG_BASE_ADDR, 16) = 0xFFFFFFFF;
    printf("Set 1 seeded\n");
    uart_write_flush();
    
    // Read 500 values for set 1
    for (int i = 0; i < 500; i++) {
        uint32_t val = *reg32(USER_FRNG_BASE_ADDR, 0);
        printf("S1[%x]: %x\n", i, val);
        if ((i + 1) % 50 == 0) {
            uart_write_flush();
        }
    }
    uart_write_flush();
    
    // SET 2: 500 values with second seed set
    printf("=== SET 2 ===\n");
    uart_write_flush();
    
    // Seed set 2
    *reg32(USER_FRNG_BASE_ADDR, 0) = 0xA5A5A5A5;
    *reg32(USER_FRNG_BASE_ADDR, 4) = 0x5A5A5A5A;
    *reg32(USER_FRNG_BASE_ADDR, 8) = 0xF0F0F0F0;
    *reg32(USER_FRNG_BASE_ADDR, 12) = 0x0F0F0F0F;
    *reg32(USER_FRNG_BASE_ADDR, 16) = 0xFFFFFFFF;
    printf("Set 2 seeded\n");
    uart_write_flush();
    
    // Read 500 values for set 2
    for (int i = 0; i < 500; i++) {
        uint32_t val = *reg32(USER_FRNG_BASE_ADDR, 0);
        printf("S2[%x]: %x\n", i, val);
        if ((i + 1) % 50 == 0) {
            uart_write_flush();
        }
    }
    uart_write_flush();
    
    // SET 3: 500 values with third seed set
    printf("=== SET 3 ===\n");
    uart_write_flush();
    
    // Seed set 3
    *reg32(USER_FRNG_BASE_ADDR, 0) = 0x13579BDF;
    *reg32(USER_FRNG_BASE_ADDR, 4) = 0xFDB97531;
    *reg32(USER_FRNG_BASE_ADDR, 8) = 0x2468ACE0;
    *reg32(USER_FRNG_BASE_ADDR, 12) = 0x0ECA8642;
    *reg32(USER_FRNG_BASE_ADDR, 16) = 0xFFFFFFFF;
    printf("Set 3 seeded\n");
    uart_write_flush();
    
    // Read 500 values for set 3
    for (int i = 0; i < 500; i++) {
        uint32_t val = *reg32(USER_FRNG_BASE_ADDR, 0);
        printf("S3[%x]: %x\n", i, val);
        if ((i + 1) % 50 == 0) {
            uart_write_flush();
        }
    }
    uart_write_flush();
    
    printf("All 1500 values collected\n");
    uart_write_flush();
    
    return 1;
}