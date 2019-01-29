Safe Text Encodings
===================

Binary data encoding schemes that are safe to be passed through processing systems that expect human readable text, without requiring escaping.



Features:
---------

### All

 * Safe for use in JSON, SGML formats, source code strings, without escaping
 * Safe for use in path, query, and fragment components of URIs
 * Safe for use in formatted documents
 * Safe for use in legacy text processing systems
 * Support for length fields
 * Liberal whitespace rules
 * No padding characters
 * Safe for use in filenames on POSIX file systems
 * Sortable in generic text sorting algorithms (such as file listings)

### Safe64 and below

 * Safe for use in all URI components without escaping
 * Safe for use in filenames on all file systems

### Safe32 and below

 * Useful for human input situations such as activation codes
 * Easily confusable characters & digits are interchangeable
 * Uppercase and lowercase characters are interchangeable



Advantages
----------

The safe encodings have been specially designed to avoid numerous issues with other binary-to-text encoding schemes. Here are the relative advantages of the various safeXYZ and baseXYZ encodings:

| Encoding   | SGML | JSON | Code | URI | File | Host | Trunc | Sort | White | Length | Human | No-Pad | Alpha | Bloat |
| ---------- | ---- | ---- | ---- | --- | ---- | ---- | ----- | ---- | ----- | ------ | ----- | ------ | ----- | ----- |
| **safe16** |   ✓  |   ✓  |   ✓  |  ✓  |   ✓  |   ✓  |   ✓   |   ✓  |   ✓   |    ✓   |   ✓   |    ✓   |   16  |  2.0  |
| **base16** |   ✓  |   ✓  |   ✓  |  ✓  |   ✓  |   ✓  |       |   ✓  |       |        |   ✓   |    ✓   |   16  |  2.0  |
| **safe32** |   ✓  |   ✓  |   ✓  |  ✓  |   ✓  |   ✓  |   ✓   |   ✓  |   ✓   |    ✓   |   ✓   |    ✓   |   32  |  1.6  |
| **base32** |   ✓  |   ✓  |   ✓  |  ✓  |   ✓  |   ✓  |       |      |       |        |       |        |   33  |  1.6  |
| **safe64** |   ✓  |   ✓  |   ✓  |  ✓  |   ✓  |      |   ✓   |   ✓  |   ✓   |    ✓   |       |    ✓   |   64  |  1.33 |
| **base64** |   ✓  |   ✓  |   ✓  |  ¹  |   ¹  |      |       |      |       |        |       |        |   65  |  1.33 |
| **safe85** |   ✓  |   ✓  |   ✓  |  ✓  |   ²  |      |   ✓   |   ✓  |   ✓   |    ✓   |       |    ✓   |   85  |  1.25 |
| **base85** |      |      |      |     |      |      |       |   ✓  |   ✓   |        |       |        |   87  |  1.25 |

##### Legend:

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
* **Bloat**:  How much the size increases from original form to encoded form

##### Notes:

* **¹**: Some restrictions do not apply if using URI and filename safe variants
* **²**: Safe only for filenames in POSIX filesystems (UNIX, Linux, BSD, Mac, etc)



Specifications
--------------

 * [Safe16](safe16-specification.md)
 * [Safe32](safe32-specification.md)
 * [Safe64](safe64-specification.md)
 * [Safe85](safe85-specification.md)



These specifications are part of the [Specification Project](https://github.com/kstenerud/specifications)


License
-------

Specifications released under Creative Commons Attribution 4.0 International Public License.
Reference implementation released under MIT License.
