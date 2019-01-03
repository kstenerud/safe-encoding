#include <gtest/gtest.h>

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

// ==================================================================


#include <safe64/safe64.h>

// #define KSLogger_LocalLevel DEBUG
#include "kslogger.h"

// ----------
// Assertions
// ----------

void assert_encode_decode(std::string expected_encoded, std::vector<unsigned char> expected_decoded)
{
    int64_t expected_encoded_length = expected_encoded.size();
    int64_t actual_encoded_length = safe64_get_encoded_length(expected_decoded.size());
    ASSERT_EQ(expected_encoded_length, actual_encoded_length);

    std::vector<char> encode_buffer(1000);
    int64_t expected_encode_used_bytes = expected_encoded.size();
    int64_t actual_encode_used_bytes = safe64_encode(expected_decoded.data(),
                                                     expected_decoded.size(),
                                                     encode_buffer.data(),
                                                     encode_buffer.size());
    std::string actual_encoded(encode_buffer.begin(), encode_buffer.begin() + actual_encode_used_bytes);
    ASSERT_EQ(actual_encode_used_bytes, expected_encode_used_bytes);
    ASSERT_EQ(expected_encoded, actual_encoded);

    int64_t expected_decoded_length = expected_decoded.size();
    int64_t actual_decoded_length = safe64_get_decoded_length(expected_encoded.size());
    ASSERT_GE(expected_decoded_length, actual_decoded_length);

    std::vector<unsigned char> decode_buffer(1000);
    int64_t expected_decode_used_bytes = expected_decoded.size();
    int64_t actual_decode_used_bytes = safe64_decode(expected_encoded.data(),
                                                     expected_encoded.size(),
                                                     decode_buffer.data(),
                                                     decode_buffer.size());
    std::vector<unsigned char> actual_decoded(decode_buffer.begin(), decode_buffer.begin() + actual_decode_used_bytes);
    ASSERT_EQ(expected_decode_used_bytes, actual_decode_used_bytes);
    ASSERT_EQ(expected_decoded, actual_decoded);
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

    for(int packet_size=length-1; packet_size >= 3; packet_size--)
    {
        KSLOG_DEBUG("packet size %d", packet_size);
        safe64_status_code status = SAFE64_STATUS_OK;
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
            const unsigned char* encode_end = e_src + packet_size;
            if(encode_end >= e_src_end)
            {
                encode_end = e_src_end;
                is_end = true;
            }
            status = safe64_encode_feed(&e_src,
                                        encode_end,
                                        &e_dst,
                                        e_dst_end,
                                        is_end);
            ASSERT_EQ(SAFE64_STATUS_OK, status);
            status = safe64_decode_feed(&d_src,
                                        e_dst,
                                        &d_dst,
                                        d_dst_end,
                                        is_end);
            ASSERT_EQ(SAFE64_STATUS_OK, status);
        }
        ASSERT_EQ(data, decode_buffer);
    }
}

void assert_chunked_encode_dst_packeted(int length)
{
    std::vector<unsigned char> data = make_bytes(length, length);
    std::vector<char> encode_buffer(length * 2);
    std::vector<unsigned char> decode_buffer(length);

    for(int packet_size=length-1; packet_size >= 4; packet_size--)
    {
        KSLOG_DEBUG("packet size %d", packet_size);
        safe64_status_code status = SAFE64_STATUS_OK;
        const unsigned char* e_src = data.data();
        const unsigned char* e_src_end = data.data() + data.size();
        char* e_dst = encode_buffer.data();
        const char* d_src = encode_buffer.data();
        unsigned char* d_dst = decode_buffer.data();
        unsigned char* d_dst_end = decode_buffer.data() + decode_buffer.size();

        while(e_src < e_src_end)
        {
            bool is_end = e_src + packet_size >= e_src_end;
            status = safe64_encode_feed(&e_src,
                                        e_src_end,
                                        &e_dst,
                                        e_dst + packet_size,
                                        is_end);
            ASSERT_TRUE(status == SAFE64_STATUS_OK || status == SAFE64_STATUS_DESTINATION_BUFFER_FULL);
            status = safe64_decode_feed(&d_src,
                                        e_dst,
                                        &d_dst,
                                        d_dst_end,
                                        is_end);
            ASSERT_EQ(SAFE64_STATUS_OK, status);
        }
        ASSERT_EQ(data, decode_buffer);
    }
}

