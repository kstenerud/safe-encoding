#include <safe16/safe16.h>
#include <safe32/safe32.h>
#include <safe64/safe64.h>
#include <safe80/safe80.h>
#include <safe85/safe85.h>

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

static FILE* open_file_read(const char* const filename)
{
    if(strcmp(filename, "-") == 0)
    {
        return stdin;
    }
    FILE* const file = fopen(filename, "rb");
    if(file == NULL)
    {
        perror_exit("Could not open %s for reading", filename);
    }
    return file;
}

static FILE* open_file_write(const char* const filename)
{
    if(strcmp(filename, "-") == 0)
    {
        return stdout;
    }
    FILE* const file = fopen(filename, "wb");
    if(file == NULL)
    {
        perror_exit("Could not open %s for writing", filename);
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

static void write_char_to_file(FILE* const file,
                          const char data)
{
    if(fwrite(&data, 1, 1, file) != 1)
    {
        perror_exit("Could not write 1 byte to file");
    }
}

static void error_unexpected_status_exit(const safe16_status status)
{
    const char* name = "Unknown";
    switch(status)
    {
        #define HANDLE_CASE(CASE) \
            case SAFE16_##CASE: \
                name = #CASE; \
                break

        HANDLE_CASE(STATUS_PARTIALLY_COMPLETE);
        HANDLE_CASE(ERROR_INVALID_SOURCE_DATA);
        HANDLE_CASE(ERROR_UNTERMINATED_LENGTH_FIELD);
        HANDLE_CASE(ERROR_TRUNCATED_DATA);
        HANDLE_CASE(ERROR_INVALID_LENGTH);
        HANDLE_CASE(ERROR_NOT_ENOUGH_ROOM);

        // This should not happen
        HANDLE_CASE(STATUS_OK);
    }
    print_error("Error: Unexpected status %d (%s)\n", status, name);
    exit(1);
}

// -------------------
// Encoding & Decoding
// -------------------

typedef enum
{
    ENCODE,
    DECODE
} DIRECTION;

typedef int64_t (*get_decoded_length_func)(int64_t encoded_length);
typedef int64_t (*decode_func)(const uint8_t* src_buffer,
                               int64_t src_buffer_length,
                               uint8_t* dst_buffer,
                               int64_t dst_buffer_length);
typedef int64_t (*decodel_func)(const uint8_t* src_buffer,
                                int64_t src_length,
                                uint8_t* dst_buffer,
                                int64_t dst_length);
typedef int64_t (*read_length_field_func)(const uint8_t* buffer,
                                          int64_t buffer_length,
                                          int64_t* length);
typedef safe16_status (*decode_feed_func)(const uint8_t** src_buffer_ptr,
                                          int64_t src_length,
                                          uint8_t** dst_buffer_ptr,
                                          int64_t dst_length,
                                          safe16_stream_state stream_state);
typedef int64_t (*get_encoded_length_func)(int64_t decoded_length,
                                           bool include_length_field);
typedef int64_t (*write_length_field_func)(const int64_t length,
                                           uint8_t* const dst_buffer,
                                           const int64_t dst_buffer_length);
typedef int64_t (*encode_func)(const uint8_t* const src_buffer,
                               const int64_t src_length,
                               uint8_t* const dst_buffer,
                               const int64_t dst_length);
typedef int64_t (*encodel_func)(const uint8_t* const src_buffer,
                                const int64_t src_length,
                                uint8_t* const dst_buffer,
                                const int64_t dst_length);
typedef safe16_status (*encode_feed_func)(const uint8_t** src_buffer_ptr,
                                          int64_t src_length,
                                          uint8_t** dst_buffer_ptr,
                                          int64_t dst_length,
                                          bool is_end_of_data);

typedef struct
{
    char* in_path;
    char* out_path;
    DIRECTION direction;
    bool use_length_fields;
    int line_break_at;
    int indent_count;
    char separator;
    int separator_at;
    bool input_hex;
    get_decoded_length_func get_decoded_length;
    decode_func decode;
    decodel_func decode_l;
    decode_feed_func decode_feed;
    read_length_field_func read_length_field;
    get_encoded_length_func get_encoded_length;
    write_length_field_func write_length_field;
    encode_func encode;
    encodel_func encode_l;
    encode_feed_func encode_feed;
} config;

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

static void insert_separator(FILE* const file, char separator)
{
    write_char_to_file(file, separator);
}

static void insert_line_break(FILE* const file)
{
    insert_separator(file, '\n');
}

static int64_t output_encoded(FILE* const file,
                              const char* const data,
                              const int length,
                              const int64_t current_offset,
                              const config* const config)
{
    const int64_t end_offset = current_offset + length;
    const char* const data_end = data + length;
    if(config->line_break_at <= 0 && config->separator_at <= 0)
    {
        write_to_file(file, data, length);
        return end_offset;
    }

    int64_t chars_until_line_break = config->line_break_at - (current_offset % config->line_break_at);
    int64_t chars_until_separator = config->separator_at - (current_offset % config->separator_at);
    const char* data_current = data;
    if(config->separator_at > 0)
    {
        while(data_current < data_end)
        {
            int chars_to_write = chars_until_separator;
            if(data_current + chars_to_write > data_end)
            {
                chars_to_write = data_end - data_current;
            }
            write_to_file(file, data_current, chars_to_write);
            data_current += chars_to_write;
            if(chars_to_write == chars_until_separator && data_current < data_end)
            {
                insert_separator(file, config->separator);
            }
            chars_until_separator = config->separator_at;
        }
    }
    else
    {
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
                insert_indentation(file, config->indent_count);
            }
            data_current += chars_to_write;
            chars_until_line_break = config->line_break_at;
        }
    }
    return end_offset;
}

