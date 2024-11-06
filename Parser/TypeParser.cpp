//
// Created by Elijah Crain on 11/5/24.
//

#ifndef TYPEPARSER_HPP
#define TYPEPARSER_HPP

#include "ParserBase.hpp"
#include "ParsingContext.hpp"

class TypeParser : public ParserBase {
public:
    explicit TypeParser(ParsingContext& context) : ParserBase(context) {}

    std::unique_ptr<Type> parseType() {
        if (isPrimitiveType()) {
            return parsePrimitiveType();
        } else if (context.currentToken.raw_val == "struct") {
            return parseStructType();
        } else if (context.currentToken.raw_val == "void") {
            context.advance();
            return std::make_unique<PrimitiveTypeNode>(PrimitiveType::Void);
        } else if (context.currentToken.raw_val == "*") {
            context.advance();
            auto baseType = parseType();
            return std::make_unique<PointerType>(std::move(baseType));
        } else {
            return nullptr;
        }
    }

private:
    bool isPrimitiveType() {
        static const std::set<std::string> primitiveTypes = {
            "char", "signed char", "unsigned char",
            "int", "unsigned int", "long", "unsigned long",
            "double"
        };
        return primitiveTypes.find(context.currentToken.raw_val) != primitiveTypes.end();
    }

    std::unique_ptr<Type> parsePrimitiveType() {
        if (context.currentToken.raw_val == "char") {
            context.advance();
            return std::make_unique<PrimitiveTypeNode>(PrimitiveType::Char);
        } else if (context.currentToken.raw_val == "signed char") {
            context.advance();
            return std::make_unique<PrimitiveTypeNode>(PrimitiveType::SChar);
        } else if (context.currentToken.raw_val == "unsigned char") {
            context.advance();
            return std::make_unique<PrimitiveTypeNode>(PrimitiveType::UChar);
        } else if (context.currentToken.raw_val == "int") {
            context.advance();
            return std::make_unique<PrimitiveTypeNode>(PrimitiveType::Int);
        } else if (context.currentToken.raw_val == "unsigned int") {
            context.advance();
            return std::make_unique<PrimitiveTypeNode>(PrimitiveType::UInt);
        } else if (context.currentToken.raw_val == "long") {
            context.advance();
            return std::make_unique<PrimitiveTypeNode>(PrimitiveType::Long);
        } else if (context.currentToken.raw_val == "unsigned long") {
            context.advance();
            return std::make_unique<PrimitiveTypeNode>(PrimitiveType::ULong);
        } else if (context.currentToken.raw_val == "double") {
            context.advance();
            return std::make_unique<PrimitiveTypeNode>(PrimitiveType::Double);
        } else {
            return nullptr;
        }
    }

    std::unique_ptr<Type> parseStructType() {
        Expect("struct");
        if (context.currentToken.kind != TokenKind::TK_IDENTIFIER) {
            throw std::runtime_error("Expected struct tag in type at line: " + std::to_string(context.getCurrentLine()));
        }
        std::string tag = context.currentToken.raw_val;
        context.advance();
        return std::make_unique<StructType>(std::move(tag));
    }
};

#endif // TYPEPARSER_HPP
