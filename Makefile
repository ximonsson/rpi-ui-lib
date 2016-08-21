CC       = gcc
AR       = ar
SRCDIR   = src
BUILD    = build
LIB      = lib
LIBRARY  = $(LIB)/librpi_ui.a
SRC      = ui.c widget.c image.c matrix.c queue.c
OBJ      = $(addprefix $(BUILD)/, $(SRC:.c=.o))
BIN      = bin
VC       = /opt/vc
PLAYER   = ../rpi-mediaplayer
TESTS    = matrix queue
EXAMPLES = basic video

ifdef VERBOSE
CFLAGS += -v
endif

# if we are cross compiling
ifdef CROSS
CC       = arm-linux-gnueabihf-gcc
AR       = arm-linux-gnueabihf-ar
SYSROOT  = $(HOME)/mnt/penguin
VC       = $(SYSROOT)/opt/vc
CFLAGS  += -mfloat-abi=hard -march=armv6 -mfpu=vfp -marm --sysroot=$(SYSROOT)
endif


CFLAGS += -Wall \
          -O3 \
          -fPIC \
          -ftree-vectorize \
          -pipe \
          -Wno-psabi \
          -DSTANDALONE \
          -D__STDC_CONSTANT_MACROS \
          -D__STDC_LIMIT_MACROS \
          -DTARGET_POSIX \
          -D_LINUX \
          -fPIC \
          -DPIC \
          -D_REENTRANT \
          -D_LARGEFILE64_SOURCE \
          -D_FILE_OFFSET_BITS=64 \
          -U_FORTIFY_SOURCE \
          -DHAVE_LIBOPENMAX=2 \
          -DOMX \
          -DOMX_SKIP64BIT \
          -ftree-vectorize \
          -pipe \
          -DUSE_EXTERNAL_OMX \
          -DHAVE_LIBBCM_HOST \
          -DUSE_EXTERNAL_LIBBCM_HOST \
          -DUSE_VCHIQ_ARM \
          -Wno-psabi

INCLUDES = -I./include \
           -I$(PLAYER)/include \
           -I$(VC)/include \
           -I$(VC)/include/interface/vcos/pthreads \
           -I$(VC)/include/interface/vmcs_host/linux \
           -I$(VC)/src/hello_pi/libs/ilclient \
           -I$(VC)/src/hello_pi/libs/vgfont

LIBRARY_PATH = -L./lib \
               -L$(PLAYER)/lib \
               -L$(VC)/lib \
               -L$(VC)/src/hello_pi/libs/ilclient \
               -L$(VC)/src/hello_pi/libs/vgfont

LIBS += -lrpi_ui \
        -lrpi_mp \
        -lilclient \
        -lopenmaxil \
        -lbcm_host \
        -lGLESv2 \
        -lEGL \
        -lvcos \
        -lvchiq_arm \
        -lpthread \
        -lrt \
        -lavcodec \
        -lavutil \
        -lavformat \
        -lm

ARCMD = rcs


all: lib

lib: $(OBJ)
	@mkdir -p $(@D)
	$(AR) $(ARCMD) $(LIBRARY) $^

$(BIN)/test_%: test/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $< $(LIBRARY_PATH) $(LIBS)

$(BIN)/example_%: example/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $< $(LIBRARY_PATH) $(LIBS)

$(BUILD)/%.o: $(SRCDIR)/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

# PHONY targets
.PHONY: $(BIN)/test_% $(BIN)/example_% clean install

install:
	@scp $(BIN)/* penguin:projects/rpi-ui-lib/bin/
	@scp $(SRCDIR)/shaders/* penguin:projects/rpi-ui-lib/src/shaders/

# create tests
tests: lib $(addprefix $(BIN)/test_, $(TESTS))


examples: lib $(addprefix $(BIN)/example_, $(EXAMPLES))

clean:
	rm -rf $(BUILD)/*.o $(BIN)/* $(LIBRARY)
