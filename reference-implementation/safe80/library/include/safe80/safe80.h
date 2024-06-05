#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

#ifndef SAFE80_PUBLIC
    #if defined _WIN32 || defined __CYGWIN__
        #define SAFE80_PUBLIC __declspec(dllimport)
    #else
        #define SAFE80_PUBLIC
    #endif
#endif

typedef enum
{
    /**
     * The operation completed successfully.
     */
    SAFE80_STATUS_OK = 0,

    /**
     * Processing has reached the end of either the source or destination
     * buffer.
     * The operation will have written a pointer to the next byte to read
     * in src_buffer_ptr, and a pointer to one past the last byte written in
     * dst_buffer_ptr. You will have to copy any unused bytes to the beginning
     * of the next buffer(s).
     */
    SAFE80_STATUS_PARTIALLY_COMPLETE = -1,

    /**
     * The source data contained an invalid character. Processing cannot
     * continue.
     * The operation will have written a pointer to the offending character
     * in src_buffer_ptr, and a pointer to one past the last byte written in
     * dst_buffer_ptr.
     */
    SAFE80_ERROR_INVALID_SOURCE_DATA = -2,

    /**
     * The data ended while processing the length field, and no character
     * in the length field had the continuation bit set to 0.
     * This means that there's a truncation error: The length field is
     * incomplete.
     */
    SAFE80_ERROR_UNTERMINATED_LENGTH_FIELD = -3,

    /**
     * The source data has been truncated.
     * This happens if the actual length of the data is shorter than the
     * length field.
     */
    SAFE80_ERROR_TRUNCATED_DATA = -4,

    /**
     * An invalid length value was detected.
     * This happens if the length is negative.
     */
    SAFE80_ERROR_INVALID_LENGTH = -5,

    /**
     * There wasn't enough room in the bufer to complete the operation.
     */
    SAFE80_ERROR_NOT_ENOUGH_ROOM = -6,
} safe80_status;

/**
 * This logical ORed field allows callers to mark the end of the source and/or
 * destination data stream.
 *
 * Use SAFE80_EXPECT_DST_STREAM_TO_END to determine which stream is expected
 * to end, and then the other fields to mark which ones actually DID end.
 */
typedef enum
{
    SAFE80_STREAM_STATE_NONE = 0,

    /**
     * Either the source or destination stream will be expected to end, but
     * never both. When this bit is set, the destination stream is expected
     * to end. When cleared, the source stream is expected to end.
     */
    SAFE80_EXPECT_DST_STREAM_TO_END = 1,

    /**
     * The source buffer endpoint marks the end of the source stream.
     */
    SAFE80_SRC_IS_AT_END_OF_STREAM = 2,

    /**
     * The destination buffer endpoint marks the end of the destination stream.
     */
    SAFE80_DST_IS_AT_END_OF_STREAM = 4,
} safe80_stream_state;



// --------------
// High Level API
// --------------

/**
 * Get the current library version as a semantic version (e.g. "1.5.2").
 *
 * @return The library version.
 */
SAFE80_PUBLIC const char* safe80_version(void);

/**
 * Estimate the number of bytes that would be occupied when decoding a safe80
 * sequence of the specified length. Since whitespace would throw this number
 * off, it is only an estimate, but it is guaranteed to be AT LEAST big enough
 * to store the decoded data.
 *
 * Can return the following status codes:
 *  * SAFE80_ERROR_INVALID_LENGTH: The length was negative.
 *
 * @param encoded_length The length of the encoded safe80 sequence.
 * @return The length of the corresponding decoded safe80 sequence.
 */
SAFE80_PUBLIC int64_t safe80_get_decoded_length(int64_t encoded_length);

