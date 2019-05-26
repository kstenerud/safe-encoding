#include <stdint.h>
#include <stdio.h>

// ==================================================================
// ==================================================================

static const int g_alphabet_size_85 = 85;
static const int g_chunks_per_group_85 = 5;

static const uint8_t g_chunk_to_encode_char_85[] =
{
    '!', '$', '(', ')', '*', '+', ',', '-',
    '.', '0', '1', '2', '3', '4', '5', '6',
    '7', '8', '9', ':', ';', '=', '>', '@',
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
    'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
    'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
    'Y', 'Z', '[', ']', '^', '_', '`', 'a',
    'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i',
    'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q',
    'r', 's', 't', 'u', 'v', 'w', 'x', 'y',
    'z', '{', '|', '}', '~',
};

static const uint8_t g_chunk_to_encode_char_85_subst[] = {};

static const uint8_t g_whitespace_85[] =
{
    0x09, 0x0a, 0x0d, 0x20
};

// ==================================================================
// ==================================================================

static const int g_alphabet_size_80 = 80;
static const int g_chunks_per_group_80 = 19;

static const uint8_t g_chunk_to_encode_char_80[] =
{
    '!', '$', '%', '(', ')', '+', ',', '-',
    '0', '1', '2', '3', '4', '5', '6', '7',
    '8', '9', ';', '@', 'A', 'B', 'C', 'D',
    'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L',
    'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T',
    'U', 'V', 'W', 'X', 'Y', 'Z', '[', ']',
    '^', '_', '`', 'a', 'b', 'c', 'd', 'e',
    'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
    'n', 'o', 'p', 'q', 'r', 's', 't', 'u',
    'v', 'w', 'x', 'y', 'z', '{', '}', '~',
};

static const uint8_t g_chunk_to_encode_char_80_subst[] = {};

static const uint8_t g_whitespace_80[] =
{
    0x09, 0x0a, 0x0d, 0x20
};

// ==================================================================
// ==================================================================

static const int g_alphabet_size_64 = 64;
static const int g_chunks_per_group_64 = 4;

static const uint8_t g_chunk_to_encode_char_64[] =
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

static const uint8_t g_chunk_to_encode_char_64_subst[] = {};

static const uint8_t g_whitespace_64[] =
{
    0x09, 0x0a, 0x0d, 0x20
};

// ==================================================================
// ==================================================================

static const int g_alphabet_size_32 = 32;
static const int g_chunks_per_group_32 = 8;

static const uint8_t g_chunk_to_encode_char_32[] =
{
    '0', '1', '2', '3', '4', '5', '6', '7',
    '8', '9', 'a', 'b', 'c', 'd', 'e', 'f',
    'g', 'h', 'j', 'k', 'm', 'n', 'p', 'q',
    'r', 's', 't', 'v', 'w', 'x', 'y', 'z',
};

static const uint8_t g_chunk_to_encode_char_32_subst[] =
{
    'A', 'a',   'B', 'b',   'C', 'c',   'D', 'd',
    'E', 'e',   'F', 'f',   'G', 'g',   'H', 'h',
    'i', '1',   'I', '1',   'J', 'j',   'K', 'k',
    'l', '1',   'L', '1',   'M', 'm',   'N', 'n',
    'o', '0',   'O', '0',   'P', 'p',   'Q', 'q',
    'R', 'r',   'S', 's',   'T', 't',   'u', 'v',
    'U', 'v',   'V', 'v',   'W', 'w',   'X', 'x',
    'Y', 'y',   'Z', 'z',
};

static const uint8_t g_whitespace_32[] =
{
    0x09, 0x0a, 0x0d, 0x20, '-'
};

// ==================================================================
// ==================================================================

static const int g_alphabet_size_16 = 16;
static const int g_chunks_per_group_16 = 2;

static const uint8_t g_chunk_to_encode_char_16[] =
{
    '0', '1', '2', '3', '4', '5', '6', '7',
    '8', '9', 'a', 'b', 'c', 'd', 'e', 'f',
};

static const uint8_t g_chunk_to_encode_char_16_subst[] =
{
    'A', 'a',   'B', 'b',   'C', 'c',   'D', 'd',
    'E', 'e',   'F', 'f',
};

static const uint8_t g_whitespace_16[] =
{
    0x09, 0x0a, 0x0d, 0x20, '-'
};

// ==================================================================
// ==================================================================





int count_bytes_per_group(int alphabet_size, int chunk_count)
{
    __int128 value = 1;
    for(int i = 0; i < chunk_count; i++)
    {
        value *= alphabet_size;
        if(value < 0)
        {
            // Error, but shouldn't happen
            return -1;
        }
    }

    int byte_count = 0;
    while(value > 0)
    {
        value >>= 8;
        byte_count++;
    }
    return byte_count - 1;
}

#define CHUNK_CODE_ERROR 0xff
#define CHUNK_CODE_WHITESPACE 0xfe
static uint8_t g_decode_table[256];
static const uint8_t* g_encode_table;
static int g_encode_table_length;
static int g_alphabet_size;
static int g_chunks_per_group;
static int g_bytes_per_group;

