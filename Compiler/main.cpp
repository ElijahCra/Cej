#include <format>
#include <iostream>
#include "Generator.cpp"
#include "Parser.cpp"
#include "Lexer.cpp"

/* Driver */
std::unordered_map<std::string, int, StringHash,std::equal_to<>> Parser::variables{};
int
main()
{
    std::string input = "main :: () int { return 4 + 2; }";
    std::unique_ptr<Token> tokens = Lexer::TokensFromInput(input);
    std::unique_ptr<Node> tree = Parser::TreeFromTokens(std::move(tokens));
    Generator::AssemblyFromTree(std::move(tree));

    return 0;
}