#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <assert.h>

#include "hgl_serialize.h"

// NB: this struct ignores some fields of the ELF header.
typedef struct __attribute__((__packed__)) {
    uint8_t ei_class;      // 1 == 32 bit, 2 == 64 bit
    uint8_t ei_data;       // 1 == little endian, 2 == big endian (starting at 0x10)
    uint8_t ei_osabi;      // target OS ABI.
    uint8_t ei_abiversion; // ABI version
    uint16_t e_type;
    uint16_t e_machine;
} ElfInfo;

const char *MACHINE_TO_STR[0x102] = {
    [0x00] = "No specific instruction set",
    [0x14] = "PowerPC",
    [0x15] = "PowerPC 64-bit",
    [0x3E] = "AMD x86-64",
    [0xB7] = "ARM 64-bits"
};

const char *TYPE_TO_STR[0x10] = {
    [0x00] = "Unknown",
    [0x01] = "Relocatable file",
    [0x02] = "Executable file",
    [0x03] = "Shared object",
    [0x04] = "Core file"
};

const char *ABI_TO_STR[0x13] = {
    [0x00] = "System V",
    [0x03] = "Linux",
    [0x06] = "Solaris"
};

int main(int argc, char *argv[])
{
    /* parse arguments */
    if (argc != 2) {
        printf("Usage: elfinfo <path-to-file>\n");
        return 1;
    }

    /* read image file into buffer */
    FILE *fp = fopen(argv[1], "rb");
    if (fp == NULL) {
        fprintf(stderr, "Error: %s\n", strerror(errno));
        return 1;
    }
    fseek(fp, 0, SEEK_END);
    ssize_t filesize = ftell(fp);
    rewind(fp);
    uint8_t *file_data = malloc(filesize * sizeof(uint8_t));
    ssize_t n_read_bytes = (ssize_t) fread(file_data, 1, filesize, fp);
    assert(n_read_bytes == filesize);
    fclose(fp);

    ElfInfo elf_info = {0};
    assert(NULL != hgl_serialize(&elf_info, file_data, "#7F#'ELF'BB-BB") && "Not an ELF-file.");
    assert(NULL != hgl_serialize(&elf_info.e_type, file_data, 
                                 (elf_info.ei_data == 1) ? "[LE]<10>%{W}": "[BE]<10>2{W}", 2));

    printf("%s: ELF, %s, %s, %s, %s\n", 
           argv[1],
           (elf_info.ei_class == 1) ? "32-bit" : "64-bit",
           MACHINE_TO_STR[elf_info.e_machine],
           TYPE_TO_STR[elf_info.e_type],
           ABI_TO_STR[elf_info.ei_osabi]);


    free(file_data);
}
