//
// Created by Elijah on 8/13/2024.
//

#include <string_view>
#include <vector>
#include <variant>
#include <stdexcept>

enum class TokenKind { TK_EOF, TK_PNT, TK_INT };

struct Token {
  TokenKind kind;
  std::variant<int,char, std::nullptr_t> value;
};

class Lexer {
  public:
  static std::vector<Token> TokensFromString(std::string_view input) { using enum TokenKind;
    std::vector<Token> tokens;
    while (!input.empty()) {
      if (std::isspace(input.front())) {
        input.remove_prefix(1);
      } else if (std::isdigit(input.front())) {
        tokens.emplace_back(TK_INT, parseInt(input));
      } else if (std::ispunct(input.front())) {
        tokens.emplace_back(TK_PNT, input.front());
        input.remove_prefix(1);
      } else {
        throw std::runtime_error("Unexpected character: " + input.front());
      }
    }

    tokens.emplace_back(TK_EOF, nullptr);
    return tokens;
  }

  static int parseInt(std::string_view& input) {
    int result = 0;

    while (!input.empty()  && (std::isdigit(input.front()) || std::isspace(input.front()))) {
      if (!std::isspace(input.front())) {
        result = result * 10 + (input.front() - '0');
      }
      input.remove_prefix(1);
    }
    return result;
  }
};
