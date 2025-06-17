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
 * hgl_serialize.h implements a simple to use binary serializing/unserializing utility.
 *
 *
 * USAGE:
 *
 * Include hgl_serialize.h file like this:
 *
 *     #define HGL_SERIALIZE_IMPLEMENTATION
 *     #include "hgl_serialize.h"
 *
 * hgl_serialize.h exports the function:
 *
 *     void *hgl_serialize(void *dst, void *src, const char *fmt, ...);
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
 *        Natural '{' Fmt '}' Fmt |      # Repeat the expression inside the braces 'Natural' times.
 *                                       # E.g. "4{B}" is equivalent to "BBBB".
 *        '%' '{' Fmt '}' Fmt     |      # Repeat the expression inside the braces N times, where N
 *                                       # is specified by the next argument in the variadic arguments
 *                                       # list.
 *
 * Atom -> '[' Endian ']'    |           # set endianness for subsequent copy operations 
 *                                       # (persistent until another endianness change is 
 *                                       # encountered). Defaults to network order/big endian.
 *
 *         ''' Ascii+ '''    |           # expect to read ascii string.
 *         '#' HexByte+ '#'  |           # expect to read bytes.
 *         '<' HexByte+ '>'  |           # move read pointer to offset inside 'src'
 *         '-'               |           # skip byte in 'src' (increment read pointer by 1)
 *
 *         '^'' Ascii+ '''   |           # write ascii string.
 *         '^#' HexByte+ '#' |           # write bytes.
 *         '^<' HexByte+ '>' |           # move write pointer to offset inside 'dst'
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
 *     assert(NULL != hgl_serialize(&elf_info, file_data, "#7F#'ELF'BB-BB") && "Not an ELF-file.");
 *     assert(NULL != hgl_serialize(&elf_info.e_type, file_data, 
 *                                  (elf_info.ei_data == 1) ? "[LE]<10>%{W}": "[BE]<10>2{W}", 2));
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


#ifndef HGL_SERIALIZE_H
#define HGL_SERIALIZE_H

#include <stdarg.h>

void *hgl_serialize(void *dst, void *src, const char *fmt, ...);

#endif /* HGL_SERIALIZE_H */

#ifdef HGL_SERIALIZE_IMPLEMENTATION

#include <stdint.h>
#include <byteswap.h>
#include <assert.h>
#include <arpa/inet.h>
#include <string.h>
#include <stddef.h>

#define HGL_SERIALIZE_EXPECT(cond) \
    do { \
        if (!(cond)) { \
            fprintf(stderr, "[hgl_serialize error]: Expected `%s` <%s:%d>\n", \
                            #cond, __FILE__, __LINE__); \
            return NULL; \
        } \
    } while(0)

#define HGL_SERIALIZE_IS_DIGIT(c) ((c) >= '0' && (c) <= '9')
#define HGL_SERIALIZE_IS_HEX_DIGIT(c) (((c) >= '0' && (c) <= '9') || \
                                       ((c) >= 'a' && (c) <= 'f') || \
                                       ((c) >= 'A' && (c) <= 'F'))

#define HGL_SERIALIZE_MACHINE_ENDIANNESS ((1 == ntohl(1)) ? BE_ORDER : LE_ORDER)
#define HGL_SERIALIZE_JUMP_STACK_HEGIHT 32
#define HGL_SERIALIZE_LOOP_ITER_CACHE_SIZE 255

typedef enum {
    BE_ORDER, 
    LE_ORDER 
} ByteOrder;

typedef struct {
    int n_iterations;
    const char *block_start;
} Jump;

typedef struct {
    ptrdiff_t idx;
    int n_iter;
} IndexValuePair;

static inline uint8_t nibble_to_int_(char nibble);
static inline uint8_t read_8_(uint8_t **src);
static inline uint16_t read_16_(uint8_t **src, ByteOrder byte_order);
static inline uint32_t read_32_(uint8_t **src, ByteOrder byte_order);
static inline uint64_t read_64_(uint8_t **src, ByteOrder byte_order);
static inline void write_8_(uint8_t **dst, uint8_t value);
static inline void write_16_(uint8_t **dst, uint16_t value);
static inline void write_32_(uint8_t **dst, uint32_t value);
static inline void write_64_(uint8_t **dst, uint64_t value);

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
#if 0
    uint16_t *src16 = (uint16_t *) *src;
    uint16_t value = (HGL_SERIALIZE_MACHINE_ENDIANNESS != byte_order) ? bswap_16(*src16) : *src16;
