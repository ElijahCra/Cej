#include <format>
#include <iostream>
#include "Generator.cpp"

int
main()
{
    std::string input = "f :: () int {return 2;} main :: () int { a:int=1;b:int=2;c:int=3; return (a+b)*f(); }";
    std::unique_ptr<Token> tokens = Lexer::TokensFromInput(input);
    std::unique_ptr<Program> tree = Parser::Parse(tokens);
    std::cout<<Generator::GenerateAssembly(tree);

    return 0;
}