/**
 * Completely decodes a safe80 sequence.
 * It is expected that src_buffer points to a COMPLETE sequence.
 *
 * Can return the following status codes:
 *  * SAFE80_ERROR_INVALID_LENGTH: A length was negative.
 *  * SAFE80_ERROR_INVALID_SOURCE_DATA: The data was invalid.
 *  * SAFE80_ERROR_NOT_ENOUGH_ROOM: The destination buffer was not big enough.
 *
 * @param src_buffer The buffer containing the complete safe80 sequence.
 * @param src_buffer_length The length in bytes of the sequence.
 * @param dst_buffer A buffer to store the decoded data.
 * @param dst_buffer_length The lenfth of the destination buffer.
 * @return the number of bytes written, or a status code.
 */
SAFE80_PUBLIC int64_t safe80_decode(const uint8_t* src_buffer,
                                    int64_t src_buffer_length,
                                    uint8_t* dst_buffer,
                                    int64_t dst_buffer_length);

/**
 * Completely decodes a safe80L (safe80 + length) sequence.
 * It is expected that src_buffer points to a COMPLETE sequence.
 *
 * Can return the following status codes:
 *  * SAFE80_ERROR_INVALID_LENGTH: A length was negative.
 *  * SAFE80_ERROR_INVALID_SOURCE_DATA: The data was invalid.
 *  * SAFE80_ERROR_NOT_ENOUGH_ROOM: The destination buffer was not big enough.
 *  * SAFE80_ERROR_UNTERMINATED_LENGTH_FIELD: The length field is truncated.
 *  * SAFE80_ERROR_TRUNCATED_DATA: The source data is truncated.
 *
 * @param src_buffer The buffer containing the complete safe80 sequence.
 * @param src_buffer_length The length in bytes of the sequence.
 * @param dst_buffer A buffer to store the decoded data.
 * @param dst_buffer_length The lenfth of the destination buffer.
 * @return the number of bytes written, or a status code.
 */
SAFE80_PUBLIC int64_t safe80l_decode(const uint8_t* src_buffer,
                                     int64_t src_length,
                                     uint8_t* dst_buffer,
                                     int64_t dst_length);

/**
 * Estimate the number of bytes required to encode some binary data.
 *
 * Can return the following status codes:
 *  * SAFE80_ERROR_INVALID_LENGTH: The length was negative.
 *
 * @param decoded_length The length of the original data.
 * @param include_length_field If true, include the length field into the calculation.
 * @return The number of bytes required to encode the data.
 */
SAFE80_PUBLIC int64_t safe80_get_encoded_length(int64_t decoded_length,
                                                bool include_length_field);

/**
 * Completely encodes some binary data.
 * It is expected that src_buffer points to the COMPLETE data.
 *
 * Can return the following status codes:
 *  * SAFE80_ERROR_INVALID_LENGTH: A length was negative.
 *  * SAFE80_STATUS_NOT_ENOUGH_ROOM: The destination buffer was not big enough.
 *
 * @param src_buffer The buffer containing the complete binary data.
 * @param src_buffer_length The length in bytes of the sequence.
 * @param dst_buffer A buffer to store the decoded data.
 * @param dst_buffer_length The lenfth of the destination buffer.
 * @return the number of bytes written, or a status code.
 */
SAFE80_PUBLIC int64_t safe80_encode(const uint8_t* src_buffer,
                                    int64_t src_buffer_length,
                                    uint8_t* dst_buffer,
                                    int64_t dst_buffer_length);

/**
 * Completely encodes a length field & some binary data.
 * It is expected that src_buffer points to the COMPLETE data.
 *
 * Can return the following status codes:
 *  * SAFE80_ERROR_INVALID_LENGTH: A length was negative.
 *  * SAFE80_STATUS_NOT_ENOUGH_ROOM: The destination buffer was not big enough.
 *
 * @param src_buffer The buffer containing the complete binary data.
 * @param src_buffer_length The length in bytes of the sequence.
 * @param dst_buffer A buffer to store the decoded data.
 * @param dst_buffer_length The lenfth of the destination buffer.
 * @return the number of bytes written, or a status code.
 */
SAFE80_PUBLIC int64_t safe80l_encode(const uint8_t* src_buffer,
                                     int64_t src_buffer_length,
                                     uint8_t* dst_buffer,
                                     int64_t dst_buffer_length);



