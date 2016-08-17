CC      = gcc
AR      = ar
SRCDIR  = src
BUILD   = build
LIB     = lib
LIBRARY = $(LIB)/librpi_ui.a
SRC     = ui.c widget.c image.c
OBJ     = $(addprefix $(BUILD)/, $(SRC:.c=.o))
EXEC    = bin/gui
VC      = /opt/vc

ifdef VERBOSE
CFLAGS += -v
endif

# if we are cross compiling
ifdef CROSS
CC      = arm-linux-gnueabihf-gcc
AR      = arm-linux-gnueabihf-ar
SYSROOT = $(HOME)/mnt/penguin
CFLAGS += -mfloat-abi=hard -march=armv6 -mfpu=vfp -marm --sysroot=$(SYSROOT)
VC      = $(SYSROOT)/opt/vc
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
           -I$(VC)/include \
           -I$(VC)/include/interface/vcos/pthreads \
           -I$(VC)/include/interface/vmcs_host/linux \
           -I$(VC)/src/hello_pi/libs/ilclient \
           -I$(VC)/src/hello_pi/libs/vgfont

LIBRARY_PATH = -L./lib \
               -L$(VC)/lib \
               -L$(VC)/src/hello_pi/libs/ilclient \
               -L$(VC)/src/hello_pi/libs/vgfont

LIBS += -lilclient \
        -lopenmaxil \
        -lbcm_host \
        -lGLESv2 \
        -lEGL \
        -lvcos \
        -lvchiq_arm \
        -lpthread \
        -lrt \
        -lm \
        -lrpi_ui

ARFLAGS = rcs


all: lib

lib: $(LIBRARY)

bin: $(EXEC)

$(LIBRARY): $(OBJ)
	@mkdir -p $(@D)
	$(AR) $(ARFLAGS) $@ $^

$(EXEC): lib
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ main.c $(LIBRARY_PATH) $(LIBS)

$(BUILD)/%.o: $(SRCDIR)/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

clean:
	rm -rf $(BUILD)/*.o $(EXEC) $(LIBRARY)
