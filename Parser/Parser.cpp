// Created by Elijah on 8/13/2024.

#ifndef PARSER_CPP
#define PARSER_CPP
#include <memory>
#include <optional>
#include <string>
#include <vector>
#include <utility>
#include "../Lexer/Lexer.cpp"
#include "ParserTypes.hpp"
#include "ExpressionParser.hpp"
#include "ParsingContext.hpp"




class Parser {
  public:
  explicit Parser(Lexer& lexer)
      : context(lexer),
        expressionParser(context),
        statementParser(context),
        functionParser(context) {

  }

  std::unique_ptr<Program> parse() {
    return parseProgram();
  }

  private:
  ParsingContext context;
  ExpressionParser expressionParser;
  StatementParser statementParser;
  FunctionParser functionParser;

  void advance() {
    currentToken = lexer.getNextToken();
  }

  std::unique_ptr<Program> parseProgram() {
    std::vector<std::unique_ptr<Function>> functions;
    while (context.currentToken && context.currentToken->kind != TokenKind::TK_EOF) {
      functions.push_back(ParseFunction());
    }
    return std::make_unique<Program>(std::move(functions));
  }

  std::unique_ptr<Function> ParseFunction() {
    std::string name = currentToken->raw_val;
    advance();

    Expect("::");
    Expect("(");
    Expect(")");
    Expect("int");
    Expect("{");

    int allocationSize = 0;
    std::vector<std::unique_ptr<Statement>> statements;
    while (currentToken && currentToken->raw_val != "}") {
      auto statement = ParseStatement();
      if (auto declare = dynamic_cast<Declare*>(statement.get())) {
        if (declare->type == "int") {
          allocationSize += 16; // Assuming int is 16 bytes as per original code
        }
      }
      statements.push_back(std::move(statement));
    }
    Expect("}");

    return std::make_unique<Function>(std::move(name), allocationSize, std::move(statements));
  }

  std::unique_ptr<Statement> ParseStatement() {
    if (currentToken && currentToken->raw_val == "return") {
      advance();
      auto exp = ParseExpression();
      Expect(";");
      return std::make_unique<Return>(std::move(exp));
    }
    if (currentToken && currentToken->kind == TokenKind::TK_IDENTIFIER) {
      std::string name = currentToken->raw_val;
      advance();

      if (currentToken && currentToken->raw_val == ":") {
        advance();
        if (currentToken && currentToken->raw_val == "int") {
          std::string type = currentToken->raw_val;
          advance();

          if (currentToken && currentToken->raw_val == "=") {
            advance();
            auto initializer = ParseExpression();
            Expect(";");
            return std::make_unique<Declare>(std::move(name), std::move(type), std::move(initializer));
          }
          Expect(";");
          return std::make_unique<Declare>(std::move(name), std::move(type));
        }
        throw std::runtime_error("Provided type not in system types in line: " + lexer.getCurrentLine());
      }

      if (currentToken && currentToken->raw_val == "=") {
        advance();
        auto exp = ParseExpression();
        Expect(";");
        return std::make_unique<ExpStatement>(std::make_unique<Assign>(std::move(name), std::move(exp)));
      }
      throw std::runtime_error("Unexpected token after identifier in line: " + lexer.getCurrentLine());
    }
    throw std::runtime_error("Unexpected statement in line: " + lexer.getCurrentLine());
  }

  std::unique_ptr<Exp> ParseExpression() {
    return ParseAdditiveExpression();
  }

  std::unique_ptr<Exp> ParseAdditiveExpression() {
    auto left = ParseMultiplicativeExpression();
    while (currentToken && (currentToken->raw_val == "+" || currentToken->raw_val == "-")) {
      BinaryOperator op = (currentToken->raw_val == "+") ? BinaryOperator::Add : BinaryOperator::Sub;
      advance();
      auto right = ParseMultiplicativeExpression();
      left = std::make_unique<BinOp>(op, std::move(left), std::move(right));
    }
    return left;
  }

  std::unique_ptr<Exp> ParseMultiplicativeExpression() {
    auto left = ParseUnaryExpression();
    while (currentToken && (currentToken->raw_val == "*" || currentToken->raw_val == "/")) {
      BinaryOperator op = (currentToken->raw_val == "*") ? BinaryOperator::Mul : BinaryOperator::Div;
      advance();
      auto right = ParseUnaryExpression();
      left = std::make_unique<BinOp>(op, std::move(left), std::move(right));
    }
    return left;
  }

  std::unique_ptr<Exp> ParseUnaryExpression() {
    if (currentToken && currentToken->raw_val == "-") {
      advance();
      auto operand = ParseUnaryExpression();
      return std::make_unique<UnOp>(UnaryOperator::Neg, std::move(operand));
    }
    return ParsePrimaryExpression();
  }

  std::unique_ptr<Exp> ParsePrimaryExpression() {
    if (currentToken && currentToken->kind == TokenKind::TK_INT) {
      int value = std::stoi(currentToken->raw_val);
      advance();
      return std::make_unique<Literal>(value);
    }
    if (currentToken && currentToken->kind == TokenKind::TK_IDENTIFIER) {
      std::string name = currentToken->raw_val;
      advance();
      if (currentToken && currentToken->raw_val == "(") {
        return ParseFunctionCall(name);
      }
      return std::make_unique<Var>(std::move(name));
    }
    if (currentToken && currentToken->raw_val == "(") {
      advance();
      auto exp = ParseExpression();
      Expect(")");
      return exp;
    }
    throw std::runtime_error("Unexpected token in primary expression in line: " + lexer.getCurrentLine());
  }

  std::unique_ptr<Exp> ParseFunctionCall(const std::string& name) {
    Expect("(");
    std::vector<std::unique_ptr<Exp>> arguments;
    if (currentToken && currentToken->raw_val != ")") {
      do {
        arguments.push_back(ParseExpression());
      } while (currentToken && currentToken->raw_val == "," && (advance(), true));
    }
    Expect(")");
    return std::make_unique<FunctionCall>(name, std::move(arguments));
  }

  void Expect(const std::string_view expected) {
    if (!currentToken || currentToken->raw_val != expected) {
      throw std::runtime_error("Expected '" + std::string(expected) + "', but got '" + (currentToken ? currentToken->raw_val : "<EOF>") + "' in line: " + lexer.getCurrentLine());
    }
    advance();
  }
};

#endif // PARSER_CPP