// -------------
// Low Level API
// -------------

/**
 * Read a length field from a buffer.
 *
 * Can return the following status codes:
 *  * SAFE80_ERROR_INVALID_LENGTH: The buffer length was negative.
 *  * SAFE80_ERROR_UNTERMINATED_LENGTH_FIELD: The length field is truncated.
 *
 * @param buffer Where to read the length field from.
 * @param buffer_length Length of the buffer.
 * @param length Pointer to where the length value should be stored.
 * @return the number of bytes processed to read the length, or an error code.
 */
SAFE80_PUBLIC int64_t safe80_read_length_field(const uint8_t* buffer,
                                               int64_t buffer_length,
                                               int64_t* length);

/**
 * Decode part of a safe80 sequence.
 *
 * This is a lower level function for buffered I/O.
 *
 * Attempts to decode and write as much as it can based on the input data.
 * This function will not attempt to write a trailing partial group unless
 * stream_state contains SAFE80_SRC_IS_AT_END_OF_STREAM or SAFE80_DST_IS_AT_END_OF_STREAM.
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
 * Can return the following status codes:
 *  * SAFE80_STATUS_OK: The process completed successfully.
 *  * SAFE80_STATUS_PARTIALLY_COMPLETE: The process completed, but not all data was written.
 *  * SAFE80_ERROR_INVALID_LENGTH: A length was negative.
 *  * SAFE80_ERROR_INVALID_SOURCE_DATA: The data was invalid.
 *  * SAFE80_ERROR_NOT_ENOUGH_ROOM: The destination buffer was not big enough.
 *  * SAFE80_ERROR_TRUNCATED_DATA: The source data was truncated.
 *
 * @param src_buffer_ptr Pointer to your source buffer pointer (input/output).
 * @param src_length Length of the source buffer.
 * @param dst_buffer_ptr Pointer to your destination buffer pointer (input/output).
 * @param dst_length Length of the destination buffer.
 * @param is_end_of_data If true, this is the last packet of data to decode.
 * @return Status code indicating the result of the operation.
 */
SAFE80_PUBLIC safe80_status safe80_decode_feed(const uint8_t** src_buffer_ptr,
                                               int64_t src_length,
                                               uint8_t** dst_buffer_ptr,
                                               int64_t dst_length,
                                               safe80_stream_state stream_state);

/**
 * Write a length field to a buffer.
 *
 * Can return the following status codes:
 *  * SAFE80_ERROR_INVALID_LENGTH: A length was negative.
 *  * SAFE80_STATUS_NOT_ENOUGH_ROOM: The destination buffer was not big enough.
 *
 * @param length The length value to write.
 * @param dst_buffer Where to write the length field.
 * @param dst_buffer_length Length of the destination buffer.
 * @return The number of bytes written, or an error code.
 */
SAFE80_PUBLIC int64_t safe80_write_length_field(int64_t length,
                                                uint8_t* dst_buffer,
                                                int64_t dst_buffer_length);

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
 * Can return the following status codes:
 *  * SAFE80_STATUS_OK: The process completed successfully.
 *  * SAFE80_STATUS_PARTIALLY_COMPLETE: The process completed, but not all data was written.
 *  * SAFE80_ERROR_INVALID_LENGTH: A length was negative.
 *
 * @param src_buffer_ptr Pointer to your source buffer pointer (input/output).
 * @param src_length Length of the source buffer.
 * @param dst_buffer_ptr Pointer to your destination buffer pointer (input/output).
 * @param dst_length Length of the destination buffer.
 * @param is_end_of_data If true, this is the last packet of data to encode.
 * @return Status code indicating the result of the operation.
 */
SAFE80_PUBLIC safe80_status safe80_encode_feed(const uint8_t** src_buffer_ptr,
                                               int64_t src_length,
                                               uint8_t** dst_buffer_ptr,
                                               int64_t dst_length,
                                               bool is_end_of_data);


#ifdef __cplusplus 
}
#endif
