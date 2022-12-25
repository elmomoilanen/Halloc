# Halloc - heap memory allocator #

[![main](https://github.com/elmomoilanen/Halloc/actions/workflows/main.yml/badge.svg)](https://github.com/elmomoilanen/Halloc/actions/workflows/main.yml)

This library implements a custom dynamic memory allocator `Halloc` in the C language that somewhat resembles the standard library function *calloc* by its public API. Halloc is constructed internally by doubly linked lists and uses the *mmap* system call to create memory mappings in the virtual address space such that the mappings are anonymous and not backed by files. For successfully created mappings, halloc returns a pointer to the starting address of the new memory allocation such that all bytes of the allocation are initialised to zero. One-time memory allocations are supported roughly up to 1 GiB by variating internally the length of mmap's memory mappings. Halloc library provides various memory statistics to be used alongside its main components that allocate and deallocate memory; see the **Usage** section below for an example. Notice that this library is not thread-safe and thus should not be used with multithreaded code.

## Build ##

Library is expected to work in most common Linux distros (e.g. Ubuntu) and macOS with Intel processors. C11 standard is used.

Build the library and run associated unit/functional tests with the following command in a shell

```bash
make && make test && make clean
```

which on success places the static library file *libhalloc.a* in top-level of this library folder structure. The last clean command removes unneeded object and test files. Notice that for macOS one might need to override the compiler parameter from the Makefile which is *gcc* by default or give an additional argument *CC=clang* after the first make in the shell command above.

In case of a build failure or tests not passing, this library may not be usable in your system "as is".

## Usage ##

Header file *include/halloc.h* defines public API for the library. Use `halloc()` to request a new memory allocation and `hfree()` to deallocate this memory. In addition for these, there are three functions to provide detailed memory statistics for total or type specific memory usage.

Following code section gives an example use case of the halloc library. The code snippet illustrates functionality that makes the library interesting, e.g. the mentioned possibility to inspect virtual memory usage statistics via three different functions, each beginning with the "halloc_print" prefix.

```C
#include <stdlib.h>
#include <assert.h>

#include "halloc.h"

typedef struct {
  double *data;
  unsigned int size;
} typeX;


int main() {
  // Allocate one unit for a struct `typeX`
  typeX *p = halloc(typeX, 1);

  assert(p != NULL);

  p->size = 25;
  // Allocate a double type pointer
  p->data = halloc(double, p->size);

  assert(p->data != NULL);
  // Data pointer ready to be used
  p->data[p->size - 1] = 11.0;

  // Inspect runtime virtual memory usage
  halloc_print_saved_page_items();
  halloc_print_total_memory_usage();

  halloc_print_type_memory_usage(typeX);
  halloc_print_type_memory_usage(double);

  // Nested allocations must be freed first
  hfree(p->data);
  hfree(p);
  
  halloc_print_total_memory_usage();
}
```

When using the halloc library in some other C program, one must indicate to compiler the include path (-I) for the header file *halloc.h* and library path (-L) and name (-l) for the static library file *libhalloc.a*. E.g. a shell command

```bash
gcc test_prog.c -Iinclude -L. -lhalloc -o test_prog -Wall -Wextra -Werror -std=c11 -g
```

would compile a *test_prog.c* source code file placed in the root directory of this library with a set of common warning flags (e.g. Wall).
