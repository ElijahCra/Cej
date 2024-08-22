#include <iostream>
#include "Generator.cpp"

/* Driver */

int
main(int argc, char *argv[])
{
    if (argc != 2) {
        Error("usage: %s <program>", argv[0]);
    }

    Token *tokens = TokensFromString(argv[1]);
    Node *tree = TreeFromTokens(&tokens);

    if (tokens->kind != TK_EOF) {
        Error("unconsumed tokens");
    }

    AssemblyFromTree(tree);

    return 0;
}