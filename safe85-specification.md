Safe85 Encoding
===============

Safe85 is a binary data encoding scheme that is safe to be passed through processing systems expecting human readable text.

### Features:

 * Safe for use in JSON, SGML formats, source code string literals, without escaping
 * Mostly safe for URLs (only sub-delimiters need to be escaped)
 * Safe for use in formatted documents
 * Safe for use in legacy text processing systems
 * Alternate Form with Support for length fields
 * Optional run-length encoding

### Advantages over base85:

 * Smaller alphabet
 * No padding characters
 * No escaping necessary
 * Liberal whitespace rules
 * Safe for filenames on Linux, UNIX, POSIX, Mac filesystems
 * Better end-of-field and truncation detection
 * Better compression
 * Sortable in generic text sorting algorithms



Encoding
--------

Safe85 encoding uses an alphabet of 85 characters from the single-byte printable UTF-8 set to represent radix-85 chunks (where each chunk has an individual value from 0 - 84). These chunks are grouped magnitudally into big-endian sequences of 5 chunks, giving a range of 85^5 = (0x108780EC5), of which only 0x100000000 values are used, giving 32 bits (4 bytes) of data storage per group. Such an ecoding scheme multiplies the size of the data by a factor of 1.25.

The following semi-accurate approximation shows the general idea:

    Original: [aaaaaaaa] [bbbbbbbb] [cccccccc] [dddddddd]
    Encoded:  [aaaaaa] [abbbbb] [bbbccc] [cccccd] [dddddd]


### Encoding Process

The encoding process encodes groups of 4 bytes, outputting 5 chunks per group. If the source data length is not a multiple of 4, then the final group is output as a partial group, using only as many chunks as is necessary to encode the remaining bytes, with the unused high portion of the highest chunk cleared.

#### 4 Byte Full Group

First, an accumulator is filled big-endian style with 4 bytes of data:

    accumulator = (bytes[0] << 24) | (bytes[1] << 16) | (bytes[2] << 8) | bytes[3]

Next, the accumulator is broken down big-endian style into radix-85 chunks:

    chunk[0] = (accumulator / 52200625) % 85
    chunk[1] = (accumulator / 614125) % 85
    chunk[2] = (accumulator / 7225) % 85
    chunk[3] = (accumulator / 85) % 85
    chunk[4] = accumulator % 85

Since the accumulator's allowed range is from 0 - 0xffffffff, any combinations of chunks that exceed 0xffffffff (any chunk sequence > `82 23 54 12 0`) are not allowed. This also implies that the first chunk cannot be larger than 82, which will be important in a later section.

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

#### Alphabet

Once the chunk values have been determined, they are output as characters according to the following alphabet:

