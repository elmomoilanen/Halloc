# Halloc - heap memory allocator #

[![main](https://github.com/elmomoilanen/Halloc/actions/workflows/main.yml/badge.svg)](https://github.com/elmomoilanen/Halloc/actions/workflows/main.yml)

`Halloc` is a custom dynamic memory allocator for C programs providing a public API that resembles the C standard library function `calloc`. Halloc is constructed internally by doubly linked lists and leverages the `mmap` system call to create anonymous memory mappings in the virtual address space. Each byte of allocated memory by Halloc is initialized to zero, ensuring a consistent and predictable initial state for new memory allocations.

Halloc allocator is designed to support the size of a single allocation up to approximately 1 GiB, achieved by adjusting the length of memory mappings created by the mmap system call. Alongside the primary memory allocation and deallocation functions, this library also provides various virtual memory statistics for use. For more information, please refer to the **Usage** section below.

This library can only be used safely with single-threaded code.

## Build ##

Halloc uses the C11 standard. It is expected to work on most common Linux distros.

To build the library, run

```bash
make
```

If the build is successful, a static library file named `libhalloc.a` will be created in the current directory.

Tests can be run as follows

```bash
make test
```

and this should be used to verify that the library is usable on the target machine.

Optionally to the previous make command, the following command installs the library and header file in the system directories specified by the PREFIX variable, which defaults to `/usr/local` in the Makefile

```bash
make install
```

To uninstall, run

```bash
make uninstall
```

If the build fails or tests do not pass, this library may not be directly usable on your system.

## Usage ##

Header file **include/halloc.h** defines the public API for the library.

Use `halloc()` to request a new memory allocation and `hfree()` to deallocate previously allocated memory. In addition, there are three functions to provide detailed memory statistics for total or type specific virtual memory usage.

Following code section gives an example use case of the library. The code snippet illustrates functionality that makes the library interesting: the possibility to inspect memory usage statistics via three different functions, each of which begins with the "halloc_print" prefix.

```C
#include <assert.h>
#include <stdlib.h>

#include "halloc.h"

typedef struct {
  double *data;
  unsigned int size;
} myType;

int main() {
  // Allocate memory for a struct `myType`
  myType *ptr = halloc(myType, 1);
  assert(ptr != NULL);

  ptr->size = 25;
  // Allocate memory for data member of the struct
  ptr->data = halloc(double, ptr->size);
  assert(ptr->data != NULL);

  ptr->data[ptr->size - 1] = 1.0;

  // Inspect runtime virtual memory usage
  halloc_print_saved_page_items();
  halloc_print_total_memory_usage();

  halloc_print_type_memory_usage(myType);
  halloc_print_type_memory_usage(double);

  // Nested allocations must be freed first to prevent memory leaks
  hfree(ptr->data);
  hfree(ptr);
  
  halloc_print_total_memory_usage();
}
```

To compile a source code file that uses Halloc, specify the include path for the header file `halloc.h` with the `-I` flag, and the library path and name for the static library file `libhalloc.a` with the `-L` and `-l` flags respectively. For example

```bash
gcc -Wall -Wextra -Werror -std=c11 -g test_prog.c -I./include -L. -lhalloc -o test_prog
```

would compile a `test_prog.c` source code file that uses this library.
