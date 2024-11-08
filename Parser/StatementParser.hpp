//
// Created by Elijah Crain on 10/29/24.
//

#ifndef STATEMENTPARSER_HPP
#define STATEMENTPARSER_HPP

#include "ParsingContext.hpp"
#include "ParserBase.hpp"
#include "ExpressionParser.hpp"

class StatementParser : public ParserBase {
    ExpressionParser expressionParser;
public:
    explicit StatementParser(ParsingContext& context) : ParserBase(context), expressionParser(context) {}

    std::unique_ptr<Statement> ParseStatement() {
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
            return std::make_unique<BlockItemDeclaration>(std::move(declaration));
        }
        // Expression statement
        auto exp = expressionParser.ParseExpression();
        Expect(";");
        return std::make_unique<ExpressionStatement>(std::move(exp));
    }

    std::unique_ptr<Declaration> ParseVariableDeclaration() {
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
  ParseBlock() {
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

    std::unique_ptr<Declaration> ParseStructDeclaration() {
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

    std::unique_ptr<Type> ParseType() {
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

    bool IsTypeName(const std::string& name) {
        return IsPrimitiveType(name) || name == "struct";
    }

private:
    bool IsPrimitiveType(const std::string& typeName) {
        static const std::set<std::string> primitiveTypes = {
            "char", "signed", "unsigned", "int", "long", "double", "void", "float"
        };
        return primitiveTypes.count(typeName) > 0;
    }

    PrimitiveType StringToPrimitiveType(const std::string& typeName) {
        if (typeName == "char") return PrimitiveType::Char;
        if (typeName == "signed") return PrimitiveType::SChar;
        if (typeName == "unsigned") return PrimitiveType::UChar;
        if (typeName == "int") return PrimitiveType::Int;
        if (typeName == "long") return PrimitiveType::Long;
        if (typeName == "double") return PrimitiveType::Double;
        if (typeName == "void") return PrimitiveType::Void;
        throw std::runtime_error("Unknown primitive type: " + typeName);
    }

    std::unique_ptr<MemberDeclaration> ParseMemberDeclaration() {
        auto memberType = ParseType();
        std::string memberName = context.currentToken.raw_val;
        context.advance();
        Expect(";");
        return std::make_unique<MemberDeclaration>(std::move(memberName), std::move(memberType));
    }

    bool IsDeclaration() {
        return IsTypeName(context.currentToken.raw_val);
    }

    // Placeholder methods for parsing statements not fully implemented
    std::unique_ptr<Statement> ParseIfStatement() { /* Implement accordingly */ }
    std::unique_ptr<Statement> ParseWhileStatement() { /* Implement accordingly */ }
    std::unique_ptr<Statement> ParseForStatement() { /* Implement accordingly */ }
};

#endif // STATEMENTPARSER_HPP
