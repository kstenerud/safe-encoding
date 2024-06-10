Safe80 Encoding
===============

Safe80 is a binary data encoding scheme that is safe to be passed through processing systems expecting human readable text.

It is specially designed to be safe for use in Windows filenames.


### Features:

 * Safe for use in JSON, SGML formats, source code string literals, without escaping
 * Safe for use in path, query, and fragment components of URIs
 * Safe for use in formatted documents
 * Safe for use in legacy text processing systems
 * Safe for filenames on Linux, UNIX, POSIX, Mac, and Windows filesystems
 * Alternate Form with Support for length fields
 * Optional run-length encoding
 * No padding
 * No escaping necessary
 * Liberal whitespace rules
 * Reliable truncation detection
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

- [Safe80 Encoding](#safe80-encoding)
    - [Features:](#features)
  - [Terms and Conventions](#terms-and-conventions)
  - [Contents](#contents)
  - [Encoding](#encoding)
  - [Encoding Process](#encoding-process)
    - [Groups](#groups)
      - [15 Byte Full Group](#15-byte-full-group)
    - [Partial Groups](#partial-groups)
      - [10 Byte Partial Group](#10-byte-partial-group)
      - [3 Byte Partial Group](#3-byte-partial-group)
      - [2 Byte Partial Group](#2-byte-partial-group)
      - [1 Byte Partial Group](#1-byte-partial-group)
    - [Final Group](#final-group)
    - [Alphabet](#alphabet)
      - [Choice of Alphabet](#choice-of-alphabet)
  - [Whitespace](#whitespace)
  - [Examples](#examples)
  - [Filenames](#filenames)
- [Safe80L Encoding](#safe80l-encoding)
  - [Encoding](#encoding-1)
  - [Whitespace](#whitespace-1)
  - [Truncation Detection](#truncation-detection)
  - [Examples](#examples-1)
      - [Example: Length field \& data:](#example-length-field--data)
  - [Filenames](#filenames-1)
  - [Version History](#version-history)
  - [License](#license)



Encoding
--------

Safe80 encoding uses an alphabet of 80 characters from the single-byte printable UTF-8 set to represent radix-80 chunks (where each chunk has an individual value from 0 - 84). These chunks are grouped magnitudally into big-endian sequences of 19 chunks, giving a range of 80^19 = (0x01158e460913d0000000000000000000), of which only 0x1000000000000000000000000000000 values are used, giving 120 bits (15 bytes) of data storage per group. Such an ecoding scheme multiplies the size of the data by a factor of 1.27.

The following semi-accurate approximation shows the general idea:

    Original: [aaaaaaaa] [bbbbbbbb] [cccccccc] [dddddddd] [eeeeeeee] [ffffffff] [gggggggg] [hhhhhhhh]
              [iiiiiiii] [jjjjjjjj] [kkkkkkkk] [llllllll] [mmmmmmmm] [nnnnnnnn] [oooooooo]
    Encoded:  [aaaaaaa] [abbbbb] [bbbccc] [cccccd] [dddddd] [deeeeee] [eeffff] [ffffgg] [ggggggh] [hhhhhh]
              [hiiiiii] [iijjjj] [jjjjkk] [kkkkkkl] [llllll] [lmmmmm] [mmmnnn] [nnnnno] [ooooooo]



Encoding Process
----------------

The encoding process encodes groups of 15 bytes, outputting 19 chunks per group. If the source data length is not a multiple of 15, then the final group is output as a partial group, using only as many chunks as is necessary to encode the remaining bytes, with the unused high portion of the highest chunk cleared.


### Groups

Encoding is normally done in 15-byte groups. However, if the source data is not a multiple of 15 bytes, the final group will be a partial group.

#### 15 Byte Full Group

First, an accumulator is filled big-endian style with 4 bytes of data:

    accumulator = (bytes[ 0] << 112) |
                  (bytes[ 1] << 104) |
                  (bytes[ 2] <<  96) |
                  (bytes[ 3] <<  88) |
                  (bytes[ 4] <<  80) |
                  (bytes[ 5] <<  72) |
                  (bytes[ 6] <<  64) |
                  (bytes[ 7] <<  56) |
                  (bytes[ 8] <<  48) |
                  (bytes[ 9] <<  40) |
                  (bytes[10] <<  32) |
                  (bytes[11] <<  24) |
                  (bytes[12] <<  16) |
                  (bytes[13] <<   8) |
                  bytes[14]

Next, the accumulator is broken down big-endian style into radix-80 chunks:

    chunk[0]  = (accumulator / 80^18) % 80
    chunk[1]  = (accumulator / 80^17) % 80
    chunk[2]  = (accumulator / 80^16) % 80
    chunk[3]  = (accumulator / 80^15) % 80
    chunk[4]  = (accumulator / 80^14) % 80
    chunk[5]  = (accumulator / 80^13) % 80
    chunk[6]  = (accumulator / 80^12) % 80
    chunk[7]  = (accumulator / 80^11) % 80
    chunk[8]  = (accumulator / 80^10) % 80
    chunk[9]  = (accumulator / 80^9) % 80
    chunk[10] = (accumulator / 80^8) % 80
    chunk[11] = (accumulator / 80^7) % 80
    chunk[12] = (accumulator / 80^6) % 80
    chunk[13] = (accumulator / 80^5) % 80
    chunk[14] = (accumulator / 80^4) % 80
    chunk[15] = (accumulator / 80^3) % 80
    chunk[16] = (accumulator / 80^2) % 80
    chunk[17] = (accumulator / 80^1) % 80
    chunk[18] = accumulator % 80

Since the accumulator's allowed range is from 0 - 0xffffffffffffffffffffffffffffff, any combinations of chunks that exceed this range are not allowed.


### Partial Groups

Partial groups are used for data sizes 1-14 bytes. Examples:

#### 10 Byte Partial Group

Approximation:

    Bytes:  [aaaaaaaa] [bbbbbbbb] [cccccccc] [dddddddd] [eeeeeeee] [ffffffff] [gggggggg] [hhhhhhhh]
    Chunks: [0aaaaa] [abbbbb] [bbbccc] [cccccd] [dddddd] [deeeeee] [eeffff] [ffffgg] [ggggggh] [hhhhhhh]

Algorithm:

    accumulator = (bytes[0] <<  72) |
                  (bytes[1] <<  64) |
                  (bytes[2] <<  56) |
                  (bytes[3] <<  48) |
                  (bytes[4] <<  40) |
                  (bytes[5] <<  32) |
                  (bytes[6] <<  24) |
                  (bytes[7] <<  16) |
                  (bytes[8] <<   8) |
                  bytes[9]

    chunk[0]  = (accumulator / 80^12) % 80
    chunk[1]  = (accumulator / 80^11) % 80
    chunk[2]  = (accumulator / 80^10) % 80
    chunk[3]  = (accumulator / 80^9) % 80
    chunk[4]  = (accumulator / 80^8) % 80
    chunk[5]  = (accumulator / 80^7) % 80
    chunk[6]  = (accumulator / 80^6) % 80
    chunk[7]  = (accumulator / 80^5) % 80
    chunk[8]  = (accumulator / 80^4) % 80
    chunk[9]  = (accumulator / 80^3) % 80
    chunk[10] = (accumulator / 80^2) % 80
    chunk[11] = (accumulator / 80^1) % 80
    chunk[12] = accumulator % 80

#### 3 Byte Partial Group

Approximation:

    Bytes:  [aaaaaaaa] [bbbbbbbb] [cccccccc]
    Chunks: [000aaa] [aaaabb] [bbbbbc] [cccccc]

Algorithm:

    accumulator = (bytes[0] << 16) | (bytes[1] << 8) | bytes[2]
    chunk[0] = (accumulator / 80^3) % 80
    chunk[1] = (accumulator / 80^2) % 80
    chunk[2] = (accumulator / 80^1) % 80
    chunk[3] = accumulator % 80

#### 2 Byte Partial Group

Approximation:

    Bytes:  [aaaaaaaa] [bbbbbbbb]
    Chunks: [0000aa] [aaaaab] [bbbbbb]

Algorithm:

    accumulator = (bytes[0] << 8) | bytes[1]
    chunk[0] = (accumulator / 80^2) % 80
    chunk[1] = (accumulator / 80^1) % 80
    chunk[2] = accumulator % 80

#### 1 Byte Partial Group

Approximation:

    Bytes:  [aaaaaaaa]
    Chunks: [00000a] [aaaaaa]

Algorithm:

    accumulator = bytes[0]
    chunk[0] = (accumulator / 80^1) % 80
    chunk[1] = accumulator % 80

### Final Group

In the last (possibly partial) group, the number of remaining characters indicates how many bytes of data remain to be decoded, and whether truncation has been detected. Note that truncation detection is not guaranteed - for that you would need to use [Safe80L encoding](#safe80l-encoding).

| Characters | Bytes | Status         |
| ---------- | ----- | -------------- |
| 1          | -     | Truncated data |
| 2          | 1     | OK             |
| 3          | 2     | OK             |
| 4          | 3     | OK             |
| 5          | -     | Truncated data |
| 6          | 4     | OK             |
| 7          | 5     | OK             |
| 8          | 6     | OK             |
| 9          | 7     | OK             |
| 10         | -     | Truncated data |
| 11         | 8     | OK             |
| 12         | 9     | OK             |
| 13         | 10    | OK             |
| 14         | 11    | OK             |
| 15         | -     | Truncated data |
| 16         | 12    | OK             |
| 17         | 13    | OK             |
| 18         | 14    | OK             |
| 19         | 15    | OK             |

### Alphabet

Chunk values are output as characters according to the following alphabet:

| Value  | Char | Value  | Char | Value  | Char | Value  | Char     | Value  | Char |
| ------ | ---- | ------ | ---- | ------ | ---- | ------ | -------- | ------ | ---- |
| **00** | `!`  | **10** | `9`  | **20** | `M`  | **30** | `^`      | **40** | `n`  |
| **01** | `$`  | **11** | `;`  | **21** | `N`  | **31** | `_`      | **41** | `o`  |
| **02** | `(`  | **12** | `=`  | **22** | `O`  | **32** | `` ` ``  | **42** | `p`  |
| **03** | `)`  | **13** | `@`  | **23** | `P`  | **33** | `a`      | **43** | `q`  |
| **04** | `+`  | **14** | `A`  | **24** | `Q`  | **34** | `b`      | **44** | `r`  |
| **05** | `,`  | **15** | `B`  | **25** | `R`  | **35** | `c`      | **45** | `s`  |
| **06** | `-`  | **16** | `C`  | **26** | `S`  | **36** | `d`      | **46** | `t`  |
| **07** | `0`  | **17** | `D`  | **27** | `T`  | **37** | `e`      | **47** | `u`  |
| **08** | `1`  | **18** | `E`  | **28** | `U`  | **38** | `f`      | **48** | `v`  |
| **09** | `2`  | **19** | `F`  | **29** | `V`  | **39** | `g`      | **49** | `w`  |
| **0a** | `3`  | **1a** | `G`  | **2a** | `W`  | **3a** | `h`      | **4a** | `x`  |
| **0b** | `4`  | **1b** | `H`  | **2b** | `X`  | **3b** | `i`      | **4b** | `y`  |
| **0c** | `5`  | **1c** | `I`  | **2c** | `Y`  | **3c** | `j`      | **4c** | `z`  |
| **0d** | `6`  | **1d** | `J`  | **2d** | `Z`  | **3d** | `k`      | **4d** | `{`  |
| **0e** | `7`  | **1e** | `K`  | **2e** | `[`  | **3e** | `l`      | **4e** | `}`  |
| **0f** | `8`  | **1f** | `L`  | **2f** | `]`  | **3f** | `m`      | **4f** | `~`  |

The alphabet is ordered according to the characters' ordinal positions in UTF-8, so that the resulting encoded text will sort in the same natural ordering as the data it represents.

#### Choice of Alphabet

In the lower 7-bit UTF-8/ASCII range, there are a total of 94 printable, non-whitespace characters. Since we only use 80 characters, 14 of these could be dropped.

Filename restrictions in Windows are described here: https://docs.microsoft.com/en-us/windows/desktop/FileIO/naming-a-file

In short, the following characters are reserved in Windows: `< > : " / \ | ? *`

As well, a Windows filename **CANNOT** not end with a `.`

If we ignore the special device Windows filename restrictions (`CON`, `PRN`, `LPT1`, etc), we end up with the following matrix of 14 disallowed characters for all systems we wish to support:

| Character | SGML | STRING | URI | FILE | Windows |
| --------- | ---- | ------ | --- | ---- | ------- |
|    `"`    |   X  |    X   |  X  |      |    X    |
|    `#`    |      |        |  X  |      |         |
|    `%`    |      |        |  X  |      |         |
|    `&`    |   X  |        |  X  |      |         |
|    `'`    |   X  |    X   |  X  |      |         |
|    `/`    |      |        |  X  |   X  |    X    |
|    `:`    |      |        |  X  |      |    X    |
|    `<`    |   X  |        |  X  |      |    X    |
|    `?`    |      |        |  X  |      |    X    |
|    `\`    |      |    X   |  X  |      |    X    |
|    `>`    |      |        |  X  |      |    X    |
|    `\|`   |      |        |  X  |      |    X    |
|    `*`    |      |        |  X  |      |    X    |
|    `.`    |      |        |     |      |    X    |

**Legend**:

* **SGML**:    Reserved in SGML (such as HTML, XML) documents
* **STRING**:  Reserved in string literals
* **URI**:     Reserved or problematic in the path, query, or fragment components of URIs
* **File**:    Reserved in POSIX file systems
* **Windows**: Reserved in Windows file systems

Although the characters `^` `` ` `` `{` `}` `[` `]` are technically not allowed in URIs (`[` and `]` allowed only in the host section), most URI scanners don't reject them.



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
    Encoded: ,4@yggKKdSTm[V+^oj

    Data:    {0xe6, 0x12, 0xa6, 0x9f, 0xf8, 0x38, 0x6d, 0x7b, 0x01, 0x99, 0x3e, 0x6c, 0x53, 0x7b, 0x60})
    Encoded: pF2U]^CJPSTQXo0KB[!

    Data:    {0x21, 0xd1, 0x7d, 0x3f, 0x21, 0xc1, 0x88, 0x99, 0x71, 0x45, 0x96, 0xad, 0xcc, 0x96, 0x79, 0xd8})
    Encoded: 2imlk)-I2HaWeWjS}}F(f



Filenames
---------

Files containing safe80 data **SHOULD** have the extension `s80`, for example `mydata.s80`.

------------------------------------------------------------------------------



Safe80L Encoding
================

While safe80 is sufficient for most systems, there are transmission mediums where no clear end marker exists for the encoded data field, or where no guarantee exists for detecting truncated data. In such cases, it is desirable to prefix a length field so that the receiving end will be sure of a complete transfer.



Encoding
--------

Safe80L works essentially the same as safe80, except that it is prefixed by a length field. The length field is built incrementally using **only the first 64 characters of the safe80 encoding alphabet** (for a 6-bit value), until sufficient bits are available to encode the length of the data. Characters `n` (index 64) and above are invalid for the length field.

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

The length field **CAN** contain whitespace at any point in the stream, following the same rules as for safe80.



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
|     31 | `011111`               | `L`          |
|     32 | `100001 000000`        | `N!`         |
|   2000 | `100001 111110 010000` | `Nl9`        |

#### Example: Length field & data:

Data:

    0x21, 0x7b, 0x01, 0x99, 0x3e, 0xd1, 0x7d, 0x3f,
    0x21, 0x8b, 0x39, 0x4c, 0x63, 0xc1, 0x88, 0x21,
    0xc1, 0x88, 0x99, 0x71, 0xa6, 0x9f, 0xf8, 0x45,
    0x96, 0xe1, 0x81, 0x39, 0xad, 0xcc, 0x96, 0x79,
    0xd8

Encoded:

    N$2b!^f__]K$k{8B@]9+v2hInzMsV{}`Hbiz0u]I@Asv

In this case, the length field is `N$` (33)



Filenames
---------

Files containing safe80l data **SHOULD** have the extension `s80l`, for example `mydata.s80l`.



Version History
---------------

 * May 26, 2019: Version 1
 * March 24, 2019: Preview Version 1



License
-------

Copyright (c) 2019 Karl Stenerud. All rights reserved.

Distributed under the Creative Commons Attribution License: https://creativecommons.org/licenses/by/4.0/legalcode
License deed: https://creativecommons.org/licenses/by/4.0/
