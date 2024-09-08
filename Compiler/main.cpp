#include <format>
#include <iostream>
#include "Generator.cpp"

int
main()
{
    std::string input = "main :: () int { a:int=1;b:int=2;c:int=3;d:int=4; return (a+b)*(c+d); }";
    std::unique_ptr<Token> tokens = Lexer::TokensFromInput(input);
    std::unique_ptr<Program> tree = Parser::Parse(tokens);
    std::cout<<Generator::GenerateAssembly(tree);

    return 0;
}