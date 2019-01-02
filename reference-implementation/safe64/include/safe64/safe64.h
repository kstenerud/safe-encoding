#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

typedef enum
{
    SAFE64_ERROR_INVALID_SOURCE_DATA = -1,
    SAFE64_ERROR_SOURCE_DATA_MISSING = -2,
    SAFE64_ERROR_NOT_ENOUGH_ROOM = -3,
} safe64_error_code;

/**
 * Get the current library version as a semantic version (e.g. "1.5.2").
 *
 * @return The library version.
 */
const char* safe64_version();



// ------------
// Decoding API
// ------------

/**
 * Get the number of bytes that would be occupied when decoding a safe64
 * sequence of the specified length.
 *
 * @param encoded_length The length of the encoded safe64 sequence.
 * @return The length of the corresponding decoded safe64 sequence.
 */
int64_t safe64_get_decoded_length(int64_t encoded_length);

/**
 * Decode part of a safe64 sequence.
 * The return value can be converted to the number of destination bytes
 * written by passing the value into safe64_get_decoded_length().
 *
 * Can return the following error codes:
 *  * SAFE64_ERROR_INVALID_SOURCE_DATA
 *  * SAFE64_ERROR_SOURCE_DATA_MISSING
 *  * SAFE64_ERROR_NOT_ENOUGH_ROOM
 *
 * @param src_buffer The source data to decode.
 * @param src_length Length of the source data.
 * @param dst_buffer A buffer to decode into.
 * @param dst_length Length of the destination buffer.
 * @param is_end_of_data If true, this is the last packet of data to decode.
 * @return The number of source bytes consumed, or a negative value as an error code.
 */
int64_t safe64_decode_feed(const char* src_buffer,
                           int64_t src_length,
                           unsigned char* dst_buffer,
                           int64_t dst_length,
                           bool is_end_of_data);

int64_t safe64_decode(const char* src_buffer,
                      int64_t src_length,
                      unsigned char* dst_buffer,
                      int64_t dst_length);

int64_t safe64_get_encoded_length(int64_t decoded_length);

int64_t safe64_encode_feed(const unsigned char* src_buffer,
                           int64_t src_length,
                           char* dst_buffer,
                           int64_t dst_length,
                           bool is_end_of_data);

int64_t safe64_encode(const unsigned char* src_buffer,
                      int64_t src_length,
                      char* dst_buffer,
                      int64_t dst_length);


#ifdef __cplusplus 
}
#endif
