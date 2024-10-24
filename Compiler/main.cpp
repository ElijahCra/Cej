#include <format>
#include <iostream>
#include "Generator.cpp"

int
main()
{
    Lexer lexer("../../example.cej");
    Parser parser(lexer);
    std::unique_ptr<Program> program = parser.Parse();
    std::cout<<Generator::GenerateAssembly(program);

    return 0;
}