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

/// @brief Example integer square root
/// @return integer square root of n
uint32_t isqrt(uint32_t n) {
    uint32_t res = 0;
    uint32_t bit = (uint32_t)1 << 30;

    while (bit > n) bit >>= 2;

    while (bit) {
        if (n >= res + bit) {
            n -= res + bit;
            res = (res >> 1) + bit;
        } else {
            res >>= 1;
        }
        bit >>= 2;
    }
    return res;
}

int main() {
    uart_init(); 
    printf("Hello World!\n");
    uart_write_flush();
    uart_write_flush();
    printf("Testing the changes\n");
    uart_write_flush();  
    printf("beginning FRNG reads\n");
    for (int i = 0; i < 2; i++) {
        uint32_t val = *reg32(USER_FRNG_BASE_ADDR, i * 4);
        printf("FRNG word, %x, %x\n", i, val);
        
    }
    uart_write_flush();
    printf("attempting writes\n");
    for (int i = 0; i < 4; i++) {
        *reg32(USER_FRNG_BASE_ADDR, i * 4) = 0x65722020;
        }
    printf("all writes through, beginning reset\n");
    uart_write_flush();
    *reg32(USER_FRNG_BASE_ADDR, 16) = 0xFFFFFFFF;
    printf("manual seeding triggered\n");
    uart_write_flush();

    printf("second salvo\n");
    for (int i = 0; i < 2; i++) {
        uint32_t val = *reg32(USER_FRNG_BASE_ADDR, i * 4);
        printf("FRNG word, %x, %x\n", i, val);
        
    }
    uart_write_flush();



    printf("It now should have been done\n");
    uart_write_flush();
    return 1;
}