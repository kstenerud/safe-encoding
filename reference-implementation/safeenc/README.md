Safe Encoder
============

An example command-line tool to encode/decode safe encoding formats (safe16, safe32, safe64, safe80, safe85)


Requirements
------------

  * Meson 0.49 or newer
  * Ninja 1.8.2 or newer
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

    meson build
    ninja -C build


Usage
-----

    Usage: safeenc [options]

    Must specify at least -i or -o.

    Options:
      -h: Print help and exit
      -v: Print version and exit
      -i path: Input path to read from (default '-' which is stdin)
      -o path: Output path to write to (default '-' which is stdout)
      -d: Decode instead of encoding
      -l: Encode/decode safe64l (with a length field)
      -n <count>: Insert a newline every <count> encoded characters
      -I <count>: Insert <count> spaces indentation on each line
      -s <count>: Insert a separator char every <count> encoded characters (overrides linebreaks)
      -S <character>: Use this char as the separator character (default space)
      -r <radix>: Encode using this radix (16, 32, 64, 80, 85). Default 16
      -x: Convert input from hex chars to binary before encoding. (causes entire file to be read rather than streaming)


Examples
--------

Convert a text UUID to safe32:

```
echo a9bef56f-c989-4b98-a03d-9709e8612122 | ./safeenc -i '-' -x -r 32 && echo
n6zfavy9h55sh81xjw4ygr9112
```

If you strip out the version (4) and variant (a) fields, then add a dash every 6 chars, you get a nice looking 120 bit activation code:

```
echo a9bef56f-c989-b98-03d-9709e8612122 | ./safeenc -i '-' -x -r 32 -s 6 -S '-' && echo
n6zfav-y9h6wr-0fcq17-m62892
```

Whoops... A user misread some confusable characters in your activation code (v, 0, and 1)! No problem, safe32 handles that:

```
echo n6zfau-y9h6wr-Ofcql7-m62892 | ./safeenc -i '-' -d -r 32 | od -t x1
0000000 a9 be f5 6f c9 89 b9 80 3d 97 09 e8 61 21 22
```

Re-add the variant (4) and version (a) fields (which you already know since you're the one generating the UUIDs), and the original UUID is restored:

```
a9bef56f-c989-b98-03d-9709e8612122
a9bef56f-c989-4b98-a03d-9709e8612122
              ^    ^
```
