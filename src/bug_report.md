## Issue:
I'm writing a native dynamic module in C.  It appears the code is not being executed properly on a 32-bit architecture (ESP32 for example) when using 64-bit integer.  It works fine when running on 64-bit linux. I'm writing a chess engine wrapper which uses 64-bit bitboard for board positions, so it is important that it works on ESP32.

I ran into some issues passing a 64-bit integer to a native module function and passed values being parsed into 32-bit. However, ESP32 does support 64-bit integer as evident shown below.  Looks like the Micropython native module functions are not converting numbers correctly.

If i assign a large value (e.g. 0xf0c00c000030c0f0) to a python variable as integer, it is stored correctly and output correctly.  But if I pass that value into a function, it converted to a 32-bit integer instead of 64-bit integer. Even casting it to `uint64_t` doesn't do a thing.

## Example testing output:
```python
MPY: soft reboot
MicroPython 699477d12 on 2022-12-28; ESP32 module (spiram) with ESP32
Type "help()" for more information.
>>> import uchess
>>> uchess.test_64bit()
default hex value: 0xf0c00c000030c0f0
num bits: 64
Using left shift with '& 0x8000000000000000 bitwise' mask
1111000011000000000011000000000000000000001100001100000011110000
Correct expected bit output:
1111000011000000000011000000000000000000001100001100000011110000

Using right shift with '& 0x0000000000000001 bitwise' mask (expected output should be flipped from above)
0000111100000011000011000000000000000000001100000000001100001111
Correct expected bit output:
0000111100000011000011000000000000000000001100000000001100001111
>>> a = 0xf0c00c000030c0f0
>>> a
17347878958773879024
>>> hex(a)
'0xf0c00c000030c0f0'
>>> uchess.test_64bit(a)
parameter passed: 3195120
num bits: 64
Using left shift with '& 0x8000000000000000 bitwise' mask
0000000000000000000000000000000000000000001100001100000011110000

Using right shift with '& 0x0000000000000001 bitwise' mask (expected output should be flipped from above)
0000111100000011000011000000000000000000000000000000000000000000
>>> uchess.test_64bit_lshift()
default hex value: 0xf0c00c000030c0f0
num bits: 64
Using left shift to generate '& bitwise' mask
0000000000110000110000001111000000000000001100001100000011110000
Correct expected bit output:
1111000011000000000011000000000000000000001100001100000011110000
>>> uchess.test_64bit_lshift(a)
parameter passed: 3195120
num bits: 64
Using left shift to generate '& bitwise' mask
0000000000110000110000001111000000000000001100001100000011110000
>>> 
```

The above output is produced with the following code snippet below.  For the `uchess.test_64bit()` function to test 64-bit integer functionality in ESP32 as a native module function.  Without a parameter, it uses a default value, otherwise it is parsed with `mp_obj_get_int` function.  It uses a fixed bitwise mask to test a bit while shifting the 64-bit integer to either left or right.

```C
STATIC mp_obj_t uchess_test_64bit(size_t n_args, const mp_obj_t *args_in) {

    uint64_t test_int = 0;

    if (n_args == 0) {
        test_int = 0xf0c00c000030c0f0;
        mp_printf(&mp_plat_print, "default hex value: 0xf0c00c000030c0f0\n");
    } else if(n_args == 1) {
        test_int = (uint64_t)mp_obj_get_int(args_in[0]);
        mp_printf(&mp_plat_print, "parameter passed: %lu\n", test_int);
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
    return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(uchess_test_64bit_obj, 0, 1, uchess_test_64bit);
```

In this code snippet, `uchess.test_64bit_lshift()` uses left shift `<<` bit operation to generate a mask for a `&` bitwise operation test.  It would function properly if `1ULL << i` is used, however, when compiled, it throws a linking error. It is unable to link `__ashldi3` symbol.  Which I guess this particular function handles 64-bit integer left/right shift operation.  I also have seen the same link error with the `__lshldi3` symbol as well.

