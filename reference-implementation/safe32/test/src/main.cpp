#include <gtest/gtest.h>

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

// ==================================================================


#include <safe32/safe32.h>

// #define KSLogger_LocalLevel DEBUG
#include "kslogger.h"

#define DECODED_BYTES_PER_GROUP 5
#define ENCODED_BYTES_PER_GROUP 8


// ----------
// Assertions
// ----------

void assert_encode_decode(std::string expected_encoded, std::vector<unsigned char> expected_decoded)
{
    int64_t expected_encoded_length = expected_encoded.size();
    int64_t actual_encoded_length = safe32_get_encoded_length(expected_decoded.size(), false);
    ASSERT_EQ(expected_encoded_length, actual_encoded_length);

    std::vector<char> encode_buffer(1000);
    int64_t actual_encode_used_bytes = safe32_encode(expected_decoded.data(),
                                                     expected_decoded.size(),
                                                     encode_buffer.data(),
                                                     encode_buffer.size());
    ASSERT_EQ(expected_encoded_length, actual_encode_used_bytes);
    std::string actual_encoded(encode_buffer.begin(), encode_buffer.begin() + actual_encode_used_bytes);
    ASSERT_EQ(actual_encoded_length, actual_encode_used_bytes);
    ASSERT_EQ(expected_encoded, actual_encoded);

    int64_t expected_decoded_length = expected_decoded.size();
    int64_t actual_decoded_length = safe32_get_decoded_length(expected_encoded.size());
    ASSERT_EQ(expected_decoded_length, actual_decoded_length);

    std::vector<unsigned char> decode_buffer(1000);
    int64_t expected_decode_used_bytes = expected_decoded.size();
    int64_t actual_decode_used_bytes = safe32_decode(expected_encoded.data(),
                                                     expected_encoded.size(),
                                                     decode_buffer.data(),
                                                     decode_buffer.size());
    ASSERT_EQ(expected_decode_used_bytes, actual_decode_used_bytes);
    std::vector<unsigned char> actual_decoded(decode_buffer.begin(), decode_buffer.begin() + actual_decode_used_bytes);
    ASSERT_EQ(expected_decoded, actual_decoded);
}

void assert_encode_decode_with_length(std::string expected_encoded, std::vector<unsigned char> expected_decoded)
{
    std::vector<char> encode_buffer(1000);
    int64_t actual_encode_used_bytes = safe32l_encode(expected_decoded.data(),
                                                      expected_decoded.size(),
                                                      encode_buffer.data(),
                                                      encode_buffer.size());
    ASSERT_GT(actual_encode_used_bytes, 0);
    std::string actual_encoded(encode_buffer.begin(), encode_buffer.begin() + actual_encode_used_bytes);
    ASSERT_EQ(expected_encoded, actual_encoded);

    std::vector<unsigned char> decode_buffer(1000);
    int64_t actual_decode_used_bytes = safe32l_decode(expected_encoded.data(),
                                                      expected_encoded.size(),
                                                      decode_buffer.data(),
                                                      decode_buffer.size());
    ASSERT_GT(actual_decode_used_bytes, 0);
    std::vector<unsigned char> actual_decoded(decode_buffer.begin(), decode_buffer.begin() + actual_decode_used_bytes);
    ASSERT_EQ(expected_decoded, actual_decoded);
}

void assert_encode_decode_with_length_status(std::string expected_encoded, int64_t force_length, int64_t expected_status)
{
    std::vector<unsigned char> decode_buffer(1000);
    int64_t actual_decode_used_bytes = safe32l_decode(expected_encoded.data(),
                                                      force_length,
                                                      decode_buffer.data(),
                                                      decode_buffer.size());
    ASSERT_EQ(expected_status, actual_decode_used_bytes);
}

std::vector<uint8_t> make_bytes(int length, int start_value)
{
    std::vector<uint8_t> vec;
    for(int i = 0; i < length; i++)
    {
        vec.push_back((uint8_t)((start_value+i) & 0xff));
    }
    return vec;
}

