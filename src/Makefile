.POSIX:

TARGET=gasmic
OBJS=main.o as.o lang.o \
	 smem.o arithmetic.o pseudo.o \
	 formats/bin.o \
	 syntax/common.o syntax/motorola.o \
	 targets/6x09/arch.o
	 #targets/z80/arch.o

CFLAGS=-Wall -Werror -std=c99

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -DNDEBUG=1 -o $(TARGET) $(OBJS)

$(TARGET).dbg: $(OBJS)
	$(CC) $(LDFLAGS) -g -o $(TARGET) $(OBJS)

clean:
	rm -f $(TARGET) $(TARGET.dbg) $(OBJS)

