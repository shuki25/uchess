/*
MicroPython native module for the uchess library

This library is a wrapper for the Vice chess engine written by
Joshua Butler, MD, MHI. The original source code is available at
https://bluefeversoft.com/.

The MIT License (MIT)

Copyright (c) 2022-2023 Joshua Butler, MD, MHI

*/

#include "py/obj.h"
#include "py/dynruntime.h"
#include "vice/defs.h"
#include "helper.h"
#include <string.h>


#if !defined(__linux__)
void *memset(void *s, int c, size_t n) {
    return mp_fun_table.memset_(s, c, n);
}
#endif

typedef struct _uchess_obj_t {
    mp_obj_base_t base;
    char fen[87];
    int32_t depth;
    int32_t move;
    int32_t halfmove;
    int32_t fullmove;
    int32_t side;
} uchess_obj_t;

uchess_obj_t uchess_obj;

/****************************************************************************
 * Function: uchess.init()
 * 
 * Initialize the uchess library.
 * 
 * Parameters:
 * None
 * 
 * Returns:
 * bool: True if the library was initialized successfully.
 * 
*/

STATIC mp_obj_t uchess_init() {
    memset(&uchess_obj, 0, sizeof(uchess_obj_t));

    strncpy(uchess_obj.fen, START_FEN, 86);
    uchess_obj.depth = 9;
    uchess_obj.move = 0;
    uchess_obj.halfmove = 0;
    uchess_obj.fullmove = 0;
    uchess_obj.side = WHITE;
    return mp_const_true;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_0(uchess_init_obj, uchess_init);

/******************************************************************************
 * Function: uchess.print_bitboard()
 * 
 * Print a bitboard in a human readable format.
 * 
 * Args:
 *  bitboard (int): The bitboard to print.
 * 
 * Returns:
 * None
 * 
 * Example:
 * >>> import uchess
 * >>> uchess.print_bitboard(0x0000000000000000)
 * 
*/

STATIC mp_obj_t uchess_print_bitboard(mp_obj_t bitboard_in) {
    uint64_t bitboard = mp_obj_get_int(bitboard_in);

    //bitboard = 0xffffffffffffffff;

    uint64_t num_bits = 0xffffffffffffffff;
    int bits = 0;
    while(num_bits) {
        num_bits >>= 1;
        
        bits++;
    }
    mp_printf(&mp_plat_print, "num bits: %d\n", bits);

    num_bits = bitboard;
    for (int i = 0; i < bits; i++) {    
        if (num_bits & 0x8000000000000000) {
            mp_printf(&mp_plat_print, "1");
        } else {
            mp_printf(&mp_plat_print, "0");
        }
        num_bits <<= 1;
    }
    mp_printf(&mp_plat_print, "\n");

    for (int rank = 7; rank >= 0; rank--) {
        for (int file = 0; file < 8; file++) {
            if (bitboard & (1UL << (rank * 8 + file))) {
                mp_printf(&mp_plat_print, "x");
            } else {
                mp_printf(&mp_plat_print, ".");
            }
        }
        mp_printf(&mp_plat_print, "\n");
    }

    mp_printf(&mp_plat_print, "%x\n", bitboard);

    return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_1(uchess_print_bitboard_obj, uchess_print_bitboard);

int get_num_bits(void) {
    uint64_t num_bits = 0xffffffffffffffff;
    int bits = 0;
    while(num_bits) {
        num_bits >>= 1;      
        bits++;
    }
    return bits;
}

/******************************************************************************
 * Function: uchess.test_64bit()
 * 
 * Test 64 bit integer support.
 * 
 * Parameters:
 * None
 * 
 * Returns:
 * None
 * 
*/

STATIC mp_obj_t uchess_test_64bit(size_t n_args, const mp_obj_t *args_in) {

    uint64_t test_int = 0;
    #if MICROPY_LONGINT_IMPL == MICROPY_LONGINT_IMPL_LONGLONG
    mp_printf(&mp_plat_print, "MICROPY_LONGINT_IMPL_LONGLONG\n");
    #elif MICROPY_LONGINT_IMPL == MICROPY_LONGINT_IMPL_MPZ
    mp_printf(&mp_plat_print, "MICROPY_LONGINT_IMPL_MPZ\n");
    #else
    mp_printf(&mp_plat_print, "MICROPY_LONGINT_IMPL_NONE\n");
    #endif

    if (n_args == 0) {
        test_int = 0xf0c00c000030c0f0;
        mp_printf(&mp_plat_print, "default hex value: 0xf0c00c000030c0f0\n");
    } else if(n_args == 1) {
        #if MICROPY_LONGINT_IMPL == MICROPY_LONGINT_IMPL_LONGLONG || MICROPY_LONGINT_IMPL == MICROPY_LONGINT_IMPL_MPZ
        test_int = (uint64_t)mp_obj_get_ll_int(args_in[0]);
        mp_printf(&mp_plat_print, "using long long value\n");
        #else
        test_int = (uint64_t)mp_obj_get_int(args_in[0]);
        #endif

        mp_printf(&mp_plat_print, "parameter passed: 0x");
        for(int i = sizeof(test_int)-1; i >= 0; i--) {
            mp_printf(&mp_plat_print, "%02x", ((uint8_t *)&test_int)[i]);
        }
        mp_printf(&mp_plat_print, "\n");
    }

    int bits = get_num_bits();
    mp_printf(&mp_plat_print, "num bits: %d\n", bits);

    uint64_t test_bits = test_int;
    
    mp_printf(&mp_plat_print, "Using left shift with '& 0x8000000000000000 bitwise' mask\n", bits);

    for (int i = 0; i < bits; i++) {    
        if (test_bits & 0x8000000000000000) {
            mp_printf(&mp_plat_print, "1");
        } else {
            mp_printf(&mp_plat_print, "0");
        }
        test_bits <<= 1;
    }
    mp_printf(&mp_plat_print, "\n");
    if (n_args == 0) {
        mp_printf(&mp_plat_print, "Correct expected bit output:\n");
        mp_printf(&mp_plat_print, "1111000011000000000011000000000000000000001100001100000011110000\n", bits);
    }

    mp_printf(&mp_plat_print, "\nUsing right shift with '& 0x0000000000000001 bitwise' mask (expected output should be flipped from above)\n", bits);

    test_bits = test_int;
    for (int i = 0; i < bits; i++) {    
        if (test_bits & 0x0000000000000001) {
            mp_printf(&mp_plat_print, "1");
        } else {
            mp_printf(&mp_plat_print, "0");
        }
        test_bits >>= 1;
    }
    mp_printf(&mp_plat_print, "\n");
    if (n_args == 0) {
        mp_printf(&mp_plat_print, "Correct expected bit output:\n");
        mp_printf(&mp_plat_print, "0000111100000011000011000000000000000000001100000000001100001111\n", bits);
    }
    return mp_obj_new_int_from_ull(test_int);
}

STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(uchess_test_64bit_obj, 0, 1, uchess_test_64bit);


/******************************************************************************
 * Function: uchess.test_64bit_lshift()
 * 
 * Test 64 bit left shift operator.
 * 
 * Parameters:
 * None
 * 
 * Returns:
 * None
 * 
*/

STATIC mp_obj_t uchess_test_64bit_lshift(size_t n_args, const mp_obj_t *args_in) {

    uint64_t test_int = 0;

    if (n_args == 0) {
        test_int = 0xf0c00c000030c0f0;
        mp_printf(&mp_plat_print, "default hex value: 0xf0c00c000030c0f0\n");
    } else if(n_args == 1) {
        #if MICROPY_LONGINT_IMPL == MICROPY_LONGINT_IMPL_LONGLONG || MICROPY_LONGINT_IMPL == MICROPY_LONGINT_IMPL_MPZ
        test_int = (uint64_t)mp_obj_get_ll_int(args_in[0]);
        mp_printf(&mp_plat_print, "using long long value\n");
        #else
        test_int = (uint64_t)mp_obj_get_int(args_in[0]);
        #endif
        mp_printf(&mp_plat_print, "parameter passed: %lu\n", test_int);
    }

    int bits = get_num_bits();
    uint64_t mask;

    mp_printf(&mp_plat_print, "num bits: %d\n", bits);

    uint64_t test_bits = test_int;
    mp_printf(&mp_plat_print, "Using left shift to generate '& bitwise' mask\n", bits);
    for (int i = bits-1; i >= 0; i--) {   
        mask = 1UL << i; 
        if (test_bits & mask) {
            mp_printf(&mp_plat_print, "1");
        } else {
            mp_printf(&mp_plat_print, "0");
        }
    }
    mp_printf(&mp_plat_print, "\n");
    if (n_args == 0) {
        mp_printf(&mp_plat_print, "Correct expected bit output:\n");
        mp_printf(&mp_plat_print, "1111000011000000000011000000000000000000001100001100000011110000\n", bits);
    }

    return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(uchess_test_64bit_lshift_obj, 0, 1, uchess_test_64bit_lshift);

// /******************************************************************************
//  * Function: uchess.test_64bit_rshift()
//  * 
//  * Test 64 bit right shift operator.
//  * 
//  * Parameters:
//  * None
//  * 
//  * Returns:
//  * None
//  * 
// */

// STATIC mp_obj_t uchess_test_64bit_rshift(mp_obj_t int_in) {
//     uint64_t test_int = mp_obj_get_int(int_in);

//     uint64_t num_bits = 0xffffffffffffffff;
//     int bits = 0;
//     while(num_bits) {
//         num_bits >>= 1;
        
//         bits++;
//     }
//     mp_printf(&mp_plat_print, "num bits: %d\n", bits);

//     uint64_t test_bits = test_int;
//     for (int i = 0; i < bits; i++) {    
//         if (test_bits & (0x8000000000000000 >> i)) {
//             mp_printf(&mp_plat_print, "1");
//         } else {
//             mp_printf(&mp_plat_print, "0");
//         }
//     }
//     mp_printf(&mp_plat_print, "\n");

//     return mp_const_none;
// }

// STATIC MP_DEFINE_CONST_FUN_OBJ_1(uchess_test_64bit_rshift_obj, uchess_test_64bit_rshift);


/******************************************************************************
 * Function: get_depth()
 * 
 * Get the current search depth.
 * 
 * Parameters:
 * None
 * 
 * Returns:
 * int: The current search depth.
 *
*/

STATIC mp_obj_t uchess_get_depth() {
    return mp_obj_new_int(uchess_obj.depth);
}

STATIC MP_DEFINE_CONST_FUN_OBJ_0(uchess_get_depth_obj, uchess_get_depth);


/******************************************************************************
 * Function: set_depth()
 * 
 * Set the current search depth.
 * 
 * Parameters:
 * depth (int): The new search depth.
 * 
 * Returns:
 * None
 * 
*/

STATIC mp_obj_t uchess_set_depth(mp_obj_t depth_in) {
    uchess_obj.depth = mp_obj_get_int(depth_in);
    return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_1(uchess_set_depth_obj, uchess_set_depth);

mp_obj_t mpy_init(mp_obj_fun_bc_t *self, size_t n_args, size_t n_kw, mp_obj_t *args) {
    MP_DYNRUNTIME_INIT_ENTRY

    mp_store_global(MP_QSTR___name__, MP_OBJ_NEW_QSTR(MP_QSTR_uchess));
    mp_store_global(MP_QSTR_init, MP_OBJ_FROM_PTR(&uchess_init_obj));
    mp_store_global(MP_QSTR_print_bitboard, MP_OBJ_FROM_PTR(&uchess_print_bitboard_obj));
    mp_store_global(MP_QSTR_get_depth, MP_OBJ_FROM_PTR(&uchess_get_depth_obj));
    mp_store_global(MP_QSTR_set_depth, MP_OBJ_FROM_PTR(&uchess_set_depth_obj));
    mp_store_global(MP_QSTR_test_64bit, MP_OBJ_FROM_PTR(&uchess_test_64bit_obj));
    mp_store_global(MP_QSTR_test_64bit_lshift, MP_OBJ_FROM_PTR(&uchess_test_64bit_lshift_obj));
    // mp_store_global(MP_QSTR_test_64bit_rshift, MP_OBJ_FROM_PTR(&uchess_test_64bit_rshift_obj));


    MP_DYNRUNTIME_INIT_EXIT
}
