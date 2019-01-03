#include <safe64/safe64.h>
#include "safe64_version.h"

#include <limits.h>

// #define KSLogger_LocalLevel DEBUG
#include "kslogger.h"

#define DECODED_BYTES_PER_GROUP 3
#define ENCODED_BYTES_PER_GROUP 4
#define ENCODED_BITS_PER_BYTE 6
#define DECODED_BITS_PER_BYTE 8

#define CHARACTER_CODE_ERROR 0x7f
#define CHARACTER_CODE_WHITESPACE 0x7e
static const char g_decode_alphabet[] =
{
#define ERRR CHARACTER_CODE_ERROR
#define WHTE CHARACTER_CODE_WHITESPACE
            ERRR, ERRR, ERRR, ERRR, ERRR, ERRR, ERRR, ERRR,
/* wh sp */ ERRR, WHTE, WHTE, ERRR, ERRR, WHTE, ERRR, ERRR,
            ERRR, ERRR, ERRR, ERRR, ERRR, ERRR, ERRR, ERRR,
            ERRR, ERRR, ERRR, ERRR, ERRR, ERRR, ERRR, ERRR,
/* space */ WHTE, ERRR, ERRR, ERRR, ERRR, ERRR, ERRR, ERRR,
/* -     */ ERRR, ERRR, ERRR, ERRR, ERRR, 0x00, ERRR, ERRR,
/* 0-7   */ 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
/* 8-9   */ 0x09, 0x0a, ERRR, ERRR, ERRR, ERRR, ERRR, ERRR,
/* A-G   */ ERRR, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11,
/* H-O   */ 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19,
/* P-W   */ 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20, 0x21,
/* X-Z,_ */ 0x22, 0x23, 0x24, ERRR, ERRR, ERRR, ERRR, 0x25,
/* a-g   */ ERRR, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c,
/* h-o   */ 0x2d, 0x2e, 0x2f, 0x30, 0x31, 0x32, 0x33, 0x34,
/* p-w   */ 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c,
/* x-z   */ 0x3d, 0x3e, 0x3f, ERRR, ERRR, ERRR, ERRR, ERRR,
            ERRR, ERRR, ERRR, ERRR, ERRR, ERRR, ERRR, ERRR,
            ERRR, ERRR, ERRR, ERRR, ERRR, ERRR, ERRR, ERRR,
            ERRR, ERRR, ERRR, ERRR, ERRR, ERRR, ERRR, ERRR,
            ERRR, ERRR, ERRR, ERRR, ERRR, ERRR, ERRR, ERRR,
            ERRR, ERRR, ERRR, ERRR, ERRR, ERRR, ERRR, ERRR,
            ERRR, ERRR, ERRR, ERRR, ERRR, ERRR, ERRR, ERRR,
            ERRR, ERRR, ERRR, ERRR, ERRR, ERRR, ERRR, ERRR,
            ERRR, ERRR, ERRR, ERRR, ERRR, ERRR, ERRR, ERRR,
            ERRR, ERRR, ERRR, ERRR, ERRR, ERRR, ERRR, ERRR,
            ERRR, ERRR, ERRR, ERRR, ERRR, ERRR, ERRR, ERRR,
            ERRR, ERRR, ERRR, ERRR, ERRR, ERRR, ERRR, ERRR,
            ERRR, ERRR, ERRR, ERRR, ERRR, ERRR, ERRR, ERRR,
            ERRR, ERRR, ERRR, ERRR, ERRR, ERRR, ERRR, ERRR,
            ERRR, ERRR, ERRR, ERRR, ERRR, ERRR, ERRR, ERRR,
            ERRR, ERRR, ERRR, ERRR, ERRR, ERRR, ERRR, ERRR,
            ERRR, ERRR, ERRR, ERRR, ERRR, ERRR, ERRR, ERRR,
#undef WHTE
#undef ERRR
};

static const char g_encode_alphabet[] =
{
    '-', '0', '1', '2', '3', '4', '5', '6',
    '7', '8', '9', 'A', 'B', 'C', 'D', 'E',
    'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
    'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U',
    'V', 'W', 'X', 'Y', 'Z', '_', 'a', 'b',
    'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j',
    'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r',
    's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
};

