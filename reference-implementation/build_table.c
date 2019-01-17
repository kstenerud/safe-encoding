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

static const uint8_t g_chunk_to_encode_char_subst_a_85[] = {};
static const uint8_t g_chunk_to_encode_char_subst_b_85[] = {};
static const uint8_t g_chunk_to_encode_char_subst_c_85[] = {};

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

static const uint8_t g_chunk_to_encode_char_subst_a_64[] = {};
static const uint8_t g_chunk_to_encode_char_subst_b_64[] = {};
static const uint8_t g_chunk_to_encode_char_subst_c_64[] = {};

static const uint8_t g_whitespace_64[] =
{
    0x09, 0x0a, 0x0d, 0x20
};

// ==================================================================
// ==================================================================

static const uint8_t g_chunk_to_encode_char_32[] =
{
    '0', '2', '3', '4', '5', '6', '7', '8',
    '9', 'a', 'b', 'c', 'd', 'e', 'f', 'g',
    'h', 'j', 'k', 'm', 'n', 'p', 'q', 'r',
    's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
};

static const uint8_t g_chunk_to_encode_char_subst_a_32[] =
{
    '0', '2', '3', '4', '5', '6', '7', '8',
    '9', 'A', 'B', 'C', 'D', 'E', 'F', 'G',
    'H', 'J', 'K', 'M', 'N', 'P', 'Q', 'R',
    'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
};
static const uint8_t g_chunk_to_encode_char_subst_b_32[] = {'o'};
static const uint8_t g_chunk_to_encode_char_subst_c_32[] = {'O'};

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

static const uint8_t g_chunk_to_encode_char_subst_a_16[] =
{
    '0', '1', '2', '3', '4', '5', '6', '7',
    '8', '9', 'A', 'B', 'C', 'D', 'E', 'F',
};
static const uint8_t g_chunk_to_encode_char_subst_b_16[] = {};
static const uint8_t g_chunk_to_encode_char_subst_c_16[] = {};

static const uint8_t g_whitespace_16[] =
{
    0x09, 0x0a, 0x0d, 0x20
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
static uint8_t g_chunk_table[256];

static void fill_chunk_table(const uint8_t* const char_table,
                             const int char_table_length,
                             const uint8_t* const subst_char_table_a,
                             const int subst_char_table_a_length,
                             const uint8_t* const subst_char_table_b,
                             const int subst_char_table_b_length,
                             const uint8_t* const subst_char_table_c,
                             const int subst_char_table_c_length,
                             const uint8_t* const whitespace_table,
                             const int whitespace_table_length)
{
    for(int i = 0; i < sizeof(g_chunk_table); i++)
    {
        g_chunk_table[i] = CHUNK_CODE_ERROR;
    }

    for(int chunk_value = 0; chunk_value < char_table_length; chunk_value++)
    {
        uint8_t character = char_table[chunk_value];
        g_chunk_table[character] = chunk_value;
    }

    for(int chunk_value = 0; chunk_value < subst_char_table_a_length; chunk_value++)
    {
        uint8_t character = subst_char_table_a[chunk_value];
        g_chunk_table[character] = chunk_value;
    }

    for(int chunk_value = 0; chunk_value < subst_char_table_b_length; chunk_value++)
    {
        uint8_t character = subst_char_table_b[chunk_value];
        g_chunk_table[character] = chunk_value;
    }

    for(int chunk_value = 0; chunk_value < subst_char_table_c_length; chunk_value++)
    {
        uint8_t character = subst_char_table_c[chunk_value];
        g_chunk_table[character] = chunk_value;
    }

    for(int whitespace = 0; whitespace < whitespace_table_length; whitespace++)
    {
        uint8_t character = whitespace_table[whitespace];
        g_chunk_table[character] = CHUNK_CODE_WHITESPACE;
    }
}


#define FILL_CHUNK_TABLE(MODE) \
    fill_chunk_table( \
        g_chunk_to_encode_char_ ## MODE, \
        sizeof(g_chunk_to_encode_char_ ## MODE), \
        g_chunk_to_encode_char_subst_a_ ## MODE, \
        sizeof(g_chunk_to_encode_char_subst_a_ ## MODE), \
        g_chunk_to_encode_char_subst_b_ ## MODE, \
        sizeof(g_chunk_to_encode_char_subst_b_ ## MODE), \
        g_chunk_to_encode_char_subst_c_ ## MODE, \
        sizeof(g_chunk_to_encode_char_subst_c_ ## MODE), \
        g_whitespace_ ## MODE, \
        sizeof(g_whitespace_ ## MODE))



int main(void)
{
    printf("%s", g_header);

    // FILL_CHUNK_TABLE(85);
    FILL_CHUNK_TABLE(64);
    // FILL_CHUNK_TABLE(32);
    // FILL_CHUNK_TABLE(16);

    for(int ch = 0; ch < sizeof(g_chunk_table); ch++)
    {
        if((ch & 7) == 0)
        {
            printf("\n    ");
        }
        uint8_t chunk_value = g_chunk_table[ch];
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

    printf("\n%s\n", g_footer);
}