void assert_chunked_encode_src_packeted(int length)
{
    std::vector<unsigned char> data = make_bytes(length, length);
    std::vector<char> encode_buffer(length * 2);
    std::vector<unsigned char> decode_buffer(length);

    for(int packet_size=length-1; packet_size >= DECODED_BYTES_PER_GROUP; packet_size--)
    {
        KSLOG_DEBUG("packet size %d", packet_size);
        safe32_status_code status = SAFE32_STATUS_OK;
        const unsigned char* e_src = data.data();
        const unsigned char* e_src_end = data.data() + data.size();
        char* e_dst = encode_buffer.data();
        char* e_dst_end = encode_buffer.data() + encode_buffer.size();
        const char* d_src = encode_buffer.data();
        unsigned char* d_dst = decode_buffer.data();
        unsigned char* d_dst_end = decode_buffer.data() + decode_buffer.size();

        while(e_src < e_src_end)
        {
            bool is_end = false;
            int encode_byte_count = packet_size;
            if(encode_byte_count > e_src_end - e_src)
            {
                encode_byte_count = e_src_end - e_src;
                is_end = true;
            }
            status = safe32_encode_feed(&e_src,
                                        encode_byte_count,
                                        &e_dst,
                                        e_dst_end - e_dst,
                                        is_end);
            ASSERT_EQ(SAFE32_STATUS_OK, status);
            status = safe32_decode_feed(&d_src,
                                        e_dst - d_src,
                                        &d_dst,
                                        d_dst_end - d_dst,
                                        is_end);
            ASSERT_EQ(SAFE32_STATUS_OK, status);
        }
        ASSERT_EQ(data, decode_buffer);
    }
}

void assert_chunked_encode_dst_packeted(int length)
{
    std::vector<unsigned char> data = make_bytes(length, length);
    std::vector<char> encode_buffer(length * 2);
    std::vector<unsigned char> decode_buffer(length);

    for(int packet_size=length-1; packet_size >= ENCODED_BYTES_PER_GROUP; packet_size--)
    {
        KSLOG_DEBUG("packet size %d", packet_size);
        safe32_status_code status = SAFE32_STATUS_OK;
        const unsigned char* e_src = data.data();
        const unsigned char* e_src_end = data.data() + data.size();
        char* e_dst = encode_buffer.data();
        const char* d_src = encode_buffer.data();
        unsigned char* d_dst = decode_buffer.data();
        unsigned char* d_dst_end = decode_buffer.data() + decode_buffer.size();

        while(e_src < e_src_end)
        {
            bool is_end = e_src + packet_size >= e_src_end;
            status = safe32_encode_feed(&e_src,
                                        e_src_end - e_src,
                                        &e_dst,
                                        packet_size,
                                        is_end);
            ASSERT_TRUE(status == SAFE32_STATUS_OK || status == SAFE32_STATUS_NOT_ENOUGH_ROOM);
            status = safe32_decode_feed(&d_src,
                                        e_dst - d_src,
                                        &d_dst,
                                        d_dst_end - d_dst,
                                        is_end);
            ASSERT_EQ(SAFE32_STATUS_OK, status);
        }
        ASSERT_EQ(data, decode_buffer);
    }
}

