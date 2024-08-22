//
// Created by Elijah on 8/12/2024.
//
#include <sstream>
#include "Nodes.cpp"

void
WalkTree(Node *node)
{
    switch (node->kind) {
        case ND_INT:
            printf("\tmov x0, #%d\n", node->value);
        return;
        case ND_NEG:
            WalkTree(node->lhs);
        printf("\tneg x0, x0\n");
        return;
    }

    WalkTree(node->rhs);
    printf("\tstr x0, [sp, #-16]!\n");
    WalkTree(node->lhs);
    printf("\tldr x1, [sp], #16\n");

    switch (node->kind) {
        case ND_ADD:
            printf("\tadd x0, x0, x1\n");
        return;
        case ND_SUB:
            printf("\tsub x0, x0, x1\n");
        return;
        case ND_MUL:
            printf("\tmul x0, x0, x1\n");
        return;
        case ND_DIV:
            printf("\tsdiv x0, x0, x1\n");
        return;
    }

    Error("invalid expression");
}

void
AssemblyFromTree(Node *tree)
{
    printf("\t.globl _main\n");
    printf("\t.align 4\n");
    printf("_main:\n");

    WalkTree(tree);

    printf("\tmov x16, #1\n");
    printf("\tsvc #0x80\n");
}


class ARMGenerator {
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