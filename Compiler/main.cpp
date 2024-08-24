#include <format>
#include <iostream>
#include "Generator.cpp"
#include "Parser.cpp"
#include "Lexer.cpp"

/* Driver */

int
main()
{
    std::string input = "x : int : 5; main :: () int {return x;}";
    std::unique_ptr<Token> tokens = Lexer::TokensFromString(input);
    std::unique_ptr<Node> tree = Parser::TreeFromTokens(std::move(tokens));
    Generator::AssemblyFromTree(std::move(tree));

    return 0;
}