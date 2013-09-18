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

void rip(FILE* file, int comment);
void print_help();
int main(int argc, char* const* argv)
{
    FILE* file = stdin;
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
        }
    }

    int index;
    for (index = optind; index < argc; index++) {
        file = fopen(argv[index], "r+");
        if(file == NULL) {
            fprintf(stderr, "Cannot open input file %s: %s\n",
                    argv[index], strerror(errno));
            exit(2);
        }
        rip(file, comment);
        if (ferror(file)) {
            fprintf(stderr, "Error occured while reading the input.\n");
            exit(1);
        }
    }
    exit(0);
}

void rip(FILE* file, int comment)
{
    int c, d;

    FILE* tmp;

    if ( (tmp = tmpfile()) == NULL )  {
        fprintf(stderr, "Could not create the temporary file: %s",
                strerror(errno));
    }

    /* Loop through all tokens */
        while ( (c = fgetc(file)) != EOF) {

            /* Comment handling */
            if (c == '/') {
                c = fgetc(file);
                /* C++ style */
                if (c == '/') {
                    if (comment) {
                        fputc('/', tmp);
                        fputc('/', tmp);
                    }
                    while ( (c = fgetc(file)) != '\n') {
                        if (comment) {
                            fputc(c, tmp);
                        }
                    }
                    if(comment) {
                        fputc('\n', tmp);
                    }
                    /* Remove the white spaces after the comment */
                    while(isspace(c = fgetc(file)))
                        ;
                    if (isprint(c)) {
                        ungetc(c, file);
                    }
                /* C style */
                } else if (c == '*'){
                    if (comment) {
                        fputc('/', tmp);
                        fputc('*', tmp);
                    }
                    char s[3];
                    s[2] = 0;
                    do {
                        s[0] = fgetc(file);
                        s[1] = fgetc(file);
                        ungetc(s[1], file);
                        if (comment) {
                            fputc(s[0], tmp);
                        }
                    } while(strcmp(s, "*/"));
                    fgetc(file);
                    if (comment) {
                        fputc('/', tmp);
                    }
                    /* Remove the white spaces after the comment */
                    while(isspace(c = fgetc(file)))
                        ;
                    if (isprint(c)) {
                        ungetc(c, file);
                    }
                /* Not comment */
                } else {
                    fputc('/', tmp);
                    fputc(c, tmp);
                }
                continue;
            }

            /** Token character check
             * @todo add space to the list!*/
            if ( (c == ';') || (c == ':') || (c == ',')|| (c == '{') || (c == '}') || (c == '(') || (c == ')') ) {
                d = c;
                while(isspace(c = fgetc(file)))
                    ;
                fputc(d, tmp);
                if (isprint(c)) {
                    ungetc(c, file);
                }
            /* Not a token character */
            } else {
                fputc(c, tmp);
            }
        }

        /* clean up the temporary file streams before entering the next loop */
//         if (fclose(file)) {
//             fprintf(stderr, "Error occured when closing the input file: %s\n",
//                     strerror(errno));
//         }

        rewind(tmp);

        /* copy the data back*/
        rewind(file);
        while ( (c = fgetc(tmp)) != EOF ) {
            fputc(c, file);
        }
        fputc('\n', file);
}

void print_help()
{
    puts("Usage:\tunformatter [options...] [files...]");
    puts("Remove unnecessary white-spaces and optionally comments in your");
    puts("C source code.");
    puts("");
    puts("Options");
    puts("\t-h\t\tshow this help text and exit");
    puts("\t-c\t\tkeep the comments.");
    puts("");
    puts("Note: C++ style comments will force a line break.");
    puts("");
    exit(0);
}