void assert_chunked_decode_dst_packeted(int length)
{
    std::vector<unsigned char> data = make_bytes(length, length);
    std::vector<char> encode_buffer(length * 2);
    std::vector<unsigned char> decode_buffer(length);
    int64_t encoded_length = safe32_encode(data.data(), data.size(), encode_buffer.data(), encode_buffer.size());
    ASSERT_GT(encoded_length, 0);
    const char* encoded_end = encode_buffer.data() + encoded_length;


    for(int packet_size=length-1; packet_size >= ENCODED_BYTES_PER_GROUP; packet_size--)
    {
        KSLOG_DEBUG("packet size %d", packet_size);
        safe32_status_code status = SAFE32_STATUS_OK;
        const char* d_src = encode_buffer.data();
        unsigned char* d_dst = decode_buffer.data();
        unsigned char* d_dst_end = decode_buffer.data() + decode_buffer.size();

        while(d_src < encoded_end)
        {
            const char* d_src_end = d_src + packet_size;
            bool is_end = false;
            if(d_src_end >= encoded_end)
            {
                d_src_end = encoded_end;
                is_end = true;
            }
            status = safe32_decode_feed(&d_src,
                                        d_src_end - d_src,
                                        &d_dst,
                                        d_dst_end - d_dst,
                                        is_end);
            ASSERT_TRUE(status == SAFE32_STATUS_OK || status == SAFE32_STATUS_NOT_ENOUGH_ROOM);
        }
        ASSERT_EQ(data, decode_buffer);
    }
}

void assert_decode(std::string expected_encoded, std::vector<unsigned char> expected_decoded)
{
    int64_t decoded_length = safe32_get_decoded_length(expected_encoded.size());
    ASSERT_GE(decoded_length, 0);
    std::vector<unsigned char> decode_buffer(decoded_length);
    int64_t expected_decode_used_bytes = expected_encoded.size();
    int64_t actual_decode_used_bytes = safe32_decode(expected_encoded.data(),
                                                     expected_encoded.size(),
                                                     decode_buffer.data(),
                                                     decode_buffer.size());
    ASSERT_GE(expected_decode_used_bytes, actual_decode_used_bytes);
    std::vector<unsigned char> actual_decoded(decode_buffer.begin(), decode_buffer.begin() + actual_decode_used_bytes);
    ASSERT_EQ(expected_decoded, actual_decoded);
}

void assert_decode_status(int buffer_size, std::string encoded, int expected_status_code)
{
    std::vector<unsigned char> decode_buffer(buffer_size);
    int64_t actual_status_code = safe32_decode(encoded.data(),
                                               encoded.size(),
                                               decode_buffer.data(),
                                               decode_buffer.size());
    ASSERT_EQ(expected_status_code, actual_status_code);
}

void assert_encode_length(uint64_t length, std::string expected_encoded)
{
    std::vector<char> encode_buffer(100);
    int64_t bytes_written = safe32_write_length_field(length, encode_buffer.data(), encode_buffer.size());
    ASSERT_GT(bytes_written, 0);
    std::string actual_encoded(encode_buffer.begin(), encode_buffer.begin() + bytes_written);
    ASSERT_EQ(expected_encoded, actual_encoded);
}

void assert_encode_decode_length(uint64_t start_length, uint64_t end_length)
{
    for(__int128 i = start_length; i <= end_length; i++)
    {
        uint64_t length = (uint64_t)i;
        std::vector<char> encode_buffer(100);
        int64_t bytes_written = safe32_write_length_field(length, encode_buffer.data(), encode_buffer.size());
        ASSERT_GT(bytes_written, 0);
        uint64_t actual_length = 0;
        int64_t bytes_read = safe32_read_length_field(encode_buffer.data(), bytes_written, &actual_length);
        ASSERT_GT(bytes_read, 0);
        ASSERT_EQ(length, actual_length);
    }
}

void assert_encode_length_status(uint64_t length, int buffer_size, int64_t expected_status)
{
    std::vector<char> encode_buffer(buffer_size);
    int64_t actual_status = safe32_write_length_field(length, encode_buffer.data(), encode_buffer.size());
    ASSERT_EQ(expected_status, actual_status);
}

void assert_decode_length(std::string encoded, uint64_t expected_length, int64_t expected_status)
{
    uint64_t actual_length;
    int64_t actual_status = safe32_read_length_field(encoded.data(), encoded.size(), &actual_length);
    ASSERT_EQ(expected_status, actual_status);
    if(expected_status == SAFE32_STATUS_OK)
    {
        ASSERT_EQ(expected_length, actual_length);
    }
}



