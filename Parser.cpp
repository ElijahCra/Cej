//
// Created by Elijah on 8/13/2024.
//
#include <fstream>
#include <memory>
#include <sstream>
#include <vector>
#include <stdexcept>
#include "Lexer.cpp"
#include "Nodes.cpp"
class ASTNode;
class Parser {
  public:
  static std::unique_ptr<ASTNode> parse(const std::vector<Token>& tokens) {
    // This is a very basic parser. You'll need to expand it for a full language.
    if (tokens.size() == 4 && tokens[1].type == TokenType::Colon &&
        tokens[2].type == TokenType::Identifier && tokens[3].type == TokenType::Semicolon) {
      // x : i32;
      auto var = std::make_unique<VariableNode>(tokens[0].value, DataType::Int32);
      return var;
        } else if (tokens.size() == 5 && tokens[1].type == TokenType::Colon &&
                   tokens[2].type == TokenType::Identifier && tokens[3].type == TokenType::Colon) {
          // x : i32 : 5;
          auto var = std::make_unique<VariableNode>(tokens[0].value, DataType::Int32);
          auto value = std::make_unique<ConstantNode>(std::stoi(tokens[4].value), DataType::Int32);
          return std::make_unique<AssignmentNode>(std::move(var), std::move(value));
                   }
    // Add more parsing rules here
    throw std::runtime_error("Unable to parse input");
  }
};
