#define HGL_IO_IMPLEMENTATION
#include "hgl_io.h"

int main()
{

    printf("swizzle32(0x%08X, 0, 1, 2, 3) = 0x%08X\n", 0x11223344, HGL_IO_SWIZZLE32(0x11223344, 0, 1, 2, 3));
    printf("swizzle32(0x%08X, 3, 2, 1, 0) = 0x%08X\n", 0x11223344, HGL_IO_SWIZZLE32(0x11223344, 3, 2, 1, 0));
    printf("swizzle32(0x%08X, 1, 2, 3, 0) = 0x%08X\n", 0x11223344, HGL_IO_SWIZZLE32(0x11223344, 1, 2, 3, 0));
    uint32_t v = HGL_IO_SWIZZLE32(0x11223344u, 1, 1, 3, 3);
    printf("v = %08X\n", v);

    uint8_t bs[] = {0x11, 0x22, 0x33, 0x44};
    uint32_t bs_v = * (uint32_t *) bs;
    printf("bs_v = %08X\n", bs_v);
    uint32_t v2 = HGL_IO_SWIZZLE32(bs_v, 1, 1, 3, 3);
    printf("v2 = %08X\n", v2);
    //HGL_IO_SWIZZLE4x8_IN_PLACE(bs, 0, 0, 2, 2);
    //printf("bs = {%02X %02X %02X %02X}\n", bs[0], bs[1], bs[2], bs[3]);

    /* create and write something */
    hgl_io_file_create("testfil_gubbe.txt", 1024);
    HglFile file = hgl_io_file_mmap("testfil_gubbe.txt");
    strncpy((char *) file.data, "Hejsan hoppsan kallecool06!\n fisk\n\npotat\n jeans ", 128);
    hgl_io_file_munmap(&file);

    file = hgl_io_file_read("testfil_gubbe.txt");
    int i = 0;
    while (i++ < 20) {
        const char *line = NULL;
        int length = hgl_io_file_get_next_line(&file, &line);
        if (line == NULL) {
            break;
        }
        printf("line = \"%.*s\"\n", length, line);
    }
    hgl_io_file_free(&file);

    const char *str = "1337\n";
    hgl_io_file_append_to("testfil_gubbe.txt", (uint8_t *) str, strlen(str));

}
