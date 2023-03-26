#ifndef __HELPER_H__
#define __HELPER_H__

#include <stddef.h> // for size_t
#include <stdint.h> // for uint64_t
#include "py/obj.h"
#include "py/objint.h"
#include "py/mpz.h"
#include "py/dynruntime.h"

char *strncpy(char *dest, const char *src, size_t n) __attribute__((nonnull(1, 2)));
uint64_t mp_obj_get_ll_int(mp_obj_t arg_in);

#endif