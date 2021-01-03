#ifndef _TYLER_SCHEME_H
#define _TYLER_SCHEME_H

#include <lang/types.h>

typedef enum TokenType {
    TOKEN_TYPE_IDENTIFIER,
    TOKEN_TYPE_BOOLEAN,
    TOKEN_TYPE_CHARACTER,
    TOKEN_TYPE_STRING,
    TOKEN_TYPE_LEFT_PARENTHESIS,
    TOKEN_TYPE_RIGHT_PARENTHESIS,
    TOKEN_TYPE_COMMA,
    TOKEN_TYPE_FIXED_POINT_NUMBER,
    TOKEN_TYPE_FLOATING_POINT_NUMBER,
    TOKEN_TYPE_EMPTY_LIST,
    NUM_TOKEN_TYPES
} TokenType;

typedef struct Token Token;

#endif