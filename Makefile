CC=gcc
CFLAGS=-Wall -Wextra -Werror -std=c11 -g

SRC=src/dll.c src/memtools.c src/halloc.c
OBJS=dll.o memtools.o halloc.o
TARGET=libhalloc.a

TEST_SRC=tests/test_dll.c tests/test_memtools.c tests/test_halloc.c tests/test_main.c
TEST_OBJS=test_dll.o test_memtools.o test_halloc.o test_main.o
TEST_TARGET=test_main

.PHONY:all clean test

all: $(TARGET) $(TEST_TARGET)

$(OBJS): $(SRC)
	$(CC) $(CFLAGS) -c -Isrc/ $(SRC)

$(TEST_OBJS): $(TEST_SRC)
	$(CC) $(CFLAGS) -c -Isrc/ $(TEST_SRC)

$(TEST_TARGET): $(OBJS) $(TEST_OBJS)
	$(CC) -o $(TEST_TARGET) $(OBJS) $(TEST_OBJS)

$(TARGET): $(OBJS)
	ar rcs $(TARGET) $(OBJS)

test:
	./$(TEST_TARGET)

clean:
	rm -f $(OBJS)
	rm -f $(TEST_OBJS)
	rm -f $(TEST_TARGET)
