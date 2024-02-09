
#include <stdio.h>
#include <assert.h>

#include "hgl_binpack.h"

typedef struct __attribute__((__packed__)) {
    uint8_t a;
    uint8_t b;
    uint16_t c;
    //uint32_t d;
    char str[4];
    float ff;
} MyStruct;

typedef union {
    float f;
    uint8_t b[4];
} MyUnion;

char my_str[10];

int main(void)
{
    printf("Hello .E.L.FWorld!\n");

    MyUnion u;
    u.f = 123.456f;
    uint8_t bytes[] = {0x00   , 0xAA   , 0xBB   , 0xCC     ,
                       'h'    , 'g'    , 'l'    , '!'      ,
                       0x00   , 'h'    , 'e'    , 'j'      ,
                       u.b[0] , u.b[1] , u.b[2] , u.b[3]};

    MyStruct my_struct; 
    memset(&my_struct, 0, sizeof(my_struct));
    
    //uint8_t *offset = hgl_binparse((uint8_t *) &my_struct, bytes, "[BE]:0x00AA:W:[BE]:.h.g.l:S:DW:");
    //void *offset = hgl_binparse((void *) &my_struct, (void *) bytes, "[BE]BBW'hgl''!'#a0#BBB");
    

    void *offset = hgl_binpack((void *) &my_struct, (void *) bytes, "[BE]%{B}W'hgl!'#00#BBB+[LE]DW", 2);
    float my_float;
    void *result = hgl_binpack(&my_float, bytes, "[LE]#00AABBCC#<0C>DW");
    printf("%s\n", (result != NULL) ? "OK": "FAILED");
    //hgl_binparse((void *) &my_str, (void *) bytes, "4{-}'hgl!'-3{B}");

    printf("my_float = %f\n", (double) my_float);
    //printf("my_str = %s\n", my_str);

    if (offset != NULL) {
        printf("a = %X\n", my_struct.a);
        printf("b = %X\n", my_struct.b);
        printf("c = %X\n", my_struct.c);
        printf("str = %s\n", my_struct.str);
        printf("ff = %f\n", (double) my_struct.ff);
        //printf("d = %X\n", my_struct.d);
        printf("%ld\n", (uint8_t*)offset - bytes);
    }


    memset(my_str, 0, sizeof(my_str));
    assert(NULL == hgl_binpack(&my_str, NULL, "[BE]^'HEJ :>'^#4848#"));
    printf("%s\n", my_str);
    
    uint8_t buf[4] = {0};
    uint32_t u32 = 0x12345678;
    hgl_binpack(buf, &u32, "[BE]DW");
    printf("{%02X, %02X, %02X, %02X}\n", buf[0], buf[1], buf[2], buf[3]);
    hgl_binpack(&u32, buf, "[BE]DW^<00>^#AA#+^#BB#");
    printf("0x%08X\n", u32);

}