// --------------------
// Common Test Patterns
// --------------------

#define TEST_DECODE(NAME, ENCODED, ...) \
TEST(Decode, NAME) { assert_decode(ENCODED, __VA_ARGS__); }

#define TEST_ENCODE_DECODE(NAME, ENCODED, ...) \
TEST(EncodeDecode, NAME) { assert_encode_decode(ENCODED, __VA_ARGS__); }

#define TEST_ENCODE_DECODE_WITH_LENGTH(NAME, ENCODED, ...) \
TEST(EncodeDecodeWithLength, NAME) { assert_encode_decode_with_length(ENCODED, __VA_ARGS__); }

#define TEST_DECODE_ERROR(NAME, BUFFER_SIZE, ENCODED, EXPECTED_STATUS_CODE) \
TEST(DecodeError, NAME) { assert_decode_status(BUFFER_SIZE, ENCODED, EXPECTED_STATUS_CODE); }

#define TEST_ENCODE_LENGTH(NAME, LENGTH, ENCODED) \
TEST(EncodeLength, NAME) { assert_encode_length(LENGTH, ENCODED); }

#define TEST_ENCODE_LENGTH_STATUS(NAME, LENGTH, BUFFER_SIZE, STATUS) \
TEST(EncodeLengthStatus, NAME) { assert_encode_length_status(LENGTH, BUFFER_SIZE, STATUS); }

#define TEST_DECODE_LENGTH(NAME, ENCODED, EXPECTED_LENGTH, EXPECTED_STATUS) \
TEST(DecodeLength, NAME) { assert_decode_length(ENCODED, EXPECTED_LENGTH, EXPECTED_STATUS); }

#define TEST_ENCODE_DECODE_LENGTH(NAME, START_LENGTH, END_LENGTH) \
TEST(EncodeDecodeLength, NAME) { assert_encode_decode_length(START_LENGTH, END_LENGTH); }

#define TEST_DECODE_WITH_LENGTH_STATUS(NAME, ENCODED, FORCE_LENGTH, EXPECTED_STATUS) \
TEST(DecodeLength, NAME) { assert_encode_decode_with_length_status(ENCODED, FORCE_LENGTH, EXPECTED_STATUS); }


// -----
// Tests
// -----

TEST(Library, version)
{
    const char* expected = "1.0.0";
    const char* actual = safe32_version();
    ASSERT_STREQ(expected, actual);
}

TEST_ENCODE_DECODE(_1_byte,  "y5",       {0xf1})
TEST_ENCODE_DECODE(_2_bytes, "6udh",     {0x2e, 0x99})
TEST_ENCODE_DECODE(_3_bytes, "y9u6d",    {0xf2, 0x34, 0x56})
TEST_ENCODE_DECODE(_4_bytes, "ab5ctn9",  {0x4a, 0x88, 0xbc, 0xd1})
TEST_ENCODE_DECODE(_5_bytes, "zxsxufnk", {0xff, 0x71, 0xdd, 0x3a, 0x92})

TEST_ENCODE_DECODE_WITH_LENGTH(_1_byte,  "2y5",       {0xf1})
TEST_ENCODE_DECODE_WITH_LENGTH(_2_bytes, "36udh",     {0x2e, 0x99})
TEST_ENCODE_DECODE_WITH_LENGTH(_3_bytes, "4y9u6d",    {0xf2, 0x34, 0x56})
TEST_ENCODE_DECODE_WITH_LENGTH(_4_bytes, "5ab5ctn9",  {0x4a, 0x88, 0xbc, 0xd1})
TEST_ENCODE_DECODE_WITH_LENGTH(_5_bytes, "6zxsxufnk", {0xff, 0x71, 0xdd, 0x3a, 0x92})