#else
    uint16_t value = 0;
    switch (byte_order) {
        case BE_ORDER: {
            value = ((uint16_t) (*src)[0] << 8) | 
                    ((uint16_t) (*src)[1]);
        } break;
        case LE_ORDER: {
            value = ((uint16_t) (*src)[1] << 8) | 
                    ((uint16_t) (*src)[0]);
        } break;
    }
#endif
    *src += 2;
    return value;
}

static inline uint32_t read_32_(uint8_t **src, ByteOrder byte_order)
{
#if 0
    uint32_t *src32 = (uint32_t *) *src;
    uint32_t value = (HGL_SERIALIZE_MACHINE_ENDIANNESS != byte_order) ? bswap_32(*src32) : *src32;
#else
    uint32_t value = 0;
    switch (byte_order) {
        case BE_ORDER: {
            value = ((uint32_t) (*src)[0] << 24) | 
                    ((uint32_t) (*src)[1] << 16) | 
                    ((uint32_t) (*src)[2] <<  8) | 
                    ((uint32_t) (*src)[3]);
        } break;
        case LE_ORDER: {
            value = ((uint32_t) (*src)[3] << 24) | 
                    ((uint32_t) (*src)[2] << 16) | 
                    ((uint32_t) (*src)[1] <<  8) | 
                    ((uint32_t) (*src)[0]);
        } break;
    }
#endif
    *src += 4;
    return value;
}

static inline uint64_t read_64_(uint8_t **src, ByteOrder byte_order)
{
#if 0
    uint64_t *src64 = (uint64_t *) *src;
    uint64_t value = (HGL_SERIALIZE_MACHINE_ENDIANNESS != byte_order) ? bswap_64(*src64) : *src64;
#else
    uint64_t value = 0;
    switch (byte_order) {
        case BE_ORDER: {
            value = ((uint64_t) (*src)[0] << 56) | 
                    ((uint64_t) (*src)[1] << 48) | 
                    ((uint64_t) (*src)[2] << 40) | 
                    ((uint64_t) (*src)[3] << 32) | 
                    ((uint64_t) (*src)[4] << 24) | 
                    ((uint64_t) (*src)[5] << 16) | 
                    ((uint64_t) (*src)[6] <<  8) | 
                    ((uint64_t) (*src)[7]);
        } break;
        case LE_ORDER: {
            value = ((uint64_t) (*src)[7] << 56) | 
                    ((uint64_t) (*src)[6] << 48) | 
                    ((uint64_t) (*src)[5] << 40) | 
                    ((uint64_t) (*src)[4] << 32) | 
                    ((uint64_t) (*src)[3] << 24) | 
                    ((uint64_t) (*src)[2] << 16) | 
                    ((uint64_t) (*src)[1] <<  8) | 
                    ((uint64_t) (*src)[0]);
        } break;
    }
#endif
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
#if 0
    *((uint16_t *)*dst) = value;
#else
    switch (HGL_SERIALIZE_MACHINE_ENDIANNESS) {
        case BE_ORDER: {
            (*dst)[0] = (value >> 8) & 0xFF;
            (*dst)[1] = (value) & 0xFF;
        } break;
        case LE_ORDER: {
            (*dst)[0] = (value) & 0xFF;
            (*dst)[1] = (value >> 8) & 0xFF;
        } break;
    }
#endif
    *dst += 2;
}

static inline void write_32_(uint8_t **dst, uint32_t value)
{
#if 0
    *((uint32_t *)*dst) = value;
#else
    switch (HGL_SERIALIZE_MACHINE_ENDIANNESS) {
        case BE_ORDER: {
            (*dst)[0] = (value >> 24) & 0xFF;
            (*dst)[1] = (value >> 16) & 0xFF;
            (*dst)[2] = (value >>  8) & 0xFF;
            (*dst)[3] = (value >>  0) & 0xFF;
        } break;
        case LE_ORDER: {
            (*dst)[0] = (value >>  0) & 0xFF;
            (*dst)[1] = (value >>  8) & 0xFF;
            (*dst)[2] = (value >> 16) & 0xFF;
            (*dst)[3] = (value >> 24) & 0xFF;
        } break;
    }
#endif
    *dst += 4;
}

