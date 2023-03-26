#include "helper.h"
#include <stdio.h>

char *strncpy(char *dest, const char *src, size_t n) {

    char *p = dest;
    while (n > 0 && *src != '\0') {
        *p++ = *src++;
        n--;
    }

    *p = '\0';

    return dest;
}

uint64_t mp_obj_get_ll_int(mp_obj_t arg_in) {
    if (arg_in == mp_const_false) {
        return 0;
    } else if (arg_in == mp_const_true) {
        return 1;
    } else if (mp_obj_is_small_int(arg_in)) {
        return MP_OBJ_SMALL_INT_VALUE(arg_in);
    } else if (mp_obj_is_type(arg_in, &mp_type_int)) {
    #if MICROPY_LONGINT_IMPL == MICROPY_LONGINT_IMPL_LONGLONG
        mp_obj_int_t *arg = arg_in;
        return arg->val;
    #elif MICROPY_LONGINT_IMPL == MICROPY_LONGINT_IMPL_MPZ
        // mp_printf(&mp_plat_print, "translating long long integer\n");
        mp_obj_int_t *arg = arg_in;
        int len = arg->mpz.len;
        uint64_t res = 0;
        for (int i = len - 1; i >= 0; --i) {
            res = (res << MPZ_DIG_SIZE) + arg->mpz.dig[i];
        }
        if (arg->mpz.neg) {
            return -res;
        }
        return res;
    #endif
    } else {
        // mp_raise_ValueError(MP_ERROR_TEXT("expected integer\n"));
        return 0;
    }
    return 0;
}
