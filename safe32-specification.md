Safe32 Encoding
===============

Safe32 provides a binary data encoding scheme that is safe to be input by humans, and safe to be passed through processing systems that expect human readable text.

It is especially useful for things requiring human input such as activation codes.

### Features:

 * Safe for use in URLs
 * Safe for use in filenames
 * Safe for use in formatted documents
 * Safe for use in legacy text processing systems
 * Support for length fields
 * Useful for human input situations such as activation codes.
 * Easily confusable characters & digits are interchangeable.
 * Uppercase and lowercase characters are interchangeable.
 * Alternate form with prefixed length

### Advantages over base32:

 * Smaller alphabet
 * No padding characters
 * No escaping necessary
 * Liberal whitespace rules
 * Safe for use in filenames
 * No proliferation of incompatible offshoots
 * Better end-of-field and truncation detection
 * Sortable in generic text sorting algorithms (such as file listings)


Encoding
--------

Safe64 encoding uses an alphabet of 32 characters from the single-byte UTF-8 set to represent 5-bit values. These characters are grouped together by 8 (forming 40 bits), which can then be used to encode 5 bytes of data per group. This multiplies the size of the encoded data by a factor of 1.6.

    Original: [aaaaaaaa] [bbbbbbbb] [cccccccc] [dddddddd] [eeeeeeee]
    Encoded:  [aaaaa] [aaabb] [bbbbb] [bcccc] [ccccd] [ddddd] [ddeee] [eeeee]

When there are less than 5 bytes of source data remaining, a partial group is created, with unused bits set to 0:

    Original: [aaaaaaaa] [bbbbbbbb] [cccccccc] [dddddddd]
    Encoded:  [aaaaa] [aaabb] [bbbbb] [bcccc] [ccccd] [ddddd] [dd000]

    Original: [aaaaaaaa] [bbbbbbbb] [cccccccc]
    Encoded:  [aaaaa] [aaabb] [bbbbb] [bcccc] [cccc0]

    Original: [aaaaaaaa] [bbbbbbbb]
    Encoded:  [aaaaa] [aaabb] [bbbbb] [b0000]

    Original: [aaaaaaaa]
    Encoded:  [aaaaa] [aaa00]

The 5-bit alphabet, chosen for safety in known text systems, and ease of human input:

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

In order to make human input less error-prone, the following substitutions are allowed in the encoded text:

 * Uppercase letters may be substituted for lowercase letters (for example, `A` may be substituted for `a`).
 * `o` and `O` may be substituted for `0`.

A human may input substitutes. A decoder must accept substitutes. An encoder must not output substitutes.


Whitespace
----------

An encoded stream may contain whitespace as needed by the medium it will be transferred across. For example, some mediums may have a maximum line length, or require indentation.

For human input, it may be helpful to break up long sequences into shorter chunks using whitespace to make the sequence easier to input without losing your place.

It is up to the encoder or human to decide when and how whitespace will occur. A decoder must accept and discard all whitespace characters while processing the stream.

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

In the last (possibly partial) group, the number of characters indicates how many bytes of data remain to be decoded:

| Characters | Remaining Bytes |
| ---------- | --------------- |
| 1          | 0               |
| 2          | 1               |
| 3          | 1               |
| 4          | 2               |
| 5          | 3               |
| 6          | 3               |
| 7          | 4               |
| 8          | 5               |

Notes:

 * Excess bits in partial groups must be set to 0, and must be discarded by the decoder.
 * Excess character lengths of 1, 3, and 6 should in theory never happen, but must nonetheless be handled by the decoder.


Examples
--------

    Data:    {0x39, 0x12, 0x82, 0xe1, 0x81, 0x39, 0xd9, 0x8b, 0x39, 0x4c, 0x63, 0x9d, 0x04, 0x8c}
    Encoded: 85a96sd288dsqfbddffha40

    Data:    {0xe6, 0x12, 0xa6, 0x9f, 0xf8, 0x38, 0x6d, 0x7b, 0x01, 0x99, 0x3e, 0x6c, 0x53, 0x7b, 0x60})
    Encoded: wsabe8zs82qrq0dt8tq67yv0

    Data:    {0x21, 0xd1, 0x7d, 0x3f, 0x21, 0xc1, 0x88, 0x99, 0x71, 0x45, 0x96, 0xad, 0xcc, 0x96, 0x79, 0xd8})
    Encoded: 589rugt2s75akwb6kuqwt6mtv0

------------------------------------------------------------------------------



Safe32L
=======

While safe32 is sufficient for most systems, there are transmission mediums where no clear end marker exists for the encoded data field, or where no guarantee exists for detecting truncated data. In such cases, it is desirable to prefix a length field so that the receiving end can be sure of a complete transfer.


Encoding
--------

Safe32L works essentially the same as safe32, except that it is prefixed by a length field. The length field is built incrementally using the same encoding alphabet as the data, until sufficient bits are available to encode the length of the data.

The length encoding uses the lower 4 bits for data, and the high bit as a continuation bit:

| Bit Position | 4 | 3 | 2 | 1 | 0 |
| ------------ | - | - | - | - | - |
| **Purpose**  | c | x | x | x | x |

 * c = continuation bit
 * x = data

When the continuation bit is set to 1, the length field is continued in the next character. Building of the length field continues until a continuation bit of 0 is encountered. The 4 bit chunks are interpreted in big endian order (the first character represents the highest 4 bits, then the next lower 4 bits, and so on).

| Characters | Bits | Maximum encodable length |
| ---------- | ---- | ------------------------ |
| 1          |    4 |                       15 |
| 2          |    8 |                      255 |
| 3          |   12 |                     4095 |
| 4          |   16 |                    65535 |
| 5          |   20 |                  1048575 |
| 6          |   24 |                 16777215 |
| ...        |  ... |                      ... |

Note: The length field encodes the length of the **non-encoded source data**, not the encoded data or the length field itself.


Whitespace
----------

The length field may also be broken up using the same whitespace rules as for safe32.


Truncation Detection
--------------------

Should truncation occur anywhere in the encoded sequence, one of two things will happen:

 1. The decoded data length won't match the length field.
 2. The length field won't have a character with the continuation bit cleared.


Examples
--------

| Length | Encoded Bits        | Length Field |
| ------ | ------------------- | ------------ |
|      1 | `00001`             | `0`          |
|     15 | `01111`             | `g`          |
|     16 | `10001 00000`       | `j0`         |
|   2000 | `10111 11101 00000` | `rx0`        |

Example: Length field & data:

Data:

    0x21, 0xd1, 0x7d, 0x3f, 0x21, 0xc1, 0x88, 0x99,
    0x71, 0x45, 0x96, 0xad, 0xcc, 0x96, 0x79, 0xd8

Encoded:

    j0589rugt2s75akwb6kuqwt6mtv0

Where `j0` is the length field (16)


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
