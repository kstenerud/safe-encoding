Safe32 Encoding
===============

Safe32 provides a binary data encoding scheme that is safe to be input by humans, or passed through processing systems that expect human readable text.

### Features:

 * Easily confusable characters & digits are interchangeable.
 * Uppercase and lowercase characters are interchangeable.
 * No escaping necessary
 * Safe for use in URLs
 * Safe for use as filenames
 * Safe for use in formatted documents
 * Safe for use in legacy text processing systems
 * Liberal whitespace rules
 * Sortable in generic text sorting algorithms (such as file listings)
 * Alternate form with prefixed length


Encoding
--------

Safe32 encodes 5 bits into each character, effectively encoding 5 bytes of data into every 8 characters, for a total size bloat of 1.6.

Safe32 uses the following alphabet to represent 5-bit values, all of which are safe for known text systems:

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

Where:

 * Uppercase can be substituted for lowercase when input by humans.
 * `o` and `O` can substitute for `0` when input by humans.
 * There is no `1`, `i`, or `l`, because they are too easily confused.

A decoder must accept substitutes. An encoder must not output substitutes.

The alphabet is ordered according to the characters' ordinal positions in UTF-8 and ASCII, so that the resulting encoded text will sort in the same order as the data it represents.


Whitespace
----------

An encoded stream may contain whitespace as needed by the medium it will be transferred across. For example, some mediums may have a maximum line length, or require indentation. It is up to the encoder to decide when and how whitespace will occur. A decoder must accept and discard all whitespace characters while processing the stream.

For the purposes of this spec, only the following characters qualify as whitespace:

| Code Point | Name            |
| ---------- | --------------- |
| 0009       | Horizontal Tab  |
| 000a       | Line Feed       |
| 000d       | Carriage Return |
| 0020       | Space           |


Termination
-----------

Termination of a safe32 sequence can be inferred by the number of trailing characters after the last group of 8:

| Trailing characters | Remaining bytes |
| ------------------- | --------------- |
| 0                   | 0               |
| 1                   | invalid         |
| 2                   | 1               |
| 3                   | invalid         |
| 4                   | 2               |
| 5                   | 3               |
| 6                   | invalid         |
| 7                   | 4               |

Any excess bits after the last 8-bit sequence must be set to 0 by the encoder, and must be discarded by the decoder.



Safe32l
=======

While safe32 is sufficient for most systems, there are transmission mediums that do not guarantee detection of truncated data. In such a case, it is desirable to prefix a length field so that the receiving end can be sure of a complete transfer.


Encoding
--------

Safe32l works essentially the same as safe32, except that it is prefixed by a length field. The length field is built incrementally using the same encoding alphabet as the data, until sufficient bits are available to encode the length of the data.

The length encoding uses the lower 4 bits for data, and the high bit as a continuation bit:

| Bit Position | 4 | 3 | 2 | 1 | 0 |
| ------------ | - | - | - | - | - |
| **Purpose**  | c | x | x | x | x |

c = continuation bit
x = data

When the continuation bit is set to 1, the length field is continued in the next character. Building of the length field continues until a continuation bit of 0 is encountered. The 5 bit chunks are interpreted in big endian order (the first character represents the highest 4 bits, then the next lower 4 bits, and so on). This allows safe32l encoded data to sort naturally in generic text sorting algorithms, with longer data sequences ranking higher.

| Characters | Bits | Maximum encodable length |
| ---------- | ---- | ------------------------ |
| 1          |    4 |                       15 |
| 2          |    8 |                      255 |
| 3          |   12 |                     4095 |
| 4          |   16 |                    65535 |
| 5          |   20 |                  1048575 |
| 6          |   24 |                 16777215 |
| ...        |  ... |                      ... |


Truncation Detection
--------------------

Should truncation occur anywhere in the encoded sequence, one of two things will happen:

 1. The data length won't match the length field.
 2. The length field won't have a character with the continuation bit cleared.


Examples
--------

| Length | Encoded Bits        | Length Field |
| ------ | ------------------- | ------------ |
|      1 | `00001`             | `0`          |
|     15 | `01111`             | `g`          |
|     16 | `10001 00000`       | `j0`         |
|   2000 | `10111 11101 00000` | `rx0`        |


Advantages over base32 padding
------------------------------

 * No extra alphabet character is necessary. The length encodes using the exact same alphabet as the data encoding.
 * Truncation is always detected. With base32 padding, truncation on an 8-character boundary will not be detected.
 * Lower data usage for smaller data. base32's padding scheme uses an average of 4 characters every time. Safe32L uses only 1 byte for lengths 31 and under, 2 bytes for lengths 255 and under, 3 bytes for lengths 4095 and under. By the time its size performance suffers in comparison to base32 (at length 65536), the character length difference is less than 0.001%.



Version History
---------------

January 1, 2019: Preview Version 1



License
-------

Copyright (c) 2019 Karl Stenerud. All rights reserved.

Distributed under the Creative Commons Attribution License: https://creativecommons.org/licenses/by/4.0/legalcode
License deed: https://creativecommons.org/licenses/by/4.0/
