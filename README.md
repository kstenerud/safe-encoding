Safe Text Encoding
==================

Binary data encoding schemes that are safe to be passed through processing systems that expect human readable text, without requiring escaping.

Alternative To:

* Base16
* Base32
* Base64
* Ascii85



Features:
---------

### All

 * Safe for use in JSON, SGML formats, source code strings - without escaping
 * Safe for use in path, query, and fragment components of URIs
 * Safe for use in formatted documents
 * Safe for use in legacy text processing systems
 * Support for length fields (to detect truncation at any point)
 * Liberal whitespace rules
 * No padding characters
 * Safe for use in filenames on POSIX file systems
 * Naturally sortable with generic text sorting algorithms (such as file listings)

### Safe80 and below

 * Safe for use in filenames on Windows file systems

### Safe64 and below

 * Safe for use in all URI components without escaping

### Safe32 and below

 * Useful for human input situations such as activation codes
 * Easily confusable characters & digits are interchangeable
 * Uppercase and lowercase characters are interchangeable



Advantages
----------

The safe encodings have been specially designed to avoid numerous issues with other binary-to-text encoding schemes. Here are the relative advantages of the various encodings:

| Encoding   | Bloat | SGML | JSON | Code | URI | File | Host | Trunc | Sort | White | Length | Human | No-Pad | Alpha |
| ---------- | ----- | ---- | ---- | ---- | --- | ---- | ---- | ----- | ---- | ----- | ------ | ----- | ------ | ----- |
| **safe16** |  2.0  |   ✓  |   ✓  |   ✓  |  ✓  |   ✓  |   ✓  |   ✓   |   ✓  |   ✓   |    ✓   |   ✓   |    ✓   |   16  |
| **safe32** |  1.6  |   ✓  |   ✓  |   ✓  |  ✓  |   ✓  |   ✓  |   ✓   |   ✓  |   ✓   |    ✓   |   ✓   |    ✓   |   32  |
| **safe64** |  1.33 |   ✓  |   ✓  |   ✓  |  ✓  |   ✓  |      |   ✓   |   ✓  |   ✓   |    ✓   |       |    ✓   |   64  |
| **safe80** |  1.27 |   ✓  |   ✓  |   ✓  |  1  |   ✓  |      |   ✓   |   ✓  |   ✓   |    ✓   |       |    ✓   |   80  |
| **safe85** |  1.25 |   ✓  |   ✓  |   ✓  |  2  |   3  |      |   ✓   |   ✓  |   ✓   |    ✓   |       |    ✓   |   85  |

* 1: Safe for most URI components. Application-specific `!` `$` `(` `)` `,` `;` may need special handling.
* 2: Safe for most URI components. Application-specific `!` `$` `(` `)` `,` `;` `*`, `=` may need special handling.
* 3: Safe only for filenames in POSIX filesystems (UNIX, Linux, BSD, Mac, etc)

For comparison, baseXY encodings:

| Encoding   | Bloat | SGML | JSON | Code | URI | File | Host | Trunc | Sort | White | Length | Human | No-Pad | Alpha |
| ---------- | ----- | ---- | ---- | ---- | --- | ---- | ---- | ----- | ---- | ----- | ------ | ----- | ------ | ----- |
| **base16** |  2.0  |   ✓  |   ✓  |   ✓  |  ✓  |   ✓  |   ✓  |       |   ✓  |       |        |       |    ✓   |   16  |
| **base32** |  1.6  |   ✓  |   ✓  |   ✓  |  ✓  |   ✓  |   ✓  |       |      |       |        |       |        |   33  |
| **base64** |  1.33 |   ✓  |   ✓  |   ✓  |  4  |   4  |      |       |      |       |        |       |        |   65  |
| **base85** |  1.25 |      |      |      |     |      |      |       |      |   ✓   |        |       |        |   87  |

* 4: Some restrictions do not apply if using URI and filename safe variants

#### Legend:

* **Bloat**:  How much the size increases from original form to encoded form
* **SGML**:   Can be used in SGML documents (such as HTML and XML) without escaping
* **JSON**:   Can be used in JSON documents without escaping
* **Code**:   Can be used in source code string literals without escaping
* **URI**:    Can be used in the path, query, and fragment components of URIs without escaping
* **File**:   Can be used in filenames
* **Host**:   Can be used in hostnames
* **Trunc**:  Can detect truncation in all cases
* **Sort**:   Sorts in the same order as the binary data it represents
* **White**:  Allows arbitrary whitespace (whitespace anywhere in the stream)
* **Length**: Supports length fields
* **Human**:  Handles easily-confused characters when input by humans
* **No-Pad**: Encoding scheme does not require padding
* **Alpha**:  Size of the encoding alphabet (including special characters)


### Performance Characteristics

#### Size Performance

The choice of radix affects compressibility. Here are some size comparisons using the Windows XP service pack 3 x86 iso from MSDN (approx 600 MB):

| Mode          | original | s16  | s32  | s64  | s80  | s85  |
| ------------- | -------- | ---- | ---- | ---- | ---- | ---- |
| iso.sxx       |     1.00 | 2.00 | 1.60 | 1.33 | 1.27 | 1.25 |
| iso.gz.sxx    |     0.93 | 1.86 | 1.49 | 1.24 | 1.18 | 1.16 |

With post-compression (e.g. gzipped HTTP response):

| Mode          | original | s16  | s32  | s64  | s80  | s85  |
| ------------- | -------- | ---- | ---- | ---- | ---- | ---- |
| iso.sxx.gz    |     1.00 | 1.06 | 1.00 | 0.95 | 0.98 | 0.96 |
| iso.gz.sxx.gz |     0.93 | 1.06 | 0.98 | 0.94 | 0.94 | 0.94 |


#### Reference Implementation Time Performance

These show the current time performance of the reference implementations (using the same 600 MB iso). Safe80 and Safe85 are considerably slower due to their use of multiplication and division. Safe80 is further slowed by its naive use of 128 bit integers. Optimized versions will of course fare much better.

| Type | Time (s) | Relative |
| ---- | -------- | -------- |
| s16  |  15.08   |   1.40   |
| s32  |  11.175  |   1.04   |
| s64  |  10.742  |   1.00   |
| s80  |  65.653  |   6.11   |
| s85  |  27.484  |   2.56   |



Specifications
--------------

These specifications are part of the [Specification Project](https://github.com/kstenerud/specifications)

 * [Safe16](safe16-specification.md)
 * [Safe32](safe32-specification.md)
 * [Safe64](safe64-specification.md)
 * [Safe80](safe80-specification.md)
 * [Safe85](safe85-specification.md)



Reference Implementations
-------------------------

The reference implementations contain libraries and command line executables:

 * [Safe16](reference-implementation/safe16)
 * [Safe32](reference-implementation/safe32)
 * [Safe64](reference-implementation/safe64)
 * [Safe80](reference-implementation/safe80)
 * [Safe85](reference-implementation/safe85)



Example App
-----------

[safeenc](reference-implementation/safeenc) is a command line program that can convert to/from any safe format.



License
-------

Specifications released under Creative Commons Attribution 4.0 International Public License.
Reference implementation released under MIT License.
