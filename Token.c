#include "Scheme.h"
#include "Internal.h"

#include <string.h>
#include <stdlib.h>

Token *Token_Create(TokenType type, const char *string)
{
    Token *newToken = malloc (sizeof *newToken);
    newToken->type = type;
    newToken->string = malloc( strlen(string) + 1 );
    strcpy(newToken->string, string);

    return newToken;
}

void Token_Destroy(Token* token) {
    if (token->string) {
        free(token->string);
        free(token);
    } else {
        printf("Token error - destroying token that was never created.");
        exit(-1);
    }
}

