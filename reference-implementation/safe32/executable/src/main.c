#include <safe32/safe32.h>

#include <libgen.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define QUOTE(str) #str
#define EXPAND_AND_QUOTE(str) QUOTE(str)


// 4096 seems optimal. Smaller than this causes a 10% performance penalty.
#define BUFFER_SIZE 4096


// ---------
// Utilities
// ---------

static void print_error_args(const char* const fmt, va_list args)
{
    vfprintf(stderr, fmt, args);
}

static void print_error(const char* const fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    print_error_args(fmt, args);
    va_end(args);
}

static void error_exit(const char* const fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    print_error_args(fmt, args);
    va_end(args);
    exit(1);
}

static void perror_exit(const char* const fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    print_error_args(fmt, args);
    va_end(args);
    fprintf(stderr, ": ");
    perror("");
    exit(1);
}

static FILE* open_file(const char* const filename)
{
    if(strcmp(filename, "-") == 0)
    {
        return stdin;
    }
    FILE* const file = fopen(filename, "rb");
    if(file == NULL)
    {
        perror_exit("Could not open %s", filename);
    }
    return file;
}

static void close_file(FILE* const file)
{
    if(file != stdin && file != stdout && file != stderr && file != NULL)
    {
        if(fclose(file) == EOF)
        {
            perror_exit("Could not close file");
        }
    }
}

static int read_from_file(FILE* const file,
                          uint8_t* const buffer,
                          const int length,
                          bool* const is_eof)
{
    const int bytes_read = fread(buffer, 1, length, file);
    if(ferror(file))
    {
        perror_exit("Could not read %d bytes from file", length);
    }
    *is_eof = feof(file);
    return bytes_read;
}

static void write_to_file(FILE* const file,
                          const char* const data,
                          const int length)
{
    if(fwrite(data, 1, length, file) != (unsigned)length)
    {
        perror_exit("Could not write %d bytes to file", length);
    }
}

static void error_unexpected_status_exit(const safe32_status status)
{
    const char* name = "Unknown";
    switch(status)
    {
        #define HANDLE_CASE(CASE) \
            case CASE: \
                name = #CASE; \
                break

        HANDLE_CASE(SAFE32_STATUS_PARTIALLY_COMPLETE);
        HANDLE_CASE(SAFE32_ERROR_INVALID_SOURCE_DATA);
        HANDLE_CASE(SAFE32_ERROR_UNTERMINATED_LENGTH_FIELD);
        HANDLE_CASE(SAFE32_ERROR_TRUNCATED_DATA);
        HANDLE_CASE(SAFE32_ERROR_INVALID_LENGTH);
        HANDLE_CASE(SAFE32_ERROR_NOT_ENOUGH_ROOM);

        // This should not happen
        HANDLE_CASE(SAFE32_STATUS_OK);
    }
    print_error("Error: Unexpected status %d (%s)\n", status, name);
    exit(1);
}

// -------------------
// Encoding & Decoding
// -------------------

static void insert_indentation(FILE* const file, const int indent_count)
{
    if(indent_count <= 0)
    {
        return;
    }
    char buffer[indent_count];
    memset(buffer, ' ', sizeof(buffer));
    write_to_file(file, buffer, sizeof(buffer));
}

static void insert_line_break(FILE* const file)
{
    write_to_file(file, "\n", 1);
}

static int64_t output_encoded(FILE* const file,
                              const char* const data,
                              const int length,
                              const int64_t current_offset,
                              const int line_break_at,
                              const int indent_count)
{
    const int64_t end_offset = current_offset + length;
    const char* const data_end = data + length;
    if(line_break_at <= 0)
    {
        write_to_file(file, data, length);
        return end_offset;
    }

    int64_t chars_until_line_break = line_break_at - (current_offset % line_break_at);
    const char* data_current = data;
    while(data_current < data_end)
    {
        int chars_to_write = chars_until_line_break;
        if(data_current + chars_to_write > data_end)
        {
            chars_to_write = data_end - data_current;
        }
        write_to_file(file, data_current, chars_to_write);
        if(chars_to_write == chars_until_line_break)
        {
            insert_line_break(file);
            insert_indentation(file, indent_count);
        }
        data_current += chars_to_write;
        chars_until_line_break = line_break_at;
    }
    return end_offset;
}

