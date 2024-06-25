
#define HGL_IO_IMPLEMENTATION
#include "hgl_io.h"

#include <math.h>

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
    HGL_IO_SWIZZLE4x8_IN_PLACE(bs, 3, 2, 1, 0);
    printf("bs = {%02X %02X %02X %02X}\n", bs[0], bs[1], bs[2], bs[3]);

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

    /* HglImage write test 1 */
    HglImage image = {
        .format = HGL_IO_PIXEL_FORMAT_RGBA8,
        .width = 256,
        .height = 256,
        .data = malloc(4 * 256 * 256),
    };
    for (size_t y = 0; y < image.height; y++) {
        for (size_t x = 0; x < image.width; x++) {
            uint8_t r = y; 
            uint8_t g = x; 
            uint8_t b = 255 - abs((int)y - (int)x);
            uint8_t alpha = 255;
            image.data[y*image.width*4 + x*4 + 0] = r;
            image.data[y*image.width*4 + x*4 + 1] = g;
            image.data[y*image.width*4 + x*4 + 2] = b;
            image.data[y*image.width*4 + x*4 + 3] = alpha;
        }
    }
    hgl_io_image_write_netpbm("testfil_image.ppm", &image);
    
    /* HglImage write test 2 */
    HglImage image2 = {
        .format = HGL_IO_PIXEL_FORMAT_RGBA32F,
        .width = 256,
        .height = 256,
        .data = malloc(16 * 256 * 256),
    };
    float *image_data32f = (float *) image2.data;
    for (size_t y = 0; y < image2.height; y++) {
        for (size_t x = 0; x < image2.width; x++) {
            float r = y / 255.0f; 
            float g = x / 255.0f; 
            float b = 1.0f - fabsf(r - g);
            float alpha = 1.0f;
            image_data32f[y*image2.width*4 + x*4 + 0] = r;
            image_data32f[y*image2.width*4 + x*4 + 1] = g;
            image_data32f[y*image2.width*4 + x*4 + 2] = b;
            image_data32f[y*image2.width*4 + x*4 + 3] = alpha;
        }
    }

    hgl_io_image_write_netpbm("testfil_image2.ppm", &image2);

}
