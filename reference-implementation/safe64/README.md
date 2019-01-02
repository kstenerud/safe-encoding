Reference Implementation for Concise Binary Encoding
====================================================

A C implementation to demonstrate a simple SAFE64 codec.


Assumptions
-----------

 * Assumes densely packed decimal encoding for C decimal types (_Decimal32, _Decimal64, _Decimal128). This is the default for gcc and other compilers using decNumber.
 * Assumes a little endian host.



Requirements
------------

  * CMake 3.5 or higher
  * A C compiler
  * A C++ compiler (for the unit tests)



Dependencies
------------

 * decimal/decimal (if using C++): For C++ decimal float types
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

### Decoding

```c
    char decode_process_backing_store[safe64_decode_process_size()];
    struct safe64_decode_process* decode_process = (struct safe64_decode_process*)decode_process_backing_store;
    const safe64_decode_callbacks callbacks =
    {
        // TODO: Fill in callback pointers
    };
    void* context = my_get_context_data();
    unsigned char* decode_buffer;
    int64_t bytes_received;
    safe64_decode_status status = SAFE64_DECODE_STATUS_OK;

    status = safe64_decode_begin(decode_process, &callbacks, context);
    if(status != SAFE64_DECODE_STATUS_OK)
    {
        // TODO: Do something about it
    }

    while(my_has_more_data())
    {
        my_get_next_packet(&decode_buffer, &bytes_received);
        status = safe64_decode_feed(decode_process, decode_buffer, bytes_received);
        int64_t bytes_consumed = safe64_decode_get_buffer_offset(decode_process);
        // Todo: Move uncomsumed bytes to the beginning of the buffer for next time around
    }

    status = safe64_decode_end(decode_process);
```


### Encoding

```c
    char encode_process_backing_store[safe64_encode_process_size()];
    struct safe64_encode_process* encode_process = (struct safe64_encode_process*)encode_process_backing_store;
    unsigned char* document_buffer = my_get_document_pointer();
    int64_t document_buffer_size = my_get_document_byte_count();
    safe64_encode_status status = SAFE64_ENCODE_STATUS_OK;

    status = safe64_encode_begin(encode_process, document_buffer, document_buffer_size);
    if(status != SAFE64_ENCODE_STATUS_OK)
    {
        // TODO: Do something about it
    }
    status = safe64_encode_begin_list(encode_process);
    status = safe64_encode_add_int_8(encode_process, 1);
    status = safe64_encode_add_string(encode_process, "Testing");
    status = safe64_encode_end_container(encode_process);
    status = safe64_encode_end(encode_process);
```



Running Tests
-------------

    ./test/safe64_test
