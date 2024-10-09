#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include "stb_image.h"
#include "frau.h"

#define _CRT_SECURE_NO_WARNINGS

typedef enum Lang
{
    LANG_C,
    LANG_CPP,
    LANG_CS,
    LANG_JAVA,
    LANG_GO,
    LANG_RS,
    LANG_JS,
    LANG_PY
} Lang;

typedef enum Flag
{
    FLAG_LANG = 1 << 0,
    FLAG_FILE = 1 << 1
} Flag;

bool ends_with(const char *const str, const char *const with)
{
    if(str == NULL) return false;
    if(with == NULL) return false;
    if(strlen(with) > strlen(str)) return false;

    for(size_t i = 0; i < strlen(with); ++i) {
        size_t idx = strlen(str) - strlen(with) + i;
        if(str[idx] != with[i]) return false;
    }

    return true;
}

int main(int argc, char **argv)
{
    // Skipping over the program name
    argc--;
    argv++;

    if(argc < 1) {
        usage();
        return 0;
    }

    Lang lang = LANG_C;
    char filename[2048] = {0};
    FILE *file;
    bool img = 0;

    Flag flag = 0;

    for(int i = 0; i < argc; ++i) {
        if(strcmp(argv[i], "-lang") == 0) {
            flag |= FLAG_LANG;

            ++i;
            if(i >= argc) {
                fprintf(stderr, "frau: error: no language specified\n");
                usage();
                return -1;
            }

            if(strcmp(argv[i], "c") == 0) {
                lang = LANG_C;
            } else
            if(strcmp(argv[i], "cpp") == 0) {
                lang = LANG_CPP;
            } else
            if(strcmp(argv[i], "cs") == 0) {
                lang = LANG_CS;
            } else
            if(strcmp(argv[i], "py") == 0) {
                lang = LANG_PY;
            } else 
            if(strcmp(argv[i], "java") == 0)  {
                lang = LANG_JAVA;
            } else
            if(strcmp(argv[i], "go") == 0) {
                lang = LANG_GO;
            } else
            if(strcmp(argv[i], "js") == 0) {
                lang = LANG_JS;
            } else 
            if(strcmp(argv[i], "rs") == 0) {
                lang = LANG_RS;
            } else {
                fprintf(stderr, "frau: error: language not implemented or invalid\n");
                usage();
                return -1;
            }
        } else
        if(strcmp(argv[i], "--help") == 0) {
            usage();
            return 0;
        } else {
            // Can you blame me?
            file = fopen(argv[i], "rb");
            if(!file) {
                fprintf(stderr, "frau: error: no such file or directory '%s'\n", argv[i]);
                return -1;
            }

            if(strlen(argv[i])+1 > sizeof(filename)) {
                fprintf(stderr, "frau: error: file name too large\n");
                return -1;
            }

            (void)memcpy(filename, argv[i], strlen(argv[i]));
            
            char* formats[] = {
                ".jpg",
                ".jpeg",
                ".tga",
                ".bmp",
                ".psd",
                ".hdr",
                ".pic",
                ".ppm",
                ".pgm",
                ".png"
            };
            for(int j = 0; j < sizeof(formats)/sizeof(formats[0]); ++j) {
                if(ends_with(argv[i], formats[j])) {
                    img = true;
                    fclose(file);
                    file = NULL;
                    break;
                }
            }

            flag |= FLAG_FILE;
        }
    }

    if(!(flag & FLAG_FILE)) {
        fprintf(stderr, "frau: error: no file specified\n");
        usage();
        exit(-1);
    }

    unsigned char *bytes;
    char *charbytes;
    size_t pixelcount = 0;
    int imwidth, imheight, imchannels;
   
    if(!file) {
        bytes = stbi_load(filename, &imwidth, &imheight, &imchannels, 0);
        pixelcount = (size_t)imwidth*(size_t)imheight;
    } else {
        size_t sz;

        fseek(file, 0, SEEK_END);
        sz = ftell(file);

        fseek(file, 0, SEEK_SET);

        charbytes = malloc(sz);

        int ch;
        while((ch = fgetc(file)) != EOF) {
            charbytes[pixelcount++] = ch;
        }
    }

    if(pixelcount < 1) return 0;

    putc('\n', stdout);
    switch(lang) {
        case LANG_C: {
            printf("unsigned char bytes[%zu] = {\n", pixelcount);
        } break;

        case LANG_CPP: {
            printf("std::array<unsigned char, %zu> bytes = {\n", pixelcount);
        } break;

        case LANG_PY: {
            printf("bytes = [\n");
        } break;

        case LANG_CS: {
            printf("byte[] bytes = new byte[] {\n");
        } break;

        case LANG_JAVA: {
            printf("char[] bytes = new char[] {\n");
        } break;

        case LANG_GO: {
            printf("bytes := []byte {\n");
        } break;

        case LANG_JS: {
            printf("var bytes = [\n");
        } break;

        case LANG_RS: {
            printf("let bytes = [\n");
        } break;
                      

        default: {
            assert(0 && "unreachable");
        } break;
    }

    if(file) {
// somehow packing rgba values into an uint32_t didn't work so i stuck with this
#define FORMAT "0x%02hhX%02hhX%02hhX%02hhX"
        // pixelcount doesn't have anything to do with pixels here, it's just the number of bytes in the file
        for(size_t i = 0; i < pixelcount; ++i) {
            char byte = charbytes[i];

            if((i+1) >= pixelcount) {
                printf("0x%hhX", byte);
            } else {
                printf("0x%hhX, ", byte);
            }
        }
        free(charbytes);
    } else {
        for(size_t i = 0; i < imwidth; ++i) {
            for(size_t j = 0; j < imheight; ++j) {
                unsigned char *pixel = bytes + (j + imwidth*i) * imchannels;
                uint8_t r = pixel[0];
                uint8_t g = pixel[1];
                uint8_t b = pixel[2];
                uint8_t a = (imchannels >= 4) ? pixel[3] : 0xFF;

                if(((j+1)+imwidth*i) >= pixelcount) {
                    printf(FORMAT, r,g,b,a);
                } else {
                    printf(FORMAT", ", r,g,b,a);
                }
#undef FORMAT
            }     
        }
        free(bytes);
    }

    switch(lang) {
        case LANG_CPP:
        case LANG_C:
        case LANG_CS: 
        case LANG_JAVA: { 
            printf("\n};\n");
        } break;
        case LANG_GO: {
            printf("\n}\n");
        } break;
        case LANG_RS: {
            printf("\n];\n");
        } break;
        case LANG_JS:
        case LANG_PY: {
            printf("\n]\n");
        } break; 
        default: {
            assert(false && "unreachable");
        } break;
    }

    // If no language was specified
    if(!(flag & FLAG_LANG)) {
        printf("no -lang value specified: defaulted to `c`\n");
    }

    if(img) {
        printf("\nwidth: %d\n", imwidth);
        printf("height: %d\n", imheight);
    }

    return 0;
}

void usage()
{
    printf("Usage: frau <file> [options]\n\n");
    printf("Options: \n");
    printf(" -lang <value>\t Specifies in which language to create a byte representation of the file\n");
    printf("\t\tvalue: c (default), cpp, cs, java, go, rs, js, py\n\n");
    printf(" --help\t Shows this message\n");
    
    putc('\n', stdout);
}
