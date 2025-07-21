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
    printf("RNG seeded\n");
    uart_write_flush();
    
    // Read 500 values for set 1
    for (int i = 0; i < 100; i++) {
        uint32_t val = *reg32(USER_FRNG_BASE_ADDR, 0);
    }
    printf("RNG READS DONE\n");
    uart_write_flush();
    printf("beginning regular RAM reads \n");
    for (int i = 0; i < 100; i++) {
        uint32_t val = *reg32(SRAM_BASE_ADDR, 0);
    }
    printf("SRAM READS DONE\n");
    
    printf("All values collected\n");
    uart_write_flush();
    
    return 1;
}