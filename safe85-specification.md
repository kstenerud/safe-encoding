Safe85 Encoding
===============

Safe85 is a binary data encoding scheme that is safe to be passed through processing systems expecting human readable text.

### Features:

 * Safe for use in JSON, SGML formats, source code string literals, without escaping
 * Safe for use in path, query, and fragment components of URIs
 * Safe for use in formatted documents
 * Safe for use in legacy text processing systems
 * Alternate Form with Support for length fields
 * Optional run-length encoding

### Advantages over base85:

 * All of the abovementioned features
 * Smaller alphabet
 * No padding
 * No escaping necessary
 * Liberal whitespace rules
 * Reliable truncation detection
 * Safe for filenames on Linux, UNIX, POSIX, Mac filesystems
 * Sortable in generic text sorting algorithms



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

- [Safe85 Encoding](#safe85-encoding)
    - [Features:](#features)
    - [Advantages over base85:](#advantages-over-base85)
  - [Terms and Conventions](#terms-and-conventions)
  - [Contents](#contents)
  - [Encoding](#encoding)
  - [Encoding Process](#encoding-process)
    - [Groups](#groups)
      - [4 Byte Full Group](#4-byte-full-group)
      - [3 Byte Partial Group](#3-byte-partial-group)
      - [2 Byte Partial Group](#2-byte-partial-group)
      - [1 Byte Partial Group](#1-byte-partial-group)
    - [Final Group](#final-group)
    - [Alphabet](#alphabet)
      - [Choice of Alphabet](#choice-of-alphabet)
  - [Whitespace](#whitespace)
  - [Examples](#examples)
  - [Filenames](#filenames)
- [Safe85L Encoding](#safe85l-encoding)
  - [Encoding](#encoding-1)
  - [Whitespace](#whitespace-1)
  - [Truncation Detection](#truncation-detection)
  - [Examples](#examples-1)
      - [Example: Length field \& data:](#example-length-field--data)
  - [Filenames](#filenames-1)
  - [Advantages over base85 padding](#advantages-over-base85-padding)
  - [Version History](#version-history)
  - [License](#license)



Encoding
--------

Safe85 encoding uses an alphabet of 85 characters from the single-byte printable UTF-8 set to represent radix-85 chunks (where each chunk has an individual value from 0 - 84). These chunks are grouped magnitudally into big-endian sequences of 5 chunks, giving a range of 85^5 = (0x108780EC5), of which only 0x100000000 values are used, giving 32 bits (4 bytes) of data storage per group. Such an ecoding scheme multiplies the size of the data by a factor of 1.25.

The following semi-accurate approximation shows the general idea:

    Original: [aaaaaaaa] [bbbbbbbb] [cccccccc] [dddddddd]
    Encoded:  [aaaaaa] [abbbbb] [bbbccc] [cccccd] [dddddd]



Encoding Process
----------------

The encoding process encodes groups of 4 bytes, outputting 5 chunks per group. If the source data length is not a multiple of 4, then the final group is output as a partial group, using only as many chunks as is necessary to encode the remaining bytes, with the unused high portion of the highest chunk cleared.


### Groups

Encoding is normally done in 4-byte groups. However, if the source data is not a multiple of 4 bytes, the final group will be a partial group.

#### 4 Byte Full Group

First, an accumulator is filled big-endian style with 4 bytes of data:

    accumulator = (bytes[0] << 24) | (bytes[1] << 16) | (bytes[2] << 8) | bytes[3]

Next, the accumulator is broken down big-endian style into radix-85 chunks:

    chunk[0] = (accumulator / 52200625) % 85
    chunk[1] = (accumulator / 614125) % 85
    chunk[2] = (accumulator / 7225) % 85
    chunk[3] = (accumulator / 85) % 85
    chunk[4] = accumulator % 85

Since the accumulator's allowed range is from 0 - 0xffffffff, any combinations of chunks that exceed 0xffffffff (any chunk sequence > `82 23 54 12 0`) are not allowed. This also implies that the first chunk **CANNOT** be larger than 82.

| Chunk 0 | Chunk 1 | Chunk 2 | Chunk 3 | Chunk 4 |
| ------- | ------- | ------- | ------- | ------- |
| 0 - 82  | 0 - 84  | 0 - 84  | 0 - 84  | 0 - 84  |

#### 3 Byte Partial Group

Approximation:

    Bytes:  [aaaaaaaa] [bbbbbbbb] [cccccccc]
    Chunks: [000aaa] [aaaabb] [bbbbbc] [cccccc]

Algorithm:

    accumulator = (bytes[0] << 16) | (bytes[1] << 8) | bytes[2]
    chunk[0] = (accumulator / 614125) % 85
    chunk[1] = (accumulator / 7225) % 85
    chunk[2] = (accumulator / 85) % 85
    chunk[3] = accumulator % 85

#### 2 Byte Partial Group

Approximation:

    Bytes:  [aaaaaaaa] [bbbbbbbb]
    Chunks: [0000aa] [aaaaab] [bbbbbb]

Algorithm:

    accumulator = (bytes[0] << 8) | bytes[1]
    chunk[0] = (accumulator / 7225) % 85
    chunk[1] = (accumulator / 85) % 85
    chunk[2] = accumulator % 85

#### 1 Byte Partial Group

Approximation:

    Bytes:  [aaaaaaaa]
    Chunks: [00000a] [aaaaaa]

Algorithm:

    accumulator = bytes[0]
    chunk[0] = (accumulator / 85) % 85
    chunk[1] = accumulator % 85


### Final Group

In the last (possibly partial) group, the number of remaining characters indicates how many bytes of data remain to be decoded, and whether truncation has been detected. Note that truncation detection is not guaranteed - for that you would need to use [Safe85L encoding](#safe85l-encoding).

| Characters | Bytes | Status         |
| ---------- | ----- | -------------- |
| 1          | -     | Truncated data |
| 2          | 1     | OK             |
| 3          | 2     | OK             |
| 4          | 3     | OK             |
| 5          | 4     | OK             |


### Alphabet

Once the chunk values have been determined, they are output as characters according to the following alphabet:

| Value  | Char | Value  | Char | Value  | Char | Value  | Char     | Value  | Char | Value  | Char |
| ------ | ---- | ------ | ---- | ------ | ---- | ------ | -------- | ------ | ---- | ------ | ---- |
| **00** | `!`  | **10** | `7`  | **20** | `I`  | **30** | `Y`      | **40** | `j`  | **50** | `z`  |
| **01** | `$`  | **11** | `8`  | **21** | `J`  | **31** | `Z`      | **41** | `k`  | **51** | `{`  |
| **02** | `(`  | **12** | `9`  | **22** | `K`  | **32** | `[`      | **42** | `l`  | **52** | `\|` |
| **03** | `)`  | **13** | `:`  | **23** | `L`  | **33** | `]`      | **43** | `m`  | **53** | `}`  |
| **04** | `*`  | **14** | `;`  | **24** | `M`  | **34** | `^`      | **44** | `n`  | **54** | `~`  |
| **05** | `+`  | **15** | `=`  | **25** | `N`  | **35** | `_`      | **45** | `o`  |        |      |
| **06** | `,`  | **16** | `>`  | **26** | `O`  | **36** | `` ` ``  | **46** | `p`  |        |      |
| **07** | `-`  | **17** | `@`  | **27** | `P`  | **37** | `a`      | **47** | `q`  |        |      |
| **08** | `.`  | **18** | `A`  | **28** | `Q`  | **38** | `b`      | **48** | `r`  |        |      |
| **09** | `0`  | **19** | `B`  | **29** | `R`  | **39** | `c`      | **49** | `s`  |        |      |
| **0a** | `1`  | **1a** | `C`  | **2a** | `S`  | **3a** | `d`      | **4a** | `t`  |        |      |
| **0b** | `2`  | **1b** | `D`  | **2b** | `T`  | **3b** | `e`      | **4b** | `u`  |        |      |
| **0c** | `3`  | **1c** | `E`  | **2c** | `U`  | **3c** | `f`      | **4c** | `v`  |        |      |
| **0d** | `4`  | **1d** | `F`  | **2d** | `V`  | **3d** | `g`      | **4d** | `w`  |        |      |
| **0e** | `5`  | **1e** | `G`  | **2e** | `W`  | **3e** | `h`      | **4e** | `x`  |        |      |
| **0f** | `6`  | **1f** | `H`  | **2f** | `X`  | **3f** | `i`      | **4f** | `y`  |        |      |

The alphabet is ordered according to the characters' ordinal positions in UTF-8, so that the resulting encoded text will sort in the same natural ordering as the data it represents.

#### Choice of Alphabet

In the lower 7-bit UTF-8/ASCII range, there are a total of 94 printable, non-whitespace characters. Since we only use 85 characters, 9 of these could be dropped. In safe85, the nine most problematic characters in modern text processing systems are dropped:

| Character | SGML | STRING | URI | FILE |
| --------- | ---- | ------ | --- | ---- |
|    `"`    |   X  |    X   |  X  |      |
|    `#`    |      |        |  X  |      |
|    `%`    |      |        |  X  |      |
|    `&`    |   X  |        |  X  |      |
|    `'`    |   X  |    X   |  X  |      |
|    `/`    |      |        |  X  |   X  |
|    `<`    |   X  |        |  X  |      |
|    `?`    |      |        |  X  |      |
|    `\`    |      |    X   |  X  |      |

**Legend**:

* **SGML**:   Reserved in SGML (such as HTML, XML) documents
* **STRING**: Reserved in string literals
* **URI**:    Reserved or problematic in URIs
* **File**:   Reserved in POSIX file systems

**Notes**:

* The three most problematic URI characters are `%` (used for percent-escaping), `?` (delimits the query portion), and `#` (delimits the fragment portion).
* Although the characters `>` `^` `` ` `` `{` `|` `}` `[` `]` are technically not allowed in URIs (`[` and `]` allowed only in the host section), most URI scanners don't reject them.
* The `>` character is only reserved in SGML after an opening `<`.



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



Examples
--------

    Data:    {0x39, 0x12, 0x82, 0xe1, 0x81, 0x39, 0xd9, 0x8b, 0x39, 0x4c, 0x63, 0x9d, 0x04, 0x8c}
    Encoded: 9F3{+RVCLI9LDzZ!4e

    Data:    {0xe6, 0x12, 0xa6, 0x9f, 0xf8, 0x38, 0x6d, 0x7b, 0x01, 0x99, 0x3e, 0x6c, 0x53, 0x7b, 0x60})
    Encoded: szEXiyl02C!Tc2o.w;X

    Data:    {0x21, 0xd1, 0x7d, 0x3f, 0x21, 0xc1, 0x88, 0x99, 0x71, 0x45, 0x96, 0xad, 0xcc, 0x96, 0x79, 0xd8})
    Encoded: 1stg+1r5~+MKP7zkj0X2



Filenames
---------

Files containing safe85 data **SHOULD** have the extension `s85`, for example `mydata.s85`.

------------------------------------------------------------------------------



Safe85L Encoding
================

While safe85 is sufficient for most systems, there are transmission mediums where no clear end marker exists for the encoded data field, or where no guarantee exists for detecting truncated data. In such cases, it is desirable to prefix a length field so that the receiving end will be sure of a complete transfer.



Encoding
--------

Safe85L works essentially the same as safe85, except that it is prefixed by a length field. The length field is built incrementally using **only the first 64 characters of the safe85 encoding alphabet** (for a 6-bit value), until sufficient bits are available to encode the length of the data. Characters `j` (index 64) and above are invalid for the length field.

The length encoding uses the lower 5 bits for data, and the high bit as a continuation bit:

| Bit Position | 5 | 4 | 3 | 2 | 1 | 0 |
| ------------ | - | - | - | - | - | - |
| **Purpose**  | c | x | x | x | x | x |

 * c = continuation bit
 * x = data

While the continuation bit is set to 1, the length field is continued in the next character. Building of the length field continues until a continuation bit of 0 is encountered. The 5-bit data chunks are interpreted in big endian order (the first character represents the highest 5 bits, then the next lower 5 bits, and so on).

| Characters | Bits | Maximum encodable length |
| ---------- | ---- | ------------------------ |
| 1          |    5 |                       31 |
| 2          |   10 |                     1023 |
| 3          |   15 |                    32767 |
| 4          |   20 |                  1048575 |
| 5          |   25 |                 33554431 |
| 6          |   30 |               1073741823 |
| ...        |  ... |                      ... |

Note: The length field encodes the length of the **non-encoded source data** - not the encoded result, and not including the length field itself.



Whitespace
----------

The length field **CAN** contain whitespace at any point in the stream, following the same rules as for safe85.



Truncation Detection
--------------------

If truncation occurs anywhere in the encoded sequence (length or data), one of two things will happen:

 1. The decoded data length won't match the length field.
 2. The length field won't have a character with the continuation bit cleared.



Examples
--------

| Length | Encoded Bits           | Length Field |
| ------ | ---------------------- | ------------ |
|      1 | `000001`               | `$`          |
|     31 | `011111`               | `H`          |
|     32 | `100001 000000`        | `J!`         |
|   2000 | `100001 111110 010000` | `Jh7`        |

#### Example: Length field & data:

Data:

    0x21, 0x7b, 0x01, 0x99, 0x3e, 0xd1, 0x7d, 0x3f,
    0x21, 0x8b, 0x39, 0x4c, 0x63, 0xc1, 0x88, 0x21,
    0xc1, 0x88, 0x99, 0x71, 0xa6, 0x9f, 0xf8, 0x45,
    0x96, 0xe1, 0x81, 0x39, 0xad, 0xcc, 0x96, 0x79,
    0xd8

Encoded:

    J$1ja=a;71mK1lIG[I+9|Mh81U!_X!`XYRvJ]as!._(W

In this case, the length field is `J$` (33)



Filenames
---------

Files containing safe85l data **SHOULD** have the extension `s85l`, for example `mydata.s85l`.


Advantages over base85 padding
------------------------------

 * Base85 requires the application developer to write code to pad the source data, leaking a lower layer's implementation details to a higher one.
 * Truncation is always detected. With base85 padding, truncation on a 5-character boundary will not be detected.
 * Lower data usage for smaller data. base85's padding scheme uses an average of 2 characters no matter the length of the data. Safe85L uses only 1 byte for lengths 31 and under. By the time its size performance suffers in comparison to base85 (at length 1024), the character length difference is already less than 0.1% of the total payload size, and shrinks from there.



Version History
---------------

 * May 26, 2019: Version 2
   - Removed `%` from the alphabet because it is used for escape encoding.
   - Added `:` to the alphabet because it is only reserved in the scheme component.
 * January 29, 2019: Version 1
 * January 14, 2019: Preview Version 1



License
-------

Copyright (c) 2019 Karl Stenerud. All rights reserved.

Distributed under the Creative Commons Attribution License: https://creativecommons.org/licenses/by/4.0/legalcode
License deed: https://creativecommons.org/licenses/by/4.0/
