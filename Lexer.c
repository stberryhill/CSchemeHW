#include "Scheme.h"
#include "Internal.h"
#include <QuickFile.h>
#include <stdlib.h>
#include <lang/types.h>
#include <string.h>

static const uint8 MAX_TOKEN_LENGTH = 255;

static const char delimeters[] = {" ()[]"};
static const int numDelimeters = 2;

static bool isNumeral(const char c) {
    if (c < '0' || c > '9') {
        return false;
    }

    return true;
}

static bool isLetter(const char c) {
    if (c >= 'a' && c <= 'z') {
        return true;
    }

    if (c >= 'A' && c <= 'Z') {
        return true;
    }

    return false;
}

static bool isAlphanumeral(const char c) { 
    return isLetter(c) || isNumeral(c);
}

static bool isDelimeter(const char c) {
    if (c == '(' || c == ')' || c == ' ' || c == QF_EOF)
        return true;
    
    return false;
}

static bool isValidIdentifierCharacter(const char c) {
    return !isDelimeter(c);
}

static void consumeWhitespace(QfFile *file) {
    char c;
    do {
        c = qf_ReadCharacter(file);
        printf("consuming %c\n", c);
    } while (c == ' ' || c == '\t' || c == '\n');

    qf_Rewind(file, 1);
    printf("rewind 1\n");
}

static void ensureMatch(const char actual, const char expected) {
    if (actual != expected) {
        printf("Lexer - failed to ensure match");
        exit(-1);
    }
}

static void readString(TokenList *tokens, QfFile *file) {
    char c = qf_ReadCharacter(file);
    char string[MAX_TOKEN_LENGTH + 1];
    int i = 0;

    while (c != '"') {
        if (c == QF_EOF) {
            printf("Lexer error - expected \" to complete string.\n");
            exit(-1);
        }

        if (c == '\\') {
            c = qf_ReadCharacter(file);
            switch (c) {
                case 'n':
                    c = '\n';
                break;
                
                case 't':
                    c = '\t';
                break;

                case 'r':
                    c = '\r';
                break;

                case '\\':
                    c = '\\';
                break;

                default:
                    printf("Lexer error - cannot backslash escape \\%c\n", c);
                    exit(-1);
                break;
            }
        }

        string[i] = c;
        c = qf_ReadCharacter(file);
        i++;

        if (i >= MAX_TOKEN_LENGTH) {
            printf("Lexer error - string length is greater than max token length, %s\n", string);
            exit(-1);
        }
    }

    string[i] = '\0';

    TokenList_AddNewToken(tokens, TOKEN_TYPE_STRING, string);
}

static void readNumber(TokenList *tokens, QfFile *file) {
    const static int MAX_DIGITS = 32;
    char string[MAX_DIGITS + 1];
    double value = 0;
    double sign = 1.0;
    int i = 0;
    char c;

    c = qf_ReadCharacter(file);
    if (c == '-') {
        sign = -1.0;
    } else {
        qf_Rewind(file, 1);
    }

    while ( isNumeral((c = qf_ReadCharacter(file))) ) {
        if (i > MAX_DIGITS) {
            printf("Lexer error - number has too many didigts - max digits is %d\n", MAX_DIGITS);
            exit (-1);
        }
        value = (value * 10) + (c - '0');
        string[i] = c;
        i++;
    }

    if (c == '.' && i < MAX_DIGITS) {
        /* it's a floating-point number */
        string[i] = c;
        i++;

        double place = 1;

        while ( isNumeral( (c = qf_ReadCharacter(file)) ) ) {
            if (i > MAX_DIGITS) {
                printf("Lexer error - number has too many digts. Max digits is %d\n", MAX_DIGITS);
                exit (-1);
            }
            place *= 10;
            value += (c - '0') / place;
            string[i] = c;
            i++;
        }

        if ( !isDelimeter(c) ) {
            string[i] = '\0';
            printf("Lexer error - expected delimeter after %s\n", string);
            exit(-1);
        }

        string[i] = '\0';
        Token *newToken = TokenList_AddNewToken(tokens, TOKEN_TYPE_FLOATING_POINT_NUMBER, string);
        newToken->data.floatingPointNumber = sign * value;

    } else if ( !isDelimeter(c) ) {
        string[i] = '\0';
        printf("Lexer error - expected delimeter after %s\n", string);
        exit(-1);

    } else {
        /* It's a fixed-point number */
        string[i] = '\0';
        Token *newToken = TokenList_AddNewToken(tokens, TOKEN_TYPE_FIXED_POINT_NUMBER, string);
        newToken->data.fixedPointNumber = (long) (sign * value);
    }
}

