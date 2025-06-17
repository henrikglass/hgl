
#include "hgl_test.h"

#define HGL_SERIALIZE_IMPLEMENTATION
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

TEST(test_unaligned_read)
{
    uint16_t value16;
    uint32_t value32;
    uint64_t value64;
    static uint8_t some_data[] = {
        0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99,
    };

    // be32
    hgl_serialize(&value32, &some_data[0], "[BE]DW");
    ASSERT(value32 == 0x11223344);
    hgl_serialize(&value32, &some_data[0], "[BE]1{-}DW");
    ASSERT(value32 == 0x22334455);
    hgl_serialize(&value32, &some_data[0], "[BE]2{-}DW");
    ASSERT(value32 == 0x33445566);
    hgl_serialize(&value32, &some_data[0], "[BE]3{-}DW");
    ASSERT(value32 == 0x44556677);
    hgl_serialize(&value32, &some_data[0], "[BE]4{-}DW");
    ASSERT(value32 == 0x55667788);

    // le32
    hgl_serialize(&value32, &some_data[0], "[LE]DW");
    ASSERT(value32 == 0x44332211);
    hgl_serialize(&value32, &some_data[0], "[LE]1{-}DW");
    ASSERT(value32 == 0x55443322);
    hgl_serialize(&value32, &some_data[0], "[LE]2{-}DW");
    ASSERT(value32 == 0x66554433);
    hgl_serialize(&value32, &some_data[0], "[LE]3{-}DW");
    ASSERT(value32 == 0x77665544);
    hgl_serialize(&value32, &some_data[0], "[LE]4{-}DW");
    ASSERT(value32 == 0x88776655);

    // be16/le16
    hgl_serialize(&value16, &some_data[0], "[BE]1{-}W");
    ASSERT(value16 == 0x2233);
    hgl_serialize(&value16, &some_data[0], "[LE]1{-}W");
    ASSERT(value16 == 0x3322);
    
    // be64/le64
    hgl_serialize(&value64, &some_data[0], "[BE]1{-}QW");
    ASSERT(value64 == 0x2233445566778899);
    hgl_serialize(&value64, &some_data[0], "[LE]1{-}QW");
    ASSERT(value64 == 0x9988776655443322);

    hgl_serialize(&value32, &some_data[1], "[BE]DW");
    ASSERT(value32 == 0x22334455);
}

TEST(test_unaligned_write)
{
    static uint8_t some_data[] = {
        0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99,
    };

    for (int i = 0; i < 2; i++) {
        hgl_serialize(&scratch, &some_data[0], "[BE]%{+}W", i);
        switch (HGL_SERIALIZE_MACHINE_ENDIANNESS) {
            case LE_ORDER: ASSERT(scratch[0 + i] == 0x22 && scratch[1 + i] == 0x11); break;
            case BE_ORDER: ASSERT(scratch[1 + i] == 0x22 && scratch[0 + i] == 0x11); break;
        }
    }

    for (int i = 0; i < 4; i++) {
        hgl_serialize(&scratch, &some_data[0], "[BE]%{+}DW", i);
        switch (HGL_SERIALIZE_MACHINE_ENDIANNESS) {
            case LE_ORDER: ASSERT(scratch[0 + i] == 0x44 && scratch[1 + i] == 0x33 && scratch[2 + i] == 0x22 && scratch[3 + i] == 0x11); break;
            case BE_ORDER: ASSERT(scratch[3 + i] == 0x44 && scratch[2 + i] == 0x33 && scratch[1 + i] == 0x22 && scratch[0 + i] == 0x11); break;
        }
    }
}

TEST(test_nested_repeat, .timeout = 1)
{
    uint8_t value;

    value = 0;
    scratch[6*2*8] = 0x42;
    hgl_serialize(&value, scratch, "6{ 2{ 8{ - } } } B");
    assert(value == 0x42);

    value = 0;
    scratch[6*2*8] = 0x69;
    hgl_serialize(&value, scratch, "6{ [BE] 2{ 8{ [LE] - } } } B");
    assert(value == 0x69);

    value = 0;
    scratch[6*2*8] = 0x16;
    hgl_serialize(&value, scratch, "%{ %{ %{ - } } } B", 6, 2, 8);
    assert(value == 0x16);

}

#define TEST_STR "Hejsan Hoppsan"
TEST(test_write, .expect_output = TEST_STR)
{ 
    ASSERT(NULL != hgl_serialize(scratch, &my_data, "^'" TEST_STR "'^#00#"));
    ASSERT_CSTR_EQ((char *)scratch, TEST_STR);
    printf("%s", (char *) scratch);
} 


