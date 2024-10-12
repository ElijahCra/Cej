#include <iostream>
#include "Generator.cpp"

int
main()
{
    std::string input = "f :: () int {return 2;} g :: () int {var:int=3; return f()+(2*var);}main :: () int { a:int=1;b:int=f();c:int=3; return (a+b)*(f()+g()); }";
    std::unique_ptr<Token> tokens = Lexer::TokensFromInput(input);
    std::unique_ptr<Program> tree = Parser::Parse(tokens);
    std::cout << std::to_string(static_cast<int>(tokens->kind));
    std::cout<<Generator::GenerateAssembly(tree);

    return 0;
}