## Compile error when using 1ULL
```bash
josh@upython-dev:~/uchess/src$ make ARCH=xtensawin V=1
GEN build/uchess.config.h
python3 ../micropython/tools/mpy_ld.py '-vvv' --arch xtensawin --preprocess -o build/uchess.config.h helper.c uchess.c
CC helper.c
xtensa-esp32-elf-gcc -I. -I../micropython -std=c99 -Os -Wall -Werror -DNDEBUG -DNO_QSTR -DMICROPY_ENABLE_DYNRUNTIME -DMP_CONFIGFILE='<build/uchess.config.h>' -fpic -fno-common -U _FORTIFY_SOURCE  -DMICROPY_FLOAT_IMPL=MICROPY_FLOAT_IMPL_FLOAT  -o build/helper.o -c helper.c
CC uchess.c
xtensa-esp32-elf-gcc -I. -I../micropython -std=c99 -Os -Wall -Werror -DNDEBUG -DNO_QSTR -DMICROPY_ENABLE_DYNRUNTIME -DMP_CONFIGFILE='<build/uchess.config.h>' -fpic -fno-common -U _FORTIFY_SOURCE  -DMICROPY_FLOAT_IMPL=MICROPY_FLOAT_IMPL_FLOAT  -o build/uchess.o -c uchess.c
LINK build/helper.o
python3 ../micropython/tools/mpy_ld.py '-vvv' --arch xtensawin --qstrs build/uchess.config.h -o build/uchess.native.mpy build/helper.o build/uchess.o
qstr vals: get_depth, init, print_bitboard, set_depth, test_64bit, test_64bit_lshift, test_64bit_rshift
qstr objs: uchess
LinkError: build/uchess.o: undefined symbol: __ashldi3
make: *** [../micropython/py/dynruntime.mk:150: build/uchess.native.mpy] Error 1
josh@upython-dev:~/uchess/src$ 
```
So I used `1UL` instead and it compiled properly but gives wrong results as seen above because it is 32-bit integer, so it couldn't do proper bit test operation.

## Code using shift operator to generate bitwise operation mask
```C
STATIC mp_obj_t uchess_test_64bit_lshift(size_t n_args, const mp_obj_t *args_in) {

    uint64_t test_int = 0;

    if (n_args == 0) {
        test_int = 0xf0c00c000030c0f0;
        mp_printf(&mp_plat_print, "default hex value: 0xf0c00c000030c0f0\n");
    } else if(n_args == 1) {
        test_int = (uint64_t)mp_obj_get_int(args_in[0]);
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
```

I would expect it to properly parse 64-bit integer on ESP32 as it is evident it can handle 64-bit integer as shown in the REPL output, but when using with custom written native module, it becomes 32-bit.

The native module works properly when executed on 64-bit platform like Linux with `1ULL`.

## Output on 64-bit architecture
```python
MicroPython 699477d12 on 2022-12-28; linux [GCC 11.3.0] version
Use Ctrl-D to exit, Ctrl-E for paste mode
>>> import uchess
>>> uchess.test_64bit()
default hex value: 0xf0c00c000030c0f0
num bits: 64
Using left shift with '& 0x8000000000000000 bitwise' mask
1111000011000000000011000000000000000000001100001100000011110000
Correct expected bit output:
1111000011000000000011000000000000000000001100001100000011110000

Using right shift with '& 0x0000000000000001 bitwise' mask (expected output should be flipped from above)
0000111100000011000011000000000000000000001100000000001100001111
Correct expected bit output:
0000111100000011000011000000000000000000001100000000001100001111
>>> a = 0xf0c00c000030c0f0
>>> a
17347878958773879024
>>> uchess.test_64bit(a)
parameter passed: 17347878958773879024
num bits: 64
Using left shift with '& 0x8000000000000000 bitwise' mask
1111000011000000000011000000000000000000001100001100000011110000

Using right shift with '& 0x0000000000000001 bitwise' mask (expected output should be flipped from above)
0000111100000011000011000000000000000000001100000000001100001111
>>> uchess.test_64bit_lshift()
default hex value: 0xf0c00c000030c0f0
num bits: 64
Using left shift to generate '& bitwise' mask
1111000011000000000011000000000000000000001100001100000011110000
Correct expected bit output:
1111000011000000000011000000000000000000001100001100000011110000
>>> uchess.test_64bit_lshift(a)
parameter passed: 17347878958773879024
num bits: 64
Using left shift to generate '& bitwise' mask
1111000011000000000011000000000000000000001100001100000011110000
>>> 
```

I think this issue can be fixed if it is able to link to `__ashldi3` or `__lashldi3` symbol at compile time.

* If you are seeing code being executed incorrectly, please provide a minimal example and expected output (e.g. comparison to CPython).

* For build issues, please include full details of your environment, compiler versions, command lines, and build output.

* Please provide as much information as possible about the version of MicroPython you're running, such as:
 - firmware: custom compiled used standard config for xtensawin
 - firmware file name
 - git commit hash and port/board: hash 699477d12, port: esp32-spiram
 - version information shown in the REPL (hit Ctrl-B to see the startup message)
 ```
MPY: soft reboot
MicroPython 699477d12 on 2022-12-28; ESP32 module (spiram) with ESP32
Type "help()" for more information.
```

* Remove all placeholder text above before submitting.
