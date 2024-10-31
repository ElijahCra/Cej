//
// Created by Elijah Crain on 10/29/24.
//

// FunctionParser.hpp

#ifndef FUNCTIONPARSER_HPP
#define FUNCTIONPARSER_HPP

#include "ParsingContext.hpp"

class FunctionParser : public ParserBase {
    StatementParser statementParser;
public:
    explicit FunctionParser(ParsingContext& context) : ParserBase(context), statementParser(context) {}

    std::unique_ptr<FunctionDef> ParseFunction() {
        std::string ns = "";
        std::string name = context.currentToken->raw_val;
        context.advance();
        while (context.currentToken && context.currentToken->raw_val == "::") {
            context.advance();
            if (ns.empty()) {
                ns = name;
            } else {
                ns += "::" + name;
            }
            if (context.currentToken && context.currentToken->kind == TokenKind::TK_IDENTIFIER) {
                name = context.currentToken->raw_val;
                context.advance();
            } else {
                throw std::runtime_error("Expected identifier after '::' in function definition in line: " + std::to_string(context.getCurrentLine()));
            }
        }

        Expect("(");
        Expect(")");
        std::string returnType = context.currentToken->raw_val;
        context.advance();
        Expect("{");

        int allocationSize = 0;
        std::vector<std::unique_ptr<Statement>> statements;
        while (context.currentToken && context.currentToken->raw_val != "}") {
            auto statement = statementParser.ParseStatement();
            if (auto declare = dynamic_cast<Declare*>(statement.get())) {
                if (declare->type == "int") {
                    allocationSize += 16; // Assuming int is 16 bytes as per original code
                }
            }
            statements.push_back(std::move(statement));
        }
        Expect("}");
        return std::make_unique<FunctionDef>(std::move(name), allocationSize, std::move(returnType), std::move(statements), ns.empty() ? context.getCurrentNamespace() : ns);
    }
};

#endif //FUNCTIONPARSER_HPP
