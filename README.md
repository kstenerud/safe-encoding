Safe64 and Safe32 Encodings
===========================

[Safe64](safe64-specification.md) and [safe32](safe32-specification.md) define binary data encoding schemes that are safe to be passed through processing systems that expect human readable text.



Features:
---------

 * Safe for use in URLs
 * Safe for use as filenames
 * Safe for use in formatted documents
 * Safe for use in legacy text processing systems
 * Support for length fields

### Safe32 Additional Features:

 * Useful for human input situations such as activation codes.
 * Easily confusable characters & digits are interchangeable.
 * Uppercase and lowercase characters are interchangeable.



Advantages over base64 and base32:
----------------------------------

 * No padding characters
 * No escaping necessary
 * Liberal whitespace rules
 * Safe for use as filenames
 * No proliferation of incompatible offshoots
 * Better end-of-field and truncation detection
 * Sortable in generic text sorting algorithms (such as file listings)



Specifications
--------------

 * [Safe64](safe64-specification.md)
 * [Safe32](safe32-specification.md)



License
-------

Specifications released under Creative Commons Attribution 4.0 International Public License.
Reference implementation released under MIT License.
