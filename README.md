# Halloc - heap memory allocator #

[![main](https://github.com/elmomoilanen/Halloc/actions/workflows/main.yml/badge.svg)](https://github.com/elmomoilanen/Halloc/actions/workflows/main.yml)

Welcome to `Halloc`, a custom dynamic memory allocator implemented in C. This library provides a public API that resembles the standard library function `calloc`. Halloc is built internally using doubly linked lists and leverages the `mmap` system call to create anonymous memory mappings in the virtual address space. Each memory allocation returned by Halloc is initialized to zero, ensuring a consistent and predictable initial state for new memory allocations.

Library is designed to support the size of a single allocation up to approximately 1 GiB, achieved by adjusting the length of the memory mappings created by the mmap system call. Alongside the primary memory allocation and deallocation functions, the library also provides various memory statistics to be used. For more information, please refer to the *Usage* section below.

Be cautious when using this library with multithreaded code, as it has not been thoroughly tested in this manner.

## Build ##

The library is expected to work on most common Linux distros (e.g. Ubuntu). Library uses the C11 standard.

To build the library, run unit tests, and clean up unneeded object files, run the following command

```bash
make && make test && make clean
```

If the build is successful, a static library file named `libhalloc.a` will be created in the current directory.

Optionally to the previous combined make command, the following command installs the library and header file in the system directories specified by the PREFIX variable, which defaults to /usr/local in the Makefile

```bash
make install
```

To uninstall, run the command

```bash
make uninstall
```

This library is a bit experimental in its nature and hence in case of a build failure or tests not passing, the library may not be directly usable in your system.

## Usage ##

Header file *include/halloc.h* defines the public API for the library.

Use `halloc()` to request a new memory allocation and `hfree()` to deallocate previously allocated memory. In addition, there are three functions to provide detailed memory statistics for total or type specific memory usage.

Following code section gives an example use case of the library. The code snippet illustrates functionality that makes the library interesting: possibility to inspect virtual memory usage statistics via three different functions, each beginning with the "halloc_print" prefix.

```C
#include <assert.h>
#include <stdlib.h>

#include "halloc.h"

typedef struct {
  double *data;
  unsigned int size;
} typeX;

int main() {
  // Allocate memory for a struct `typeX`
  typeX *ptr = halloc(typeX, 1);
  assert(ptr != NULL);

  ptr->size = 25;
  // Allocate memory for data member of the struct
  ptr->data = halloc(double, ptr->size);
  assert(ptr->data != NULL);

  ptr->data[ptr->size - 1] = 11.0;

  // Inspect runtime virtual memory usage
  halloc_print_saved_page_items();
  halloc_print_total_memory_usage();

  halloc_print_type_memory_usage(typeX);
  halloc_print_type_memory_usage(double);

  // Nested allocations must be freed first to prevent memory leaks
  hfree(ptr->data);
  hfree(ptr);
  
  halloc_print_total_memory_usage();
}
```

To compile a source code file that uses this library, specify the include path for the header file halloc.h with the -I flag, and the library path and name for the static library file libhalloc.a with the -L and -l flags respectively. For example

```bash
gcc -Wall -Wextra -Werror -std=c11 -g test_prog.c -I./include -L. -lhalloc -o test_prog
```

would compile a `test_prog.c` source code file that uses this library.
