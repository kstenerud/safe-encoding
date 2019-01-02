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
	ASSERT_EQ(actual_encode_used_bytes, expected_encode_used_bytes); \
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
	ASSERT_EQ(actual_decode_used_bytes, expected_decode_used_bytes); \
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
	ASSERT_EQ(actual_decode_used_bytes, expected_decode_used_bytes); \
	ASSERT_EQ(expected_decoded, actual_decoded); \
}



TEST_ENCODE_DECODE(Safe64Encode, _1_byte,  "wF",      {0xf1})
TEST_ENCODE_DECODE(Safe64Encode, _2_bytes, "AdZ",     {0x2e, 0x99})
TEST_ENCODE_DECODE(Safe64Encode, _3_bytes, "wYGL",    {0xf2, 0x34, 0x56})
TEST_ENCODE_DECODE(Safe64Encode, _4_bytes, "HcXwoF",  {0x4a, 0x88, 0xbc, 0xd1})
TEST_ENCODE_DECODE(Safe64Encode, _5_bytes, "zr6SDd7", {0xff, 0x71, 0xdd, 0x3a, 0x92})

TEST_DECODE(Safe64Decode, _1_byte,  "wF",      {0xf1})
TEST_DECODE(Safe64Decode, _2_bytes, "AdZ",     {0x2e, 0x99})
TEST_DECODE(Safe64Decode, _3_bytes, "wYGL",    {0xf2, 0x34, 0x56})
TEST_DECODE(Safe64Decode, _4_bytes, "HcXwoF",  {0x4a, 0x88, 0xbc, 0xd1})
TEST_DECODE(Safe64Decode, _5_bytes, "zr6SDd7", {0xff, 0x71, 0xdd, 0x3a, 0x92})
