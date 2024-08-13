#include <iostream>
#include "Generator.cpp"


int main() {
    // Create an AST for: x : i32 : 5;
    auto constNode = std::make_unique<ConstantNode>(5, DataType::Int32);
    auto varNode = std::make_unique<VariableNode>("x", DataType::Int32);
    auto assignNode = std::make_unique<AssignmentNode>(std::move(varNode), std::move(constNode));

    ARMCodeGenerator codeGen;
    std::string armCode = codeGen.generateCode(assignNode.get());
    std::cout << armCode << std::endl;

    return 0;
}