TEST_DECODE_ERROR(dst_buffer_too_short_4, 4, "zxsxufnk", SAFE32_STATUS_NOT_ENOUGH_ROOM)
TEST_DECODE_ERROR(dst_buffer_too_short_3, 3, "zxsxufnk", SAFE32_STATUS_NOT_ENOUGH_ROOM)
TEST_DECODE_ERROR(dst_buffer_too_short_2, 2, "zxsxufnk", SAFE32_STATUS_NOT_ENOUGH_ROOM)
TEST_DECODE_ERROR(dst_buffer_too_short_1, 1, "zxsxufnk", SAFE32_STATUS_NOT_ENOUGH_ROOM)

TEST_DECODE_ERROR(invalid_0, 100, ".zxsxufnk", SAFE32_ERROR_INVALID_SOURCE_DATA)
TEST_DECODE_ERROR(invalid_1, 100, "z.xsxufnk", SAFE32_ERROR_INVALID_SOURCE_DATA)
TEST_DECODE_ERROR(invalid_2, 100, "zx.sxufnk", SAFE32_ERROR_INVALID_SOURCE_DATA)
TEST_DECODE_ERROR(invalid_3, 100, "zxs.xufnk", SAFE32_ERROR_INVALID_SOURCE_DATA)
TEST_DECODE_ERROR(invalid_4, 100, "zxsx.ufnk", SAFE32_ERROR_INVALID_SOURCE_DATA)
TEST_DECODE_ERROR(invalid_5, 100, "zxsxu.fnk", SAFE32_ERROR_INVALID_SOURCE_DATA)
TEST_DECODE_ERROR(invalid_6, 100, "zxsxuf.nk", SAFE32_ERROR_INVALID_SOURCE_DATA)
TEST_DECODE_ERROR(invalid_7, 100, "zxsxufn.k", SAFE32_ERROR_INVALID_SOURCE_DATA)
TEST_DECODE_ERROR(invalid_8, 100, "zxsxufnk.", SAFE32_ERROR_INVALID_SOURCE_DATA)

TEST_DECODE(space_0, "-zxsxufnk", {0xff, 0x71, 0xdd, 0x3a, 0x92})
TEST_DECODE(space_1, "z-xsxufnk", {0xff, 0x71, 0xdd, 0x3a, 0x92})
TEST_DECODE(space_2, "zx-sxufnk", {0xff, 0x71, 0xdd, 0x3a, 0x92})
TEST_DECODE(space_3, "zxs-xufnk", {0xff, 0x71, 0xdd, 0x3a, 0x92})
TEST_DECODE(space_4, "zxsx-ufnk", {0xff, 0x71, 0xdd, 0x3a, 0x92})
TEST_DECODE(space_5, "zxsxu-fnk", {0xff, 0x71, 0xdd, 0x3a, 0x92})
TEST_DECODE(space_6, "zxsxuf-nk", {0xff, 0x71, 0xdd, 0x3a, 0x92})
TEST_DECODE(space_7, "zxsxufn-k", {0xff, 0x71, 0xdd, 0x3a, 0x92})
TEST_DECODE(space_8, "zxsxufnk-", {0xff, 0x71, 0xdd, 0x3a, 0x92})

TEST_DECODE(lots_of_whitespace, "- z  x\t\ns\r\n\r\nx\t\tu---fnk", {0xff, 0x71, 0xdd, 0x3a, 0x92})

TEST(Packetized, encode_dst_packeted)
{
    assert_chunked_encode_dst_packeted(163);
    assert_chunked_encode_dst_packeted(10);
    assert_chunked_encode_dst_packeted(200);
}

TEST(Packetized, encode_src_packeted)
{
    assert_chunked_encode_src_packeted(131);
    assert_chunked_encode_src_packeted(15);
    assert_chunked_encode_src_packeted(230);
}