static void fill_chunk_table(const uint8_t* const char_table,
                             const int char_table_length,
                             const uint8_t* const subst_char_pairs_table,
                             const int subst_char_pairs_table_length,
                             const uint8_t* const whitespace_table,
                             const int whitespace_table_length,
                             const int alphabet_size,
                             const int chunks_per_group)
{
    g_alphabet_size = alphabet_size;
    g_chunks_per_group = chunks_per_group;
    g_bytes_per_group = count_bytes_per_group(alphabet_size, chunks_per_group);
    g_encode_table = char_table;
    g_encode_table_length = char_table_length;

    for(int i = 0; i < sizeof(g_decode_table); i++)
    {
        g_decode_table[i] = CHUNK_CODE_ERROR;
    }

    for(int chunk_value = 0; chunk_value < char_table_length; chunk_value++)
    {
        uint8_t character = char_table[chunk_value];
        g_decode_table[character] = chunk_value;
    }

    const uint8_t* pairs_end = subst_char_pairs_table + subst_char_pairs_table_length;
    for(const uint8_t* pair = subst_char_pairs_table; pair < pairs_end; pair += 2)
    {
        uint8_t substitute = pair[0];
        uint8_t index = pair[1];
        g_decode_table[substitute] = g_decode_table[index];
    }

    for(int whitespace = 0; whitespace < whitespace_table_length; whitespace++)
    {
        uint8_t character = whitespace_table[whitespace];
        g_decode_table[character] = CHUNK_CODE_WHITESPACE;
    }
}


#define FILL_CHUNK_TABLE(MODE) \
    fill_chunk_table( \
        g_chunk_to_encode_char_ ## MODE, \
        sizeof(g_chunk_to_encode_char_ ## MODE), \
        g_chunk_to_encode_char_ ## MODE ## _subst, \
        sizeof(g_chunk_to_encode_char_ ## MODE ## _subst), \
        g_whitespace_ ## MODE, \
        sizeof(g_whitespace_ ## MODE), \
        g_alphabet_size_ ## MODE, \
        g_chunks_per_group_ ## MODE)

void print_consts()
{
    printf("static const int g_bytes_per_group       = %d;\n", g_bytes_per_group);
    printf("static const int g_chunks_per_group      = %d;\n", g_chunks_per_group);
    printf("\n");
    printf("#define CHUNK_CODE_ERROR      0xff\n");
    printf("#define CHUNK_CODE_WHITESPACE 0xfe\n");
    printf("\n");
}

void print_char_to_chunk_table()
{
    printf("static const uint8_t g_encode_char_to_chunk[] =\n\
{\n\
#define ERRR CHUNK_CODE_ERROR\n\
#define WHSP CHUNK_CODE_WHITESPACE");

    for(int ch = 0; ch < sizeof(g_decode_table); ch++)
    {
        if((ch & 7) == 0)
        {
            printf("\n    ");
        }
        uint8_t chunk_value = g_decode_table[ch];
        switch(chunk_value)
        {
            case CHUNK_CODE_ERROR:
                printf("ERRR,");
                break;
            case CHUNK_CODE_WHITESPACE:
                printf("WHSP,");
                break;
            default:
                printf("0x%02x,", chunk_value);
        }
    }

    printf("\n#undef WHTE\n\
#undef ERRR\n\
};\n");
    printf("\n");
}

void print_chunk_to_char_table()
{
    printf("static const uint8_t g_chunk_to_encode_char[] =\n{");
    for(int i = 0; i < g_encode_table_length; i++)
    {
        if((i & 7) == 0)
        {
            printf("\n   ");
        }
        printf(" '%c',", (char)g_encode_table[i]);
    }
    printf("\n};\n");
    printf("\n");
}

int count_complete_bytes_inside_chunks(int alphabet_size, int chunk_count)
{
    __int128 value = 1;
    for(int i = 0; i < chunk_count; i++)
    {
        value *= alphabet_size;
    }
    int byte_count = 0;
    while(value >= 0xff)
    {
        byte_count++;
        value >>= 8;
    }
    return byte_count;
}

int count_chunks_required_for_bytes(int alphabet_size, int byte_count)
{
    __int128 value = 0;
    for(int i = 0; i < byte_count; i++)
    {
        if(value == 0)
        {
            value = 0xff;
        }
        else
        {
            value <<= 8;
        }
    }
    int chunk_count = 0;
    while(value > 0)
    {
        value /= alphabet_size;
        chunk_count++;
    }
    return chunk_count;
}

void print_chunk_to_byte_count()
{
    printf("static const int g_chunk_to_byte_count[]   = { ");
    for(int i = 0; i <= g_chunks_per_group; i++)
    {
        int byte_count = count_complete_bytes_inside_chunks(g_alphabet_size, i);
        printf("%d", byte_count);
        if(i != g_chunks_per_group)
        {
            printf(", ");
        }
    }
    printf(" };");
    printf("\n");
}

void print_byte_to_chunk_count()
{
    printf("static const int g_byte_to_chunk_count[]   = { ");
    for(int i = 0; i <= g_bytes_per_group; i++)
    {
        int chunk_count = count_chunks_required_for_bytes(g_alphabet_size, i);
        printf("%d", chunk_count);
        if(i != g_bytes_per_group)
        {
            printf(", ");
        }
    }
    printf(" };");
    printf("\n");
}

// ==================================================================
// ==================================================================

int main(void)
{
    // FILL_CHUNK_TABLE(16);
    // FILL_CHUNK_TABLE(32);
    // FILL_CHUNK_TABLE(64);
    // FILL_CHUNK_TABLE(80);
    FILL_CHUNK_TABLE(85);

    print_consts();
    print_char_to_chunk_table();
    print_chunk_to_char_table();
    print_chunk_to_byte_count();
    print_byte_to_chunk_count();
}
