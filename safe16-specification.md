Safe16 Encoding
===============

Safe16 provides a binary data encoding scheme that is safe to be input by humans, and safe to be passed through processing systems that expect human readable text.

It is especially useful for things requiring human input such as activation codes.

### Features:

 * Safe for use in JSON, SGML formats, source code strings, without escaping
 * Safe for use in URLs without escaping
 * Safe for use in filenames
 * Safe for use in formatted documents
 * Safe for use in legacy text processing systems
 * Support for length fields
 * Human readable values
 * Uppercase and lowercase characters are interchangeable.
 * Alternate form with prefixed length

### Advantages over base16:

 * Liberal whitespace rules
 * Truncation detection


Encoding
--------

Safe64 encoding uses an alphabet of 16 characters from the single-byte UTF-8 set to represent 5-bit values. These characters are grouped together by 8 (forming 40 bits), which can then be used to encode 5 bytes of data per group. This multiplies the size of the encoded data by a factor of 1.6.

    Original: [aaaaaaaa]
    Encoded:  [aaaa] [aaaaa]

The 4-bit alphabet:

| Value  | Char |
| ------ | ---- |
| **00** | `0`  |
| **01** | `1`  |
| **02** | `2`  |
| **03** | `3`  |
| **04** | `4`  |
| **05** | `5`  |
| **06** | `6`  |
| **07** | `7`  |
| **08** | `8`  |
| **09** | `9`  |
| **0a** | `a`  |
| **0b** | `b`  |
| **0c** | `c`  |
| **0d** | `d`  |
| **0e** | `e`  |
| **0f** | `f`  |

The alphabet is ordered according to the characters' ordinal positions in UTF-8, so that the resulting encoded text will sort in the same order as the data it represents.

Uppercase letters may be substituted for lowercase letters (for example, `A` may be substituted for `a`).

A human may input substitutes. A decoder must accept substitutes. An encoder must not output substitutes.


Whitespace
----------

An encoded stream may contain whitespace as needed by the medium it will be transferred across. For example, some mediums may have a maximum line length, or require indentation.

For human input, it may be helpful to break up long sequences into shorter chunks using whitespace to make the sequence easier to input without losing your place.

It is up to the encoder or human to decide when and how whitespace will occur. Whitespace may occur at any point in the data stream. A decoder must accept and discard all whitespace characters while processing the stream.

For the purposes of this spec, only the following characters qualify as whitespace:

| Code Point | Name            |
| ---------- | --------------- |
| 0009       | Horizontal Tab  |
| 000a       | Line Feed       |
| 000d       | Carriage Return |
| 0020       | Space           |
| 002D       | Dash `-`        |

Note: Dash is included as "whitespace" to allow human input sequences such as:

    85a9-6ed2-88dd-09bc


Examples
--------

    Data:    {0x39, 0x12, 0x82, 0xe1, 0x81, 0x39, 0xd9, 0x8b, 0x39, 0x4c, 0x63, 0x9d, 0x04, 0x8c}
    Encoded: 391282e18139d98b394c639d048c

    Data:    {0xe6, 0x12, 0xa6, 0x9f, 0xf8, 0x38, 0x6d, 0x7b, 0x01, 0x99, 0x3e, 0x6c, 0x53, 0x7b, 0x60})
    Encoded: e612a69ff8386d7b01993e6c537b60

    Data:    {0x21, 0xd1, 0x7d, 0x3f, 0x21, 0xc1, 0x88, 0x99, 0x71, 0x45, 0x96, 0xad, 0xcc, 0x96, 0x79, 0xd8})
    Encoded: 21d17d3f21c18899714596adcc9679d8

------------------------------------------------------------------------------



Safe16L
=======

While safe16 is sufficient for most systems, there are transmission mediums where no clear end marker exists for the encoded data field, or where no guarantee exists for detecting truncated data. In such cases, it is desirable to prefix a length field so that the receiving end can be sure of a complete transfer.


Encoding
--------

Safe16L works essentially the same as safe16, except that it is prefixed by a length field. The length field is built incrementally using the same encoding alphabet as the data, until sufficient bits are available to encode the length of the data.

The length encoding uses the lower 3 bits for data, and the high bit as a continuation bit:

| Bit Position | 3 | 2 | 1 | 0 |
| ------------ | - | - | - | - |
| **Purpose**  | c | x | x | x |

 * c = continuation bit
 * x = data

When the continuation bit is set to 1, the length field is continued in the next character. Building of the length field continues until a continuation bit of 0 is encountered. The 3 bit chunks are interpreted in big endian order (the first character represents the highest 3 bits, then the next lower 3 bits, and so on).

| Characters | Bits | Maximum encodable length |
| ---------- | ---- | ------------------------ |
| 1          |    3 |                        7 |
| 2          |    6 |                       63 |
| 3          |    9 |                      511 |
| 4          |   12 |                     4095 |
| 5          |   15 |                    32767 |
| 6          |   18 |                   262143 |
| ...        |  ... |                      ... |

Note: The length field encodes the length of the **non-encoded source data**, not the encoded data or the length field itself.


Whitespace
----------

The length field may also be broken up using the same whitespace rules as for safe16.


Truncation Detection
--------------------

Should truncation occur anywhere in the encoded sequence, one of two things will happen:

 1. The decoded data length won't match the length field.
 2. The length field won't have a character with the continuation bit cleared.


Examples
--------

| Length | Encoded Bits          | Length Field |
| ------ | --------------------- | ------------ |
|      1 | `0001`                | `1`          |
|      7 | `0111`                | `7`          |
|      8 | `1001 0000`           | `90`         |
|   2000 | `1011 1111 1010 0000` | `bfa0`       |

Example: Length field & data:

Data:

    0x21, 0xd1, 0x7d, 0x3f, 0x21, 0xc1, 0x88, 0x99,
    0x71, 0x45, 0x96, 0xad, 0xcc, 0x96, 0x79, 0xd8

Encoded:

    a021d17d3f21c18899714596adcc9679d8

Where `a0` is the length field (16)


Advantages over base16
----------------------

 * Truncation is detected. With base16, there is no truncation detection.



Version History
---------------

 * January 14, 2019: Preview Version 1



License
-------

Copyright (c) 2019 Karl Stenerud. All rights reserved.

Distributed under the Creative Commons Attribution License: https://creativecommons.org/licenses/by/4.0/legalcode
License deed: https://creativecommons.org/licenses/by/4.0/
