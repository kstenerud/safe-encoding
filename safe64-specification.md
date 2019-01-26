Safe64 Encoding
===============

Safe64 is a binary data encoding scheme that is safe to be passed through processing systems expecting human readable text.

### Features:

 * Safe for use in JSON, SGML formats, source code string literals, URLs, without escaping
 * Safe for use in filenames
 * Safe for use in formatted documents
 * Safe for use in legacy text processing systems
 * Alternate Form with Support for length fields

### Advantages over base64:

 * Smaller alphabet
 * No padding characters
 * No escaping necessary
 * Liberal whitespace rules
 * Safe for use in filenames
 * Reliable truncation detection
 * Sortable in generic text sorting algorithms (such as file listings)


Encoding
--------

Safe64 encoding uses an alphabet of 64 characters from the single-byte printable UTF-8 set to represent radix-64 chunks (where each chunk has an individual value from 0 - 63). These chunks are grouped magnitudally into big-endian sequences of 4 chunks, giving a range of 64^4 = 16777216 (0x1000000), which allows 24 bits (3 bytes) of data storage per group. Such an ecoding scheme multiplies the size of the data by a factor of 1.33.

Layout:

    Bytes:  [aaaaaaaa] [bbbbbbbb] [cccccccc]
    Chunks: [aaaaaa] [aabbbb] [bbbbcc] [cccccc]


### Encoding Process

The encoding process encodes groups of 3 bytes, outputting 4 chunks per group. If the source data length is not a multiple of 3, then the final group is output as a partial group, using only as many chunks as is necessary to encode the remaining bytes, with the unused high portion of the highest chunk cleared.

#### 3 Byte Full Group

First, an accumulator is filled big-endian style with 3 bytes of data:

    accumulator = (bytes[0] << 16) | (bytes[1] << 8) | bytes[2]

Next, the accumulator is broken down big-endian style into radix-64 chunks:

    chunk[0] = (accumulator >> 18) & 0x3f
    chunk[1] = (accumulator >> 12) & 0x3f
    chunk[2] = (accumulator >> 6) & 0x3f
    chunk[3] = accumulator & 0x3f

| Chunk 0 | Chunk 1 | Chunk 2 | Chunk 3 |
| ------- | ------- | ------- | ------- |
| 0 - 63  | 0 - 63  | 0 - 63  | 0 - 63  |

#### 2 Byte Partial Group

    Bytes:  [aaaaaaaa] [bbbbbbbb]
    Chunks: [00aaaa] [aaabbb] [bbbbbb]

    accumulator = (bytes[0] << 8) | bytes[1]
    chunk[0] = (accumulator >> 12) & 0x3f
    chunk[1] = (accumulator >> 6) & 0x3f
    chunk[2] = accumulator & 0x3f

#### 1 Byte Partial Group

    Bytes:  [aaaaaaaa]
    Chunks: [0000aa] [aaaaaa]

    accumulator = bytes[0]
    chunk[0] = (accumulator >> 6) & 0x3f
    chunk[1] = accumulator & 0x3f

#### Alphabet

Once the chunk values have been determined, they are output as characters according to the following alphabet:

| Value  | Char | Value  | Char | Value  | Char | Value  | Char |
| ------ | ---- | ------ | ---- | ------ | ---- | ------ | ---- |
| **00** | `-`  | **10** | `F`  | **20** | `V`  | **30** | `k`  |
| **01** | `0`  | **11** | `G`  | **21** | `W`  | **31** | `l`  |
| **02** | `1`  | **12** | `H`  | **22** | `X`  | **32** | `m`  |
| **03** | `2`  | **13** | `I`  | **23** | `Y`  | **33** | `n`  |
| **04** | `3`  | **14** | `J`  | **24** | `Z`  | **34** | `o`  |
| **05** | `4`  | **15** | `K`  | **25** | `_`  | **35** | `p`  |
| **06** | `5`  | **16** | `L`  | **26** | `a`  | **36** | `q`  |
| **07** | `6`  | **17** | `M`  | **27** | `b`  | **37** | `r`  |
| **08** | `7`  | **18** | `N`  | **28** | `c`  | **38** | `s`  |
| **09** | `8`  | **19** | `O`  | **29** | `d`  | **39** | `t`  |
| **0a** | `9`  | **1a** | `P`  | **2a** | `e`  | **3a** | `u`  |
| **0b** | `A`  | **1b** | `Q`  | **2b** | `f`  | **3b** | `v`  |
| **0c** | `B`  | **1c** | `R`  | **2c** | `g`  | **3c** | `w`  |
| **0d** | `C`  | **1d** | `S`  | **2d** | `h`  | **3d** | `x`  |
| **0e** | `D`  | **1e** | `T`  | **2e** | `i`  | **3e** | `y`  |
| **0f** | `E`  | **1f** | `U`  | **2f** | `j`  | **3f** | `z`  |

The alphabet is ordered according to the characters' ordinal positions in UTF-8, so that the resulting encoded text will sort in the same order as the data it represents.


Whitespace
----------

An encoded stream may contain whitespace at any point. A decoder must accept and discard all whitespace characters while processing the stream.

