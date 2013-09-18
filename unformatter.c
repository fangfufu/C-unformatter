/**
 * @file unformatter.c
 * @brief C-unformatter -- removes the whitespace and comment in your C code
 * @details
 * - inspired by Ralph's Facebook comment... Massively increase the
 * masculinity of your C source code.
 * - apologies for spaghetti code -- I didn't think it would grow this big.
 */

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void rip(FILE* in, FILE* out, int comment);
void print_help();
int main(int argc, char* const* argv)
{
    FILE* in = stdin;
    FILE* out = stdout;
    int c;
    int comment = 0;

    if (argc == 1) {
        print_help();
    }

    while ((c = getopt(argc, argv, "chi:o:")) != -1) {
        switch (c) {
            case 'h':
                print_help();
                break;
            case 'c':
                comment = 1;
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
    rip(in, out, comment);

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

void rip(FILE* in, FILE* out, int comment)
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

    /* Loop through all tokens */
    while (*token != '\0') {
        while ( (c = fgetc(tmp1)) != EOF) {

            /* Comment handling */
            if (c == '/') {
                c = fgetc(tmp1);
                /* C++ style */
                if (c == '/') {
                    if (comment) {
                        fputc('/', tmp2);
                        fputc('/', tmp2);
                    }
                    while ( (c = fgetc(tmp1)) != '\n') {
                        if (comment) {
                            fputc(c, tmp2);
                        }
                    }
                    if(comment) {
                        fputc('\n', tmp2);
                    }
                /* C style */
                } else if (c == '*'){
                    if (comment) {
                        fputc('/', tmp2);
                        fputc('*', tmp2);
                    }
                    char s[3];
                    s[2] = 0;
                    do {
                        s[0] = fgetc(tmp1);
                        s[1] = fgetc(tmp1);
                        ungetc(s[1], tmp1);
                        if (comment) {
                            fputc(s[0], tmp2);
                        }
                    } while(strcmp(s, "*/"));
                    fgetc(tmp1);
                    if (comment) {
                        fputc('/', tmp2);
                    }
                    /* Remove the white spaces after the comment */
                    while(isspace(c = fgetc(tmp1)))
                        ;
                    if (isprint(c)) {
                        ungetc(c, tmp1);
                    }
                /* Not comment */
                } else {
                    fputc('/', tmp2);
                    fputc(c, tmp2);
                }
                continue;
            }

            /* Token character check */
            if ( c == *token ) {
                d = c;
                while(isspace(c = fgetc(tmp1)))
                    ;
                fputc(d, tmp2);
                if (isprint(c)) {
                    ungetc(c, tmp1);
                }
            /* Not a token character */
            } else {
                fputc(c, tmp2);
            }
        }

        /* clean up the temporary file streams before entering the next loop */
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
    puts("Remove unnecessary white-spaces and optionally comments in your");
    puts("C source code.");
    puts("");
    puts("Options");
    puts("\t-h\t\tshow this help text and exit");
    puts("\t-c\t\tkeep the comments.");
    puts("\t-i\t\tthe input source file");
    puts("\t-o\t\ttheoutput source file");
    puts("");
    puts("This program will use the standard input/output, if you don't");
    puts("specify an input and/or an output file.");
    puts("");
    puts("Note: C++ style comments will force a line break.");
    puts("");
    exit(0);
}