static void encode(const char* const filename,
                   const bool use_length_fields,
                   const int line_break_at,
                   const int indent_count)
{
    FILE* const src_file = open_file(filename);
    FILE* const dst_file = stdout;

    uint8_t decoded_buffer[BUFFER_SIZE];
    uint8_t encoded_buffer[safe32_get_encoded_length(sizeof(decoded_buffer), false)];
    int decoded_buffer_offset = 0;
    int64_t current_offset = 0;
    bool is_at_end = false;

    insert_indentation(dst_file, indent_count);

    if(use_length_fields)
    {
        if(src_file == stdin)
        {
            error_exit("Cannot determine the length of stdin");
        }
        if(fseek(src_file, 0L, SEEK_END) != 0)
        {
            perror_exit("Error seeking to end of source file");
        }
        long file_size = ftell(src_file);
        if(file_size < 0)
        {
            perror_exit("Error reading source file position");
        }
        if(fseek(src_file, 0L, SEEK_SET) != 0)
        {
            perror_exit("Error seeking to beginning of source file");
        }
        int64_t bytes_to_write = safe32_write_length_field(file_size, encoded_buffer, sizeof(encoded_buffer));

        current_offset = output_encoded(dst_file,
                                        (char*)encoded_buffer,
                                        bytes_to_write,
                                        current_offset,
                                        line_break_at,
                                        indent_count);
    }

    while(!is_at_end)
    {
        const int bytes_to_read = sizeof(decoded_buffer) - decoded_buffer_offset;
        const int bytes_read = read_from_file(src_file,
                                              decoded_buffer + decoded_buffer_offset,
                                              bytes_to_read, &is_at_end);

        const int bytes_to_process = decoded_buffer_offset + bytes_read;
        const uint8_t* src = decoded_buffer;
        uint8_t* dst = encoded_buffer;
        const safe32_status status = safe32_encode_feed(&src,
                                                        bytes_to_process,
                                                        &dst,
                                                        sizeof(encoded_buffer),
                                                        is_at_end);
        if(status != SAFE32_STATUS_OK && status != SAFE32_STATUS_PARTIALLY_COMPLETE)
        {
            error_unexpected_status_exit(status);
        }
        const int bytes_processed = src - decoded_buffer;
        const int bytes_to_write = dst - encoded_buffer;

        current_offset = output_encoded(dst_file,
                                        (char*)encoded_buffer,
                                        bytes_to_write,
                                        current_offset,
                                        line_break_at,
                                        indent_count);

        decoded_buffer_offset = bytes_to_process - bytes_processed;
        memmove(decoded_buffer, src, decoded_buffer_offset);
    }

    close_file(src_file);
    close_file(dst_file);
}

