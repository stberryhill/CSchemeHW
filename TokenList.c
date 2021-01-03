
#include "Scheme.h"
#include "Internal.h"

#include <stdlib.h>

TokenList *TokenList_Create() {
    TokenList *list = malloc(sizeof *list);
    list->head = 0;
    list->size = 0;

    return list;
}

void TokenList_Destroy(TokenList *list) {
    if (list) {
        Token *n = list->head;
        int i;
        for (i = 0; i < list->size; i++) {
            Token *next = n->next;
            Token_Destroy(n);
            n = next;
        }
    }
}
Token * TokenList_GetHead(TokenList *list) {
    return list->head;
}

int TokenList_GetSize(TokenList *list) {
    return list->size;
}

Token *TokenList_AddNewToken(TokenList *list, const TokenType type, const char *string ) {
    Token *newNode = Token_Create(type, string);
    newNode->next = 0;

    if (list->size == 0) {
        newNode->previous = 0;
        list->head = newNode;
        
    } else {
        Token *node = list->head;
        while (node->next) {
            node = node->next;
        }

        newNode->previous = node;
        node->next = newNode;
    }

    list->size++;

    return newNode;
}

/*Token *TokenList_addToken(TokenList *list, const Token token) {

    TokenListNode *newNode = malloc(sizeof *newNode);
    newNode->data = malloc(sizeof *newNode->data);
    memcpy(newNode->data, &token, sizeof(*newNode->data));
    newNode->next = 0;

    if (list->size == 0) {
        newNode->previous = 0;
        list->head = newNode;
        
    } else {
        TokenListNode *node = list->head;
        while (node->next) {
            node = node->next;
        }

        newNode->previous = node;
        node->next = newNode;
    }

    list->size++;

    return newNode->data;
}*/

void TokenList_removeNode(TokenList *list, Token *node);
void TokenList_removeNodeByIndex(TokenList *list, int indexOfNode);