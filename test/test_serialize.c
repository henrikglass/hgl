
#include "hgl_test.h"

#include "hgl_serialize.h"


#define STR_SIZE 24
typedef struct __attribute__((__packed__))
{
    uint8_t a;
    uint32_t b;
    char str[STR_SIZE];
    uint32_t c;
} SomeDataType;

static uint8_t scratch[4096];

static SomeDataType my_data = {
    .a = 255,
    .b = 0xAABBCCDD,
    .str = "fisk",
    .c = 0xFF000000,
};

TEST(test_struct_size)
{
    ASSERT(sizeof(SomeDataType) == (1 + 4 + 4 + STR_SIZE));
}

TEST(test_serialize_unserialize)
{
    SomeDataType my_data_unserialized;

    /* serialize data */
    ASSERT(NULL != hgl_serialize(scratch, &my_data, "[BE] B DW %{B} [LE]DW", STR_SIZE));

    /* unserialize data */
    ASSERT(NULL != hgl_serialize(&my_data_unserialized, scratch, "[BE]BDW %{B} [LE] DW", STR_SIZE));

    /* assert that the unserialized data is equal to the original data */
    ASSERT(my_data.a == my_data_unserialized.a);
    ASSERT(my_data.b == my_data_unserialized.b);
    ASSERT_CSTR_EQ(my_data.str, my_data_unserialized.str);
    ASSERT(my_data.c == my_data_unserialized.c);
}

TEST(test_endianness)
{
    /* serialize data */
    ASSERT(NULL != hgl_serialize(scratch, &my_data, "[BE]-DW%{-}[LE]DW", STR_SIZE));

    /* assert that the first u32 is stored in BE order */
    ASSERT(scratch[0] == 0xAA);
    ASSERT(scratch[1] == 0xBB);
    ASSERT(scratch[2] == 0xCC);
    ASSERT(scratch[3] == 0xDD);

    /* assert that the second u32 is stored in LE order immediately afterwards */
    ASSERT(scratch[4] == 0x00);
    ASSERT(scratch[5] == 0x00);
    ASSERT(scratch[6] == 0x00);
    ASSERT(scratch[7] == 0xFF);
}

TEST(test_offset)
{
    uint32_t word;

    ASSERT(NULL != hgl_serialize(scratch, &my_data, "[BE]-+++DW"));
    ASSERT(NULL != hgl_serialize(&word, scratch, "---[BE]DW"));
    
    ASSERT(word == 0xAABBCCDD);

    ASSERT(NULL != hgl_serialize(&word, scratch, "<03>[LE]DW"));
    
    ASSERT(word == 0xDDCCBBAA);
}


TEST(test_expect)
{
    ASSERT(NULL != hgl_serialize(scratch, &my_data, "#FF#4{-}'fisk'"));
    ASSERT(NULL == hgl_serialize(scratch, &my_data, "#FF#4{-}'pisk'"));
    ASSERT(NULL == hgl_serialize(scratch, &my_data, "#00#4{-}'fisk'"));
}

#define TEST_STR "Hejsan Hoppsan"
TEST(test_write, .expect_output = TEST_STR)
{ 
    ASSERT(NULL != hgl_serialize(scratch, &my_data, "^'" TEST_STR "'^#00#"));
    ASSERT_CSTR_EQ((char *)scratch, TEST_STR);
    printf("%s", (char *) scratch);
} 


