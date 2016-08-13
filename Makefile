CC		= gcc
AR      = ar
SRCDIR	= src
BUILD	= build
LIB     = lib
LIBRARY = $(LIB)/librpi_ui.a
SRC		= ui.c widgets.c utils.c
OBJ		= $(addprefix $(BUILD)/, $(SRC:.c=.o))
EXEC	= bin/gui

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
		  -U_FORTIFY_SOURCE
		  -DHAVE_LIBOPENMAX=2 \
		  -DOMX \
		  -DOMX_SKIP64BIT \
		  -ftree-vectorize \
		  -pipe \
		  -DUSE_EXTERNAL_OMX \
		  -DHAVE_LIBBCM_HOST \
		  -DUSE_EXTERNAL_LIBBCM_HOST \
		  -DUSE_VCHIQ_ARM \
		  -Wno-psabi \

INCLUDES = -I./include \
           -I/opt/vc/include \
           -I/opt/vc/include/interface/vcos/pthreads \
           -I/opt/vc/include/interface/vmcs_host/linux \
           -I/opt/vc/src/hello_pi/libs/ilclient \
           -I/opt/vc/src/hello_pi/libs/vgfont \

LIB_PATH = -L./lib/ \
           -L/opt/vc/lib/ \
           -L/opt/vc/src/hello_pi/libs/ilclient \
           -L/opt/vc/src/hello_pi/libs/vgfont

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

test:
	$(CC) $(CFLAGS) $(INCLUDES) -o test.out triangle.c $(LIB_PATH) $(LIBS)

$(BUILD)/%.o: $(SRCDIR)/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

$(LIBRARY): $(OBJ)
	@mkdir -p $(@D)
	$(AR) $(ARFLAGS) $@ $^

$(EXEC): lib
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $(SRCDIR)/main.c $(LIB_PATH) $(LIBS)

clean:
	rm -rf $(BUILD)/*.o $(EXEC) $(LIBRARY)
