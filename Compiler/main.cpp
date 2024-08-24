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
    std::string input = "y : int = 4; main :: () int { return y; }";
    std::unique_ptr<Token> tokens = Lexer::TokensFromString(input);
    std::unique_ptr<Node> tree = Parser::TreeFromTokens(std::move(tokens));
    Generator::AssemblyFromTree(std::move(tree));

    return 0;
}