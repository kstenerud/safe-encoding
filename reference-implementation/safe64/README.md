Reference Implementation for the Safe64 Encoding System
=======================================================

A C implementation to demonstrate a simple SAFE64 codec.


Requirements
------------

  * CMake 3.5 or higher
  * A C compiler
  * A C++ compiler (for the unit tests)



Dependencies
------------

 * stdbool.h: For bool type
 * stdint.h: Fot int types



Building
--------

    mkdir build
    cd build
    cmake ..
    make


Usage
-----

Note: Using C++ to make the string & data code simpler.

### Decoding

```c++
    std::string my_source_data = "DG91sN3tqNgtI5DS07k";

    int64_t decoded_length = safe64_get_decoded_length(my_source_data.size());
    std::vector<unsigned char> decode_buffer(decoded_length);

    int64_t used_bytes = safe64_decode(my_source_data.data(),
                                       my_source_data.size(),
                                       decode_buffer.data(),
                                       decode_buffer.size());
    if(used_bytes < 0)
    {
        // TODO: used_bytes is an error code.
    }
    std::vector<unsigned char> decoded_data(decode_buffer.begin(), decode_buffer.begin() + used_bytes);
    my_receive_decoded_data_function(decoded_data);
```

### Decoding (with length field)

```c++
    std::string my_source_data = "DDG91sN3tqNgtI5DS07k";

    int64_t decoded_length = safe64_get_decoded_length(my_source_data.size());
    std::vector<unsigned char> decode_buffer(decoded_length);

    int64_t used_bytes = safe64l_decode(my_source_data.data(),
                                        my_source_data.size(),
                                        decode_buffer.data(),
                                        decode_buffer.size());
    if(used_bytes < 0)
    {
        // TODO: used_bytes is an error code.
    }
    std::vector<unsigned char> decoded_data(decode_buffer.begin(), decode_buffer.begin() + used_bytes);
    my_receive_decoded_data_function(decoded_data);
```

### Encoding

```C++
    std::vector<unsigned char> my_source_data({0x39, 0x12, 0x82, 0xe1, 0x81, 0x39, 0xd9, 0x8b, 0x39, 0x4c, 0x63, 0x9d, 0x04, 0x8c});

    bool should_include_length = false;
    int64_t encoded_length = safe64_get_encoded_length(my_source_data.size(), should_include_length);
    std::vector<char> encode_buffer(encoded_length);

    int64_t used_bytes = safe64_encode(my_source_data.data(),
                                       my_source_data.size(),
                                       encode_buffer.data(),
                                       encode_buffer.size());
    if(used_bytes < 0)
    {
        // TODO: used_bytes is an error code.
    }
    std::string encoded_data(encode_buffer.begin(), encode_buffer.begin() + used_bytes);
    my_receive_encoded_data_function(encoded_data);
```

### Encoding (with length field)

```c++
    std::vector<unsigned char> my_source_data({0x39, 0x12, 0x82, 0xe1, 0x81, 0x39, 0xd9, 0x8b, 0x39, 0x4c, 0x63, 0x9d, 0x04, 0x8c});

    bool should_include_length = true;
    int64_t encoded_length = safe64_get_encoded_length(my_source_data.size(), should_include_length);
    std::vector<char> encode_buffer(encoded_length);

    int64_t used_bytes = safe64l_encode(my_source_data.data(),
                                        my_source_data.size(),
                                        encode_buffer.data(),
                                        encode_buffer.size());
    if(used_bytes < 0)
    {
        // TODO: used_bytes is an error code.
    }
    std::string encoded_data(encode_buffer.begin(), encode_buffer.begin() + used_bytes);
    my_receive_encoded_data_function(encoded_data);
```



Running Tests
-------------

    ./test/safe64_test