static const int g_encoded_remainder_to_decoded_remainder[] =
{
    0,
    1,
    1,
    2,
    3,
};

static const int g_decoded_remainder_to_encoded_remainder[] =
{
    0,
    2,
    3,
    4,
};

static const int g_encoded_remainder_to_bit_padding[] =
{
    0,
    0,
    4,
    2,
    0,
};

static const int g_decoded_remainder_to_bit_padding[] =
{
    0,
    4,
    2,
    0,
};

const char* safe64_version()
{
    return SAFE64_VERSION;
}

int64_t safe64_get_decoded_length(const int64_t encoded_length)
{
    int64_t groups = encoded_length / ENCODED_BYTES_PER_GROUP;
    int remainder = g_encoded_remainder_to_decoded_remainder[encoded_length % ENCODED_BYTES_PER_GROUP];
    return groups * DECODED_BYTES_PER_GROUP + remainder;
}

safe64_status_code safe64_decode_feed(const char** src_buffer_ptr,
                                      const char* src_buffer_end,
                                      unsigned char** dst_buffer_ptr,
                                      unsigned char* dst_buffer_end,
                                      bool is_end_of_data)
{
    int64_t accumulator = 0;
    int src_group_char_count = 0;
    const char* src = *src_buffer_ptr;
    unsigned char* dst = *dst_buffer_ptr;

    const char* const src_end = src_buffer_end;
    const unsigned char* const dst_end = dst_buffer_end;
    const int src_bits_per_byte = ENCODED_BITS_PER_BYTE;
    const int dst_bits_per_byte = DECODED_BITS_PER_BYTE;
    const char* const alphabet = g_decode_alphabet;
    const int src_chars_per_group = ENCODED_BYTES_PER_GROUP;
    const int dst_mask = 0xff;
    const int* const src_to_dst_remainder = g_encoded_remainder_to_decoded_remainder;
    const int* const remainder_to_bit_padding = g_encoded_remainder_to_bit_padding;

    KSLOG_DEBUG("Decode %d chars into %d bytes, ending %d", src_end - src, dst_end - dst, is_end_of_data);

#define WRITE_BYTES(BYTE_COUNT) \
    { \
        int bytes_to_write = src_to_dst_remainder[BYTE_COUNT]; \
        KSLOG_DEBUG("Writing %d chars as %d decoded bytes", BYTE_COUNT, bytes_to_write); \
        if(dst + bytes_to_write > dst_end) \
        { \
            KSLOG_DEBUG("Error: Need %d bytes but only %d available", bytes_to_write, dst_end - dst); \
            *src_buffer_ptr = last_src; \
            *dst_buffer_ptr = dst; \
            return SAFE64_STATUS_NOT_ENOUGH_ROOM; \
        } \
        for(int i = bytes_to_write-1; i >= 0; i--) \
        { \
            *dst++ = (accumulator >> (dst_bits_per_byte * i)) & dst_mask; \
            KSLOG_DEBUG("Write: Extract pos %d: %02x", \
                (dst_bits_per_byte * i), \
                (accumulator >> (dst_bits_per_byte * i)) & dst_mask); \
        } \
    }

    const char* last_src = src;
    while(src < src_end)
    {
        int code = alphabet[(int)(*src++)];
        if(code == CHARACTER_CODE_WHITESPACE)
        {
            KSLOG_TRACE("Whitespace");
            continue;
        }
        if(code == CHARACTER_CODE_ERROR)
        {
            KSLOG_DEBUG("Error: Invalid source data: %02x: [%c]", src[-1], src[-1]);
            *src_buffer_ptr = src - 1;
            *dst_buffer_ptr = dst;
            return SAFE64_ERROR_INVALID_SOURCE_DATA;
        }
        accumulator = (accumulator << src_bits_per_byte) | code;
        src_group_char_count++;
        KSLOG_DEBUG("Accumulate #%d [%c] (%02x). Value = %x", src_group_char_count, src[-1], code, accumulator);
        if(src_group_char_count >= src_chars_per_group)
        {
            WRITE_BYTES(src_group_char_count);
            src_group_char_count = 0;
            accumulator = 0;
            last_src = src;
        }
    }

    if(src_group_char_count > 0)
    {
        if(is_end_of_data)
        {
            int phantom_bits = remainder_to_bit_padding[src_group_char_count];
            KSLOG_DEBUG("E Phantom bits: %d", phantom_bits);
            accumulator >>= phantom_bits;
            WRITE_BYTES(src_group_char_count);
        }
        else
        {
            src -= src_group_char_count;
        }
        last_src = src;
    }

    *src_buffer_ptr = last_src;
    *dst_buffer_ptr = dst;

    return SAFE64_STATUS_OK;
#undef WRITE_BYTES
}