For the purposes of this spec, only the following characters qualify as whitespace:

| Code Point | Name            |
| ---------- | --------------- |
| 0009       | Horizontal Tab  |
| 000a       | Line Feed       |
| 000d       | Carriage Return |
| 0020       | Space           |


Termination
-----------

In the last (possibly partial) group, the number of remaining characters indicates how many bytes of data remain to be decoded:

| Characters | Remaining Bytes |
| ---------- | --------------- |
| 1          | invalid         |
| 2          | 1               |
| 3          | 2               |
| 4          | 3               |


Examples
--------

    Data:    {0x39, 0x12, 0x82, 0xe1, 0x81, 0x39, 0xd9, 0x8b, 0x39, 0x4c, 0x63, 0x9d, 0x04, 0x8c}
    Encoded: DG91sN3tqNgtI5DS-HB

    Data:    {0xe6, 0x12, 0xa6, 0x9f, 0xf8, 0x38, 0x6d, 0x7b, 0x01, 0x99, 0x3e, 0x6c, 0x53, 0x7b, 0x60})
    Encoded: tW9abzVsQMg0aItgJrhV

    Data:    {0x21, 0xd1, 0x7d, 0x3f, 0x21, 0xc1, 0x88, 0x99, 0x71, 0x45, 0x96, 0xad, 0xcc, 0x96, 0x79, 0xd8})
    Encoded: 7S4xEm60X8_lGOPhn8Ot2N

------------------------------------------------------------------------------



Safe64L
=======

While safe64 is sufficient for most systems, there are transmission mediums where no clear end marker exists for the encoded data field, or where no guarantee exists for detecting truncated data. In such cases, it is desirable to prefix a length field so that the receiving end can be sure of a complete transfer.


Encoding
--------

Safe64L works essentially the same as safe64, except that it is prefixed by a length field. The length field is built incrementally using the same encoding alphabet as the data, until sufficient bits are available to encode the length of the data.

The length encoding uses the lower 5 bits for data, and the high bit as a continuation bit:

| Bit Position | 5 | 4 | 3 | 2 | 1 | 0 |
| ------------ | - | - | - | - | - | - |
| **Purpose**  | c | x | x | x | x | x |

 * c = continuation bit
 * x = data

While the continuation bit is set to 1, the length field is continued in the next character. Building of the length field continues until a continuation bit of 0 is encountered. The 5 bit chunks are interpreted in big endian order (the first character represents the highest 5 bits, then the next lower 5 bits, and so on).

| Characters | Bits | Maximum encodable length |
| ---------- | ---- | ------------------------ |
| 1          |    5 |                       31 |
| 2          |   10 |                     1023 |
| 3          |   15 |                    32767 |
| 4          |   20 |                  1048575 |
| 5          |   25 |                 33554431 |
| 6          |   30 |               1073741823 |
| ...        |  ... |                      ... |

Note: The length field encodes the length of the **non-encoded source data**, not the encoded result or the length field itself.


Whitespace
----------

The length field may contain whitespace at any point in the stream, following the same rules as for safe64.


Truncation Detection
--------------------

Should truncation occur anywhere in the encoded sequence (length or data), one of two things will happen:

 1. The decoded data length won't match the length field.
 2. The length field won't have a character with the continuation bit cleared.


Examples
--------

| Length | Encoded Bits           | Length Field |
| ------ | ---------------------- | ------------ |
|      1 | `000001`               | `0`          |
|     31 | `011111`               | `U`          |
|     32 | `100001 000000`        | `W-`         |
|   2000 | `100001 111110 010000` | `WyF`        |

#### Example: Length field & data:

Data:

    0x21, 0x7b, 0x01, 0x99, 0x3e, 0xd1, 0x7d, 0x3f,
    0x21, 0x8b, 0x39, 0x4c, 0x63, 0xc1, 0x88, 0x21,
    0xc1, 0x88, 0x99, 0x71, 0xa6, 0x9f, 0xf8, 0x45,
    0x96, 0xe1, 0x81, 0x39, 0xad, 0xcc, 0x96, 0x79,
    0xd8

Encoded:

    W07Mg0aIvGUIwWXn_BNw577R57aM5abzW4_i50DPrB_bbN

In this case, the length field is `W0` (33)


Advantages over base64 padding
------------------------------

 * No extra alphabet character is necessary. The length encodes using the exact same alphabet as the data encoding.
 * Truncation is always detected. With base64 padding, truncation on a 4-character boundary will not be detected.
 * Lower data usage for smaller data. base64's padding scheme uses an average of 1.5 characters no matter the length of the data. Safe64L uses only 1 byte for lengths 31 and under. By the time its size performance suffers in comparison to base64 (at length 32), the character length difference is already less than 2% of the total payload size, and shrinks from there.



Version History
---------------

 * January 1, 2019: Preview Version 1



License
-------

Copyright (c) 2019 Karl Stenerud. All rights reserved.

Distributed under the Creative Commons Attribution License: https://creativecommons.org/licenses/by/4.0/legalcode
License deed: https://creativecommons.org/licenses/by/4.0/