| Value  | Char | Value  | Char | Value  | Char | Value  | Char     | Value  | Char | Value  | Char |
| ------ | ---- | ------ | ---- | ------ | ---- | ------ | -------- | ------ | ---- | ------ | ---- |
| **00** | `!`  | **10** | `6`  | **20** | `I`  | **30** | `Y`      | **40** | `j`  | **50** | `z`  |
| **01** | `$`  | **11** | `7`  | **21** | `J`  | **31** | `Z`      | **41** | `k`  | **51** | `{`  |
| **02** | `%`  | **12** | `8`  | **22** | `K`  | **32** | `[`      | **42** | `l`  | **52** | `|`  |
| **03** | `(`  | **13** | `9`  | **23** | `L`  | **33** | `]`      | **43** | `m`  | **53** | `}`  |
| **04** | `)`  | **14** | `;`  | **24** | `M`  | **34** | `^`      | **44** | `n`  | **54** | `~`  |
| **05** | `*`  | **15** | `=`  | **25** | `N`  | **35** | `_`      | **45** | `o`  |        |      |
| **06** | `+`  | **16** | `>`  | **26** | `O`  | **36** | `` ` ``  | **46** | `p`  |        |      |
| **07** | `,`  | **17** | `@`  | **27** | `P`  | **37** | `a`      | **47** | `q`  |        |      |
| **08** | `-`  | **18** | `A`  | **28** | `Q`  | **38** | `b`      | **48** | `r`  |        |      |
| **09** | `.`  | **19** | `B`  | **29** | `R`  | **39** | `c`      | **49** | `s`  |        |      |
| **0a** | `0`  | **1a** | `C`  | **2a** | `S`  | **3a** | `d`      | **4a** | `t`  |        |      |
| **0b** | `1`  | **1b** | `D`  | **2b** | `T`  | **3b** | `e`      | **4b** | `u`  |        |      |
| **0c** | `2`  | **1c** | `E`  | **2c** | `U`  | **3c** | `f`      | **4c** | `v`  |        |      |
| **0d** | `3`  | **1d** | `F`  | **2d** | `V`  | **3d** | `g`      | **4d** | `w`  |        |      |
| **0e** | `4`  | **1e** | `G`  | **2e** | `W`  | **3e** | `h`      | **4e** | `x`  |        |      |
| **0f** | `5`  | **1f** | `H`  | **2f** | `X`  | **3f** | `i`      | **4f** | `y`  |        |      |

The alphabet is ordered according to the characters' ordinal positions in UTF-8, so that the resulting encoded text will sort in the same order as the data it represents.


### Run-Length Encoding

Certain kinds of data will contain the same byte value repeated many times. In such cases, it's more space efficient to encode the data in run-lengths.

Since the initial chunk values `83` and `84` are invalid (they'd place the accumulator outside of the range 0 - 0xffffffff no matter what the other chunks are), we assign these special meanings as run-length encoding initiators.


#### 3-Character Run-Length Encoding

3-character run-length encoding is marked by the special initial chunk value `83`. Upon encountering this chunk, the two subsequent characters are decoded and combined mathematically to build a joint repeat and byte value field:

| chunk | value 1 | value 0 |
| ----- | ------- | ------- |
| 83    | 0 - 84  | 0 - 84  |

The joint field is built like so: `joint_field = value1 * 85 + value0`

This gives a field value in the range 0 - 7224 (0x1c38), which is then subdivided into a repeat count, and the byte value to be repeated:

| Bits 8-12    | Bits 0-7   |
| ------------ | ---------- |
| Repeat Count | Byte Value |
| 0 - 28       | 0 - 255    |

Since a run-length sequence is 3 characters long, which in normal encoding can encode 2 bytes of data, it doesn't make sense to run-length encode sequences of less than 3 bytes of source data. We therefore implicitly add 3 to the repeat count value, giving a repeat range of 3 - 31.

    joint_field = value1 * 85 + value0
    byte_value = joint_field & 0xff
    repeat_count = (joint_field >> 8) + 3

Note: Since the value 7224 (0x1c38) does not fall on a bit boundary, only byte values from 0x00 - 0x38 can have a repeat count of 31 (28 + 3). All other byte values have a maximum repeat count of 30.

With this encoding scheme, we can encode up to 31 bytes of repeating data into 3 characters. Normal encoding would require 39 characters to encode the same data. It is thus possible to achieve up to 13x space savings over uncompressed data.


#### 4-Character Run-Length Encoding

If you have very long sequences of repeating byte values, the 4-character run-length encoding (chunk `84`) provides better compression:

| chunk | value 2 | value 1 | value 0 |
| ----- | ------- | ------- | ------- |
| 84    | 0 - 84  | 0 - 84  | 0 - 84  |

    joint_field = value2 * 7225 + value1 * 85 + value0

This gives a field value in the range 0 - 614124 (0x95eec), which is then subdivided into a repeat count, and the byte value to be repeated:

| Bits 8-19    | Bits 0-7   |
| ------------ | ---------- |
| Repeat Count | Byte Value |
| 0 - 2398     | 0 - 255    |

Since the 3-byte run-length encoding allows repeat counts up to 30 for all byte values, we implicitly add 31 to the 4-byte run-length repeat count to avoid overlap, giving a range of 31-2429.

    joint_field = value2 * 7225 + value1 * 85 + value0
    byte_value = joint_field & 0xff
    repeat_count = (joint_field >> 8) + 31

Note: Since the value 614124 (0x95eec) does not fall on a bit boundary, only byte values from 0x00 - 0xec can have a repeat count of 2429 (2398 + 31). All other byte values have a maximum count value of 2428.

With this encoding scheme, we can encode up to 2429 bytes of repeating data into 4 characters. Normal encoding would require 3037 characters to encode the same data. It is thus possible to achieve up to a 760x space savings over uncompressed data, and up to 50x savings over 3-character run-length encoding.



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
| 5          | 4               |



Examples
--------

    Data:    {0x39, 0x12, 0x82, 0xe1, 0x81, 0x39, 0xd9, 0x8b, 0x39, 0x4c, 0x63, 0x9d, 0x04, 0x8c}
    Encoded: 8F2{*RVCLI8LDzZ!3e

    Data:    {0xe6, 0x12, 0xa6, 0x9f, 0xf8, 0x38, 0x6d, 0x7b, 0x01, 0x99, 0x3e, 0x6c, 0x53, 0x7b, 0x60})
    Encoded: szEXiyl.1C!Tc1o-w;X

    Data:    {0x21, 0xd1, 0x7d, 0x3f, 0x21, 0xc1, 0x88, 0x99, 0x71, 0x45, 0x96, 0xad, 0xcc, 0x96, 0x79, 0xd8})
    Encoded: 0stg*0r4~*MKP6zkj.X1

------------------------------------------------------------------------------



Safe85L
=======

While safe85 is sufficient for most systems, there are transmission mediums where no clear end marker exists for the encoded data field, or where no guarantee exists for detecting truncated data. In such cases, it is desirable to prefix a length field so that the receiving end can be sure of a complete transfer.


Encoding
--------

Safe85L works essentially the same as safe85, except that it is prefixed by a length field. The length field is built incrementally using **only the first 64 characters of the safe85 encoding alphabet** (for a 6-bit value), until sufficient bits are available to encode the length of the data. Characters `j` (index 64) and above are invalid for the length field.

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

The length field may contain whitespace at any point in the stream, following the same rules as for safe85.


Truncation Detection
--------------------

Should truncation occur anywhere in the encoded sequence (length or data), one of two things will happen:

 1. The decoded data length won't match the length field.
 2. The length field won't have a character with the continuation bit cleared.


Examples
--------

| Length | Encoded Bits           | Length Field |
| ------ | ---------------------- | ------------ |
|      1 | `000001`               | `$`          |
|     31 | `011111`               | `H`          |
|     32 | `100001 000000`        | `J!`         |
|   2000 | `100001 111110 010000` | `Jh6`        |

#### Example: Length field & data:

Data:

    0x21, 0x7b, 0x01, 0x99, 0x3e, 0xd1, 0x7d, 0x3f,
    0x21, 0x8b, 0x39, 0x4c, 0x63, 0xc1, 0x88, 0x21,
    0xc1, 0x88, 0x99, 0x71, 0xa6, 0x9f, 0xf8, 0x45,
    0x96, 0xe1, 0x81, 0x39, 0xad, 0xcc, 0x96, 0x79,
    0xd8

Encoded:

    J$0ja=a;60mK0lIG[I*8|Mh70U!_X!`XYRvJ]as!-_%W

In this case, the length field is `J$` (33)


Advantages over base85 padding
------------------------------

 * No extra alphabet character is necessary. The length encodes using the exact same alphabet as the data encoding.
 * Truncation is always detected. With base85 padding, truncation on a 5-character boundary will not be detected.
 * Lower data usage for smaller data. base85's padding scheme uses an average of 2 characters no matter the length of the data. Safe85L uses only 1 byte for lengths 31 and under. By the time its size performance suffers in comparison to base85 (at length 1024), the character length difference is already less than 0.1% of the total payload size, and shrinks from there.



Version History
---------------

 * January 14, 2019: Preview Version 1



License
-------

Copyright (c) 2019 Karl Stenerud. All rights reserved.

Distributed under the Creative Commons Attribution License: https://creativecommons.org/licenses/by/4.0/legalcode
License deed: https://creativecommons.org/licenses/by/4.0/
