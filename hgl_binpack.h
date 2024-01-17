/**
 * LICENSE:
 *
 * MIT License
 *
 * Copyright (c) 2024 Henrik A. Glass
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 * MIT License
 *
 *
 * ABOUT:
 *
 * hgl_binpack.h implements a simple to use binary packing/unpacking utility.
 *
 *
 * USAGE:
 *
 * Include hgl_binpack.h file like this:
 *
 *     #include "hgl_binpack.h"
 *
 * hgl_binpack.h exports the function:
 *
 *     void *hgl_binpack(void *dst, void *src, const char *fmt);
 *
 * where
 *  
 *     'src' is a pointer to an array of bytes we want to parse.
 *     'dst' is a pointer to the destination to where the parsed data should be written.
 *     'fmt' is a an expression describing how the data in 'src' should be parsed.
 *
 * The function returns a pointer to the next unread byte in 'src', or NULL if there was 
 * an error. E.g. if a single 32-bit word is read ('fmt' = "DW"), then the returned pointer 
 * has the value 'src' + 4.
 *
 * the 'fmt' string takes an expression of the form Fmt (expressed in Backus Naur-form):
 *
 * Fmt -> <epsilon>               |
 *        Atom Fmt                | 
 *        Natural '{' Fmt '}' Fmt        # Repeat the expression inside the braces 'Natural' times.
 *                                       # E.g. "4{B}" is equivalent to "BBBB".
 *
 * Atom -> '[' Endian ']'    |           # set endianness for subsequent copy operations 
 *                                       # (persistent until another endianness change is 
 *                                       # encountered). Defaults to network order/big endian.
 *
 *         '\'' Ascii+ '\''  |           # expect to read ascii string.
 *         '#' HexByte+ '#'  |           # expect to read bytes.
 *         '<' HexByte+ '>'  |           # move read pointer to offset inside 'src'
 *         '-'               |           # skip byte in 'src' (increment read pointer by 1)
 *
 *         '^\'' Ascii+ '\'' |           # write ascii string.
 *         '^#' HexByte+ '#' |           # write bytes.
 *         '^<' HexByte+ '>' |           # move write pointer to offset inside 'src'
 *         '+'               |           # skip byte in 'dst' (increment write pointer by 1).
 *
 *         'B'               |           # copy "byte" (8 bits)
 *         'W'               |           # copy "word" (16 bit word)
 *         'DW'              |           # copy "double word" (32 bit word)
 *         'QW'                          # copy "quad word" (64 bit word)
 *
 * Endian -> 'BE' | 'LE'
 *
 * where 'Natural' is a natural number, 'HexByte' is a pair of hexadecimal digits, and 
 * '<epsilon>' denotes the empty string. Each copy operation increments both the write and 
 * read pointers by the amount of bytes read. Whitespaces inside the fmt string are ignored.
 *
 *
 * EXAMPLE:
 *
 * In this example we parse an ELF file and print some information about it (like the 'file' 
 * command in linux):
 *
 *     // NB: this struct ignores some fields of the ELF header such as EI_VERSION.
 *     typedef struct __attribute__((__packed__)) {
 *         uint8_t ei_class;      // 1 == 32 bit, 2 == 64 bit
 *         uint8_t ei_data;       // 1 == little endian, 2 == big endian (starting at 0x10)
 *         uint8_t ei_osabi;      // target OS ABI.
 *         uint8_t ei_abiversion; // ABI version
 *         uint16_t e_type;
 *         uint16_t e_machine;
 *     } ElfInfo;
 *
 *              /.../
 *
 *     const char *filepath = "./my_program"
 *     uint8_t *file_data = hgl_open(filepath, "r");
 *     ElfInfo elf_info = {0};
 *     assert(NULL != hgl_binpack(&elf_info, file_data, "#7F#'ELF'BB-BB") && "Not an ELF-file.");
 *     assert(NULL != hgl_binpack(&elf_info.e_type, file_data, 
 *                                (elf_info.ei_data == 1) ? "[LE]<10>2{W}": "[BE]<10>2{W}"));
 *
 *     printf("%s: ELF, %s, %s, %s, %s\n", 
 *            argv[1],
 *            (elf_info.ei_class == 1) ? "32-bit" : "64-bit",
 *            machine_to_str[elf_info.e_machine],
 *            type_to_str[elf_info.e_type],
 *            abi_to_str[elf_info.ei_osabi]);
 *
 *
 * AUTHOR: Henrik A. Glass
 *
 */


#ifndef HGL_BINPARSE_H
#define HGL_BINPARSE_H

/*--- Include files ---------------------------------------------------------------------*/

#include <stdint.h>
#include <byteswap.h>
#include <arpa/inet.h>
#include <string.h>

