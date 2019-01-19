Safe32 Encoding
===============

Safe32 is a binary data encoding scheme that is safe to be passed through processing systems expecting human readable text.

### Features:

 * Safe for use in JSON, SGML formats, source code string literals, URLs, without escaping
 * Safe for use in filenames
 * Safe for use in formatted documents
 * Safe for use in legacy text processing systems
 * Alternate Form with Support for length fields
 * Useful for human input situations such as activation codes.
 * Easily confusable characters & digits are interchangeable.
 * Uppercase and lowercase characters are interchangeable.

### Advantages over base32:

 * Smaller alphabet
 * No padding characters
 * No escaping necessary
 * Liberal whitespace rules
 * Safe for use in filenames
 * Better end-of-field and truncation detection
 * Sortable in generic text sorting algorithms (such as file listings)


Encoding
--------

Safe32 encoding uses an alphabet of 32 characters from the single-byte printable UTF-8 set to represent radix-32 chunks (where each chunk has an individual value from 0 - 31). These chunks are grouped magnitudally into big-endian sequences of 8 chunks, giving a range of 32^8 = 1099511628000 (0x10000000000), which allows 40 bits (5 bytes) of data storage per group. Such an ecoding scheme multiplies the size of the data by a factor of 1.6.

Layout:

    Bytes:  [aaaaaaaa] [bbbbbbbb] [cccccccc] [dddddddd] [eeeeeeee]
    Chunks: [aaaaa] [aaabb] [bbbbb] [bcccc] [ccccd] [ddddd] [ddeee] [eeeee]


### Encoding Process

The encoding process encodes groups of 5 bytes, outputting 8 chunks per group. If the source data length is not a multiple of 5, then the final group is output as a partial group, using only as many chunks as is necessary to encode the remaining bytes, with the unused high portion of the highest chunk cleared.

#### 5 Byte Full Group

First, an accumulator is filled big-endian style with 5 bytes of data:

    accumulator = (bytes[0] << 32) | (bytes[1] << 24) | (bytes[2] << 16) | (bytes[3] << 8) | bytes[4]

Next, the accumulator is broken down big-endian style into radix-32 chunks:

    chunk[0] = (accumulator >> 35) & 0x1f
    chunk[1] = (accumulator >> 30) & 0x1f
    chunk[2] = (accumulator >> 25) & 0x1f
    chunk[3] = (accumulator >> 20) & 0x1f
    chunk[4] = (accumulator >> 15) & 0x1f
    chunk[5] = (accumulator >> 10) & 0x1f
    chunk[6] = (accumulator >> 5) & 0x1f
    chunk[7] = accumulator & 0x1f

| Chunk 0 | Chunk 1 | Chunk 2 | Chunk 3 | Chunk 4 | Chunk 5 | Chunk 6 | Chunk 7 |
| ------- | ------- | ------- | ------- | ------- | ------- | ------- | ------- |
| 0 - 31  | 0 - 31  | 0 - 31  | 0 - 31  | 0 - 31  | 0 - 31  | 0 - 31  | 0 - 31  |

#### 4 Byte Partial Group

    Bytes:  [aaaaaaaa] [bbbbbbbb] [cccccccc] [dddddddd]
    Chunks: [000aa] [aaaaa] [abbbb] [bbbbc] [ccccc] [ccddd] [ddddd]

    accumulator = (bytes[0] << 24) | (bytes[1] << 16) | (bytes[2] << 8) | bytes[3]
    chunk[0] = (accumulator >> 30) & 0x1f
    chunk[1] = (accumulator >> 25) & 0x1f
    chunk[2] = (accumulator >> 20) & 0x1f
    chunk[3] = (accumulator >> 15) & 0x1f
    chunk[4] = (accumulator >> 10) & 0x1f
    chunk[5] = (accumulator >> 5) & 0x1f
    chunk[6] = accumulator & 0x1f

#### 3 Byte Partial Group

    Bytes:  [aaaaaaaa] [bbbbbbbb] [cccccccc]
    Chunks: [0aaaa] [aaaab] [bbbbb] [bbccc] [ccccc]

    accumulator = (bytes[0] << 16) | (bytes[1] << 8) | bytes[2]
    chunk[0] = (accumulator >> 20) & 0x1f
    chunk[1] = (accumulator >> 15) & 0x1f
    chunk[2] = (accumulator >> 10) & 0x1f
    chunk[3] = (accumulator >> 5) & 0x1f
    chunk[4] = accumulator & 0x1f

#### 2 Byte Partial Group

    Bytes:  [aaaaaaaa] [bbbbbbbb]
    Chunks: [0000a] [aaaaa] [aabbb] [bbbbb]

    accumulator = (bytes[0] << 8) | bytes[1]
    chunk[0] = (accumulator >> 15) & 0x1f
    chunk[1] = (accumulator >> 10) & 0x1f
    chunk[2] = (accumulator >> 5) & 0x1f
    chunk[3] = accumulator & 0x1f

#### 1 Byte Partial Group

    Bytes:  [aaaaaaaa]
    Chunks: [00aaa] [aaaaa]

    accumulator = bytes[0]
    chunk[0] = (accumulator >> 5) & 0x1f
    chunk[1] = accumulator & 0x1f

#### Alphabet

Once the chunk values have been determined, they are output as characters according to the following alphabet:

