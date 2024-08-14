//
// Created by Elijah on 8/13/2024.
//

#include <sstream>
#include <vector>
#include <stdexcept>
enum class TokenType {
  Identifier,
  Integer,
  Colon,
  Semicolon,
  Equals,
  ColonColon,
  ColonEquals
};

struct Token {
  TokenType type;
  std::string value;
};

class Lexer {
  public:
  [[nodiscard]] std::vector<Token> tokenize(const std::string& input) const {
    std::vector<Token> tokens;
    std::istringstream iss(input);
    std::string word;
    while (iss >> word) {
      using enum TokenType;
      if (isalpha(word[0])) {
        tokens.emplace_back(Identifier, word);
      } else if (isdigit(word[0])) {
        tokens.emplace_back(Integer, word);
      } else if (word == ":") {
        tokens.emplace_back(Colon, word);
      } else if (word == ";") {
        tokens.emplace_back(Semicolon, word);
      } else if (word == "=") {
        tokens.emplace_back(Equals, word);
      } else if (word == "::") {
        tokens.emplace_back(ColonColon, word);
      } else if (word == ":=") {
        tokens.emplace_back(ColonEquals, word);
      } else {
        throw std::runtime_error("Unknown token: " + word);
      }
    }
    return tokens;
  }
};