int64_t safe64_decode(const char* src_buffer,
                      int64_t src_length,
                      unsigned char* dst_buffer,
                      int64_t dst_length)
{
    const char* src = src_buffer;
    unsigned char* dst = dst_buffer;
    int64_t result = safe64_decode_feed(&src, src + src_length, &dst, dst + dst_length, true);
    if(result < 0)
    {
        return result;
    }
    return dst - dst_buffer;
}

int64_t safe64_get_encoded_length(const int64_t decoded_length)
{
    int64_t groups = decoded_length / 3;
    int remainder = g_decoded_remainder_to_encoded_remainder[decoded_length % 3];
    return groups * 4 + remainder;
}

safe64_status_code safe64_encode_feed(const unsigned char** src_buffer_ptr,
                                      const unsigned char* src_buffer_end,
                                      char** dst_buffer_ptr,
                                      char* dst_buffer_end,
                                      bool is_end_of_data)
{
    int64_t accumulator = 0;
    int src_group_char_count = 0;
    const unsigned char* src = *src_buffer_ptr;
    char* dst = *dst_buffer_ptr;

    const unsigned char* const src_end = src_buffer_end;
    const char* const dst_end = dst_buffer_end;
    const int src_bits_per_byte = DECODED_BITS_PER_BYTE;
    const int dst_bits_per_byte = ENCODED_BITS_PER_BYTE;
    const char* const alphabet = g_encode_alphabet;
    const int src_chars_per_group = DECODED_BYTES_PER_GROUP;
    const int dst_mask = 0x3f;
    const int* const src_to_dst_remainder = g_decoded_remainder_to_encoded_remainder;
    const int* const remainder_to_bit_padding = g_decoded_remainder_to_bit_padding;

    KSLOG_DEBUG("Encode %d bytes into %d encoded chars, ending %d", src_end - src, dst_end - dst, is_end_of_data);

#define WRITE_BYTES(SRC_BYTE_COUNT) \
    { \
        int bytes_to_write = src_to_dst_remainder[SRC_BYTE_COUNT]; \
        KSLOG_DEBUG("Writing %d bytes as %d encoded chars", SRC_BYTE_COUNT, bytes_to_write); \
        if(dst + bytes_to_write > dst_end) \
        { \
            KSLOG_DEBUG("Error: Need %d chars but only %d available", bytes_to_write, dst_end - dst); \
            *src_buffer_ptr = last_src; \
            *dst_buffer_ptr = dst; \
            return SAFE64_STATUS_NOT_ENOUGH_ROOM; \
        } \
        for(int i = bytes_to_write-1; i >= 0; i--) \
        { \
            *dst++ = alphabet[(accumulator >> (dst_bits_per_byte * i)) & dst_mask]; \
            KSLOG_DEBUG("Write: Extract pos %02d: %02x: %c", \
                (dst_bits_per_byte * i), \
                (accumulator >> (dst_bits_per_byte * i)) & dst_mask, \
                alphabet[(accumulator >> (dst_bits_per_byte * i)) & dst_mask]); \
        } \
    }

    const unsigned char* last_src = src;
    while(src < src_end)
    {
        accumulator = (accumulator << src_bits_per_byte) | *src++;
        src_group_char_count++;
        KSLOG_DEBUG("Accumulate #%d (%02x). Value = %x", src_group_char_count, src[-1], accumulator);
        if(src_group_char_count >= src_chars_per_group)
        {
            WRITE_BYTES(src_group_char_count);
            src_group_char_count = 0;
            accumulator = 0;
            last_src = src;
        }
    }

    if(src_group_char_count > 0)
    {
        if(is_end_of_data)
        {
            int phantom_bits = remainder_to_bit_padding[src_group_char_count];
            KSLOG_DEBUG("E Phantom bits: %d", phantom_bits);
            accumulator <<= phantom_bits;
            WRITE_BYTES(src_group_char_count);
        }
        else
        {
            src -= src_group_char_count;
        }
        last_src = src;
    }

    *src_buffer_ptr = last_src;
    *dst_buffer_ptr = dst;

    return SAFE64_STATUS_OK;
#undef WRITE_BYTES
}

