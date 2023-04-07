CC=gcc
CFLAGS=-Wall -Wextra -Werror -std=c11 -g -O2

PREFIX ?= /usr/local

INCLUDES=-Iinclude -Isrc
SRCDIR=src
TESTDIR=tests

SRC=$(wildcard $(SRCDIR)/*.c)
OBJ=$(SRC:.c=.o)
TARGET=libhalloc.a

TEST_SRC=$(wildcard $(TESTDIR)/*.c)
TEST_OBJ=$(TEST_SRC:.c=.o)
TEST_TARGET=halloc_test

.PHONY: all clean test install uninstall help

all: $(TARGET) $(TEST_TARGET)

$(OBJ): %.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(TARGET): $(OBJ)
	ar rcs $@ $^

$(TEST_OBJ): %.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(TEST_TARGET): $(OBJ) $(TEST_OBJ)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $^

test: $(TEST_TARGET)
	./$(TEST_TARGET)

install: $(TARGET)
	install -d $(PREFIX)/lib/
	install $(TARGET) $(PREFIX)/lib/
	install -d $(PREFIX)/include/halloc/
	install include/halloc.h $(PREFIX)/include/halloc/
	rm -f $(OBJ) $(TARGET)

uninstall:
	rm -f $(PREFIX)/lib/$(TARGET)
	rm -rf $(PREFIX)/include/halloc/

clean:
	rm -f $(OBJ) $(TEST_OBJ) $(TEST_TARGET)

help:
	@echo "Available targets:"
	@echo "  all:           Build library and test executable"
	@echo "  test:          Run test executable"
	@echo "  install:       Install library and header files to system directories specified by PREFIX"
	@echo "  uninstall:     Remove files installed by the 'install' target"
	@echo "  clean:         Remove object files and test executable"
	@echo "  help:          Display this help message"