static inline void write_64_(uint8_t **dst, uint64_t value)
{
#if 0
    *((uint64_t *)*dst) = value;
#else
    switch (HGL_SERIALIZE_MACHINE_ENDIANNESS) {
        case BE_ORDER: {
            (*dst)[0] = (value >> 56) & 0xFF;
            (*dst)[1] = (value >> 48) & 0xFF;
            (*dst)[2] = (value >> 40) & 0xFF;
            (*dst)[3] = (value >> 32) & 0xFF;
            (*dst)[4] = (value >> 24) & 0xFF;
            (*dst)[5] = (value >> 16) & 0xFF;
            (*dst)[6] = (value >>  8) & 0xFF;
            (*dst)[7] = (value >>  0) & 0xFF;
        } break;
        case LE_ORDER: {
            (*dst)[0] = (value >>  0) & 0xFF;
            (*dst)[1] = (value >>  8) & 0xFF;
            (*dst)[2] = (value >> 16) & 0xFF;
            (*dst)[3] = (value >> 24) & 0xFF;
            (*dst)[4] = (value >> 32) & 0xFF;
            (*dst)[5] = (value >> 40) & 0xFF;
            (*dst)[6] = (value >> 48) & 0xFF;
            (*dst)[7] = (value >> 56) & 0xFF;
        } break;
    }
#endif
    *dst += 8;
}

