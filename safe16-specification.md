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
 * Easily confusable characters & digits are interchangeable.
 * Alternate form with prefixed length

### Advantages over base16:

 * All of the abovementioned features
 * Liberal whitespace rules
 * Truncation detection



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

- [Safe16 Encoding](#safe16-encoding)
    - [Features:](#features)
    - [Advantages over base16:](#advantages-over-base16)
  - [Terms and Conventions](#terms-and-conventions)
  - [Contents](#contents)
  - [Encoding](#encoding)
    - [Encoding Process](#encoding-process)
    - [Final Group](#final-group)
    - [Alphabet](#alphabet)
  - [Whitespace](#whitespace)
  - [Examples](#examples)
  - [Filenames](#filenames)
- [Safe16L Encoding](#safe16l-encoding)
  - [Encoding](#encoding-1)
  - [Whitespace](#whitespace-1)
  - [Truncation Detection](#truncation-detection)
  - [Examples](#examples-1)
  - [Filenames](#filenames-1)
  - [Advantages over base16](#advantages-over-base16-1)
  - [Version History](#version-history)
  - [License](#license)



Encoding
--------

Safe16 encoding uses an alphabet of 16 characters from the single-byte printable UTF-8 set to represent radix-16 chunks (where each chunk has an individual value from 0 - 15). These chunks are grouped magnitudally into big-endian sequences of 2 chunks, giving a range of 16^2 = 256 (0x100), which allows 8 bits (1 byte) of data storage per group. Such an ecoding scheme multiplies the size of the data by a factor of 2.0.

Layout:

    Original: [aaaaaaaa]
    Encoded:  [aaaa] [aaaaa]


### Encoding Process

Each byte is split into a group of two radix-16 chunks:

    chunk[0] = (accumulator >> 4) & 0x0f
    chunk[1] = accumulator & 0x0f

| Chunk 0 | Chunk 1 |
| ------- | ------- |
| 0 - 15  | 0 - 15  |


### Final Group

In the last (possibly partial) group of the input stream, the number of remaining characters indicates how many bytes of data remain to be decoded, and whether truncation has been detected. Note that truncation detection is not guaranteed - for that you would need to use [Safe16L encoding](#safe16l-encoding).

| Characters | Bytes | Status         |
| ---------- | ----- | -------------- |
| 1          | -     | Truncated data |
| 2          | 1     | OK             |

### Alphabet

Chunk values are output as characters according to the following alphabet:

| Value  | Char | Value  | Char |
| ------ | ---- | ------ | ---- |
| **00** | `0`  | **08** | `8`  |
| **01** | `1`  | **09** | `9`  |
| **02** | `2`  | **0a** | `a`  |
| **03** | `3`  | **0b** | `b`  |
| **04** | `4`  | **0c** | `c`  |
| **05** | `5`  | **0d** | `d`  |
| **06** | `6`  | **0e** | `e`  |
| **07** | `7`  | **0f** | `f`  |

The alphabet is ordered according to the characters' ordinal positions in UTF-8, so that the resulting encoded text will sort in the same natural ordering as the data it represents.

Decoders **MUST** accept a wider range of characters in order to mitigate human error:

 * All letters **CAN** be substituted with their capitals.
 * `0` **CAN** be substituted with `o` or its capital.
 * `1` **CAN** be substituted with `l` or `i` or their capitals.

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

**Note**: Dash is included as "whitespace" to allow human input sequences such as:

    85a9-6ed2-88dd-09bc



Examples
--------

    Data:    {0x39, 0x12, 0x82, 0xe1, 0x81, 0x39, 0xd9, 0x8b, 0x39, 0x4c, 0x63, 0x9d, 0x04, 0x8c}
    Encoded: 391282e18139d98b394c639d048c

    Data:    {0xe6, 0x12, 0xa6, 0x9f, 0xf8, 0x38, 0x6d, 0x7b, 0x01, 0x99, 0x3e, 0x6c, 0x53, 0x7b, 0x60})
    Encoded: e612a69ff8386d7b01993e6c537b60

    Data:    {0x21, 0xd1, 0x7d, 0x3f, 0x21, 0xc1, 0x88, 0x99, 0x71, 0x45, 0x96, 0xad, 0xcc, 0x96, 0x79, 0xd8})
    Encoded: 21d17d3f21c18899714596adcc9679d8



Filenames
---------

Files containing safe16 data **SHOULD** have the extension `s16`, for example `mydata.s16`.

------------------------------------------------------------------------------



Safe16L Encoding
================

While safe16 is sufficient for most systems, there are transmission mediums where no clear end marker exists for the encoded data field, or where no guarantee exists for detecting truncated data. In such cases, it is sometimes desirable to prefix a length field so that the receiving end will be sure of a complete transfer.



Encoding
--------

Safe16L works essentially the same as safe16, except that it is prefixed by a length field. The length field is built incrementally with 4-bit chunks encoded using the same encoding alphabet as is used for regular safe16 data, until sufficient bits are available to encode the length of the data.

In each chunk, the lower 3 bits contain data, and the high bit is a continuation bit:

| Bit Position | 3 | 2 | 1 | 0 |
| ------------ | - | - | - | - |
| **Purpose**  | c | x | x | x |

 * c = continuation bit
 * x = data

While the continuation bit is set to 1, the length field is continued in the next character. Building of the length field continues until a continuation bit of 0 is encountered. The 3-bit data chunks are interpreted in big endian order (the first character represents the highest 3 bits, then the next lower 3 bits, and so on).

| Characters | Bits | Maximum encodable length |
| ---------- | ---- | ------------------------ |
| 1          |    3 |                        7 |
| 2          |    6 |                       63 |
| 3          |    9 |                      511 |
| 4          |   12 |                     4095 |
| 5          |   15 |                    32767 |
| 6          |   18 |                   262143 |
| ...        |  ... |                      ... |

**Note**: The length field encodes the length of the **non-encoded source data** - not the encoded result, and not including the length field itself.



Whitespace
----------

The length field **CAN** also be broken up using the same whitespace rules as for safe16.



Truncation Detection
--------------------

If truncation occurs anywhere in the encoded sequence, one of two things will happen:

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



Filenames
---------

Files containing safe16l data **SHOULD** have the extension `s16l`, for example `mydata.s16l`.



Advantages over base16
----------------------

 * Truncation is detected. With base16, there is no truncation detection.



Version History
---------------

 * June 9, 2024: Version 2
   - Added missing support for substitution characters.
 * January 29, 2019: Version 1
 * January 14, 2019: Preview Version 1



License
-------

Copyright (c) 2019 Karl Stenerud. All rights reserved.

Distributed under the Creative Commons Attribution License: https://creativecommons.org/licenses/by/4.0/legalcode
License deed: https://creativecommons.org/licenses/by/4.0/
