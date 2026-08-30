.POSIX:

TARGET=gasmic
OBJS=main.o as.o lang.o \
	 smem.o arithmetic.o pseudo.o \
	 formats/bin.o \
	 syntax/common.o syntax/motorola.o \
	 targets/6x09/arch.o
#targets/z80/arch.o

MAIN_CFLAGS=-Wall -Werror -std=c99 -D_POSIX_C_SOURCE=200112L
CFLAGS=$(MAIN_CFLAGS) -DNDEBUG=1
DBG_CFLAGS=$(MAIN_CFLAGS) -g

.c.o:
	$(CC) $(CFLAGS) -o $@ -c $<

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $(TARGET) $(OBJS)

$(TARGET).dbg:
	$(MAKE) clean $(TARGET) CFLAGS="$(DBG_CFLAGS)" LDFLAGS="$(LDFLAGS) -g"
	mv $(TARGET) $(TARGET).dbg

clean:
	rm -f $(TARGET) $(TARGET).dbg $(OBJS)

