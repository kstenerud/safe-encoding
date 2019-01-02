#include <gtest/gtest.h>

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

// ==================================================================


#include <safe64/safe64.h>

TEST(Library, version)
{
    const char* expected = "1.0.0";
    const char* actual = safe64_version();
    ASSERT_STREQ(expected, actual);
}

#define TEST_ENCODE(SECTION, NAME, ENCODED, ...) \
TEST(SECTION, NAME) \
{ \
	std::vector<unsigned char> expected_decoded = __VA_ARGS__; \
	std::string expected_encoded = ENCODED; \
	\
	int64_t expected_encoded_length = expected_encoded.size(); \
	int64_t actual_encoded_length = safe64_get_encoded_length(expected_decoded.size()); \
	ASSERT_EQ(expected_encoded_length, actual_encoded_length); \
	\
	std::vector<char> encode_buffer(expected_encoded_length); \
	int64_t expected_encode_used_bytes = expected_decoded.size(); \
	int64_t actual_encode_used_bytes = safe64_encode(expected_decoded.data(), \
		                                             expected_decoded.size(), \
		                                             encode_buffer.data(), \
		                                             encode_buffer.size()); \
	std::string actual_encoded(encode_buffer.begin(), encode_buffer.end()); \
	ASSERT_EQ(expected_encode_used_bytes, actual_encode_used_bytes); \
	ASSERT_EQ(expected_encoded, actual_encoded); \
}

#define TEST_DECODE(SECTION, NAME, ENCODED, ...) \
TEST(SECTION, NAME) \
{ \
	std::vector<unsigned char> expected_decoded = __VA_ARGS__; \
	std::string expected_encoded = ENCODED; \
	\
	int64_t expected_decoded_length = expected_decoded.size(); \
	int64_t actual_decoded_length = safe64_get_decoded_length(expected_encoded.size()); \
	ASSERT_EQ(expected_decoded_length, actual_decoded_length); \
	\
	std::vector<unsigned char> actual_decoded(expected_decoded_length); \
	int64_t expected_decode_used_bytes = expected_encoded.size(); \
	int64_t actual_decode_used_bytes = safe64_decode(expected_encoded.data(), \
		                                             expected_encoded.size(), \
		                                             actual_decoded.data(), \
		                                             actual_decoded.size()); \
	ASSERT_EQ(expected_decode_used_bytes, actual_decode_used_bytes); \
	ASSERT_EQ(expected_decoded, actual_decoded); \
}

#define TEST_ENCODE_DECODE(SECTION, NAME, ENCODED, ...) \
TEST(SECTION, NAME) \
{ \
	std::vector<unsigned char> expected_decoded = __VA_ARGS__; \
	std::string expected_encoded = ENCODED; \
	\
	int64_t expected_encoded_length = expected_encoded.size(); \
	int64_t actual_encoded_length = safe64_get_encoded_length(expected_decoded.size()); \
	ASSERT_EQ(expected_encoded_length, actual_encoded_length); \
	\
	std::vector<char> encode_buffer(expected_encoded_length); \
	int64_t expected_encode_used_bytes = expected_decoded.size(); \
	int64_t actual_encode_used_bytes = safe64_encode(expected_decoded.data(), \
		                                             expected_decoded.size(), \
		                                             encode_buffer.data(), \
		                                             encode_buffer.size()); \
	std::string actual_encoded(encode_buffer.begin(), encode_buffer.end()); \
	ASSERT_EQ(actual_encode_used_bytes, expected_encode_used_bytes); \
	ASSERT_EQ(expected_encoded, actual_encoded); \
	\
	int64_t expected_decoded_length = expected_decoded.size(); \
	int64_t actual_decoded_length = safe64_get_decoded_length(expected_encoded.size()); \
	ASSERT_EQ(expected_decoded_length, actual_decoded_length); \
	\
	std::vector<unsigned char> actual_decoded(expected_decoded_length); \
	int64_t expected_decode_used_bytes = expected_encoded.size(); \
	int64_t actual_decode_used_bytes = safe64_decode(expected_encoded.data(), \
		                                             expected_encoded.size(), \
		                                             actual_decoded.data(), \
		                                             actual_decoded.size()); \
	ASSERT_EQ(expected_decode_used_bytes, actual_decode_used_bytes); \
	ASSERT_EQ(expected_decoded, actual_decoded); \
}

#define TEST_DECODE_ERROR(SECTION, NAME, ERROR_CODE, DECODE_BUFFER_SIZE, ENCODED) \
TEST(SECTION, NAME) \
{ \
	std::string encoded = ENCODED; \
	std::vector<unsigned char> decode_buffer(DECODE_BUFFER_SIZE); \
	int64_t expected_status_code = ERROR_CODE; \
	int64_t actual_status_code = safe64_decode(encoded.data(), \
		                                       encoded.size(), \
		                                       decode_buffer.data(), \
		                                       decode_buffer.size()); \
	ASSERT_EQ(expected_status_code, actual_status_code); \
}



TEST_ENCODE_DECODE(Encode, _1_byte,  "wF",      {0xf1})
TEST_ENCODE_DECODE(Encode, _2_bytes, "AdZ",     {0x2e, 0x99})
TEST_ENCODE_DECODE(Encode, _3_bytes, "wYGL",    {0xf2, 0x34, 0x56})
TEST_ENCODE_DECODE(Encode, _4_bytes, "HcXwoF",  {0x4a, 0x88, 0xbc, 0xd1})
TEST_ENCODE_DECODE(Encode, _5_bytes, "zr6SDd7", {0xff, 0x71, 0xdd, 0x3a, 0x92})

TEST_DECODE_ERROR(DecodeError, src_data_missing, SAFE64_ERROR_SOURCE_DATA_MISSING, 100, "w")

TEST_DECODE_ERROR(DecodeError, dst_buffer_too_short_4, SAFE64_ERROR_NOT_ENOUGH_ROOM, 4, "zr6SDd7")
TEST_DECODE_ERROR(DecodeError, dst_buffer_too_short_3, SAFE64_ERROR_NOT_ENOUGH_ROOM, 3, "zr6SDd7")
TEST_DECODE_ERROR(DecodeError, dst_buffer_too_short_2, SAFE64_ERROR_NOT_ENOUGH_ROOM, 2, "zr6SDd7")
TEST_DECODE_ERROR(DecodeError, dst_buffer_too_short_1, SAFE64_ERROR_NOT_ENOUGH_ROOM, 1, "zr6SDd7")

TEST_DECODE_ERROR(Encode, invalid_0, SAFE64_ERROR_INVALID_SOURCE_DATA, 100, ".r6SDd7")
TEST_DECODE_ERROR(Encode, invalid_1, SAFE64_ERROR_INVALID_SOURCE_DATA, 100, "z.6SDd7")
TEST_DECODE_ERROR(Encode, invalid_2, SAFE64_ERROR_INVALID_SOURCE_DATA, 100, "zr.SDd7")
TEST_DECODE_ERROR(Encode, invalid_3, SAFE64_ERROR_INVALID_SOURCE_DATA, 100, "zr6.Dd7")
TEST_DECODE_ERROR(Encode, invalid_4, SAFE64_ERROR_INVALID_SOURCE_DATA, 100, "zr6S.d7")
TEST_DECODE_ERROR(Encode, invalid_5, SAFE64_ERROR_INVALID_SOURCE_DATA, 100, "zr6SD.7")
TEST_DECODE_ERROR(Encode, invalid_6, SAFE64_ERROR_INVALID_SOURCE_DATA, 100, "zr6SDd.")