TEST(Packetized, decode_dst_packeted)
{
    assert_chunked_decode_dst_packeted(102);
    assert_chunked_decode_dst_packeted(20);
    assert_chunked_decode_dst_packeted(250);
}

TEST_ENCODE_LENGTH(_0, 0, "0")
TEST_ENCODE_LENGTH(_1, 1, "2")
TEST_ENCODE_LENGTH(_10, 10, "b")
TEST_ENCODE_LENGTH(_15, 15, "g")
TEST_ENCODE_LENGTH(_16, 16, "j0")
TEST_ENCODE_LENGTH(_17, 17, "j2")
TEST_ENCODE_LENGTH(_255, 255, "zg")
TEST_ENCODE_LENGTH(_256, 256, "jh0")
TEST_ENCODE_LENGTH(_257, 257, "jh2")
TEST_ENCODE_LENGTH(_4095, 4095, "zzg")
TEST_ENCODE_LENGTH(_4096, 4096, "jhh0")
TEST_ENCODE_LENGTH(_4097, 4097, "jhh2")
TEST_ENCODE_LENGTH(_65535, 65535, "zzzg")
TEST_ENCODE_LENGTH(_65536, 65536, "jhhh0")
TEST_ENCODE_LENGTH(_65537, 65537, "jhhh2")

TEST_ENCODE_DECODE_LENGTH(_0_2000, 0, 2000)
TEST_ENCODE_DECODE_LENGTH(_32000_33000, 32000, 33000)

TEST_ENCODE_LENGTH_STATUS(_1_length_0,   1, 0, SAFE32_STATUS_NOT_ENOUGH_ROOM)
TEST_ENCODE_LENGTH_STATUS(_1_length_1,   1, 1, 1)
TEST_ENCODE_LENGTH_STATUS(_32_length_1, 32, 1, SAFE32_STATUS_NOT_ENOUGH_ROOM)
TEST_ENCODE_LENGTH_STATUS(_32_length_2, 32, 2, 2)

TEST_DECODE_LENGTH(_0,        "0",      0, 1)
TEST_DECODE_LENGTH(_15,       "g",     15, 1)
TEST_DECODE_LENGTH(_16_bad,   "j",     16, SAFE32_ERROR_UNTERMINATED_LENGTH_FIELD)
TEST_DECODE_LENGTH(_16,       "j0",    16, 2)
TEST_DECODE_LENGTH(_256_bad,  "jh",   256, SAFE32_ERROR_UNTERMINATED_LENGTH_FIELD)
TEST_DECODE_LENGTH(_256,      "jh0",  256, 3)

TEST_DECODE(substitution, "oO3ANDS", {0x00, 0x04, 0x9a, 0x33})

TEST_DECODE_WITH_LENGTH_STATUS(_x,  "j0589rugt2s75akwb6kuqwt6mtv0", 15, SAFE32_ERROR_TRUNCATED_DATA)



// // Specification Examples:

TEST_ENCODE_DECODE(example_1, "85a96sd288dsqfbddffha40", {0x39, 0x12, 0x82, 0xe1, 0x81, 0x39, 0xd9, 0x8b, 0x39, 0x4c, 0x63, 0x9d, 0x04, 0x8c})
TEST_ENCODE_DECODE(example_2, "wsabe8zs82qrq0dt8tq67yv0", {0xe6, 0x12, 0xa6, 0x9f, 0xf8, 0x38, 0x6d, 0x7b, 0x01, 0x99, 0x3e, 0x6c, 0x53, 0x7b, 0x60})
TEST_ENCODE_DECODE(example_3, "589rugt2s75akwb6kuqwt6mtv0", {0x21, 0xd1, 0x7d, 0x3f, 0x21, 0xc1, 0x88, 0x99, 0x71, 0x45, 0x96, 0xad, 0xcc, 0x96, 0x79, 0xd8})
TEST_DECODE_LENGTH(example_1,    "0",      1, 1)
TEST_DECODE_LENGTH(example_31,   "g",     15, 1)
TEST_DECODE_LENGTH(example_32,   "j0",    16, 2)
TEST_DECODE_LENGTH(example_2000, "rx0", 2000, 3)
TEST_ENCODE_DECODE_WITH_LENGTH(example_w_length, "j0589rugt2s75akwb6kuqwt6mtv0", {0x21, 0xd1, 0x7d, 0x3f, 0x21, 0xc1, 0x88, 0x99, 0x71, 0x45, 0x96, 0xad, 0xcc, 0x96, 0x79, 0xd8})