static void decode(const char* const filename, const bool use_length_field)
{
    FILE* const src_file = open_file(filename);
    FILE* const dst_file = stdout;

    uint8_t decoded_buffer[BUFFER_SIZE];
    uint8_t encoded_buffer[safe32_get_encoded_length(sizeof(decoded_buffer), false)];
    int encoded_buffer_offset = 0;
    bool is_at_end = false;
    safe32_stream_state stream_state = SAFE32_STREAM_STATE_NONE;
    int64_t expected_bytes_decoded = -1;

    if(use_length_field)
    {
        const int bytes_to_read = 100;
        const int bytes_read = read_from_file(src_file,
                                              encoded_buffer,
                                              bytes_to_read,
                                              &is_at_end);
        int64_t bytes_processed = safe32_read_length_field(encoded_buffer, bytes_read, &expected_bytes_decoded);
        if(bytes_processed < 0)
        {
            error_unexpected_status_exit(bytes_processed);
        }
        encoded_buffer_offset = bytes_read - bytes_processed;
        memmove(encoded_buffer, encoded_buffer + bytes_processed, encoded_buffer_offset);
    }

    while(!is_at_end)
    {
        const int bytes_to_read = sizeof(encoded_buffer) - encoded_buffer_offset;
        const int bytes_read = read_from_file(src_file,
                                              encoded_buffer + encoded_buffer_offset,
                                              bytes_to_read,
                                              &is_at_end);
        if(is_at_end)
        {
            stream_state = SAFE32_SRC_IS_AT_END_OF_STREAM;
        }

        const int bytes_to_process = encoded_buffer_offset + bytes_read;
        const uint8_t* src = encoded_buffer;
        uint8_t* dst = decoded_buffer;
        safe32_status status = safe32_decode_feed(&src,
                                                  bytes_to_process,
                                                  &dst,
                                                  sizeof(decoded_buffer),
                                                  stream_state);
        if(status != SAFE32_STATUS_OK && status != SAFE32_STATUS_PARTIALLY_COMPLETE)
        {
            error_unexpected_status_exit(status);
        }
        const int bytes_processed = src - encoded_buffer;
        const int bytes_to_write = dst - decoded_buffer;

        write_to_file(dst_file, (const char*)decoded_buffer, bytes_to_write);

        encoded_buffer_offset = bytes_to_process - bytes_processed;
        memmove(encoded_buffer, src, encoded_buffer_offset);
    }

    close_file(src_file);
    close_file(dst_file);
}


// ---------------------------
// Startup & command line args
// ---------------------------

static char* g_argv_0;

static void print_usage(void)
{
    print_error("\
Safe32 v%s: Encodes binary data into a radix-32 text format safe for use in modern text processors.\n\
Copyright (c) 2019 Karl Stenerud, License: MIT, NO WARRANTIES IMPLIED\n\
\n\
Usage: %s [options] [file]\n\
Where the default behavior is to encode from stdin to stdout.\n\
\n\
Options:\n\
  -h: Print help and exit\n\
  -v: Print version and exit\n\
  -d: Decode instead of encoding\n\
  -l: Encode/decode safe64l (with a length field)\n\
  -n <count>: Insert a newline every <count> encoded characters\n\
  -i <count>: Insert <count> spaces indentation on each line\n\
\n\
File: If not specified, - (read from stdin) is assumed.\n\
", EXPAND_AND_QUOTE(PROJECT_VERSION), basename(g_argv_0));
}

static void print_usage_error_exit(void)
{
    print_usage();
    exit(1);
}

static void print_version(void)
{
    printf("%s\n", EXPAND_AND_QUOTE(PROJECT_VERSION));
}

int main(const int argc, char** const argv)
{
    g_argv_0 = argv[0];
    int ch;
    bool is_encoding = true;
    bool use_length_fields = false;
    int line_break_at = 0;
    int indent_count = 0;

    while((ch = getopt(argc, argv, "?hvdln:i:")) >= 0)
    {
        switch(ch)
        {
            case '?':
            case 'h':
                print_usage();
                exit(0);
            case 'v':
                print_version();
                exit(0);
            case 'd':
                is_encoding = false;
                break;
            case 'l':
                use_length_fields = true;
                break;
            case 'n':
                sscanf(optarg, "%d", &line_break_at);
                break;
            case 'i':
                sscanf(optarg, "%d", &indent_count);
                break;
            default:
                printf("Unknown option: %d %c\n", ch, ch);
                print_usage_error_exit();
        }
    }

    const char* const filename = (optind < argc) ? argv[optind] : "-";

    if(is_encoding)
    {
        encode(filename, use_length_fields, line_break_at, indent_count);
    }
    else
    {
        decode(filename, use_length_fields);
    }

    return 0;
}
