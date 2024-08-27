//
// Created by Elijah on 8/13/2024.
//

#ifndef LEXER_CPP
#define LEXER_CPP

#include <string>
#include <memory>
#include <string_view>
#include <stdexcept>
#include <array>

enum class TokenKind {
  TK_EOF,
  TK_INT,
  TK_KEYWORD,
  TK_IDENTIFIER,
  TK_PLUS,
  TK_MINUS,
  TK_SLASH,
  TK_ASTERISK,
  TK_COLON,
  TK_COLONCOLON,
  TK_EQUAL,
  TK_COLONEQUAL,
  TK_SEMICOLON,
  TK_OPEN_BRACE,
  TK_CLOSE_BRACE,
  TK_OPEN_PAREN,
  TK_CLOSE_PAREN,
  TK_COMMA
};

struct Token {
  TokenKind kind{};
  std::string literal;
  std::unique_ptr<Token> next;
};

class TokenList {
  public:

  std::unique_ptr<Token> head;
  explicit TokenList(std::unique_ptr<Token> head) : head(std::move(head)) {}

  class Iterator {
    Token* current;

    public:
    explicit Iterator(Token* start) : current(start) {}

    Token& operator*() const { return *current; }
    Token* operator->() const { return current; }

    Iterator& operator++() {
      if (current) current = current->next.get();
      return *this;
    }

    bool operator!=(const Iterator& other) const {
      return current != other.current;
    }
  };

  [[nodiscard]] Iterator begin() const { return Iterator(head.get()); }
  [[nodiscard]] static Iterator end() { return Iterator(nullptr); }
};

class Lexer {
  public:
  static constexpr std::array<std::string_view,3> keyWords = {"return", "ret","int"};

  static std::unique_ptr<Token>
  MakeToken(const TokenKind kind, const std::string_view str) {
    auto token = std::make_unique<Token>();
    token->kind = kind;
    token->literal = std::string(str);
    return token;
  }

  static std::unique_ptr<Token>
  TokensFromInput(std::string_view input) {
    std::unique_ptr<Token> head = nullptr;
    Token* tail = nullptr;

    while (!input.empty()) {
      std::unique_ptr<Token> newToken;

      if (std::isspace(input.front())) {
        input.remove_prefix(1);
        continue;
      }
      if (std::isdigit(input.front())) {
        newToken = makeTokenFromInt(input);
      } else if (std::isalpha(input.front())) {
        newToken = makeTokenFromText(input);
      } else if (std::ispunct(input.front())) {
        newToken = makeTokenFromPunctuation(input);
      } else {
        throw std::runtime_error("Unexpected character: " + std::string(1, input.front()));
      }

      if (!head) {
        head = std::move(newToken);
        tail = head.get();
      } else {
        tail->next = std::move(newToken);
        tail = tail->next.get();
      }
    }

    if (tail) {
      tail->next = MakeToken(TokenKind::TK_EOF, "");
    } else {
      head = MakeToken(TokenKind::TK_EOF, "");
    }

    return head;
  }

  static std::unique_ptr<Token>
  makeTokenFromInt(std::string_view& input) { using enum TokenKind;
    const std::string_view copy = input;
    input.remove_prefix(1);  // Remove the first character (already checked)
    while (!input.empty()  && (std::isdigit(input.front()) || std::isspace(input.front()))) {
      input.remove_prefix(1);
    }
    std::string str(copy.begin(), input.begin());
    std::erase_if(str, isspace);
    return MakeToken(TK_INT, str);
  }

  static std::unique_ptr<Token>
  makeTokenFromText(std::string_view& input) { using enum TokenKind;
    const std::string_view copy = input;
    input.remove_prefix(1);  // Remove the first character (already checked)
    while (!input.empty() && (std::isalnum(input.front()) || input.front() == '_')) {
      input.remove_prefix(1);
    }
    std::string_view text{copy.begin(),input.begin()};
    if (std::ranges::find(keyWords,text) != keyWords.end()) {
      return MakeToken(TK_KEYWORD,text);
    }
    return MakeToken(TK_IDENTIFIER,text);
  }

  static std::unique_ptr<Token>
  makeTokenFromPunctuation(std::string_view& input) {
    using enum TokenKind;

    if (input.size() >= 2) {
      if (input.starts_with("::")) {
        input.remove_prefix(2);
        return MakeToken(TK_COLONCOLON, "::");
      }
      if (input.starts_with(":=")) {
        input.remove_prefix(2);
        return MakeToken(TK_COLONEQUAL, ":=");
      }
    }

    switch (input.front()) {
      case '+':
        input.remove_prefix(1);
      return MakeToken(TK_PLUS, "+");
      case '-':
        input.remove_prefix(1);
      return MakeToken(TK_MINUS, "-");
      case '/':
        input.remove_prefix(1);
      return MakeToken(TK_SLASH, "/");
      case '*':
        input.remove_prefix(1);
      return MakeToken(TK_ASTERISK, "*");
      case ':':
        input.remove_prefix(1);
      return MakeToken(TK_COLON, ":");
      case '=':
        input.remove_prefix(1);
      return MakeToken(TK_EQUAL, "=");
      case ';':
        input.remove_prefix(1);
      return MakeToken(TK_SEMICOLON, ";");
      case '{':
        input.remove_prefix(1);
      return MakeToken(TK_OPEN_BRACE, "{");
      case '}':
        input.remove_prefix(1);
      return MakeToken(TK_CLOSE_BRACE, "}");
      case '(':
        input.remove_prefix(1);
      return MakeToken(TK_OPEN_PAREN, "(");
      case ')':
        input.remove_prefix(1);
      return MakeToken(TK_CLOSE_PAREN, ")");
      case ',':
        input.remove_prefix(1);
      return MakeToken(TK_COMMA, ",");
      default:
        throw std::runtime_error("non valid punctuation in program found in lexer");
    }
  }
};
#endif// LEXER_CPP