// README Examples:

static void my_receive_decoded_data_function(std::vector<unsigned char>& data)
{
    (void)data;
}
static void my_receive_encoded_data_function(std::string& data)
{
    (void)data;
}

TEST(Example, decoding)
{
    std::string my_source_data = "85a96sd288dsqfbddffha40";

    int64_t decoded_length = safe32_get_decoded_length(my_source_data.size());
    std::vector<unsigned char> decode_buffer(decoded_length);

    int64_t used_bytes = safe32_decode(my_source_data.data(),
                                       my_source_data.size(),
                                       decode_buffer.data(),
                                       decode_buffer.size());
    if(used_bytes < 0)
    {
        // TODO: used_bytes is an error code.
    }
    std::vector<unsigned char> decoded_data(decode_buffer.begin(), decode_buffer.begin() + used_bytes);
    my_receive_decoded_data_function(decoded_data);
}

TEST(Example, decoding_with_length)
{
    std::string my_source_data = "f85a96sd288dsqfbddffha40";

    int64_t decoded_length = safe32_get_decoded_length(my_source_data.size());
    std::vector<unsigned char> decode_buffer(decoded_length);

    int64_t used_bytes = safe32l_decode(my_source_data.data(),
                                        my_source_data.size(),
                                        decode_buffer.data(),
                                        decode_buffer.size());
    if(used_bytes < 0)
    {
        // TODO: used_bytes is an error code.
    }
    std::vector<unsigned char> decoded_data(decode_buffer.begin(), decode_buffer.begin() + used_bytes);
    my_receive_decoded_data_function(decoded_data);
}

TEST(Example, encoding)
{
    std::vector<unsigned char> my_source_data({0x39, 0x12, 0x82, 0xe1, 0x81, 0x39, 0xd9, 0x8b, 0x39, 0x4c, 0x63, 0x9d, 0x04, 0x8c});

    bool should_include_length = false;
    int64_t encoded_length = safe32_get_encoded_length(my_source_data.size(), should_include_length);
    std::vector<char> encode_buffer(encoded_length);

    int64_t used_bytes = safe32_encode(my_source_data.data(),
                                       my_source_data.size(),
                                       encode_buffer.data(),
                                       encode_buffer.size());
    if(used_bytes < 0)
    {
        // TODO: used_bytes is an error code.
    }
    std::string encoded_data(encode_buffer.begin(), encode_buffer.begin() + used_bytes);
    my_receive_encoded_data_function(encoded_data);
}

TEST(Example, encoding_with_length)
{
    std::vector<unsigned char> my_source_data({0x39, 0x12, 0x82, 0xe1, 0x81, 0x39, 0xd9, 0x8b, 0x39, 0x4c, 0x63, 0x9d, 0x04, 0x8c});

    bool should_include_length = true;
    int64_t encoded_length = safe32_get_encoded_length(my_source_data.size(), should_include_length);
    std::vector<char> encode_buffer(encoded_length);

    int64_t used_bytes = safe32l_encode(my_source_data.data(),
                                        my_source_data.size(),
                                        encode_buffer.data(),
                                        encode_buffer.size());
    if(used_bytes < 0)
    {
        // TODO: used_bytes is an error code.
    }
    std::string encoded_data(encode_buffer.begin(), encode_buffer.begin() + used_bytes);
    my_receive_encoded_data_function(encoded_data);
}