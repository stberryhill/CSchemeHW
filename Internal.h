/*
 * author: S. Tyler Berryhill
 * email: sean.tyler.berryhill@gmail.com
 */

#ifndef _TYLER_SCHEME_INTERNAL_H
#define _TYLER_SCHEME_INTERNAL_H

#include "Scheme.h"
#include <lang/types.h>
#include <QuickFile.h>

typedef enum Keyword {
    KEYWORD_DEFINE,
    KEYWORD_LAMBDA,
    NUM_KEYWORDS
} Keyword;

typedef struct TokenList {
    struct Token *head;
    int size;
} TokenList;

struct Token {
    TokenType type;
    char *string;
    Token *next;
    Token *previous;

    union {
        double floatingPointNumber;
        long fixedPointNumber;
        char character;
        bool booleanValue;
        Keyword keyword;
    } data;
};

/* LL(1) recursive-descent lexer functions */
void Lexer_Create();
void Lexer_StartWithFile(const char *filePath);
void Lexer_Stop();
TokenList *Lexer_AnalyzeAndCreateTokenList(const char *filePath);
Token Lexer_NextToken();

/* TokenList (doubly linked list) */
TokenList *TokenList_Create();
void TokenList_Destroy(TokenList *list);
Token *TokenList_GetHead(TokenList *list);
int TokenList_GetSize(TokenList *list);
Token *TokenList_AddNewToken(TokenList *list, const TokenType type, const char *string);

/* Token creation */
Token *Token_Create(TokenType type, const char *string);
void Token_Destroy(Token* token);
const char *Token_GetName(const Token *input);

#endif