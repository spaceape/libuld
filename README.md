# uld
Dynamic ELF Loader for ARM microcontrollers

## Description
Library for loading relocatable

## Dependencies
- [FAT Filesystem Access](http://elm-chan.org/fsw/ff/00index_e.html) for loading ELF binaries from persistent storage;
- Storage device driver to serve as as a disk interface for the filesystem access library, such as [no-OS-FatFs-SD-SPI](https://github.com/carlk3/no-OS-FatFS-SD-SPI-RPi-Pico).

## Supported Architectures
- ARM
- ARM Thumb

## Supported binary formats

  ULD currently supports loading and executing relocatable ELF 32 bit object, shared library and executable files.

  Typical compilation command for a relocatable object file would look something like:
  > /usr/bin/arm-none-eabi-gcc -mcpu=cortex-m0plus -mthumb -ffreestanding -nostdlib -fno-common -falign-functions=4 -std=c11 -fpic -c hello.c -o hello.o

## Supported features

### ELF32 file loading


## Architecture-specific features

### ARM

### ARM Thumb/2

#### long calls

  Typical thumb relative jumps use a 22-bit immediate address, which would be insufficient for calling functions from the standard library.
  As such, every function non-local to the loaded binary should be declared with the `long_call` attribute:
  ```c
  void* malloc(size_t)  __attribute__((long_call));
  void free(void*) __attribute__((long_call));
  ```

See the `test` directory for usage examples.

### Using other binary formats

## API

ULD exposes a very simple interface for setting up, loading and executing code from ELF binaries:

### `uld::target`

- A basic configuration object that controls the behaviour of the loader

### `uld::image`

- Binary image

> define(symbol_name, symbol_address);

> load(filename)

.o
.so
executable


## Development
