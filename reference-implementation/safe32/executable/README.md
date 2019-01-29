Reference Executable for the Safe32 Encoding System
===================================================

A C implementation to demonstrate a simple SAFE32 codec.


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

    Safe32: Encodes binary data into a radix-32 text format safe for use in modern text processors.
    Output data is written to stdout. Input data may be a file, or stdin (by specifying the filename: '-')

    Usage: safe32 [options] <filename>

    Options:
      -h: Print help and exit
      -v: Print version and exit
      -d: Decode a file to stdout
      -e: Encode a file to stdout
      -l: Use/expect a length field. MAKE SURE YOU MATCH THIS SETTING IN ENCODE AND DECODE
      -b <count>: Insert a line break every <count> output characters
      -i <count>: Insert <count> spaces indentation at the start of every line
