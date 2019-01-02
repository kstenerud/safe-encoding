#include <safe64/safe64.h>
#include "safe64_version.h"

#include <limits.h>

// #define KSLogger_LocalLevel DEBUG
#include "kslogger.h"

#define DECODED_BYTES_PER_GROUP 3
#define ENCODED_BYTES_PER_GROUP 4
#define ENCODED_BITS_PER_BYTE 6
#define DECODED_BITS_PER_BYTE 8


static const int32_t g_decode_alphabet[] =
{
#define ERRR INT_MAX
    // Control chars
    ERRR, ERRR, ERRR, ERRR, ERRR, ERRR, ERRR, ERRR,
    ERRR, ERRR, ERRR, ERRR, ERRR, ERRR, ERRR, ERRR,
    ERRR, ERRR, ERRR, ERRR, ERRR, ERRR, ERRR, ERRR,
    ERRR, ERRR, ERRR, ERRR, ERRR, ERRR, ERRR, ERRR,
    ERRR, ERRR, ERRR, ERRR, ERRR, ERRR, ERRR, ERRR,
    /* -   */ ERRR, ERRR, ERRR, ERRR, ERRR, 0x00, ERRR, ERRR,
    /* 0-7 */ 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
    /* 8-9 */ 0x09, 0x0a, ERRR, ERRR, ERRR, ERRR, ERRR, ERRR,
    /* A-G */ ERRR, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11,
    /* H-O */ 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19,
    /* P-W */ 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20, 0x21,
    /* X-_ */ 0x22, 0x23, 0x24, ERRR, ERRR, ERRR, ERRR, 0x25,
    /* a-g */ ERRR, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c,
    /* h-o */ 0x2d, 0x2e, 0x2f, 0x30, 0x31, 0x32, 0x33, 0x34,
    /* p-w */ 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c,
    /* x-z */ 0x3d, 0x3e, 0x3f, ERRR, ERRR, ERRR, ERRR, ERRR,
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
#define ERROR_STATE INT_MIN
    0,
    ERROR_STATE,
    1,
    2,
#undef ERROR_STATE
};

