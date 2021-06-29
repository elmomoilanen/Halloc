# Halloc - heap memory allocator #

This library implements a custom and experimental dynamic memory allocator `Halloc` in C that somewhat resembles the C standard library function calloc. Halloc uses `mmap()` to create memory mappings in the virtual address space such that the mappings are anonymous and thus not backed by files. For successful mappings, halloc returns a pointer to starting address of the new memory allocation and all bytes of the allocation are initialised to 0. One-time memory allocations are supported roughly up to 1 GiB by variating the length of mmap's memory mappings. Halloc library provides various memory statistics to be used alongside its main components that allocate and deallocate memory.

## Build ##

Build the library and run tests with the following command in a shell

```bash
make && make test && make clean
```

In case of build failure or tests not passing, this library may not be usable in your system "as is". Library uses C11 standard and is designed mainly for Linux. 

## Usage ##

Header file `halloc.h` defines public APIs for the library. Use `halloc()` to request new memory allocation and `hfree()` to deallocate this memory. In addition, there are three functions to provide detailed memory statistics for total or type specific memory usage.

Following code section gives an example use case of halloc, containing code necessary only to halloc. The code snippet shows what makes the halloc library interesting, which is the mentioned availability of couple of virtual memory usage statistics, each beginning with the "halloc_print" prefix.

```C
#include "halloc.h"

typedef struct {
  char name[25];
  long values[10];
} typeX;

int main() {
  typeX *p = halloc(typeX, 10);
  
  assert(p != NULL);
  
  halloc_print_saved_page_items();
  
  halloc_print_total_memory_usage();
  halloc_print_type_memory_usage(typeX);
  
  hfree(p);
  
  halloc_print_total_memory_usage();
}
```

When using halloc library in your C program, remember to indicate the compiler the include path (-I) for the header file `halloc.h` and library path (-L) and name (-l) for the shared library file `libhalloc.a`. E.g. a shell command

```bash
gcc test_prog.c -Iinclude -L. -lhalloc -o test_prog -Wall -Wextra -Werror -std=c11 -g
```

would compile a `test_prog.c` source code file placed in root directory of the halloc library.
