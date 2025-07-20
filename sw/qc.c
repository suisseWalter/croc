// Copyright (c) 2024 ETH Zurich and University of Bologna.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0/
//
// Authors:
// - Philippe Sauter <phsauter@iis.ee.ethz.ch>

#include "uart.h"
#include "print.h"
#include "timer.h"
#include "gpio.h"
#include "util.h"



int main() {
    uart_init(); 
    uart_write_flush();  
    uart_init();
    printf("Testing the changes\n");
    uart_write_flush();
    printf("beginning FRNG reads\n");
    for (int i = 0; i < 2; i++) {
        uint32_t val = *reg32(USER_FRNG_BASE_ADDR, i * 4);
        printf("FRNG word, %x, %x\n", i, val);
    }
    uart_write_flush();
    
    // Configuration 1: Seeds 0xDEADBEEF, 0xCAFEBABE, 0x12345678, 0x87654321
    printf("Config 1: Starting seeding\n");
    uart_write_flush();
    *reg32(USER_FRNG_BASE_ADDR, 0) = 0xDEADBEEF;
    *reg32(USER_FRNG_BASE_ADDR, 4) = 0xCAFEBABE;
    *reg32(USER_FRNG_BASE_ADDR, 8) = 0x12345678;
    *reg32(USER_FRNG_BASE_ADDR, 12) = 0x87654321;
    *reg32(USER_FRNG_BASE_ADDR, 16) = 0xFFFFFFFF;
    printf("Config 1: Manual seeding triggered\n");
    uart_write_flush();
    for (int i = 0; i < 500; i++) {
        uint32_t val = *reg32(USER_FRNG_BASE_ADDR, 0);
        printf("Config 1 word %d: %x\n", i, val);
    }
    uart_write_flush();
    
    // Configuration 2: Seeds 0xABCDEF00, 0x11223344, 0x55667788, 0x99AABBCC
    printf("Config 2: Starting seeding\n");
    uart_write_flush();
    *reg32(USER_FRNG_BASE_ADDR, 0) = 0xABCDEF00;
    *reg32(USER_FRNG_BASE_ADDR, 4) = 0x11223344;
    *reg32(USER_FRNG_BASE_ADDR, 8) = 0x55667788;
    *reg32(USER_FRNG_BASE_ADDR, 12) = 0x99AABBCC;
    *reg32(USER_FRNG_BASE_ADDR, 16) = 0xFFFFFFFF;
    printf("Config 2: Manual seeding triggered\n");
    uart_write_flush();
    for (int i = 0; i < 500; i++) {
        uint32_t val = *reg32(USER_FRNG_BASE_ADDR, 0);
        printf("Config 2 word %d: %x\n", i, val);
    }
    uart_write_flush();
    
    // Configuration 3: Seeds 0x13579BDF, 0x2468ACE0, 0xFEDCBA98, 0x76543210
    printf("Config 3: Starting seeding\n");
    uart_write_flush();
    *reg32(USER_FRNG_BASE_ADDR, 0) = 0x13579BDF;
    *reg32(USER_FRNG_BASE_ADDR, 4) = 0x2468ACE0;
    *reg32(USER_FRNG_BASE_ADDR, 8) = 0xFEDCBA98;
    *reg32(USER_FRNG_BASE_ADDR, 12) = 0x76543210;
    *reg32(USER_FRNG_BASE_ADDR, 16) = 0xFFFFFFFF;
    printf("Config 3: Manual seeding triggered\n");
    uart_write_flush();
    for (int i = 0; i < 500; i++) {
        uint32_t val = *reg32(USER_FRNG_BASE_ADDR, 0);
        printf("Config 3 word %d: %x\n", i, val);
    }
    uart_write_flush();
    
    // Configuration 4: Seeds 0x00000001, 0x00000002, 0x00000003, 0x00000004
    printf("Config 4: Starting seeding\n");
    uart_write_flush();
    *reg32(USER_FRNG_BASE_ADDR, 0) = 0x00000001;
    *reg32(USER_FRNG_BASE_ADDR, 4) = 0x00000002;
    *reg32(USER_FRNG_BASE_ADDR, 8) = 0x00000003;
    *reg32(USER_FRNG_BASE_ADDR, 12) = 0x00000004;
    *reg32(USER_FRNG_BASE_ADDR, 16) = 0xFFFFFFFF;
    printf("Config 4: Manual seeding triggered\n");
    uart_write_flush();
    for (int i = 0; i < 500; i++) {
        uint32_t val = *reg32(USER_FRNG_BASE_ADDR, 0);
        printf("Config 4 word %d: %x\n", i, val);
    }
    uart_write_flush();
    
    // Configuration 5: Seeds 0xFFFFFFFF, 0xAAAAAAAA, 0x55555555, 0x0F0F0F0F
    printf("Config 5: Starting seeding\n");
    uart_write_flush();
    *reg32(USER_FRNG_BASE_ADDR, 0) = 0xFFFFFFFF;
    *reg32(USER_FRNG_BASE_ADDR, 4) = 0xAAAAAAAA;
    *reg32(USER_FRNG_BASE_ADDR, 8) = 0x55555555;
    *reg32(USER_FRNG_BASE_ADDR, 12) = 0x0F0F0F0F;
    *reg32(USER_FRNG_BASE_ADDR, 16) = 0xFFFFFFFF;
    printf("Config 5: Manual seeding triggered\n");
    uart_write_flush();
    for (int i = 0; i < 500; i++) {
        uint32_t val = *reg32(USER_FRNG_BASE_ADDR, 0);
        printf("Config 5 word %d: %x\n", i, val);
    }
    uart_write_flush();
    
    // Configuration 6: Seeds 0x31415926, 0x27182818, 0x14142135, 0x17320508
    printf("Config 6: Starting seeding\n");
    uart_write_flush();
    *reg32(USER_FRNG_BASE_ADDR, 0) = 0x31415926;
    *reg32(USER_FRNG_BASE_ADDR, 4) = 0x27182818;
    *reg32(USER_FRNG_BASE_ADDR, 8) = 0x14142135;
    *reg32(USER_FRNG_BASE_ADDR, 12) = 0x17320508;
    *reg32(USER_FRNG_BASE_ADDR, 16) = 0xFFFFFFFF;
    printf("Config 6: Manual seeding triggered\n");
    uart_write_flush();
    for (int i = 0; i < 500; i++) {
        uint32_t val = *reg32(USER_FRNG_BASE_ADDR, 0);
        printf("Config 6 word %d: %x\n", i, val);
    }
    uart_write_flush();
    
    // Configuration 7: Seeds 0x8000000, 0x40000000, 0x20000000, 0x10000000
    printf("Config 7: Starting seeding\n");
    uart_write_flush();
    *reg32(USER_FRNG_BASE_ADDR, 0) = 0x80000000;
    *reg32(USER_FRNG_BASE_ADDR, 4) = 0x40000000;
    *reg32(USER_FRNG_BASE_ADDR, 8) = 0x20000000;
    *reg32(USER_FRNG_BASE_ADDR, 12) = 0x10000000;
    *reg32(USER_FRNG_BASE_ADDR, 16) = 0xFFFFFFFF;
    printf("Config 7: Manual seeding triggered\n");
    uart_write_flush();
    for (int i = 0; i < 500; i++) {
        uint32_t val = *reg32(USER_FRNG_BASE_ADDR, 0);
        printf("Config 7 word %d: %x\n", i, val);
    }
    uart_write_flush();
    
    // Configuration 8: Seeds 0x65722020, 0x65722021, 0x65722022, 0x65722023
    printf("Config 8: Starting seeding\n");
    uart_write_flush();
    *reg32(USER_FRNG_BASE_ADDR, 0) = 0x65722020;
    *reg32(USER_FRNG_BASE_ADDR, 4) = 0x65722021;
    *reg32(USER_FRNG_BASE_ADDR, 8) = 0x65722022;
    *reg32(USER_FRNG_BASE_ADDR, 12) = 0x65722023;
    *reg32(USER_FRNG_BASE_ADDR, 16) = 0xFFFFFFFF;
    printf("Config 8: Manual seeding triggered\n");
    uart_write_flush();
    for (int i = 0; i < 500; i++) {
        uint32_t val = *reg32(USER_FRNG_BASE_ADDR, 0);
        printf("Config 8 word %d: %x\n", i, val);
    }
    uart_write_flush();
    
    // Configuration 9: Seeds 0xF0F0F0F0, 0x0F0F0F0F, 0xF0F0F0F0, 0x0F0F0F0F
    printf("Config 9: Starting seeding\n");
    uart_write_flush();
    *reg32(USER_FRNG_BASE_ADDR, 0) = 0xF0F0F0F0;
    *reg32(USER_FRNG_BASE_ADDR, 4) = 0x0F0F0F0F;
    *reg32(USER_FRNG_BASE_ADDR, 8) = 0xF0F0F0F0;
    *reg32(USER_FRNG_BASE_ADDR, 12) = 0x0F0F0F0F;
    *reg32(USER_FRNG_BASE_ADDR, 16) = 0xFFFFFFFF;
    printf("Config 9: Manual seeding triggered\n");
    uart_write_flush();
    for (int i = 0; i < 500; i++) {
        uint32_t val = *reg32(USER_FRNG_BASE_ADDR, 0);
        printf("Config 9 word %d: %x\n", i, val);
    }
    uart_write_flush();
    
    // Configuration 10: Seeds 0x12481632, 0x36486421, 0x98765432, 0x13572468
    printf("Config 10: Starting seeding\n");
    uart_write_flush();
    *reg32(USER_FRNG_BASE_ADDR, 0) = 0x12481632;
    *reg32(USER_FRNG_BASE_ADDR, 4) = 0x36486421;
    *reg32(USER_FRNG_BASE_ADDR, 8) = 0x98765432;
    *reg32(USER_FRNG_BASE_ADDR, 12) = 0x13572468;
    *reg32(USER_FRNG_BASE_ADDR, 16) = 0xFFFFFFFF;
    printf("Config 10: Manual seeding triggered\n");
    uart_write_flush();
    for (int i = 0; i < 500; i++) {
        uint32_t val = *reg32(USER_FRNG_BASE_ADDR, 0);
        printf("Config 10 word %d: %x\n", i, val);
    }
    uart_write_flush();
    
    printf("All configurations complete\n");
    uart_write_flush();
    return 1;
}