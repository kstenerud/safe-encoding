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

 * All of the abovementioned features
 * Smaller alphabet
 * No padding characters
 * No escaping necessary
 * Liberal whitespace rules
 * Safe for use in filenames
 * Reliable truncation detection
 * Sortable in generic text sorting algorithms (such as file listings)



Terms and Conventions
---------------------

**The following bolded, capitalized terms have specific meanings in this document**:

| Term             | Meaning                                                                                                               |
| ---------------- | --------------------------------------------------------------------------------------------------------------------- |
| **MUST (NOT)**   | If this directive is not adhered to, the document or implementation is invalid.                                       |
| **SHOULD (NOT)** | Every effort should be made to follow this directive, but the document/implementation is still valid if not followed. |
| **MAY (NOT)**    | It is up to the implementation to decide whether to do something or not.                                              |
| **CAN**          | Refers to a possibility which **MUST** be accommodated by the implementation.                                         |
| **CANNOT**       | Refers to a situation which **MUST NOT** be allowed by the implementation.                                            |



Contents
--------

- [Safe32 Encoding](#safe32-encoding)
    - [Features:](#features)
    - [Advantages over base32:](#advantages-over-base32)
  - [Terms and Conventions](#terms-and-conventions)
  - [Contents](#contents)
  - [Encoding](#encoding)
    - [Encoding Process](#encoding-process)
      - [5 Byte Full Group](#5-byte-full-group)
      - [4 Byte Partial Group](#4-byte-partial-group)
      - [3 Byte Partial Group](#3-byte-partial-group)
      - [2 Byte Partial Group](#2-byte-partial-group)
      - [1 Byte Partial Group](#1-byte-partial-group)
    - [Final Group](#final-group)
    - [Alphabet](#alphabet)
  - [Whitespace](#whitespace)
  - [Examples](#examples)
  - [Filenames](#filenames)
- [Safe32L Encoding](#safe32l-encoding)
  - [Encoding](#encoding-1)
  - [Whitespace](#whitespace-1)
  - [Truncation Detection](#truncation-detection)
  - [Examples](#examples-1)
      - [Example: Length field \& data:](#example-length-field--data)
  - [Filenames](#filenames-1)
  - [Advantages over base32 padding](#advantages-over-base32-padding)
  - [Version History](#version-history)
  - [License](#license)



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


### Final Group

In the last (possibly partial) group, the number of remaining characters indicates how many bytes of data remain to be decoded, and whether truncation has been detected. Note that truncation detection is not guaranteed - for that you would need to use [Safe32L encoding](#safe32l-encoding).

| Characters | Bytes | Status         |
| ---------- | ----- | -------------- |
| 1          | -     | Truncated data |
| 2          | 1     | OK             |
| 3          | -     | Truncated data |
| 4          | 2     | OK             |
| 5          | 3     | OK             |
| 6          | -     | Truncated data |
| 7          | 4     | OK             |
| 8          | 5     | OK             |


### Alphabet

Chunk values are output as characters according to the following alphabet:

| Value  | Char | Value  | Char | Value  | Char | Value  | Char |
| ------ | ---- | ------ | ---- | ------ | ---- | ------ | ---- |
| **00** | `0`  | **08** | `8`  | **10** | `g`  | **18** | `r`  |
| **01** | `1`  | **09** | `9`  | **11** | `h`  | **19** | `s`  |
| **02** | `2`  | **0a** | `a`  | **12** | `j`  | **1a** | `t`  |
| **03** | `3`  | **0b** | `b`  | **13** | `k`  | **1b** | `v`  |
| **04** | `4`  | **0c** | `c`  | **14** | `m`  | **1c** | `w`  |
| **05** | `5`  | **0d** | `d`  | **15** | `n`  | **1d** | `x`  |
| **06** | `6`  | **0e** | `e`  | **16** | `p`  | **1e** | `y`  |
| **07** | `7`  | **0f** | `f`  | **17** | `q`  | **1f** | `z`  |

The alphabet is ordered according to the characters' ordinal positions in UTF-8, so that the resulting encoded text will sort in the same natural ordering as the data it represents.

Decoders **MUST** accept a wider range of characters in order to mitigate human error:

 * All letters **CAN** be substituted with their capitals.
 * `0` **CAN** be substituted with `o` or its capital.
 * `1` **CAN** be substituted with `l` or `i` or their capitals.
 * `v` **CAN** be substituted with `u` or its capital.

Encoders are more restricted in what they're allowed to produce:

 * All letters **MAY** be substituted with their capitals.
 * Letter case **MUST NOT** be mixed within the generated output - only _all_ uppercase or _all_ lowercase.
 * All other substitution characters **MUST NOT** be generated.
 * Encoders **MAY** produce whitespace characters.



Whitespace
----------

An encoded stream **CAN** contain whitespace at any point. A decoder **MUST** accept and discard all whitespace characters while processing the stream.

For the purposes of this spec, only the following characters qualify as whitespace:

| Code Point | Name            |
| ---------- | --------------- |
| 0009       | Horizontal Tab  |
| 000a       | Line Feed       |
| 000d       | Carriage Return |
| 0020       | Space           |
| 002D       | Dash `-`        |

**Note**: Dash is included as "whitespace" to allow human-input sequences such as:

    85a9-6sd2-88ds-qfbd



Examples
--------

    Data:    {0x39, 0x12, 0x82, 0xe1, 0x81, 0x39, 0xd9, 0x8b, 0x39, 0x4c, 0x63, 0x9d, 0x04, 0x8c}
    Encoded: 74985rc177crpeac1hst14c

    Data:    {0xe6, 0x12, 0xa6, 0x9f, 0xf8, 0x38, 0x6d, 0x7b, 0x01, 0x99, 0x3e, 0x6c, 0x53, 0x7b, 0x60})
    Encoded: wr9ad7zr71pqp0cs7sp56yv0

    Data:    {0x21, 0xd1, 0x7d, 0x3f, 0x21, 0xc1, 0x88, 0x99, 0x71, 0x45, 0x96, 0xad, 0xcc, 0x96, 0x79, 0xd8})
    Encoded: 478qtfs1r649jwa5jtpws5ks6r



Filenames
---------

Files containing safe32 data **SHOULD** have the extension `s32`, for example `mydata.s32`.

------------------------------------------------------------------------------



Safe32L Encoding
================

While safe32 is sufficient for most systems, there are transmission mediums where no clear end marker exists for the encoded data field, or where no guarantee exists for detecting truncated data. In such cases, it is sometimes desirable to prefix a length field so that the receiving end will be sure of a complete transfer.



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

The length field **CAN** also be broken up using the same whitespace rules as for safe32.



Truncation Detection
--------------------

If truncation occurs anywhere in the encoded sequence (length or data), one of two things will happen:

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

    h0478qtfs1r649jwa5jtpws5ks6r

In this case, the length field is `h0` (16)



Filenames
---------

Files containing safe32l data **SHOULD** have the extension `s32l`, for example `mydata.s32l`.



Advantages over base32 padding
------------------------------

 * No extra alphabet character is necessary. The length encodes using the exact same alphabet as the data encoding.
 * Truncation is always detected. With base32 padding, truncation on an 8-character boundary will not be detected.
 * Lower data usage for smaller data. base32's padding scheme uses an average of 4 characters every time. Safe32L uses only 1 byte for lengths 31 and under, 2 bytes for lengths 255 and under, 3 bytes for lengths 4095 and under. By the time its size performance suffers in comparison to base32 (at length 65536), the character length difference is less than 0.001%.



Version History
---------------

 * March 23, 2019: Version 2
 * January 29, 2018: Version 1
 * January 1, 2019: Preview Version 1



License
-------

Copyright (c) 2019 Karl Stenerud. All rights reserved.

Distributed under the Creative Commons Attribution License: https://creativecommons.org/licenses/by/4.0/legalcode
License deed: https://creativecommons.org/licenses/by/4.0/
