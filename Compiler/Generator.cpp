//
// Created by Elijah on 8/12/2024.
//
#ifndef GENERATOR_CPP
#define GENERATOR_CPP
#include <sstream>
#include "Parser.cpp"

struct Node;
class Generator {
public:

  static void
  WalkTree(std::unique_ptr<Node> node){ using enum NodeKind;
      switch (node->kind) {
        case ND_INT:
          printf("\tmov x0, #%d\n", node->value.value());
        return;
        case ND_NEG:
          WalkTree(std::move(node->lhs));
        printf("\tneg x0, x0\n");
        return;
        default: ;
      }

      WalkTree(std::move(node->rhs));
      printf("\tstr x0, [sp, #-16]!\n");
      WalkTree(std::move(node->lhs));
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
        default:
          throw std::runtime_error("invalid expression");
      }
    }

  static void
  AssemblyFromTree(std::unique_ptr<Node> tree)
  {
    printf("\t.globl _main\n");
    printf("\t.align 4\n");
    printf("_main:\n");

    WalkTree(std::move(tree));

    printf("\tmov x16, #1\n");
    printf("\tsvc #0x80\n");
  }

};

#endif // GENERATOR_CPP