//
// Created by Elijah on 10/29/2024.
//

#ifndef PARSERBASE_HPP
#define PARSERBASE_HPP

#include <memory>
#include <optional>
#include <string>
#include <vector>
#include <utility>
#include "../Lexer/Lexer.cpp"
#include "ParserTypes.hpp"
#include "ParsingContext.hpp"

class ParserBase {
    protected:
    ParsingContext& context;

    void Expect(const std::string_view expected) {
        if (context.currentToken.raw_val != expected) {
            throw std::runtime_error("Expected '" + std::string(expected) + "', but got '" + (context.currentToken.kind == TokenKind::TK_EOF ? context.currentToken.raw_val : "<EOF>") + "' in line: " + std::to_string(context.getCurrentLine()) + " and position: " + std::to_string(context.getCurrentPosition())+'\n');
        }
        context.advance();
    }

    public:
    explicit ParserBase(ParsingContext& context) : context(context) {}
};

#endif //PARSERBASE_HPP
