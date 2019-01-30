Reference Executable for the Safe16 Encoding System
===================================================

A C implementation to demonstrate a simple SAFE16 codec.


Requirements
------------

  * CMake 3.5 or higher
  * A C compiler


Dependencies
------------

 * libgen.h
 * stdarg.h
 * stdbool.h
 * stdint.h
 * stdio.h
 * stdlib.h
 * string.h
 * unistd.h


Building
--------

    mkdir build
    cd build
    cmake ..
    make


Usage
-----

    Usage: safe16 [options] [file]
    Where the default behavior is to encode from stdin to stdout.

    Options:
      -h: Print help and exit
      -v: Print version and exit
      -d: Decode instead of encoding
      -l: Encode/decode safe64l (with a length field)
      -n <count>: Insert a newline every <count> encoded characters
      -i <count>: Insert <count> spaces indentation on each line

    File: If not specified, - (read from stdin) is assumed.
