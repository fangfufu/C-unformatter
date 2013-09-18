/**
 * @file unformatter.c
 * @brief C-unformatter -- removes the whitespace and comment in your C code
 * @details inspired by Ralph's Facebook comment... Massively increase the
 * masculinity of your C source code.
 */

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void rip(FILE* in, FILE* out);
void print_help();
int main(int argc, char* const* argv)
{
    FILE* in = stdin;
    FILE* out = stdout;
    int c;

    if (argc == 1) {
        print_help();
    }

    while ((c = getopt(argc, argv, "hi:o:")) != -1) {
        switch (c) {
            case 'h':
                print_help();
                break;
            case 'i':
                in = fopen(optarg, "r");
                if(in == NULL) {
                    fprintf(stderr, "Cannot open input file %s: %s\n",
                           optarg, strerror(errno));
                    exit(2);
                }
                break;
            case 'o':
                out = fopen(optarg, "w");
                if(out == NULL) {
                    fprintf(stderr, "Cannot open output file %s: %s\n",
                           optarg, strerror(errno));
                    exit(2);
                }
                break;
        }
    }

    /* The actual processing function */
    rip(in, out);

    if (ferror(in)) {
        fprintf(stderr, "Error occured while reading the input.\n");
        exit(1);
    }

    if (ferror(out)) {
        fprintf(stderr, "Error occured while writing to the output.\n");
        exit(1);
    }


    exit(0);
}

void rip(FILE* in, FILE* out)
{
    int c, d;
    char* token_list = ";:,{}() ";
    char* token = token_list;
    FILE* tmp1 = tmpfile();
    FILE* tmp2 = tmpfile();

    if ( (tmp1 == NULL) || (tmp2 == NULL) ) {
        fprintf(stderr, "Could not create temporary files: %s",
                strerror(errno));
    }

    /* copy data to the temporary file stream*/
    while ( (c = fgetc(in)) != EOF ) {
        fputc(c, tmp1);
    }
    rewind(tmp1);

    while (*token != '\0') {
        while ( (c = fgetc(tmp1)) != EOF) {
            /* Skip C++ style comment */
            if (c == '/') {
                fputc('/', tmp2);
                c = fgetc(tmp1);
                if (c == '/') {
                    fputc('/', tmp2);
                    while ( (c = fgetc(tmp1)) != '\n') {
                        fputc(c, tmp2);
                    }
                    fputc('\n', tmp2);
                /* C style comment */
                } else if (c == '*'){
                    fputc('*', tmp2);
                    fprintf(stderr, "1. c: %c, pos %ld\n", c, ftell(tmp1));
                } else {
                    fputc(c, tmp2);
                }
                continue;
            }
            /* check if it is a token character */
            if ( c == *token ) {
                d = c;
                while(isspace(c = fgetc(tmp1)))
                    ;
                fputc(d, tmp2);
                if (isgraph(c)) {
                    ungetc(c, tmp1);
                }
            } else {
                /* the default approach */
                fputc(c, tmp2);
            }
        }

        /* clean up the temporary file streams */
        if (fclose(tmp1)) {
            fprintf(stderr, "Error occured when closing tmp2: %s\n",
                    strerror(errno));
        }
        rewind(tmp2);
        tmp1 = tmp2;
        if ( (tmp2 = tmpfile()) == NULL ) {
            fprintf(stderr, "Could not recreate tmp2: %s",
                        strerror(errno));
        }
        token++;
    }

    /* copy the data back*/
    while ( (c = fgetc(tmp1)) != EOF ) {
        fputc(c, out);
    }
}

void print_help()
{
    puts("Usage:\tunformatter [options...]");
    puts("");
    puts("Options");
    puts("\t-h\t\tshow this help text and exit");
    puts("\t-i\t\tthe input source file");
    puts("\t-o\t\ttheoutput source file");
    puts("");
    puts("This program will use the standard input/output, if you don't");
    puts("specify an input and/or an output file.");
    puts("");
    exit(0);
}