int64_t safe64_encode(const unsigned char* src_buffer,
                      int64_t src_length,
                      char* dst_buffer,
                      int64_t dst_length)
{
    const unsigned char* src = src_buffer;
    char* dst = dst_buffer;
    int64_t result = safe64_encode_feed(&src, src + src_length, &dst, dst + dst_length, true);
    if(result < 0)
    {
        return result;
    }
    return dst - dst_buffer;
}

int64_t safe64_write_length_field(uint64_t length, char* dst_buffer, int64_t dst_buffer_length)
{
    const int bits_per_chunk = ENCODED_BITS_PER_BYTE - 1;
    const int continuation_bit = 1 << bits_per_chunk;
    const int chunk_mask = continuation_bit - 1;
    KSLOG_DEBUG("bits %d, continue %02x, mask %02x", bits_per_chunk, continuation_bit, chunk_mask);

    int chunk_count = 0;
    for(uint64_t i = length; i; i >>= bits_per_chunk, chunk_count++)
    {
    }
    if(chunk_count == 0)
    {
        chunk_count = 1;
    }
    KSLOG_DEBUG("Value: %lu, chunk count %d", length, chunk_count);

    if(chunk_count > dst_buffer_length)
    {
        KSLOG_DEBUG("Error: Require %d bytes but only %d available", chunk_count, dst_buffer_length);
        return SAFE64_STATUS_NOT_ENOUGH_ROOM;
    }

    char* dst = dst_buffer;
    for(int i = chunk_count-1; i >= 0; i--)
    {
        int should_continue = (i == 0) ? 0 : continuation_bit;
        int code = ((length>>(bits_per_chunk*i)) & chunk_mask) + should_continue;
        *dst++ = g_encode_alphabet[code];
        KSLOG_DEBUG("Chunk %d: '%c' (%d), continue %d",
            i,
            dst[-1],
            ((length>>(bits_per_chunk*i)) & chunk_mask),
            should_continue
            );
    }
    return chunk_count;
}

safe64_status_code safe64_read_length_field(const char* buffer, int64_t buffer_length, uint64_t* length)
{
    const int bits_per_chunk = ENCODED_BITS_PER_BYTE - 1;
    const int continuation_bit = 1 << bits_per_chunk;
    const int chunk_mask = continuation_bit - 1;
    KSLOG_DEBUG("bits %d, continue %02x, mask %02x", bits_per_chunk, continuation_bit, chunk_mask);

    const char* buffer_end = buffer + buffer_length;
    uint64_t value = 0;
    int decoded;

    for(const char* src = buffer; src < buffer_end; src++)
    {
        decoded = g_decode_alphabet[(int)*src];
        value = (value << bits_per_chunk) | (decoded & chunk_mask);
        KSLOG_DEBUG("Chunk %d: '%c' (%d), continue %d, value portion = %d",
            src - buffer,
            *src,
            decoded,
            decoded & continuation_bit,
            (decoded & chunk_mask)
            );
        if(!(decoded & continuation_bit))
        {
            break;
        }
    }
    if(decoded & continuation_bit)
    {
        KSLOG_DEBUG("Error: Unterminated length field");
        return SAFE64_ERROR_UNTERMINATED_LENGTH_FIELD;
    }
    *length = value;
    return SAFE64_STATUS_OK;
}
