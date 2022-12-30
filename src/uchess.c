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
#include "vice-src/defs.h"

#if !defined(__linux__)
void *memset(void *s, int c, size_t n) {
    return mp_fun_table.memset_(s, c, n);
}
#endif

typedef struct _uchess_obj_t {
    mp_obj_base_t base;
    mp_obj_t buf_obj; // neet to store this to prevent GC from collecting it
    char fen[87];
    int32_t depth;
    int32_t move;
    int32_t halfmove;
    int32_t fullmove;
    int32_t side;
} uchess_obj_t;

mp_obj_full_type_t uchess_type;

STATIC mp_obj_t uchess_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    mp_arg_check_num(n_args, n_kw, 0, 0, false);
    
    uchess_obj_t *self = mp_obj_malloc(uchess_obj_t, type);
    self->buf_obj = args[0];
    
    
    // strcpy(self->fen, START_FEN);
    self->depth = 9;
    self->move = 0;
    self->halfmove = 0;
    self->fullmove = 0;
    self->side = WHITE;
    return MP_OBJ_FROM_PTR(self);
}

STATIC void uchess_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    uchess_obj_t *self = MP_OBJ_TO_PTR(self_in);
    mp_printf(print, "uchess(depth=%d, move=%d, halfmove=%d, fullmove=%d, side=%d)", self->depth, self->move, self->halfmove, self->fullmove, self->side);
}

// Class methods

STATIC mp_obj_t uchess_depth(mp_obj_t self_in) {
    uchess_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return mp_obj_new_int(self->depth);
}
MP_DEFINE_CONST_FUN_OBJ_1(uchess_depth_obj, uchess_depth);

/******************************************************************************
 * Method: uchess.print_bitboard()
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

STATIC mp_obj_t uchess_print_bitboard(mp_obj_t self_in, mp_obj_t bitboard_in) {
    // uchess_obj_t *self = MP_OBJ_TO_PTR(self_in);
    // uint64_t bitboard = mp_obj_get_int(bitboard_in);
    uint64_t bitboard = mp_obj_get_int(bitboard_in);

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

    return mp_const_none;
}

MP_DEFINE_CONST_FUN_OBJ_2(uchess_print_bitboard_obj, uchess_print_bitboard);

mp_map_elem_t uchess_locals_dict_table[2]; 
STATIC MP_DEFINE_CONST_DICT(uchess_locals_dict, uchess_locals_dict_table);

// const mp_obj_type_t uchess_type = {
//     { &mp_type_type },
//     .name = MP_QSTR_uchess,
//     .print = uchess_print,
//     .make_new = uchess_make_new,
//     .locals_dict = (mp_obj_dict_t*)&uchess_locals_dict,
// };

mp_obj_t mpy_init(mp_obj_fun_bc_t *self, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    MP_DYNRUNTIME_INIT_ENTRY

    mp_store_global(MP_QSTR_uchess, MP_OBJ_FROM_PTR(&uchess_type));
    uchess_type.base.type = (void*)&mp_type_type;
    uchess_type.name = MP_QSTR_uchess;
    MP_OBJ_TYPE_SET_SLOT(&uchess_type, make_new, uchess_make_new, 0);
    MP_OBJ_TYPE_SET_SLOT(&uchess_type, print, uchess_print, 1);
    uchess_locals_dict_table[0] = (mp_map_elem_t){ MP_OBJ_NEW_QSTR(MP_QSTR_depth), MP_OBJ_FROM_PTR(&uchess_depth_obj) };
    uchess_locals_dict_table[1] = (mp_map_elem_t){ MP_OBJ_NEW_QSTR(MP_QSTR_print_bitboard), MP_OBJ_FROM_PTR(&uchess_print_bitboard_obj) };
    MP_OBJ_TYPE_SET_SLOT(&uchess_type, locals_dict, (void*)&uchess_locals_dict, 2);

    mp_store_global(MP_QSTR_uchess, MP_OBJ_FROM_PTR(&uchess_type));

    MP_DYNRUNTIME_INIT_EXIT
}
