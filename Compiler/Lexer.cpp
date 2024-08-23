//
// Created by Elijah on 8/13/2024.
//
#ifndef LEXER_CPP
#define LEXER_CPP

#include <memory>
#include <string_view>
#include <vector>
#include <variant>
#include <stdexcept>

enum class TokenKind { TK_EOF, TK_PNT, TK_INT };

struct Token {
  TokenKind kind;
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

  static std::unique_ptr<Token> MakeToken(const TokenKind kind, const std::string_view str) {
    auto token = std::make_unique<Token>();
    token->kind = kind;
    token->literal = std::string(str);
    return token;
  }

  static std::unique_ptr<Token>
  TokensFromString(std::string_view input) {
    std::unique_ptr<Token> head = nullptr;
    Token* tail = nullptr;

    while (!input.empty()) {
      std::unique_ptr<Token> newToken;

      if (std::isspace(input.front())) {
        input.remove_prefix(1);
        continue;
      }
      if (std::isdigit(input.front())) {
        newToken = MakeToken(TokenKind::TK_INT, parseInt(input));
      } else if (std::ispunct(input.front())) {
        newToken = MakeToken(TokenKind::TK_PNT, std::string{input.front()});
        input.remove_prefix(1);
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

  static std::string parseInt(std::string_view& input) {
    const std::string_view copy = input;
    while (!input.empty()  && (std::isdigit(input.front()) || std::isspace(input.front()))) {
      input.remove_prefix(1);
    }
    std::string str(copy.begin(), input.begin());
    std::erase_if(str, isspace);
    return str;

  }
};
#endif// LEXER_CPP

