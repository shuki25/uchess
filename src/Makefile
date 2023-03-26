# Location of top-level MicroPython directory
MPY_DIR = ../micropython

# Name of module
MOD = uchess

# Source files (.c or .py)
SRC = helper.c uchess.c

# Architecture to build for (x86, x64, armv7m, xtensa, xtensawin)
ARCH = xtensawin

# Options to pass compiler
CFLAGS += -DMICROPY_LONGINT_IMPL=MICROPY_LONGINT_IMPL_MPZ

# Include to get the rules for compiling and linking the module
include $(MPY_DIR)/py/dynruntime.mk

# LIBS += $(shell $(CC) $(CFLAGS) -print-libgcc-file-name)