void assert_chunked_decode_dst_packeted(int length)
{
    std::vector<unsigned char> data = make_bytes(length, length);
    std::vector<char> encode_buffer(length * 2);
    std::vector<unsigned char> decode_buffer(length);
    int64_t encoded_length = safe64_encode(data.data(), data.size(), encode_buffer.data(), encode_buffer.size());
    ASSERT_GT(encoded_length, 0);
    const char* encoded_end = encode_buffer.data() + encoded_length;


    for(int packet_size=length-1; packet_size >= 4; packet_size--)
    {
        KSLOG_DEBUG("packet size %d", packet_size);
        safe64_status_code status = SAFE64_STATUS_OK;
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
            status = safe64_decode_feed(&d_src,
                                        d_src_end,
                                        &d_dst,
                                        d_dst_end,
                                        is_end);
            ASSERT_TRUE(status == SAFE64_STATUS_OK || status == SAFE64_STATUS_DESTINATION_BUFFER_FULL);
        }
        ASSERT_EQ(data, decode_buffer);
    }
}

void assert_decode(std::string expected_encoded, std::vector<unsigned char> expected_decoded)
{
    int64_t decoded_length = safe64_get_decoded_length(expected_encoded.size());
    std::vector<unsigned char> decode_buffer(decoded_length);
    int64_t expected_decode_used_bytes = expected_encoded.size();
    int64_t actual_decode_used_bytes = safe64_decode(expected_encoded.data(),
                                                     expected_encoded.size(),
                                                     decode_buffer.data(),
                                                     decode_buffer.size());
    std::vector<unsigned char> actual_decoded(decode_buffer.begin(), decode_buffer.begin() + actual_decode_used_bytes);
    ASSERT_GE(expected_decode_used_bytes, actual_decode_used_bytes);
    ASSERT_EQ(expected_decoded, actual_decoded);
}

void assert_decode_status(int buffer_size, std::string encoded, int expected_status_code)
{
    std::vector<unsigned char> decode_buffer(buffer_size);
    int64_t actual_status_code = safe64_decode(encoded.data(),
                                               encoded.size(),
                                               decode_buffer.data(),
                                               decode_buffer.size());
    ASSERT_EQ(expected_status_code, actual_status_code);
}



// --------------------
// Common Test Patterns
// --------------------

#define TEST_DECODE(NAME, ENCODED, ...) \
TEST(Decode, NAME) { assert_decode(ENCODED, __VA_ARGS__); }

#define TEST_ENCODE_DECODE(NAME, ENCODED, ...) \
TEST(EncodeDecode, NAME) { assert_encode_decode(ENCODED, __VA_ARGS__); }

#define TEST_DECODE_ERROR(NAME, BUFFER_SIZE, ENCODED, EXPECTED_STATUS_CODE) \
TEST(DecodeError, NAME) { assert_decode_status(BUFFER_SIZE, ENCODED, EXPECTED_STATUS_CODE); }



// -----
// Tests
// -----

TEST(Library, version)
{
    const char* expected = "1.0.0";
    const char* actual = safe64_version();
    ASSERT_STREQ(expected, actual);
}

TEST_ENCODE_DECODE(_1_byte,  "wF",      {0xf1})
TEST_ENCODE_DECODE(_2_bytes, "AdZ",     {0x2e, 0x99})
TEST_ENCODE_DECODE(_3_bytes, "wYGL",    {0xf2, 0x34, 0x56})
TEST_ENCODE_DECODE(_4_bytes, "HcXwoF",  {0x4a, 0x88, 0xbc, 0xd1})
TEST_ENCODE_DECODE(_5_bytes, "zr6SDd7", {0xff, 0x71, 0xdd, 0x3a, 0x92})

