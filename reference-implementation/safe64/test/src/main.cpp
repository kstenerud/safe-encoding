#include <gtest/gtest.h>

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

// ==================================================================


#include <safe64/safe64.h>


// ----------
// Assertions
// ----------

void assert_encode_decode(std::string expected_encoded, std::vector<unsigned char> expected_decoded)
{
	int64_t expected_encoded_length = expected_encoded.size();
	int64_t actual_encoded_length = safe64_get_encoded_length(expected_decoded.size());
	ASSERT_EQ(expected_encoded_length, actual_encoded_length);

	std::vector<char> encode_buffer(expected_encoded_length);
	int64_t expected_encode_used_bytes = expected_encoded.size();
	int64_t actual_encode_used_bytes = safe64_encode(expected_decoded.data(),
		                                             expected_decoded.size(),
		                                             encode_buffer.data(),
		                                             encode_buffer.size());
	std::string actual_encoded(encode_buffer.begin(), encode_buffer.end());
	ASSERT_EQ(actual_encode_used_bytes, expected_encode_used_bytes);
	ASSERT_EQ(expected_encoded, actual_encoded);

	int64_t expected_decoded_length = expected_decoded.size();
	int64_t actual_decoded_length = safe64_get_decoded_length(expected_encoded.size());
	ASSERT_GE(expected_decoded_length, actual_decoded_length);

	std::vector<unsigned char> actual_decoded(expected_decoded_length);
	int64_t expected_decode_used_bytes = expected_decoded.size();
	int64_t actual_decode_used_bytes = safe64_decode(expected_encoded.data(),
		                                             expected_encoded.size(),
		                                             actual_decoded.data(),
		                                             actual_decoded.size());
	ASSERT_EQ(expected_decode_used_bytes, actual_decode_used_bytes);
	ASSERT_EQ(expected_decoded, actual_decoded);
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
	ASSERT_GE(expected_decode_used_bytes, actual_decode_used_bytes);
	std::vector<unsigned char> actual_decoded(decode_buffer.begin(), decode_buffer.begin() + actual_decode_used_bytes);
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

TEST_DECODE_ERROR(dst_buffer_too_short_4, 4, "zr6SDd7", SAFE64_ERROR_NOT_ENOUGH_ROOM)
TEST_DECODE_ERROR(dst_buffer_too_short_3, 3, "zr6SDd7", SAFE64_ERROR_NOT_ENOUGH_ROOM)
TEST_DECODE_ERROR(dst_buffer_too_short_2, 2, "zr6SDd7", SAFE64_ERROR_NOT_ENOUGH_ROOM)
TEST_DECODE_ERROR(dst_buffer_too_short_1, 1, "zr6SDd7", SAFE64_ERROR_NOT_ENOUGH_ROOM)

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
