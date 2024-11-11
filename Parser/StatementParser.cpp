//
// Created by Elijah on 11/10/2024.
//

#include "StatementParser.hpp"

std::unique_ptr<Statement>
StatementParser::ParseStatement() {
  if (context.currentToken.raw_val == "return") {
    context.advance();
    std::optional<std::unique_ptr<Exp>> exp = std::nullopt;
    if (context.currentToken.raw_val != ";") {
      exp = expressionParser.ParseExpression();
    }
    Expect(";");
    return std::make_unique<ReturnStatement>(std::move(exp));
  }
  if (context.currentToken.raw_val == "{") {
    return std::make_unique<CompoundStatement>(ParseBlock());
  }
  if (context.currentToken.raw_val == "if") {
    return ParseIfStatement();
  }
  if (context.currentToken.raw_val == "while") {
    return ParseWhileStatement();
  }
  if (context.currentToken.raw_val == "for") {
    return ParseForStatement();
  }
  if (IsDeclaration()) {
    auto declaration = ParseVariableDeclaration();
    auto blockDecItem = BlockItemDeclaration(std::move(declaration));
    std::unique_ptr<BlockItem> blockItemPtr = std::make_unique<BlockItem>(std::move(blockDecItem));
    std::vector<std::unique_ptr<BlockItem>> vecBlockItemPtr {};
    vecBlockItemPtr.push_back(std::move(blockItemPtr));
    auto block = Block(std::move(vecBlockItemPtr));
    auto blockPtr = std::make_unique<Block>(std::move(block));
    auto compound = CompoundStatement{std::move(blockPtr)};
    auto compoundPtr = std::make_unique<CompoundStatement>(std::move(compound));
    return compoundPtr;
  }
  // Expression statement
  auto exp = expressionParser.ParseExpression();
  Expect(";");
  return std::make_unique<ExpressionStatement>(std::move(exp));
}

std::unique_ptr<Declaration>
StatementParser::ParseVariableDeclaration() {
  auto varType = ParseType();
  std::string name = context.currentToken.raw_val;
  context.advance();
  std::optional<std::unique_ptr<Initializer>> initializer = std::nullopt;

  if (context.currentToken.raw_val == "=") {
    context.advance();
    auto exp = expressionParser.ParseExpression();
    initializer = std::make_unique<SingleInit>(std::move(exp));
  }

  Expect(";");

  return std::make_unique<VariableDeclaration>(std::move(name), std::move(varType), std::move(initializer));
}

std::unique_ptr<Block>
StatementParser::ParseBlock() {
  Expect("{");
  std::vector<std::unique_ptr<BlockItem>> items;
  while (context.currentToken.raw_val != "}") {
    if (IsDeclaration()) {
      auto declaration = ParseVariableDeclaration();
      items.push_back(std::make_unique<BlockItemDeclaration>(std::move(declaration)));
    } else {
      auto statement = ParseStatement();
      items.push_back(std::make_unique<BlockItemStatement>(std::move(statement)));
    }
  }
  Expect("}");
  return std::make_unique<Block>(std::move(items));
}

std::unique_ptr<Declaration>
StatementParser::ParseStructDeclaration() {
  Expect("struct");
  std::string tag = context.currentToken.raw_val;
  context.advance();
  Expect("{");
  std::vector<std::unique_ptr<MemberDeclaration>> members;
  while (context.currentToken.raw_val != "}") {
    auto member = ParseMemberDeclaration();
    members.push_back(std::move(member));
  }
  Expect("}");
  Expect(";");
  return std::make_unique<StructDeclaration>(std::move(tag), std::move(members));
}

std::unique_ptr<Type>
StatementParser::ParseType() {
  if (IsPrimitiveType(context.currentToken.raw_val)) {
    PrimitiveType primType = StringToPrimitiveType(context.currentToken.raw_val);
    context.advance();

    std::unique_ptr<Type> typeNode = std::make_unique<PrimitiveTypeNode>(primType);

    while (context.currentToken.raw_val == "*") {
      context.advance();
      typeNode = std::make_unique<PointerType>(std::move(typeNode));
    }

    return typeNode;
  }
  if (context.currentToken.raw_val == "struct") {
    context.advance();
    std::string tag = context.currentToken.raw_val;
    context.advance();
    auto typeNode = std::make_unique<StructType>(std::move(tag));

    while (context.currentToken.raw_val == "*") {
      context.advance();
      auto pointerType = std::make_unique<PointerType>(std::move(typeNode));
      return pointerType;
    }

    return typeNode;
  }
  throw std::runtime_error("Unknown type at line: " + std::to_string(context.getCurrentLine()));
}