static const int g_decoded_remainder_to_encoded_remainder[] =
{
    0,
    2,
    3,
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

int64_t safe64_decode_feed(const char* src_buffer,
                           int64_t src_length,
                           unsigned char* dst_buffer,
                           int64_t dst_length,
                           bool is_end_of_data)
{
    const uint32_t max_accumulator_value = (1 << (DECODED_BITS_PER_BYTE * DECODED_BYTES_PER_GROUP)) - 1;
    const int32_t src_bytes_per_group = ENCODED_BYTES_PER_GROUP;
    const int32_t dst_bytes_per_group = DECODED_BYTES_PER_GROUP;
    const int32_t src_bits_per_byte = ENCODED_BITS_PER_BYTE;
    const int32_t dst_bits_per_byte = DECODED_BITS_PER_BYTE;
    const int dst_mask = 0xff;
    const int32_t* const alphabet = g_decode_alphabet;
    const int* const src_to_dst_remainder = g_encoded_remainder_to_decoded_remainder;
    const char* src = src_buffer;
    unsigned char* dst = dst_buffer;

    int64_t src_group_count = src_length / src_bytes_per_group;
    int64_t dst_group_count = dst_length / dst_bytes_per_group;
    int64_t group_count = src_group_count <= dst_group_count ? src_group_count : dst_group_count;

    KSLOG_DEBUG("Decode %d chars into %d bytes, ending %d", src_length, dst_length, is_end_of_data);
    for(int64_t group = 0; group < group_count; group++)
    {
        uint32_t accumulator = 0;
        for(int i = 0; i < src_bytes_per_group; i++)
        {
            accumulator = (accumulator << src_bits_per_byte) | alphabet[(int)(*src++)];
            KSLOG_DEBUG("Accumulate %c (%02x). Total = %x", src[-1], alphabet[(int)src[-1]], accumulator);
        }
        if(accumulator > max_accumulator_value)
        {
            KSLOG_DEBUG("Error: accumulator (%x) > max accum %x", accumulator, max_accumulator_value);
            return SAFE64_ERROR_INVALID_SOURCE_DATA;
        }
        for(int i = dst_bytes_per_group-1; i >= 0; i--)
        {
            *dst++ = (accumulator >> (dst_bits_per_byte * i)) & dst_mask;
            KSLOG_DEBUG("Extract pos %d: %02x",
                (dst_bits_per_byte * i),
                (accumulator >> (dst_bits_per_byte * i)) & dst_mask);
        }
    }

    int64_t src_remainder = 0;
    if(is_end_of_data)
    {
        src_remainder = src_length - src_group_count * src_bytes_per_group;
        int64_t dst_remainder = src_to_dst_remainder[src_remainder];
        KSLOG_DEBUG("E Remainder src = %d, dst = %d", src_remainder, dst_remainder);
        if(dst_remainder < 0)
        {
            KSLOG_DEBUG("Error: dst_remainder (%d) < 0", dst_remainder);
            return SAFE64_ERROR_SOURCE_DATA_MISSING;
        }
        if((src_group_count * dst_bytes_per_group + dst_remainder) > dst_length)
        {
            KSLOG_DEBUG("Error: require dst buffer length %d, but only have %d",
                src_group_count * dst_bytes_per_group + dst_remainder, dst_length);
            return SAFE64_ERROR_NOT_ENOUGH_ROOM;
        }

        uint32_t accumulator = 0;
        for(int i = 0; i < src_remainder; i++)
        {
            accumulator = (accumulator << src_bits_per_byte) | alphabet[(int)(*src++)];
            KSLOG_DEBUG("E Accumulate %c (%02x). Total = %x", src[-1], alphabet[(int)src[-1]], accumulator);
        }
        if(accumulator > max_accumulator_value)
        {
            KSLOG_DEBUG("Error: accumulator (%x) > max accum %x", accumulator, max_accumulator_value);
            return SAFE64_ERROR_INVALID_SOURCE_DATA;
        }
        int phantom_bits = src_remainder * src_bits_per_byte - dst_remainder * dst_bits_per_byte;
        KSLOG_DEBUG("E Phantom bits: %d", phantom_bits);
        accumulator >>= phantom_bits;
        for(int i = dst_remainder-1; i >= 0; i--)
        {
            *dst++ = (accumulator >> (dst_bits_per_byte * i)) & dst_mask;
            KSLOG_DEBUG("E Extract pos %d: %02x",
                (dst_bits_per_byte * i),
                (accumulator >> (dst_bits_per_byte * i)) & dst_mask);
        }
    }

    return src_group_count * src_bytes_per_group + src_remainder;
}

int64_t safe64_decode(const char* src_buffer,
                      int64_t src_length,
                      unsigned char* dst_buffer,
                      int64_t dst_length)
{
    return safe64_decode_feed(src_buffer, src_length, dst_buffer, dst_length, true);
}

int64_t safe64_get_encoded_length(const int64_t decoded_length)
{
    int64_t groups = decoded_length / 3;
    int remainder = g_decoded_remainder_to_encoded_remainder[decoded_length % 3];
    return groups * 4 + remainder;
}

int64_t safe64_encode_feed(const unsigned char* src_buffer,
                           int64_t src_length,
                           char* dst_buffer,
                           int64_t dst_length,
                           bool is_end_of_data)
{
    const int32_t src_bytes_per_group = DECODED_BYTES_PER_GROUP;
    const int32_t dst_bytes_per_group = ENCODED_BYTES_PER_GROUP;
    const int32_t src_bits_per_byte = DECODED_BITS_PER_BYTE;
    const int32_t dst_bits_per_byte = ENCODED_BITS_PER_BYTE;
    const int dst_mask = 0x3f;
    const char* const alphabet = g_encode_alphabet;
    const int* const src_to_dst_remainder = g_decoded_remainder_to_encoded_remainder;
    const unsigned char* src = src_buffer;
    char* dst = dst_buffer;

    int64_t src_group_count = src_length / src_bytes_per_group;
    int64_t dst_group_count = dst_length / dst_bytes_per_group;
    int64_t group_count = src_group_count <= dst_group_count ? src_group_count : dst_group_count;

    KSLOG_DEBUG("Encode %d bytes into %d chars, ending %d", src_length, dst_length, is_end_of_data);
    for(int64_t group = 0; group < group_count; group++)
    {
        int32_t accumulator = 0;
        for(int i = 0; i < src_bytes_per_group; i++)
        {
            accumulator = (accumulator << src_bits_per_byte) | *src++;
            KSLOG_DEBUG("Accumulate %02x. Total = %x", src[-1], accumulator);
        }
        for(int i = dst_bytes_per_group-1; i >= 0; i--)
        {
            *dst++ = alphabet[(accumulator >> (dst_bits_per_byte * i)) & dst_mask];
            KSLOG_DEBUG("Extract pos %d: %02x and convert to %c",
                (dst_bits_per_byte * i),
                (accumulator >> (dst_bits_per_byte * i)) & dst_mask,
                dst[-1]);
        }
    }

    int64_t src_remainder = 0;
    if(is_end_of_data)
    {
        src_remainder = src_length - src_group_count * src_bytes_per_group;
        int64_t dst_remainder = src_to_dst_remainder[src_remainder];
        KSLOG_DEBUG("E Remainder src = %d, dst = %d", src_remainder, dst_remainder);
        if((src_group_count * dst_bytes_per_group + dst_remainder) > dst_length)
        {
            return SAFE64_ERROR_NOT_ENOUGH_ROOM;
        }

        int32_t accumulator = 0;
        for(int i = 0; i < src_remainder; i++)
        {
            accumulator = (accumulator << src_bits_per_byte) | *src++;
            KSLOG_DEBUG("E Accumulate %02x. Total = %x", src[-1], accumulator);
        }
        int phantom_bits = dst_remainder * dst_bits_per_byte - src_remainder * src_bits_per_byte;
        KSLOG_DEBUG("E Phantom bits: %d", phantom_bits);
        accumulator <<= phantom_bits;
        for(int i = dst_remainder-1; i >= 0; i--)
        {
            *dst++ = alphabet[(accumulator >> (dst_bits_per_byte * i)) & dst_mask];
            KSLOG_DEBUG("E Extract pos %d: %02x and convert to %c",
                (dst_bits_per_byte * i),
                (accumulator >> (dst_bits_per_byte * i)) & dst_mask,
                dst[-1]);
        }
    }

    return src_group_count * src_bytes_per_group + src_remainder;
}

int64_t safe64_encode(const unsigned char* src_buffer,
                      int64_t src_length,
                      char* dst_buffer,
                      int64_t dst_length)
{
    return safe64_encode_feed(src_buffer, src_length, dst_buffer, dst_length, true);
}

