#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

typedef enum
{
    SAFE64_STATUS_OK = 0,

    /**
     * There wasn't enough room in the destination buffer to complete the
     * operation. Clear out the written portion of the destination buffer
     * and retry from where src_buffer_ptr points.
     * The operation will have written a pointer to the next byte to read
     * in src_buffer_ptr, and a pointer to one past the last byte written in
     * dst_buffer_ptr.
     */
    SAFE64_STATUS_NOT_ENOUGH_ROOM = -1,

    /**
     * The source data contained an invalid character. Processing cannot
     * continue.
     * The operation will have written a pointer to the offending character
     * in src_buffer_ptr, and a pointer to one past the last byte written in
     * dst_buffer_ptr.
     */
    SAFE64_ERROR_INVALID_SOURCE_DATA = -2,

    /**
     * The data ended while processing the length field, and no character
     * in the length field had the continuation bit set to 0.
     * This means that there's a truncation error: The length field is
     * incomplete.
     */
    SAFE64_ERROR_UNTERMINATED_LENGTH_FIELD = -3,

    /**
     * The source data has been truncated.
     * This happens if the actual length of the data is shorter than the
     * length field.
     */
    SAFE64_ERROR_TRUNCATED_DATA = -4,
} safe64_status_code;



// --------------
// High Level API
// --------------

/**
 * Get the current library version as a semantic version (e.g. "1.5.2").
 *
 * @return The library version.
 */
const char* safe64_version();

/**
 * Estimate the number of bytes that would be occupied when decoding a safe64
 * sequence of the specified length. Since whitespace would throw this number
 * off, it is only an estimate, but it is guaranteed to be AT LEAST big enough
 * to store the decoded data.
 *
 * @param encoded_length The length of the encoded safe64 sequence.
 * @return The length of the corresponding decoded safe64 sequence.
 */
int64_t safe64_get_decoded_length(int64_t encoded_length);

/**
 * Completely decodes a safe64 sequence.
 * It is expected that src_buffer points to a COMPLETE sequence.
 *
 * Can return the following error codes:
 *  * SAFE64_ERROR_INVALID_SOURCE_DATA: The data was invalid.
 *  * SAFE64_STATUS_NOT_ENOUGH_ROOM: The destination buffer was not big enough.
 *
 * @param src_buffer The buffer containing the complete safe64 sequence.
 * @param src_buffer_length The length in bytes of the sequence.
 * @param dst_buffer A buffer to store the decoded data.
 * @param dst_buffer_length The lenfth of the destination buffer.
 * @return the number of bytes written, or a status code.
 */
int64_t safe64_decode(const char* src_buffer,
                      int64_t src_buffer_length,
                      unsigned char* dst_buffer,
                      int64_t dst_buffer_length);

/**
 * Completely decodes a safe64L (safe64 + length) sequence.
 * It is expected that src_buffer points to a COMPLETE sequence.
 *
 * Can return the following error codes:
 *  * SAFE64_ERROR_INVALID_SOURCE_DATA: The data was invalid.
 *  * SAFE64_STATUS_NOT_ENOUGH_ROOM: The destination buffer was not big enough.
 *  * SAFE64_ERROR_UNTERMINATED_LENGTH_FIELD: The length field is truncated.
 *  * SAFE64_ERROR_TRUNCATED_DATA: The source data is truncated.
 *
 * @param src_buffer The buffer containing the complete safe64 sequence.
 * @param src_buffer_length The length in bytes of the sequence.
 * @param dst_buffer A buffer to store the decoded data.
 * @param dst_buffer_length The lenfth of the destination buffer.
 * @return the number of bytes written, or a status code.
 */
int64_t safe64l_decode(const char* src_buffer,
                       int64_t src_length,
                       unsigned char* dst_buffer,
                       int64_t dst_length);

/**
 * Estimate the number of bytes required to encode some binary data.
 *
 * @param decoded_length The length of the original data.
 * @param include_length_field If true, include the length field into the calculation.
 * @return The number of bytes required to encode the data.
 */
int64_t safe64_get_encoded_length(int64_t decoded_length, bool include_length_field);

/**
 * Completely encodes some binary data.
 * It is expected that src_buffer points to the COMPLETE data.
 *
 * Can return the following error codes:
 *  * SAFE64_STATUS_NOT_ENOUGH_ROOM: The destination buffer was not big enough.
 *
 * @param src_buffer The buffer containing the complete binary data.
 * @param src_buffer_length The length in bytes of the sequence.
 * @param dst_buffer A buffer to store the decoded data.
 * @param dst_buffer_length The lenfth of the destination buffer.
 * @return the number of bytes written, or a status code.
 */
