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

/* primitive functions */
/**
 * @brief skip spaces in a file stream.
 */
void skip_space(FILE * input);

/**
 * @brief check if character c is the beginning of a comment.
 */
int check_comment(FILE * input, FILE * output, int c, int comment);

/**
 * @brief check if character c is the beginning of a quoted string.
 */
int check_quoted_strings(FILE * input, FILE * output, int c);


/* stream processing functions */
void rip(FILE * input, FILE * output, int comment);
int rip_file(char *filename, int comment);

void print_help();

int main(int argc, char *const *argv)
{
    int c;
    int comment = 0;
    int main_loop = 0;

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
        main_loop = 1;
        if ((r = rip_file(argv[index], comment)) != 0) {
            fprintf(stderr, "Error occured while processing file %s.\n",
                    argv[index]);
        };
    }
    if (!main_loop) {
        rip(stdin, stdout, comment);
        if (ferror(stdin) || ferror(stdout)) {
            fprintf(stderr, "Error occured while processing the file.\n");
            exit(1);
        }
    }

    exit(r);
}

int rip_file(char *filename, int comment)
{
    int c;
    FILE *input;
    FILE *tmp;

    if ((input = fopen(filename, "r")) == NULL) {
        fprintf(stderr, "Cannot open input file %s: %s\n",
                filename, strerror(errno));
        return 2;
    }

    if ((tmp = tmpfile()) == NULL) {
        fprintf(stderr, "Could not create the temporary file: %s\n",
                strerror(errno));
        return 2;
    }

    /* The actual processing function */
    rip(input, tmp, comment);

    if (ferror(input) || ferror(tmp)) {
        fprintf(stderr, "Error occured while processing the file.\n");
        return 2;
    }

    if (fclose(input)) {
        fprintf(stderr, "Could not close the input stream: %s\n",
                strerror(errno));
        return 2;
    }

    if ((input = fopen(filename, "w")) == NULL) {
        fprintf(stderr, "Cannot open input file %s: %s\n",
                filename, strerror(errno));
        return 2;
    }

    /* actual copying process */
    rewind(tmp);
    while ((c = fgetc(tmp)) != EOF) {
        fputc(c, input);
    }
    fputc('\n', input);

    if (ferror(input) || ferror(tmp)) {
        fprintf(stderr, "Error occured while processing the file.\n");
        return 1;
    }

    if (fclose(tmp)) {
        fprintf(stderr, "Could not close the temporary stream: %s\n",
                strerror(errno));
        return 1;
    }

    return 0;
}

void rip(FILE * input, FILE * output, int comment)
{
    /* Loop through all characters */
    int c;
    while ((c = fgetc(input)) != EOF) {
        if (check_quoted_strings(input, output, c)) {
            continue;
        }

        if (check_comment(input, output, c, comment)) {
            continue;
        }

        /* Check if the current character is a token character */
        if (strchr(";:,{}() ", c)) {
            int token = c;
            skip_space(input);
            fputc(token, output);
        } else {
            fputc(c, output);
        }
    }

}

void print_help()
{
    puts("Usage:\tunformatter [options...] [input files...]");
    puts("Remove unnecessary white-spaces and optionally comments in your");
    puts("C source code.");
    puts("");
    puts("Options");
    puts("\t-h\t\tshow this help text and exit");
    puts("\t-c\t\tkeep the comments.");
    puts("");
    puts("This program defaults to the standard input/output, if you don't");
    puts("specify an input file.");
    puts("");
    puts("Note: C++ style comments will force a line break.");
    puts("");
    exit(0);
}

void skip_space(FILE * input)
{
    int c;
    while (isspace(c = fgetc(input)));
    if (isprint(c)) {
        ungetc(c, input);
    }
}

int check_comment(FILE * input, FILE * output, int c, int comment)
{
    /* Comment handling */
    if (c == '/') {
        c = fgetc(input);
        /* C++ style */
        if (c == '/') {
            if (comment) {
                fputc('/', output);
                fputc('/', output);
            }
            while ((c = fgetc(input)) != '\n') {
                if (comment) {
                    fputc(c, output);
                }
                if (c == EOF) {
                    break;
                }
            }
            if (comment) {
                fputc('\n', output);
            }
            skip_space(input);
            /* C style */
        } else if (c == '*') {
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
                if ( (s[0] == EOF) || (s[1] == EOF) ) {
                    break;
                }
            } while (strcmp(s, "*/"));
            fgetc(input);
            if (comment) {
                fputc('/', output);
            }
            skip_space(input);
            /* Not comment */
        } else {
            fputc('/', output);
            fputc(c, output);
        }
        return 1;
    }
    return 0;
}

int check_quoted_strings(FILE * input, FILE * output, int c)
{
    /* Simon's code */
    if (c == '"' || c == '\'') {
        char chrEnd = c;
        fputc(c, output);
        do {
            c = fgetc(input);
            fputc(c, output);
            if (c == '\\') {
                c = fgetc(input);
                fputc(c, output);
            } else if (c == chrEnd) {
                break;
            }
        }
        while (c != EOF);
        return 1;
    }
    return 0;
}
