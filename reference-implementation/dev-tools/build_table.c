#include <stdint.h>
#include <stdio.h>

// ==================================================================
// ==================================================================

static const uint8_t g_chunk_to_encode_char_85[] =
{
    '!', '$', '%', '(', ')', '*', '+', ',',
    '-', '.', '0', '1', '2', '3', '4', '5',
    '6', '7', '8', '9', ';', '=', '>', '@',
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



static const char* const g_header = "#define CHUNK_CODE_ERROR      0xff\n\
#define CHUNK_CODE_WHITESPACE 0xfe\n\
\n\
static const uint8_t g_encode_char_to_chunk[] =\n\
{\n\
#define ERRR CHUNK_CODE_ERROR\n\
#define WHSP CHUNK_CODE_WHITESPACE";

static const char* const g_footer = "#undef WHTE\n\
#undef ERRR\n\
};";


#define CHUNK_CODE_ERROR 0xff
#define CHUNK_CODE_WHITESPACE 0xfe
static uint8_t g_decode_table[256];
static const uint8_t* g_encode_table;
static int g_encode_table_length;

static void fill_chunk_table(const uint8_t* const char_table,
                             const int char_table_length,
                             const uint8_t* const subst_char_pairs_table,
                             const int subst_char_pairs_table_length,
                             const uint8_t* const whitespace_table,
                             const int whitespace_table_length)
{
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
        sizeof(g_whitespace_ ## MODE))



int main(void)
{
    printf("%s", g_header);

    // FILL_CHUNK_TABLE(16);
    FILL_CHUNK_TABLE(32);
    // FILL_CHUNK_TABLE(64);
    // FILL_CHUNK_TABLE(85);

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

    printf("\n%s\n\n", g_footer);

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
}