static void encode(FILE* const src_file, FILE* const dst_file, const config* const config)
{
    uint8_t decoded_buffer[BUFFER_SIZE];
    uint8_t encoded_buffer[config->get_encoded_length(sizeof(decoded_buffer), false)];
    int decoded_buffer_offset = 0;
    int64_t current_offset = 0;
    bool is_at_end = false;

    insert_indentation(dst_file, config->indent_count);

    if(config->use_length_fields)
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
        int64_t bytes_to_write = config->write_length_field(file_size, encoded_buffer, sizeof(encoded_buffer));

        current_offset = output_encoded(dst_file,
                                        (char*)encoded_buffer,
                                        bytes_to_write,
                                        current_offset,
                                        config);
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
        const safe16_status status = config->encode_feed(&src,
                                                        bytes_to_process,
                                                        &dst,
                                                        sizeof(encoded_buffer),
                                                        is_at_end);
        if(status != SAFE16_STATUS_OK && status != SAFE16_STATUS_PARTIALLY_COMPLETE)
        {
            error_unexpected_status_exit(status);
        }
        const int bytes_processed = src - decoded_buffer;
        const int bytes_to_write = dst - encoded_buffer;

        current_offset = output_encoded(dst_file,
                                        (char*)encoded_buffer,
                                        bytes_to_write,
                                        current_offset,
                                        config);

        decoded_buffer_offset = bytes_to_process - bytes_processed;
        memmove(decoded_buffer, src, decoded_buffer_offset);
    }

    close_file(src_file);
    close_file(dst_file);
}

static void decode(FILE* const src_file, FILE* const dst_file, const config* const config)
{
    uint8_t decoded_buffer[BUFFER_SIZE];
    uint8_t encoded_buffer[config->get_encoded_length(sizeof(decoded_buffer), false)];
    int encoded_buffer_offset = 0;
    bool is_at_end = false;
    safe16_stream_state stream_state = SAFE16_STREAM_STATE_NONE;
    int64_t expected_bytes_decoded = -1;
    // int64_t total_bytes_decoded = 0;

    if(config->use_length_fields)
    {
        const int bytes_to_read = 100;
        const int bytes_read = read_from_file(src_file,
                                              encoded_buffer,
                                              bytes_to_read,
                                              &is_at_end);
        int64_t bytes_processed = config->read_length_field(encoded_buffer, bytes_read, &expected_bytes_decoded);
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
            stream_state = SAFE16_SRC_IS_AT_END_OF_STREAM;
        }

        const int bytes_to_process = encoded_buffer_offset + bytes_read;
        const uint8_t* src = encoded_buffer;
        uint8_t* dst = decoded_buffer;
        safe16_status status = config->decode_feed(&src,
                                                  bytes_to_process,
                                                  &dst,
                                                  sizeof(decoded_buffer),
                                                  stream_state);
        if(status != SAFE16_STATUS_OK && status != SAFE16_STATUS_PARTIALLY_COMPLETE)
        {
            error_unexpected_status_exit(status);
        }
        const int bytes_processed = src - encoded_buffer;
        const int bytes_to_write = dst - decoded_buffer;

        write_to_file(dst_file, (const char*)decoded_buffer, bytes_to_write);
        // total_bytes_decoded += bytes_to_write;

        encoded_buffer_offset = bytes_to_process - bytes_processed;
        memmove(encoded_buffer, src, encoded_buffer_offset);
    }
}


// ---------------------------
// Startup & command line args
// ---------------------------

static char* g_argv_0;