static Token *readIdentifier(QfFile *file) {
    char word[MAX_TOKEN_LENGTH];
    uint16 pos = 0;
    
    char c;

    do {
        c = qf_ReadCharacter(file);
        word[pos] = c;
        pos++;

        if (pos >= MAX_TOKEN_LENGTH) {
            printf ("Lexer error - token larger than max supported size (%d)\n", MAX_TOKEN_LENGTH - 1);
            exit(-1);
        }
    } while (isValidIdentifierCharacter(c));
    
    word[pos] = '\0';

    return word;
}

static void ensureReadMatchesDelimetedString(QfFile *file, const char *expected) {
    int expectedLength = strlen(expected);
    char c;

    int i;
    for (i = 0; i < expectedLength; i++) {
        c = qf_ReadCharacter(file);
        if (c != expected[i]) { 
            printf("Lexer error - expected %c but got %c\n", expected[i], c);
            exit(-1);
        }
    }

    printf("mach current char=%c, next char=%c\n", c, qf_PeekCharacter(file));

    if ( !isDelimeter(qf_PeekCharacter(file)) ) {
        printf("Lexer error - expected delimeter to complete match %s\n", expected);
        exit(-1);
    }
}

TokenList *Lexer_AnalyzeAndCreateTokenList(const char *filePath) {
    QfFile *file = qf_OpenFile(filePath, QF_FILE_MODE_READ);
    TokenList *tokens = TokenList_Create();
    char c;

    consumeWhitespace(file);

    while ( !qf_ReachedEndOfFile(file) ) {
        c = qf_ReadCharacter(file);
        printf("%c\n", c);

        if (c == '#') {
            c = qf_ReadCharacter(file);
            printf("%c\n", c);

            if (c == 't') {
                Token *newToken = TokenList_AddNewToken(tokens, TOKEN_TYPE_BOOLEAN, "#t");
                newToken->data.booleanValue = true;
            } else if (c == 'f') {
                Token *newToken = TokenList_AddNewToken(tokens, TOKEN_TYPE_BOOLEAN, "#f");
                newToken->data.booleanValue = false;
            } else if (c == '\\') {
                printf("reading char\n");
                c = qf_ReadCharacter(file);
                printf("%c\n", c);

                if ( isDelimeter(qf_PeekCharacter(file)) ) {
                    qf_ReadCharacter(file);
                    printf("made it!\n");
                    char string[4] = "#\\";
                    string[2] = c;
                    Token *newToken = TokenList_AddNewToken(tokens, TOKEN_TYPE_CHARACTER, string);
                    newToken->data.character = c;
                } else {
                    printf("special character\n");
                    qf_Rewind(file, 1);
                    printf("peek %c\n", qf_PeekCharacter(file));
                    printf("peek %c\n", qf_PeekCharacter(file));

                    switch(qf_PeekCharacter(file)) {
                        case 's':
                            ensureReadMatchesDelimetedString(file, "space");
                            Token *spaceToken = TokenList_AddNewToken(tokens, TOKEN_TYPE_CHARACTER, "#\\space");
                            spaceToken->data.character = ' ';
                        break;
                        case 't':
                            ensureReadMatchesDelimetedString(file, "tab");
                            Token *tabToken = TokenList_AddNewToken(tokens, TOKEN_TYPE_CHARACTER, "#\\tab");
                            tabToken->data.character = '\t';
                        break;
                        case 'n':
                            printf("newline\n");
                            ensureReadMatchesDelimetedString(file, "newline");
                            Token *newlineToken = TokenList_AddNewToken(tokens, TOKEN_TYPE_CHARACTER, "#\\newline");
                            newlineToken->data.character = '\n';
                        break;
                        default:
                            printf("Lexer error - invalid character  - expected special character\n");
                            exit(-1);
                        break;
                    }
                }
            }
        } else if ( isNumeral(c) || (c == '-' && isNumeral(qf_PeekCharacter(file))) ) {
            qf_Rewind(file, 1);
            readNumber(tokens, file);
        } else if (c == '"') {
            readString(tokens, file);
        } else if (c == '(') {
            if (qf_ReadCharacter(file) == ')') {
                TokenList_AddNewToken(tokens, TOKEN_TYPE_EMPTY_LIST, "()");
            } else {
                printf("Lexer error - lists not yet supported\n");
                exit(-1);
            }
        } else {
            printf("Unsupported synax.\n");
            exit(-1);
        }

        consumeWhitespace(file);
    }

    return tokens;
}