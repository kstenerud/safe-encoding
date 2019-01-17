#include <safe64/safe64.h>
#include "safe64_version.h"

// #define KSLogger_LocalLevel TRACE
#include "kslogger.h"

static const int g_bytes_per_group       = 3;
static const int g_chunks_per_group      = 4;
static const int g_bits_per_byte         = 8;
static const int g_bits_per_chunk        = 6;
static const int g_bits_per_length_chunk = 5;

#define CHUNK_CODE_ERROR      0xff
#define CHUNK_CODE_WHITESPACE 0xfe

static const uint8_t g_encode_char_to_chunk[] =
{
#define ERRR CHUNK_CODE_ERROR
#define WHTE CHUNK_CODE_WHITESPACE
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

static const uint8_t g_chunk_to_encode_char[] =
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

static const int g_chunk_to_byte_count[]   = { 0, 0, 1, 2, 3 };

static const int g_chunk_size_to_padding[] = { 0, 0, 12, 6, 0 };

static const int g_byte_to_chunk_count[]   = { 0, 2, 3, 4 };

static const int g_byte_size_to_padding[]  = { 0, 16, 8, 0 };

static inline int64_t accumulate_byte(const int64_t accumulator, const uint8_t byte_value)
{
    const int64_t new_accumulator = (accumulator << g_bits_per_byte) | byte_value;
    KSLOG_DEBUG("Accumulate byte value %02x. Accumulator now = %lx", byte_value, new_accumulator);
    return new_accumulator;
}

static inline int64_t accumulate_chunk(const int64_t accumulator, const uint8_t next_chunk)
{
    const int64_t new_accumulator = (accumulator << g_bits_per_chunk) | next_chunk;
    KSLOG_DEBUG("Accumulate chunk value %02x. Accumulator now = %lx", next_chunk, new_accumulator);
    return new_accumulator;
}

static inline int extract_byte_from_accumulator(const int64_t accumulator, const int byte_index_hi_first)
{
    const int byte_mask = (1 << g_bits_per_byte) - 1;
    const int shift_amount = (g_bytes_per_group - byte_index_hi_first - 1) * g_bits_per_byte;
    const int extracted_byte = (int)(accumulator >> shift_amount) & byte_mask;
    KSLOG_DEBUG("Extract byte %d from %lx: %02x", byte_index_hi_first, accumulator, extracted_byte);
    return extracted_byte;
}

static inline int extract_chunk_from_accumulator(const int64_t accumulator, const int chunk_index_hi_first)
{
    const int chunk_mask = (1 << g_bits_per_chunk) - 1;
    const int shift_amount = (g_chunks_per_group - chunk_index_hi_first - 1) * g_bits_per_chunk;
    const int extracted_chunk = (int)(accumulator >> shift_amount) & chunk_mask;
    KSLOG_DEBUG("Extract chunk %d from %lx: %02x (%c)", chunk_index_hi_first, accumulator, extracted_chunk,
        g_chunk_to_encode_char[extracted_chunk]);
    return extracted_chunk;
}

static inline int64_t pad_accumulator_for_decoded_byte_count(const int64_t accumulator, const int decoded_byte_count)
{
    const int padding_bits = g_byte_size_to_padding[decoded_byte_count];
    const int64_t new_accumulator = accumulator << padding_bits;
    KSLOG_DEBUG("Padding %d bits for %d bytes. Accumulator now = %lx", padding_bits,
        decoded_byte_count, new_accumulator);
    return new_accumulator;
}

static inline int64_t pad_accumulator_for_encoded_chunk_count(const int64_t accumulator, const int encoded_chunk_count)
{
    const int padding_bits = g_chunk_size_to_padding[encoded_chunk_count];
    const int64_t new_accumulator = accumulator << padding_bits;
    KSLOG_DEBUG("Padding %d bits for %d chunks. Accumulator now = %lx", padding_bits,
        encoded_chunk_count, new_accumulator);
    return new_accumulator;

}


// ===========================================================================
// Code below this point is the same in all safeXX codecs (with a different
// function name prefix).
// After changing anything below this point, please copy the changes to all
// other codecs.
// ===========================================================================

static inline int calculate_length_chunk_count(int64_t length)
{
    int chunk_count = 0;
    for(uint64_t i = length; i; i >>= g_bits_per_length_chunk, chunk_count++)
    {
    }

    if(chunk_count == 0)
    {
        chunk_count = 1;
    }

    return chunk_count;
}

const char* safe64_version()
{
    return SAFE64_VERSION;
}

int64_t safe64_get_decoded_length(const int64_t encoded_length)
{
    if(encoded_length < 0)
    {
        return SAFE64_ERROR_INVALID_LENGTH;
    }
    const int64_t group_count = encoded_length / g_chunks_per_group;
    const int byte_count = g_chunk_to_byte_count[encoded_length % g_chunks_per_group];
    const int64_t result = group_count * g_bytes_per_group + byte_count;

    KSLOG_DEBUG("Encoded Length %d, groups %d, mod %d, byte_count %d, result %d",
        encoded_length, group_count, encoded_length % g_chunks_per_group, byte_count, result);
    return result;
}

safe64_status safe64_decode_feed(const uint8_t** const src_buffer_ptr,
                                 const int64_t src_length,
                                 uint8_t** const dst_buffer_ptr,
                                 const int64_t dst_length,
                                 const safe64_stream_state stream_state)
{
    if(src_length < 0 || dst_length < 0)
    {
        return SAFE64_ERROR_INVALID_LENGTH;
    }
    const uint8_t* src = *src_buffer_ptr;
    uint8_t* dst = *dst_buffer_ptr;

    const uint8_t* const src_end = src + src_length;
    const uint8_t* const dst_end = dst + dst_length;

    KSLOG_DEBUG("Decode %d chars into %d bytes, stream state %d",
                src_end - src, dst_end - dst, stream_state);

    #define WRITE_BYTES(CHUNK_COUNT) \
    { \
        const int bytes_to_write = g_chunk_to_byte_count[CHUNK_COUNT]; \
        KSLOG_DEBUG("Writing %d chunks as %d decoded bytes", CHUNK_COUNT, bytes_to_write); \
        for(int i = 0; i < bytes_to_write; i++) \
        { \
            *dst++ = extract_byte_from_accumulator(accumulator, i); \
            KSLOG_DEBUG("Wrote byte %02x to index %d", dst[-1], i); \
        } \
    }

    const uint8_t* last_src = src;
    int current_group_chunk_count = 0;
    int64_t accumulator = 0;

    while(src < src_end)
    {
        const uint8_t next_char = *src++;
        const uint8_t next_chunk = g_encode_char_to_chunk[next_char];
        if(next_chunk == CHUNK_CODE_WHITESPACE)
        {
            KSLOG_TRACE("Whitespace");
            continue;
        }
        if(next_chunk == CHUNK_CODE_ERROR)
        {
            KSLOG_DEBUG("Error: Invalid source data: %02x: [%c]", next_char, next_char);
            *src_buffer_ptr = src - 1;
            *dst_buffer_ptr = dst;
            return SAFE64_ERROR_INVALID_SOURCE_DATA;
        }
        accumulator = accumulate_chunk(accumulator, next_chunk);
        current_group_chunk_count++;
        KSLOG_DEBUG("Accumulated chunk %d of %d", current_group_chunk_count, g_chunks_per_group);
        if(dst + g_chunk_to_byte_count[current_group_chunk_count] >= dst_end)
        {
            break;
        }
        if(current_group_chunk_count == g_chunks_per_group)
        {
            WRITE_BYTES(current_group_chunk_count);
            current_group_chunk_count = 0;
            accumulator = 0;
            last_src = src;
        }
    }

    // Skip over any trailing whitespace
    for(; src < src_end; src++)
    {
        if(g_encode_char_to_chunk[*src] != CHUNK_CODE_WHITESPACE)
        {
            last_src = src;
            break;
        }
    }

    bool src_is_at_end = (stream_state & SAFE64_SRC_IS_AT_END_OF_STREAM) && src >= src_end;
    bool dst_is_at_end = (stream_state & SAFE64_DST_IS_AT_END_OF_STREAM) && dst + g_chunk_to_byte_count[current_group_chunk_count] >= dst_end;

    if(current_group_chunk_count > 0 && (src_is_at_end || dst_is_at_end))
    {
        accumulator = pad_accumulator_for_encoded_chunk_count(accumulator, current_group_chunk_count);
        WRITE_BYTES(current_group_chunk_count);
        last_src = src;
        dst_is_at_end = (stream_state & SAFE64_DST_IS_AT_END_OF_STREAM) && dst + g_chunk_to_byte_count[current_group_chunk_count] >= dst_end;
    }

    *src_buffer_ptr = last_src;
    *dst_buffer_ptr = dst;

    KSLOG_DEBUG("stream_state %d, src>end %d, dst>end %d", stream_state, src >= src_end, dst >= dst_end);
    KSLOG_TRACE("src %p %p, dst %p %p", src, src_end, dst, dst_end);
    KSLOG_DEBUG("src_is_at_end %d, dst_is_at_end %d", src_is_at_end, dst_is_at_end);
    if(src_is_at_end || dst_is_at_end)
    {
        if(stream_state & SAFE64_EXPECT_DST_STREAM_TO_END)
        {
            if(dst_is_at_end)
            {
                KSLOG_DEBUG("OK: Dest is at end of stream & controls end of stream");
                return SAFE64_STATUS_OK;
            }
            else
            {
                KSLOG_DEBUG("Error: Dest controls end of stream, but source is at end");
                return SAFE64_ERROR_TRUNCATED_DATA;
            }
        }
        else
        {
            if(src_is_at_end)
            {
                KSLOG_DEBUG("OK: Source is at end of stream & controls end of stream");
                return SAFE64_STATUS_OK;
            }
            else
            {
                KSLOG_DEBUG("Error: Source controls end of stream, but dest is at end");
                return SAFE64_ERROR_NOT_ENOUGH_ROOM;
            }
        }
    }

    KSLOG_DEBUG("Decode partially complete");
    return SAFE64_STATUS_PARTIALLY_COMPLETE;

    #undef WRITE_BYTES
}

int64_t safe64_read_length_field(const uint8_t* const buffer,
                                 const int64_t buffer_length,
                                 uint64_t* const length)
{
    if(buffer_length < 0)
    {
        return SAFE64_ERROR_INVALID_LENGTH;
    }
    const int continuation_bit = 1 << g_bits_per_length_chunk;
    const int chunk_mask = continuation_bit - 1;
    KSLOG_DEBUG("bits %d, continue %02x, mask %02x",
                g_bits_per_length_chunk, continuation_bit, chunk_mask);

    const uint8_t* buffer_end = buffer + buffer_length;
    uint64_t value = 0;
    int next_chunk = 0;

    const uint8_t* src = buffer;
    while(src < buffer_end)
    {
        next_chunk = g_encode_char_to_chunk[(int)*src];
        value = (value << g_bits_per_length_chunk) | (next_chunk & chunk_mask);
        KSLOG_DEBUG("Chunk %d: '%c' (%d), continue %d, value portion = %d",
                    src - buffer, *src, next_chunk, next_chunk & continuation_bit,
                    (next_chunk & chunk_mask));
        src++;
        if(!(next_chunk & continuation_bit))
        {
            break;
        }
    }
    if(next_chunk & continuation_bit)
    {
        KSLOG_DEBUG("Error: Unterminated length field");
        return SAFE64_ERROR_UNTERMINATED_LENGTH_FIELD;
    }
    *length = value;
    KSLOG_DEBUG("Length = %d, chunks = %d", value, src - buffer);
    return src - buffer;
}

int64_t safe64_decode(const uint8_t* const src_buffer,
                      const int64_t src_length,
                      uint8_t* const dst_buffer,
                      const int64_t dst_length)
{
    if(src_length < 0 || dst_length < 0)
    {
        return SAFE64_ERROR_INVALID_LENGTH;
    }
    const uint8_t* src = src_buffer;
    uint8_t* dst = dst_buffer;
    const safe64_status status = safe64_decode_feed(
                                    &src,
                                    src_length,
                                    &dst,
                                    dst_length,
                                    SAFE64_SRC_IS_AT_END_OF_STREAM | SAFE64_DST_IS_AT_END_OF_STREAM);
    if(status != SAFE64_STATUS_OK)
    {
        if(status == SAFE64_STATUS_PARTIALLY_COMPLETE)
        {
            return SAFE64_ERROR_NOT_ENOUGH_ROOM;
        }
        return status;
    }
    return dst - dst_buffer;
}

int64_t safe64l_decode(const uint8_t* const src_buffer,
                       const int64_t src_length,
                       uint8_t* const dst_buffer,
                       const int64_t dst_length)
{
    if(src_length < 0 || dst_length < 0)
    {
        return SAFE64_ERROR_INVALID_LENGTH;
    }
    uint64_t length = 0;
    const int64_t bytes_used = safe64_read_length_field(src_buffer, src_length, &length);
    if(bytes_used < 0)
    {
        return bytes_used;
    }
    if(length > (uint64_t)(src_length - bytes_used))
    {
        KSLOG_DEBUG("Require %d bytes but only %d available", length, src_length - bytes_used);
        return SAFE64_ERROR_TRUNCATED_DATA;
    }
    KSLOG_DEBUG("Used %d bytes", bytes_used);
    const int64_t read_length = src_length - bytes_used;
    const uint8_t* src = src_buffer + bytes_used;
    uint8_t* dst = dst_buffer;
    const safe64_status status = safe64_decode_feed(
                                    &src,
                                    read_length,
                                    &dst,
                                    dst_length,
                                    SAFE64_SRC_IS_AT_END_OF_STREAM | SAFE64_DST_IS_AT_END_OF_STREAM);
    if(status != SAFE64_STATUS_OK)
    {
        if(status == SAFE64_STATUS_PARTIALLY_COMPLETE)
        {
            return SAFE64_ERROR_NOT_ENOUGH_ROOM;
        }
        return status;
    }
    return dst - dst_buffer;
}

int64_t safe64_get_encoded_length(const int64_t decoded_length,
                                  const bool include_length_field)
{
    if(decoded_length < 0)
    {
        return SAFE64_ERROR_INVALID_LENGTH;
    }
    const int64_t group_count = decoded_length / g_bytes_per_group;
    const int chunk_count = g_byte_to_chunk_count[decoded_length % g_bytes_per_group];
    int length_chunk_count = 0;
    if(include_length_field)
    {
        length_chunk_count = calculate_length_chunk_count(decoded_length);
    }
    KSLOG_DEBUG("Decoded Length %d, groups %d, mod %d, chunk_count %d, length_chunk_count %d, result %d",
                decoded_length, group_count, decoded_length % g_bytes_per_group, chunk_count,
                length_chunk_count, group_count * g_bytes_per_group + chunk_count + length_chunk_count);
    return group_count * g_chunks_per_group + chunk_count + length_chunk_count;
}

safe64_status safe64_encode_feed(const uint8_t** const dst_buffer_ptr,
                                 const int64_t dst_length,
                                 uint8_t** const src_buffer_ptr,
                                 const int64_t src_length,
                                 const bool is_end_of_data)
{
    if(dst_length < 0 || src_length < 0)
    {
        return SAFE64_ERROR_INVALID_LENGTH;
    }
    const uint8_t* dst = *dst_buffer_ptr;
    uint8_t* src = *src_buffer_ptr;

    const uint8_t* const dst_end = dst + dst_length;
    const uint8_t* const src_end = src + src_length;

    KSLOG_DEBUG("Encode %d bytes into %d encoded chars, ending %d",
                dst_end - dst, src_end - src, is_end_of_data);

    #define WRITE_CHUNKS(DEC_BYTE_COUNT) \
    { \
        int chunks_to_write = g_byte_to_chunk_count[DEC_BYTE_COUNT]; \
        KSLOG_DEBUG("Writing %d bytes of %lx as %d chunks", DEC_BYTE_COUNT, accumulator, chunks_to_write); \
        if(src + chunks_to_write > src_end) \
        { \
            KSLOG_DEBUG("Error: Need %d chars but only %d available", chunks_to_write, src_end - src); \
            *dst_buffer_ptr = last_dst; \
            *src_buffer_ptr = src; \
            return SAFE64_STATUS_PARTIALLY_COMPLETE; \
        } \
        for(int i = 0; i < chunks_to_write; i++) \
        { \
            *src++ = g_chunk_to_encode_char[extract_chunk_from_accumulator(accumulator, i)]; \
            KSLOG_DEBUG("Wrote chunk %c to index %d", src[-1], i); \
        } \
    }

    const uint8_t* last_dst = dst;
    int current_group_byte_count = 0;
    int64_t accumulator = 0;

    while(dst < dst_end)
    {
        const uint8_t next_byte = *dst++;
        accumulator = accumulate_byte(accumulator, next_byte);
        current_group_byte_count++;
        KSLOG_DEBUG("Accumulated byte %d of %d", current_group_byte_count, g_bytes_per_group);
        if(current_group_byte_count == g_bytes_per_group)
        {
            WRITE_CHUNKS(current_group_byte_count);
            current_group_byte_count = 0;
            accumulator = 0;
            last_dst = dst;
        }
    }

    if(current_group_byte_count > 0)
    {
        if(is_end_of_data)
        {
            accumulator = pad_accumulator_for_decoded_byte_count(accumulator, current_group_byte_count);
            WRITE_CHUNKS(current_group_byte_count);
        }
        else
        {
            dst -= current_group_byte_count;
        }
        last_dst = dst;
    }

    *dst_buffer_ptr = last_dst;
    *src_buffer_ptr = src;

    return SAFE64_STATUS_OK;
#undef WRITE_CHUNKS
}

int64_t safe64_write_length_field(const uint64_t length,
                                  uint8_t* const src_buffer,
                                  const int64_t src_buffer_length)
{
    if(src_buffer_length < 0)
    {
        return SAFE64_ERROR_INVALID_LENGTH;
    }
    const int continuation_bit = 1 << g_bits_per_length_chunk;
    const int chunk_mask = continuation_bit - 1;
    KSLOG_DEBUG("bits %d, continue %02x, mask %02x", g_bits_per_length_chunk, continuation_bit, chunk_mask);

    int chunk_count = 0;
    for(uint64_t i = length; i; i >>= g_bits_per_length_chunk, chunk_count++)
    {
    }
    if(chunk_count == 0)
    {
        chunk_count = 1;
    }
    KSLOG_DEBUG("Value: %lu, chunk count %d", length, chunk_count);

    if(chunk_count > src_buffer_length)
    {
        KSLOG_DEBUG("Error: Require %d bytes but only %d available", chunk_count, src_buffer_length);
        return SAFE64_ERROR_NOT_ENOUGH_ROOM;
    }

    uint8_t* src = src_buffer;
    for(int shift_amount = chunk_count - 1; shift_amount >= 0; shift_amount--)
    {
        const int should_continue = (shift_amount == 0) ? 0 : continuation_bit;
        const int chunk_value = ((length>>(g_bits_per_length_chunk * shift_amount)) & chunk_mask) + should_continue;
        const uint8_t next_char = g_chunk_to_encode_char[chunk_value];
        *src++ = next_char;
        KSLOG_DEBUG("Chunk %d: '%c' (%d), continue %d", shift_amount, next_char,
                    ((length>>(g_bits_per_length_chunk * shift_amount)) & chunk_mask), should_continue);
    }
    return chunk_count;
}

int64_t safe64_encode(const uint8_t* const dst_buffer,
                      const int64_t dst_length,
                      uint8_t* const src_buffer,
                      const int64_t src_length)
{
    if(dst_length < 0 || src_length < 0)
    {
        return SAFE64_ERROR_INVALID_LENGTH;
    }
    const uint8_t* dst = dst_buffer;
    uint8_t* src = src_buffer;
    const safe64_status status = safe64_encode_feed(&dst, dst_length, &src, src_length, true);
    if(status != SAFE64_STATUS_OK)
    {
        if(status == SAFE64_STATUS_PARTIALLY_COMPLETE)
        {
            return SAFE64_ERROR_NOT_ENOUGH_ROOM;
        }
        return status;
    }
    return src - src_buffer;
}

int64_t safe64l_encode(const uint8_t* const dst_buffer,
                       const int64_t dst_length,
                       uint8_t* const src_buffer,
                       const int64_t src_length)
{
    if(dst_length < 0 || src_length < 0)
    {
        return SAFE64_ERROR_INVALID_LENGTH;
    }
    int64_t bytes_used = safe64_write_length_field(dst_length, src_buffer, src_length);
    if(bytes_used < 0)
    {
        return bytes_used;
    }

    const uint8_t* dst = dst_buffer;
    uint8_t* src = src_buffer + bytes_used;
    safe64_status status = safe64_encode_feed(&dst, dst_length, &src, src_length, true);
    if(status != SAFE64_STATUS_OK)
    {
        if(status == SAFE64_STATUS_PARTIALLY_COMPLETE)
        {
            return SAFE64_ERROR_NOT_ENOUGH_ROOM;
        }
        return status;
    }
    return src - src_buffer;
}
