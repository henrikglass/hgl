#define HGL_IO_IMPLEMENTATION
#include "hgl_io.h"

int main()
{
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