int64_t safe64_encode(const unsigned char* src_buffer,
                      int64_t src_buffer_length,
                      char* dst_buffer,
                      int64_t dst_buffer_length);

/**
 * Completely encodes a length field & some binary data.
 * It is expected that src_buffer points to the COMPLETE data.
 *
 * Can return the following error codes:
 *  * SAFE64_STATUS_NOT_ENOUGH_ROOM: The destination buffer was not big enough.
 *
 * @param src_buffer The buffer containing the complete binary data.
 * @param src_buffer_length The length in bytes of the sequence.
 * @param dst_buffer A buffer to store the decoded data.
 * @param dst_buffer_length The lenfth of the destination buffer.
 * @return the number of bytes written, or a status code.
 */
int64_t safe64l_encode(const unsigned char* src_buffer,
                      int64_t src_buffer_length,
                      char* dst_buffer,
                      int64_t dst_buffer_length);



// -------------
// Low Level API
// -------------

/**
 * Read a length field from a buffer.
 *
 * Can return the following error codes:
 *  * SAFE64_ERROR_UNTERMINATED_LENGTH_FIELD: The length field is truncated.
 *
 * @param buffer Where to read the length field from.
 * @param buffer_length Length of the buffer.
 * @param length Pointer to where the length falue should be stored.
 * @return the number of bytes processed to read the length, or an error code.
 */
int64_t safe64_read_length_field(const char* buffer, int64_t buffer_length, uint64_t* length);

/**
 * Decode part of a safe64 sequence.
 *
 * This is a lower level function for buffered I/O.
 *
 * Attempts to decode and write as much as it can based on the input data.
 * This function will not attempt to write a trailing partial group unless
 * is_end_of_data is set.
 *
 * Upon return:
 *
 *   src_buffer_ptr will point to the next character it will read.
 *   If it's not pointing to the end of the input buffer, the remaining
 *   characters need to be moved to the beginning of the buffer, and then more
 *   input data added.
 *
 *   dst_buffer_ptr will point to one past the last byte written.
 *
 * Can return the following error codes:
 *  * SAFE64_ERROR_INVALID_SOURCE_DATA: The data was invalid.
 *  * SAFE64_STATUS_NOT_ENOUGH_ROOM: The destination buffer is full.
 *
 * @param src_buffer_ptr Pointer to your source buffer pointer (input/output).
 * @param src_length Length of the source buffer.
 * @param dst_buffer_ptr Pointer to your destination buffer pointer (input/output).
 * @param dst_length Length of the destination buffer.
 * @param is_end_of_data If true, this is the last packet of data to decode.
 * @return Status code indicating the result of the operation.
 */
safe64_status_code safe64_decode_feed(const char** src_buffer_ptr,
                                      int64_t src_length,
                                      unsigned char** dst_buffer_ptr,
                                      int64_t dst_length,
                                      bool is_end_of_data);

/**
 * Write a length field to a buffer.
 *
 * Can return the following error codes:
 *  * SAFE64_STATUS_NOT_ENOUGH_ROOM: The destination buffer was not big enough.
 *
 * @param length The length value to write.
 * @param dst_buffer Where to write the length field.
 * @param dst_buffer_length Length of the destination buffer.
 * @return The number of bytes written, or an error code.
 */
int64_t safe64_write_length_field(uint64_t length, char* dst_buffer, int64_t dst_buffer_length);

/**
 * Encode a partial sequence of binary data.
 *
 * This is a lower level function for buffered I/O.
 *
 * Attempts to encode and write as much as it can based on the input data.
 * This function will not attempt to write a trailing partial group unless
 * is_end_of_data is set.
 *
 * Upon return:
 *
 *   src_buffer_ptr will point to the next character it will read.
 *   If it's not pointing to the end of the input buffer, the remaining
 *   characters need to be moved to the beginning of the buffer, and then more
 *   input data added.
 *
 *   dst_buffer_ptr will point to one past the last byte written.
 *
 * Can return the following error codes:
 *  * SAFE64_STATUS_NOT_ENOUGH_ROOM: The destination buffer is full.
 *
 * @param src_buffer_ptr Pointer to your source buffer pointer (input/output).
 * @param src_length Length of the source buffer.
 * @param dst_buffer_ptr Pointer to your destination buffer pointer (input/output).
 * @param dst_length Length of the destination buffer.
 * @param is_end_of_data If true, this is the last packet of data to encode.
 * @return Status code indicating the result of the operation.
 */
safe64_status_code safe64_encode_feed(const unsigned char** src_buffer_ptr,
                                      int64_t src_length,
                                      char** dst_buffer_ptr,
                                      int64_t dst_length,
                                      bool is_end_of_data);


#ifdef __cplusplus 
}
#endif
