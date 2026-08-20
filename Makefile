TARGET = routefix
OBJS = main.o

CFLAGS = -O2 -G0 -Wall
ASFLAGS = $(CFLAGS)

BUILD_PRX = 1
PRX_EXPORTS = exports.exp
PSP_FW_VERSION = 661

PSPSDK := $(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build_prx.mak
