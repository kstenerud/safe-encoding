Safe64 Encoding
===============

Safe64 provides a binary data encoding scheme that is safe to be passed through processing systems that expect human readable text.

### Features:

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

Safe64 encodes 6 bits into each character, effectively encoding 3 bytes into every 4 characters, for a total size bloat of 1.33.

Safe64 uses the following alphabet to represent 6-bit values, all of which are safe for known text systems:

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

Termination of a safe64 sequence can be inferred by the number of trailing characters after the last group of 4:

| Trailing characters | Remaining bytes |
| ------------------- | --------------- |
| 0                   | 0               |
| 1                   | invalid         |
| 2                   | 1               |
| 3                   | 2               |

Any excess bits after the last 8-bit sequence must be set to 0 by the encoder, and must be discarded by the decoder.



Safe64l
=======

While safe64 is sufficient for most systems, there are transmission mediums that do not guarantee detection of truncated data. In such a case, it is desirable to prefix a length field so that the receiving end can be sure of a complete transfer.


Encoding
--------

Safe64l works essentially the same as safe64, except that it is prefixed by a length field. The length field is built incrementally using the same encoding alphabet as the data, until sufficient bits are available to encode the length of the data.

The length encoding uses the lower 5 bits for data, and the high bit as a continuation bit:

| Bit Position | 5 | 4 | 3 | 2 | 1 | 0 |
| ------------ | - | - | - | - | - | - |
| **Purpose**  | c | x | x | x | x | x |

c = continuation bit
x = data

When the continuation bit is set to 1, the length field is continued in the next character. Building of the length field continues until a continuation bit of 0 is encountered. The 5 bit chunks are interpreted in big endian order (the first character represents the highest 5 bits, then the next lower 5 bits, and so on). This allows safe64l encoded data to sort naturally in generic text sorting algorithms, with longer data sequences ranking higher.

| Characters | Bits | Maximum encodable length |
| ---------- | ---- | ------------------------ |
| 1          |    5 |                       31 |
| 2          |   10 |                     1023 |
| 3          |   15 |                    32767 |
| 4          |   20 |                  1048575 |
| 5          |   25 |                 33554431 |
| 6          |   30 |               1073741823 |
| ...        |  ... |                      ... |


Truncation Detection
--------------------

Should truncation occur anywhere in the encoded sequence, one of two things will happen:

 1. The data length won't match the length field.
 2. The length field won't have a character with the continuation bit cleared.


Examples
--------

| Length | Encoded Bits           | Length Field |
| ------ | ---------------------- | ------------ |
|      1 | `000001`               | `0`          |
|     31 | `011111`               | `U`          |
|     32 | `100001 000000`        | `W-`         |
|   2000 | `100001 111110 010000` | `WyF`        |


Advantages over base64 padding
------------------------------

 * No extra alphabet character is necessary. The length encodes using the exact same alphabet as the data encoding.
 * Truncation is always detected. With base64 padding, truncation on a 4-character boundary will not be detected.
 * Lower data usage for smaller data. base64's padding scheme uses an average of 1.5 characters no matter the length of the data. Safe64l uses only 1 byte for lengths 31 and under. By the time its size performance suffers in comparison to base64 (at length 32), the character length difference is already less than 2% of the total payload size, and shrinks from there.



Version History
---------------

January 1, 2019: Preview Version 1



License
-------

Copyright (c) 2019 Karl Stenerud. All rights reserved.

Distributed under the Creative Commons Attribution License: https://creativecommons.org/licenses/by/4.0/legalcode
License deed: https://creativecommons.org/licenses/by/4.0/
