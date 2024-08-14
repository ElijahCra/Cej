//
// Created by Elijah on 8/12/2024.
//

#include <string>
#include <utility>
#include <vector>
#include <memory>

enum class NodeType {
  Constant,
  Variable,
  BinaryOp,
  Assignment
};

enum class DataType {
  Int32
};

class ASTNode {
  public:
  virtual ~ASTNode() = default;
  [[nodiscard]] virtual NodeType getType() const = 0;
};

class ConstantNode : public ASTNode { //literal
  public:
  ConstantNode(const int value, const DataType type) : value(value), dataType(type) {}
  [[nodiscard]] NodeType getType() const override { return NodeType::Constant; }
  int value;
  DataType dataType;
};

class VariableNode final : public ASTNode { // var x
  public:
  VariableNode(std::string  name, DataType type) : name(std::move(name)), dataType(type) {}
  [[nodiscard]] NodeType getType() const override { return NodeType::Variable; }
  std::string name;
  DataType dataType;
};

class BinaryOpNode final : public ASTNode {
  public:
  BinaryOpNode(char op, std::unique_ptr<ASTNode> left, std::unique_ptr<ASTNode> right)
      : op(op), left(std::move(left)), right(std::move(right)) {}
  [[nodiscard]] NodeType getType() const override { return NodeType::BinaryOp; }
  char op;
  std::unique_ptr<ASTNode> left;
  std::unique_ptr<ASTNode> right;
};

class AssignmentNode final : public ASTNode {
  public:
  AssignmentNode(std::unique_ptr<VariableNode> var, std::unique_ptr<ASTNode> value)
      : variable(std::move(var)), value(std::move(value)) {}
  [[nodiscard]] NodeType getType() const override { return NodeType::Assignment; }
  std::unique_ptr<VariableNode> variable;
  std::unique_ptr<ASTNode> value;
};