//
// Created by Elijah on 8/12/2024.
//
#include <sstream>
#include "Nodes.cpp"

class ARMCodeGenerator {
public:
    std::string generateCode(const ASTNode* node) {
        std::ostringstream oss;
        generateCodeImpl(node, oss);
        return oss.str();
    }

private:
    void generateCodeImpl(const ASTNode* node, std::ostringstream& oss) {
        switch (node->getType()) {
            case NodeType::Constant:
                generateConstant(static_cast<const ConstantNode*>(node), oss);
                break;
            case NodeType::Variable:
                generateVariable(static_cast<const VariableNode*>(node), oss);
                break;
            case NodeType::BinaryOp:
                generateBinaryOp(static_cast<const BinaryOpNode*>(node), oss);
                break;
            case NodeType::Assignment:
                generateAssignment(static_cast<const AssignmentNode*>(node), oss);
                break;
        }
    }

    static void generateConstant(const ConstantNode* node, std::ostringstream& oss) {
        oss << "    mov r0, #" << node->value << "\n";
    }

    static void generateVariable(const VariableNode* node, std::ostringstream& oss) {
        oss << "    ldr r0, =" << node->name << "\n";
    }

    void generateBinaryOp(const BinaryOpNode* node, std::ostringstream& oss) {
        generateCodeImpl(node->left.get(), oss);
        oss << "    push {r0}\n";
        generateCodeImpl(node->right.get(), oss);
        oss << "    pop {r1}\n";
        switch (node->op) {
            case '+':
                oss << "    add r0, r1, r0\n";
                break;
            case '-':
                oss << "    sub r0, r1, r0\n";
                break;
            case '*':
                oss << "    mul r0, r1, r0\n";
                break;
            // Add more operations as needed
        }
    }

    void generateAssignment(const AssignmentNode* node, std::ostringstream& oss) {
        generateCodeImpl(node->value.get(), oss);
        oss << "    str r0, =" << node->variable->name << "\n";
    }
};