/*--- Public macros ---------------------------------------------------------------------*/

#define EXPECT(cond) \
    do { \
        if (!(cond)) { \
            fprintf(stderr, "[hgl_binpack error]: Expected %s <%s:%d>\n", \
                            #cond, __FILE__, __LINE__); \
            return NULL; \
        } \
    } while(0)

#define IS_DIGIT(c) ((c) >= '0' && (c) <= '9')
#define IS_HEX_DIGIT(c) (((c) >= '0' && (c) <= '9') || \
                         ((c) >= 'a' && (c) <= 'f') || \
                         ((c) >= 'A' && (c) <= 'F'))

#define HGL_MACHINE_ENDIANNESS ((1 == ntohl(1)) ? BE_ORDER : LE_ORDER)

/*--- Public type definitions -----------------------------------------------------------*/

typedef enum {
    BE_ORDER, 
    LE_ORDER 
} ByteOrder;

typedef struct {
    int n_iterations;
    const char *block_start;
} Jump;

/*--- Public functions ------------------------------------------------------------------*/

static inline uint8_t nibble_to_int_(char nibble)
{
    if (nibble >= '0' && nibble <= '9') {
        return nibble - '0';
    } else if (nibble >= 'a' && nibble <= 'f') {
        return nibble - 'a' + 10;
    } else if (nibble >= 'A' && nibble <= 'F') {
        return nibble - 'A' + 10;
    }
    return 0;
}

static inline uint8_t read_8_(uint8_t **src)
{
    uint8_t value = **src;
    *src += 1;
    return value;
}

static inline uint16_t read_16_(uint8_t **src, ByteOrder byte_order)
{
    uint16_t *src16 = (uint16_t *) *src;
    uint16_t value = (HGL_MACHINE_ENDIANNESS != byte_order) ? bswap_16(*src16) : *src16;
    *src += 2;
    return value;
}

static inline uint32_t read_32_(uint8_t **src, ByteOrder byte_order)
{
    uint32_t *src32 = (uint32_t *) *src;
    uint32_t value = (HGL_MACHINE_ENDIANNESS != byte_order) ? bswap_32(*src32) : *src32;
    *src += 4;
    return value;
}

static inline uint64_t read_64_(uint8_t **src, ByteOrder byte_order)
{
    uint64_t *src64 = (uint64_t *) *src;
    uint64_t value = (HGL_MACHINE_ENDIANNESS != byte_order) ? bswap_64(*src64) : *src64;
    *src += 8;
    return value;
}

static inline void write_8_(uint8_t **dst, uint8_t value)
{
    **dst = value;
    *dst += 1;
}

static inline void write_16_(uint8_t **dst, uint16_t value)
{
    *((uint16_t *)*dst) = value;
    *dst += 2;
}

static inline void write_32_(uint8_t **dst, uint32_t value)
{
    *((uint32_t *)*dst) = value;
    *dst += 4;
}

static inline void write_64_(uint8_t **dst, uint64_t value)
{
    *((uint64_t *)*dst) = value;
    *dst += 8;
}