| Value  | Char | Value  | Char |
| ------ | ---- | ------ | ---- |
| **00** | `0`  | **10** | `h`  |
| **01** | `2`  | **11** | `j`  |
| **02** | `3`  | **12** | `k`  |
| **03** | `4`  | **13** | `m`  |
| **04** | `5`  | **14** | `n`  |
| **05** | `6`  | **15** | `p`  |
| **06** | `7`  | **16** | `q`  |
| **07** | `8`  | **17** | `r`  |
| **08** | `9`  | **18** | `s`  |
| **09** | `a`  | **19** | `t`  |
| **0a** | `b`  | **1a** | `u`  |
| **0b** | `c`  | **1b** | `v`  |
| **0c** | `d`  | **1c** | `w`  |
| **0d** | `e`  | **1d** | `x`  |
| **0e** | `f`  | **1e** | `y`  |
| **0f** | `g`  | **1f** | `z`  |

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
| 002D       | Dash `-`        |

Note: Dash is included as "whitespace" to allow human input sequences such as:

    85a9-6sd2-88ds-qfbd


Termination
-----------

In the last (possibly partial) group, the number of remaining characters indicates how many bytes of data remain to be decoded:

| Characters | Remaining Bytes |
| ---------- | --------------- |
| 1          | invalid         |
| 2          | 1               |
| 3          | invalid         |
| 4          | 2               |
| 5          | 3               |
| 6          | invalid         |
| 7          | 4               |
| 8          | 5               |


Examples
--------

    Data:    {0x39, 0x12, 0x82, 0xe1, 0x81, 0x39, 0xd9, 0x8b, 0x39, 0x4c, 0x63, 0x9d, 0x04, 0x8c}
    Encoded: 85a96sd288dsqfbd2jtu25d

    Data:    {0xe6, 0x12, 0xa6, 0x9f, 0xf8, 0x38, 0x6d, 0x7b, 0x01, 0x99, 0x3e, 0x6c, 0x53, 0x7b, 0x60})
    Encoded: wsabe8zs82qrq0dt8tq67yv0

    Data:    {0x21, 0xd1, 0x7d, 0x3f, 0x21, 0xc1, 0x88, 0x99, 0x71, 0x45, 0x96, 0xad, 0xcc, 0x96, 0x79, 0xd8})
    Encoded: 589rugt2s75akwb6kuqwt6mt7s

------------------------------------------------------------------------------



Safe32L
=======

While safe32 is sufficient for most systems, there are transmission mediums where no clear end marker exists for the encoded data field, or where no guarantee exists for detecting truncated data. In such cases, it is desirable to prefix a length field so that the receiving end can be sure of a complete transfer.


Encoding
--------

Safe32L works essentially the same as safe32, except that it is prefixed by a length field. The length field is built incrementally using the same encoding alphabet as the data, until sufficient bits are available to encode the length of the data.

The length encoding uses the lower 5 bits for data, and the high bit as a continuation bit:

| Bit Position | 4 | 3 | 2 | 1 | 0 |
| ------------ | - | - | - | - | - |
| **Purpose**  | c | x | x | x | x |

 * c = continuation bit
 * x = data

While the continuation bit is set to 1, the length field is continued in the next character. Building of the length field continues until a continuation bit of 0 is encountered. The 5 bit chunks are interpreted in big endian order (the first character represents the highest 5 bits, then the next lower 5 bits, and so on).

| Characters | Bits | Maximum encodable length |
| ---------- | ---- | ------------------------ |
| 1          |    4 |                       15 |
| 2          |    8 |                      255 |
| 3          |   12 |                     4095 |
| 4          |   16 |                    65535 |
| 5          |   20 |                  1048575 |
| 6          |   24 |                 16777215 |
| ...        |  ... |                      ... |

Note: The length field encodes the length of the **non-encoded source data**, not the encoded result or the length field itself.


Whitespace
----------

The length field may contain whitespace at any point in the stream, following the same rules as for safe32.


Truncation Detection
--------------------

Should truncation occur anywhere in the encoded sequence (length or data), one of two things will happen:

 1. The decoded data length won't match the length field.
 2. The length field won't have a character with the continuation bit cleared.


Examples
--------

| Length | Encoded Bits        | Length Field |
| ------ | ------------------- | ------------ |
|      1 | `00001`             | `2`          |
|     15 | `01111`             | `g`          |
|     16 | `10001 00000`       | `j0`         |
|   2000 | `10111 11101 00000` | `rx0`        |

#### Example: Length field & data:

Data:

    0x21, 0xd1, 0x7d, 0x3f, 0x21, 0xc1, 0x88, 0x99,
    0x71, 0x45, 0x96, 0xad, 0xcc, 0x96, 0x79, 0xd8

Encoded:

    j0589rugt2s75akwb6kuqwt6mt7s

In this case, the length field is `j0` (16)


Advantages over base32 padding
------------------------------

 * No extra alphabet character is necessary. The length encodes using the exact same alphabet as the data encoding.
 * Truncation is always detected. With base32 padding, truncation on an 8-character boundary will not be detected.
 * Lower data usage for smaller data. base32's padding scheme uses an average of 4 characters every time. Safe32L uses only 1 byte for lengths 31 and under, 2 bytes for lengths 255 and under, 3 bytes for lengths 4095 and under. By the time its size performance suffers in comparison to base32 (at length 65536), the character length difference is less than 0.001%.



Version History
---------------

 * January 1, 2019: Preview Version 1



License
-------

Copyright (c) 2019 Karl Stenerud. All rights reserved.

Distributed under the Creative Commons Attribution License: https://creativecommons.org/licenses/by/4.0/legalcode
License deed: https://creativecommons.org/licenses/by/4.0/
