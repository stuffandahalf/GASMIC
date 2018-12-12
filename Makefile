ifndef TARGET
TARGET=6x09
endif

SOURCE_DIR=src
BUILD_DIR=build
DIST_DIR=dist

TARGET_DIR=$(SOURCE_DIR)/targets/$(TARGET)

CC=cc
CFLAGS=-c -I$(SOURCE_DIR) -I$(TARGET_DIR)
LFLAGS=

_OBJS=main.o arch.o
OBJS=$(patsubst %,$(BUILD_DIR)/%,$(_OBJS))

.PHONY: all
all: $(DIST_DIR)/as-$(TARGET)

.PHONY: run
run: $(DIST_DIR)/as-$(TARGET)
	$<

.PHONY: debug
debug: $(DIST_DIR)/as-$(TARGET)
	valgrind --leak-check=full --track-origins=yes $<

$(BUILD_DIR)/%.o: $(TARGET_DIR)/%.c $(TARGET_DIR)/%.h
	$(CC) $(CFLAGS) -o $@ $<

$(BUILD_DIR)/%.o: $(SOURCE_DIR)/%.c
	$(CC) $(CFLAGS) -o $@ $<

$(DIST_DIR)/as-$(TARGET): $(BUILD_DIR) $(OBJS) $(DIST_DIR)
	$(CC) $(LFLAGS) -o $@ $(OBJS)

$(BUILD_DIR):
	mkdir $(BUILD_DIR)

$(DIST_DIR):
	mkdir $(DIST_DIR)

.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)
	rm -rf $(DIST_DIR)