static inline void *hgl_binpack(void *dst, void *src, const char *fmt)
{
    uint8_t *read_ptr = (uint8_t *) src;
    uint8_t *write_ptr = (uint8_t *) dst;
    ByteOrder byte_order = BE_ORDER;

    char digit_stack[10] = {0};
    int digit_stack_idx = 0;

    int jump_stack_top = 0;
    Jump jump_stack[32] = {0};
    
    while (*fmt != '\0') {
        
        /* handle repeating block */
        if (IS_DIGIT(*fmt)) {
            int n_iter = 0;

            /* read digts */
            while (IS_DIGIT(*fmt) && digit_stack_idx < 10) {
                digit_stack[digit_stack_idx] = *fmt;
                digit_stack_idx++;
                fmt++;
            } 
            EXPECT(*fmt == '{');

            /* convert to integer */
            int coeff = 1;
            while (digit_stack_idx > 0) {
                digit_stack_idx--;
                n_iter += coeff * (digit_stack[digit_stack_idx] - '0');
                coeff *= 10;
            }
            
            /* add entry to jump stack */
            jump_stack[jump_stack_top].n_iterations = n_iter;
            jump_stack[jump_stack_top].block_start = fmt;
            jump_stack_top++;
            fmt++;
            continue;
        }

        switch (*fmt) {
            /* ignore whitespace*/
            case ' ': case '\n': case '\r': case '\t': {
                fmt++;
            } break;

            /* handle jump-backs */
            case '}': {
                EXPECT(jump_stack_top > 0);
                jump_stack[jump_stack_top - 1].n_iterations--;
                if (jump_stack[jump_stack_top - 1].n_iterations > 0) {
                    fmt = jump_stack[jump_stack_top - 1].block_start;
                } else {
                    jump_stack_top--;
                }
                fmt++;
            } break;

            /* persistent endianness modifier */
            case '[': {
                if (0 == strncmp(fmt, "[BE]", 4)) {
                    byte_order = BE_ORDER; 
                } else if (0 == strncmp(fmt, "[LE]", 4)) {
                    byte_order = LE_ORDER;
                } else {
                    EXPECT(0); // fail.
                }
                fmt += 4;
            } break;

            /* increment read pointer */
            case '-': {
                read_ptr++;
                fmt++;
            } break;
            
            /* increment write pointer */
            case '+': {
                write_ptr++;
                fmt++;
            } break;

            /* do something with write pointer */
            case '^': {
                fmt++;
                switch (fmt[0]) {
                    /* move write pointer */
                    case '<': {
                        size_t offset = 0;
                        fmt++;
                        while (fmt[0] != '>') {
                            EXPECT(IS_HEX_DIGIT(fmt[0]));
                            EXPECT(IS_HEX_DIGIT(fmt[1]));
                            char high_nibble = *fmt++;
                            char low_nibble = *fmt++;
                            offset <<= 8;
                            offset  |= (nibble_to_int_(high_nibble) << 4) +
                                        nibble_to_int_(low_nibble);
                        }
                        fmt++;
                        write_ptr = ((uint8_t *) dst) + offset;
                    } break;

                    /* write string literal */
                    case '\'': {
                        fmt++;
                        while (fmt[0] != '\'') {
                            write_8_(&write_ptr, *fmt++);  
                        }
                        fmt++;
                    } break;

                    /* write bytes */
                    case '#': {
                        uint8_t to_write = 0;

                        fmt++;
                        while (fmt[0] != '#') {
                            EXPECT(IS_HEX_DIGIT(fmt[0]));
                            EXPECT(IS_HEX_DIGIT(fmt[1]));
                            char high_nibble = *fmt++;
                            char low_nibble = *fmt++;
                            to_write = (nibble_to_int_(high_nibble) << 4) +
                                        nibble_to_int_(low_nibble);
                            write_8_(&write_ptr, to_write); 
                        }
                        fmt++;
                    } break;

                    default:
                        EXPECT(0);
                } 
            } break;

            /* move read pointer */
            case '<': {
                size_t offset = 0;
                fmt++;
                while (fmt[0] != '>') {
                    EXPECT(IS_HEX_DIGIT(fmt[0]));
                    EXPECT(IS_HEX_DIGIT(fmt[1]));
                    char high_nibble = *fmt++;
                    char low_nibble = *fmt++;
                    offset <<= 8;
                    offset  |= (nibble_to_int_(high_nibble) << 4) +
                                nibble_to_int_(low_nibble);
                }
                fmt++;
                read_ptr = ((uint8_t *) src) + offset;
            } break;

            /* expect string constant literal */
            case '\'': {
                fmt++;
                while (*fmt != '\'') {
                    char expected = *fmt++;
                    char actual = read_8_(&read_ptr);  
                    EXPECT(actual == expected);
                }
                fmt++;
            } break;

            /* expect numeric (hex) constant literal */
            case '#': {
                uint8_t actual = 0;
                uint8_t expected = 0;

                fmt++;
                while (*fmt != '#') {
                    EXPECT(IS_HEX_DIGIT(fmt[0]));
                    EXPECT(IS_HEX_DIGIT(fmt[1]));
                    char high_nibble = *fmt++;
                    char low_nibble = *fmt++;
                    expected = (nibble_to_int_(high_nibble) << 4) +
                                nibble_to_int_(low_nibble);
                    actual = read_8_(&read_ptr); 
                    EXPECT(actual == expected);
                }
                fmt++;
            } break;
            
            /* copy operations */
            case 'B': 
                write_8_(&write_ptr, read_8_(&read_ptr)); 
                fmt++; 
                break;
            case 'W': 
                write_16_(&write_ptr, read_16_(&read_ptr, byte_order)); 
                fmt++; 
                break;
            case 'D': 
                EXPECT(fmt[1] == 'W'); 
                write_32_(&write_ptr, read_32_(&read_ptr, byte_order)); 
                fmt += 2; 
                break;
            case 'Q': 
                EXPECT(fmt[1] == 'W'); 
                write_64_(&write_ptr, read_64_(&read_ptr, byte_order)); 
                fmt += 2; 
                break;

            /* unknown character */
            default:
                EXPECT(0);
        }
    }
   
    return (void *) read_ptr;
}

#endif /* HGL_BINPARSE_H */

