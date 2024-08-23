#include <format>
#include <iostream>
import Generator;
import Lexer;
import Parser;

/* Driver */
int
main()
{
    const std::string input = "123 + 456";
    std::unique_ptr<Token> tokens = Lexer::TokensFromString(input);
    std::unique_ptr<Node> tree = Parser::TreeFromTokens(std::move(tokens));
    Generator::AssemblyFromTree(std::move(tree));

    return 0;
}