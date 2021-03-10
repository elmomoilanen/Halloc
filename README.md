# Halloc - heap memory allocator in C #

This library implements a custom dynamic memory allocator `Halloc` that somewhat resembles calloc defined in the C standard library. Halloc uses `mmap()` to create memory mappings in the virtual address space such that the mappings are anonymous and thus not backed by files. For successful mappings, halloc returns a pointer to starting address of the new memory allocation and all bytes of the allocation are initialised to 0. One-time memory allocations are supported roughly up to 1 GiB by variating the length of mmap memory mappings. Halloc library provides various memory statistics to be used alongside its main components that allocate and deallocate memory.

## Build ##

Build the library and run tests with the following command in a shell

```bash
make && make test && make clean
```

In case of build failure or tests not passing, this library may not be usable in your system "as is". Library uses C11 standard and is designed mainly for Linux. 

## Usage ##

Header file `halloc.h` defines public APIs for the library. Use `halloc()` to request new memory allocation and `hfree()` to deallocate this memory. In addition, there are three functions to provide detailed memory statistics of total or type specific memory usage.

```C

halloc()

```
