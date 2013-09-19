/**
 * @file unformatter.c
 * @brief C-unformatter -- removes the whitespace and comment in your C code
 */

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

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
int check_quoted(FILE * input, FILE * output, int c);

/**
 * @brief check if character c is the start of a preprocessor statement.
 */
int check_preprocessor_statements(FILE * input, FILE * output, int c);

/**
 * @brief rip stuff from an input stream, output to another stream.
 */
void rip(FILE * input, FILE * output, int comment);

/**
 * @brief rip stuff from a file.
 */
int rip_file(char *filename, int comment);

/**
 * @brief print out help statements
 */
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
    int c;
    /* Loop through all characters */
    while ((c = fgetc(input)) != EOF) {
        if (check_quoted(input, output, c)) {
            continue;
        }

        if (check_comment(input, output, c, comment)) {
            continue;
        }

        if (check_preprocessor_statements(input, output, c)) {
            continue;
        }

        /* Check if the current character is a token character. Tokens include
         * most of the C operators. The idea is that you don't put a space after
         * a token. */
        if (strchr("=+-*/%><!~&.?:|^;,{}()", c) || isspace(c)) {
            int token = c;
            if (isspace(token)) {
                token = ' ';
            }
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
        /* C++ style comment*/
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
            /* C style comment*/
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
            /* Not a comment */
        } else {
            fputc('/', output);
            fputc(c, output);
        }
        return 1;
    }
    return 0;
}

int check_quoted(FILE * input, FILE * output, int c)
{
    /* Simon's code */
    if (c == '"' || c == '\'') {
        int chrEnd = c;
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

int check_preprocessor_statements(FILE * input, FILE * output, int c)
{
    if (c == '#') {
        fputc('#', output);
        while( (c = fgetc(input)) != EOF ) {
            fputc(c, output);
            if (c == '\n') {
                break;
            }
        }
        return 1;
    }
    return 0;
}