void *hgl_serialize(void *dst, void *src, const char *fmt, ...)
{
    const char *fmt_start = fmt;

    uint8_t *read_ptr = (uint8_t *) src;
    uint8_t *write_ptr = (uint8_t *) dst;
    ByteOrder byte_order = BE_ORDER;

    /* 
     * This is user to parse numbers.
     */
    static char digit_stack[10] = {0};
    int digit_stack_idx = 0;

    /* 
     * This is to keep track of repeat statement starting positions.
     */
    static Jump jump_stack[HGL_SERIALIZE_JUMP_STACK_HEGIHT] = {0};
    int jump_stack_top = 0;

    /* 
     * This is to keep track of the number of iterations for each repeat statement
     * declared with a format specifier, e.g. "%{DW}".
     */
    static IndexValuePair loop_iter_cache[HGL_SERIALIZE_LOOP_ITER_CACHE_SIZE] = {0};
    int loop_iter_cache_idx = 0;

    va_list args;
    va_start(args, fmt);

    while (*fmt != '\0') {
        
        /* handle repeating block */
        if ((*fmt == '%') || HGL_SERIALIZE_IS_DIGIT(*fmt)) {
            int n_iter = 0;

            if (HGL_SERIALIZE_IS_DIGIT(*fmt)) {
                /* read digts */
                while (HGL_SERIALIZE_IS_DIGIT(*fmt) && digit_stack_idx < 10) {
                    digit_stack[digit_stack_idx] = *fmt;
                    digit_stack_idx++;
                    fmt++;
                } 

                /* convert to integer */
                int coeff = 1;
                while (digit_stack_idx > 0) {
                    digit_stack_idx--;
                    n_iter += coeff * (digit_stack[digit_stack_idx] - '0');
                    coeff *= 10;
                }
            } else {
                /* check if number-of-iterations value has been cached */
                n_iter = -1;
                ptrdiff_t idx = fmt - fmt_start;
                for (int i = 0; i < loop_iter_cache_idx; i++) {
                    if (idx == loop_iter_cache[i].idx) {
                        n_iter = loop_iter_cache[i].n_iter;
                        break;
                    }
                }

                /* No hit? Read next value from variadic args list & add to cache */
                if (n_iter == -1) {
                    n_iter = va_arg(args, int);
                    loop_iter_cache[loop_iter_cache_idx].idx = idx;
                    loop_iter_cache[loop_iter_cache_idx].n_iter = n_iter;
                    loop_iter_cache_idx++;
                    HGL_SERIALIZE_EXPECT(loop_iter_cache_idx < HGL_SERIALIZE_LOOP_ITER_CACHE_SIZE);
                }

                /* Proceed to '{' */
                fmt++;
            }

            HGL_SERIALIZE_EXPECT(*fmt == '{');
            
            if (n_iter == 0) {
                int n_open_curlies = 1;
                fmt++;
                while (n_open_curlies > 0) {
                    HGL_SERIALIZE_EXPECT(*fmt != '\0');
                    if (*fmt == '{') n_open_curlies++;
                    if (*fmt == '}') n_open_curlies--;
                    fmt++;
                }
            } else {
                /* add entry to jump stack */
                jump_stack[jump_stack_top].n_iterations = n_iter;
                jump_stack[jump_stack_top].block_start = fmt;
                jump_stack_top++;
                fmt++;
            }
            continue;
        }

        switch (*fmt) {
            /* ignore whitespace*/
            case ' ': case '\n': case '\r': case '\t': {
                fmt++;
            } break;

            /* handle jump-backs */
            case '}': {
                HGL_SERIALIZE_EXPECT(jump_stack_top > 0);
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
                    HGL_SERIALIZE_EXPECT(0); // fail.
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
                            HGL_SERIALIZE_EXPECT(HGL_SERIALIZE_IS_HEX_DIGIT(fmt[0]));
                            HGL_SERIALIZE_EXPECT(HGL_SERIALIZE_IS_HEX_DIGIT(fmt[1]));
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
                            HGL_SERIALIZE_EXPECT(HGL_SERIALIZE_IS_HEX_DIGIT(fmt[0]));
                            HGL_SERIALIZE_EXPECT(HGL_SERIALIZE_IS_HEX_DIGIT(fmt[1]));
                            char high_nibble = *fmt++;
                            char low_nibble = *fmt++;
                            to_write = (nibble_to_int_(high_nibble) << 4) +
                                        nibble_to_int_(low_nibble);
                            write_8_(&write_ptr, to_write); 
                        }
                        fmt++;
                    } break;

                    default:
                        HGL_SERIALIZE_EXPECT(0);
                } 
            } break;

            /* move read pointer */
            case '<': {
                size_t offset = 0;
                fmt++;
                while (fmt[0] != '>') {
                    HGL_SERIALIZE_EXPECT(HGL_SERIALIZE_IS_HEX_DIGIT(fmt[0]));
                    HGL_SERIALIZE_EXPECT(HGL_SERIALIZE_IS_HEX_DIGIT(fmt[1]));
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
                    HGL_SERIALIZE_EXPECT(actual == expected);
                }
                fmt++;
            } break;

            /* expect numeric (hex) constant literal */
            case '#': {
                uint8_t actual = 0;
                uint8_t expected = 0;

                fmt++;
                while (*fmt != '#') {
                    HGL_SERIALIZE_EXPECT(HGL_SERIALIZE_IS_HEX_DIGIT(fmt[0]));
                    HGL_SERIALIZE_EXPECT(HGL_SERIALIZE_IS_HEX_DIGIT(fmt[1]));
                    char high_nibble = *fmt++;
                    char low_nibble = *fmt++;
                    expected = (nibble_to_int_(high_nibble) << 4) +
                                nibble_to_int_(low_nibble);
                    actual = read_8_(&read_ptr); 
                    HGL_SERIALIZE_EXPECT(actual == expected);
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
                HGL_SERIALIZE_EXPECT(fmt[1] == 'W'); 
                write_32_(&write_ptr, read_32_(&read_ptr, byte_order)); 
                fmt += 2; 
                break;
            case 'Q': 
                HGL_SERIALIZE_EXPECT(fmt[1] == 'W'); 
                write_64_(&write_ptr, read_64_(&read_ptr, byte_order)); 
                fmt += 2; 
                break;

            /* unknown character */
            default:
                HGL_SERIALIZE_EXPECT(0);
        }
    }
   
    va_end(args);

    return (void *) read_ptr;
}

#endif /* HGL_SERIALIZE_IMPLEMENTATION */