TEST_DECODE_ERROR(dst_buffer_too_short_4, 4, "zr6SDd7", SAFE64_STATUS_DESTINATION_BUFFER_FULL)
TEST_DECODE_ERROR(dst_buffer_too_short_3, 3, "zr6SDd7", SAFE64_STATUS_DESTINATION_BUFFER_FULL)
TEST_DECODE_ERROR(dst_buffer_too_short_2, 2, "zr6SDd7", SAFE64_STATUS_DESTINATION_BUFFER_FULL)
TEST_DECODE_ERROR(dst_buffer_too_short_1, 1, "zr6SDd7", SAFE64_STATUS_DESTINATION_BUFFER_FULL)

TEST_DECODE_ERROR(invalid_0, 100, ".r6SDd7", SAFE64_ERROR_INVALID_SOURCE_DATA)
TEST_DECODE_ERROR(invalid_1, 100, "z.6SDd7", SAFE64_ERROR_INVALID_SOURCE_DATA)
TEST_DECODE_ERROR(invalid_2, 100, "zr.SDd7", SAFE64_ERROR_INVALID_SOURCE_DATA)
TEST_DECODE_ERROR(invalid_3, 100, "zr6.Dd7", SAFE64_ERROR_INVALID_SOURCE_DATA)
TEST_DECODE_ERROR(invalid_4, 100, "zr6S.d7", SAFE64_ERROR_INVALID_SOURCE_DATA)
TEST_DECODE_ERROR(invalid_5, 100, "zr6SD.7", SAFE64_ERROR_INVALID_SOURCE_DATA)
TEST_DECODE_ERROR(invalid_6, 100, "zr6SDd.", SAFE64_ERROR_INVALID_SOURCE_DATA)

TEST_DECODE(space_0, " zr6SDd7", {0xff, 0x71, 0xdd, 0x3a, 0x92})
TEST_DECODE(space_1, "z r6SDd7", {0xff, 0x71, 0xdd, 0x3a, 0x92})
TEST_DECODE(space_2, "zr 6SDd7", {0xff, 0x71, 0xdd, 0x3a, 0x92})
TEST_DECODE(space_3, "zr6 SDd7", {0xff, 0x71, 0xdd, 0x3a, 0x92})
TEST_DECODE(space_4, "zr6S Dd7", {0xff, 0x71, 0xdd, 0x3a, 0x92})
TEST_DECODE(space_5, "zr6SD d7", {0xff, 0x71, 0xdd, 0x3a, 0x92})
TEST_DECODE(space_6, "zr6SDd 7", {0xff, 0x71, 0xdd, 0x3a, 0x92})
TEST_DECODE(space_7, "zr6SDd7 ", {0xff, 0x71, 0xdd, 0x3a, 0x92})

TEST_DECODE(lots_of_whitespace, "z\t\tr\r\n\n 6   SD\t \t\td7", {0xff, 0x71, 0xdd, 0x3a, 0x92})

TEST_ENCODE_DECODE(example_1, "DG91sN3tqNgtI5DS07k", {0x39, 0x12, 0x82, 0xe1, 0x81, 0x39, 0xd9, 0x8b, 0x39, 0x4c, 0x63, 0x9d, 0x04, 0x8c})
TEST_ENCODE_DECODE(example_2, "tW9abzVsQMg0aItgJrhV", {0xe6, 0x12, 0xa6, 0x9f, 0xf8, 0x38, 0x6d, 0x7b, 0x01, 0x99, 0x3e, 0x6c, 0x53, 0x7b, 0x60})
TEST_ENCODE_DECODE(example_3, "7S4xEm60X8_lGOPhn8Otq-", {0x21, 0xd1, 0x7d, 0x3f, 0x21, 0xc1, 0x88, 0x99, 0x71, 0x45, 0x96, 0xad, 0xcc, 0x96, 0x79, 0xd8})

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
