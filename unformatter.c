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

int rip(char* filename, int comment);
void print_help();
int main(int argc, char* const* argv)
{
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
    int r = 0;
    for (index = optind; index < argc; index++) {
        if ( (r = rip(argv[index], comment)) != 0 ) {
            fprintf(stderr, "Error occured while processing file %s.",
                    argv[index]);
        };
    }
    exit(r);
}

int rip(char* filename, int comment)
{
    int c;
    FILE* input;
    FILE* output;

    if( (input = fopen(filename, "r")) == NULL ) {
        fprintf(stderr, "Cannot open input file %s: %s\n",
                filename, strerror(errno));
        return 2;
    }

    if ( (output = tmpfile()) == NULL )  {
        fprintf(stderr, "Could not create the temporary file: %s\n",
                strerror(errno));
        return 2;
    }

    /* Loop through all characters */
    while ( (c = fgetc(input)) != EOF ) {
        /* Simon's code */
        if ( c == '"' || c == '\'' ) {
            char token = c;
            fputc(c, output);
            do {
                c = fgetc(input);
                fputc(c, output);
                if ( c == '\\' ) {
                    c = fgetc(input);
                    fputc(c, output);
                } else if ( c == token ) {
                    break;
                }
            } while ( c != EOF );
            continue;
        }

        /* Comment handling */
        if (c == '/') {
            c = fgetc(input);
            /* C++ style */
            if (c == '/') {
                if (comment) {
                    fputc('/', output);
                    fputc('/', output);
                }
                while ( (c = fgetc(input)) != '\n') {
                    if (comment) {
                        fputc(c, output);
                    }
                }
                if(comment) {
                    fputc('\n', output);
                }
                /* Remove the white spaces after the comment */
                while(isspace(c = fgetc(input)))
                    ;
                if (isprint(c)) {
                    ungetc(c, input);
                }
            /* C style */
            } else if (c == '*'){
                if (comment) {
                    fputc('/', output);
                    fputc('*', output);
                }
                char s[3];
                s[2] = 0;
                do {
                    s[0] = fgetc(input);
                    s[1] = fgetc(input);
                    ungetc(s[1], input);
                    if (comment) {
                        fputc(s[0], output);
                    }
                } while(strcmp(s, "*/"));
                fgetc(input);
                if (comment) {
                    fputc('/', output);
                }
                /* Remove the white spaces after the comment */
                while(isspace(c = fgetc(input)))
                    ;
                if (isprint(c)) {
                    ungetc(c, input);
                }
            /* Not comment */
            } else {
                fputc('/', output);
                fputc(c, output);
            }
            continue;
        }

        if (strchr(";:,{}() ", c)) {
            char token = c;
            while(isspace(c = fgetc(input)))
                ;
            fputc(token, output);
            if (isprint(c)) {
                ungetc(c, input);
            }
        /* Not a token character */
        } else {
            fputc(c, output);
        }
    }

    if (ferror(input) || ferror(output)) {
        fprintf(stderr, "Error occured while processing the file.\n");
        return 2;
    }

    if (fclose(input)) {
        fprintf(stderr, "Could not close the input stream: %s\n", strerror(errno));
        return 2;
    }

    if( (input = fopen(filename, "w")) == NULL ) {
        fprintf(stderr, "Cannot open input file %s: %s\n",
                filename, strerror(errno));
        return 2;
    }

    /* actual copying process */
    rewind(output);
    while ( (c = fgetc(output)) != EOF ) {
        fputc(c, input);
    }
    fputc('\n', input);

    if (ferror(input) || ferror(output)) {
        fprintf(stderr, "Error occured while processing the file.\n");
        return 1;
    }

    if (fclose(output)) {
        fprintf(stderr, "Could not close the temporary stream: %s\n", strerror(errno));
        return 1;
    }

    return 0;
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