static void print_usage(void)
{
    print_error("\
Safeenc v%s: Converts binary data into a safe-encoding text format (or vice versa).\n\
Copyright (c) 2024 Karl Stenerud, License: MIT, NO WARRANTIES IMPLIED\n\
\n\
Usage: %s [options]\n\
\n\
Must specify at least -i or -o.\n\
\n\
Options:\n\
  -h: Print help and exit\n\
  -v: Print version and exit\n\
  -i path: Input path to read from (default '-' which is stdin)\n\
  -o path: Output path to write to (default '-' which is stdout)\n\
  -d: Decode instead of encoding\n\
  -l: Encode/decode safe64l (with a length field)\n\
  -n <count>: Insert a newline every <count> encoded characters\n\
  -I <count>: Insert <count> spaces indentation on each line\n\
  -s <count>: Insert a separator char every <count> encoded characters (overrides linebreaks)\n\
  -S <character>: Use this char as the separator character (default space)\n\
  -r <radix>: Encode using this radix (16, 32, 64, 80, 85). Default 16\n\
  -x: Convert input from hex chars to binary before encoding. (causes entire file to be read rather than streaming)\n\
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

void select_codec(config* config, int radix)
{
#define CASE_IMPL(RADIX) \
            config->get_decoded_length = safe##RADIX##_get_decoded_length; \
            config->decode = safe##RADIX##_decode; \
            config->decode_l = safe##RADIX##l_decode; \
            config->read_length_field = safe##RADIX##_read_length_field; \
            config->decode_feed = (decode_feed_func)safe##RADIX##_decode_feed; \
            config->get_encoded_length = safe##RADIX##_get_encoded_length; \
            config->write_length_field = safe##RADIX##_write_length_field; \
            config->encode = safe##RADIX##_encode; \
            config->encode_l = safe##RADIX##l_encode; \
            config->encode_feed = (encode_feed_func)safe##RADIX##_encode_feed; \
            break
#define CASE(RADIX) \
        case RADIX: \
        CASE_IMPL(RADIX)

    switch(radix)
    {
        default:
            printf("%d: Unknown radix. Using 16", radix);
            CASE_IMPL(16);
        CASE(16);
        CASE(32);
        CASE(64);
        CASE(80);
        CASE(85);
    }
}

int main(const int argc, char** const argv)
{
    g_argv_0 = argv[0];
    config conf = {0};
    conf.in_path = "-";
    conf.out_path = "-";
    conf.separator = ' ';
    conf.separator_at = -1;
    conf.line_break_at = -1;
    select_codec(&conf, 16);

    bool selected_path = false;
    int opt;
    while((opt = getopt(argc, argv, "?hvdln:i:o:I:s:S:r:x")) >= 0)
    {
        switch(opt)
        {
            case '?':
            case 'h':
                print_usage();
                exit(0);
            case 'v':
                print_version();
                exit(0);
            case 'i':
                conf.in_path = strdup(optarg);
                selected_path = true;
                break;
            case 'o':
                conf.out_path = strdup(optarg);
                selected_path = true;
                break;
            case 'd':
                conf.direction = DECODE;
                break;
            case 'l':
                conf.use_length_fields = true;
                break;
            case 'n':
                sscanf(optarg, "%d", &conf.line_break_at);
                break;
            case 'I':
                sscanf(optarg, "%d", &conf.indent_count);
                break;
            case 'S':
                sscanf(optarg, "%c", &conf.separator);
                break;
            case 's':
                sscanf(optarg, "%d", &conf.separator_at);
                break;
            case 'r':
            {
                int radix = 0;
                sscanf(optarg, "%d", &radix);
                select_codec(&conf, radix);
                break;
            }
            case 'x':
                conf.input_hex = true;
                break;
            default:
                fprintf(stderr, "Error: Unknown option: %d %c\n", opt, opt);
                print_usage_error_exit();
        }
    }

    if(!selected_path) {
        fprintf(stderr, "Error: Must specify at least -i or -o\n\n");
        print_usage_error_exit();
    }

    FILE* src_file = open_file_read(conf.in_path);
    FILE* dst_file = open_file_write(conf.out_path);

    if (conf.input_hex)
    {
        FILE* out_file = dst_file;

        dst_file = tmpfile();
        config conf2 = {0};
        select_codec(&conf2, 16);
        decode(src_file, dst_file, &conf2);

        rewind(dst_file);
        src_file = dst_file;
        dst_file = out_file;
    }

    if(conf.direction == ENCODE)
    {
        encode(src_file, dst_file, &conf);
    }
    else
    {
        decode(src_file, dst_file, &conf);
    }

    return 0;
}
