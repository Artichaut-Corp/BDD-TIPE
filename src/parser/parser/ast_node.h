#include <memory>
#include <utility>
#include <vector>

#include "node_type.h"
#include "../lexer/tokens.h"

#ifndef ASTNODE_H
#define ASTNODE_H

namespace Compiler::Parsing {
  
class AstNode {
    private:
        NodeType type;
        Lexing::Token token;
        
        std::vector<std::unique_ptr<AstNode>> children;

    public:
        AstNode(NodeType type, Lexing::Token token)
            : type(type)
            , token(token)
        {
        }

        AstNode(NodeType type, Lexing::Token token, std::vector<std::unique_ptr<AstNode>> children)
            : type(type)
            , token(token)
            , children(std::move(children))
        {
        }
        
        AstNode(const AstNode& other) 
            : type(other.type)
            , token(other.token)
        {
          children.reserve(other.children.size());

          for (const auto& ptr : other.children) {
            children.push_back(std::make_unique<AstNode>(*ptr));
          }
        }

        AstNode& operator=(const AstNode& other) {
          type = other.type;
          token = other.token;
          children.reserve(other.children.size());

          for (const auto& ptr : other.children) {
            children.push_back(std::make_unique<AstNode>(*ptr));
          }
          
          return *this;
        }

        AstNode(AstNode&& other) = default;

        AstNode& operator=(AstNode&& other) = default;

        ~AstNode() {};
      
        std::vector<std::unique_ptr<AstNode>>& GetChildren() { return children; }
      
        bool IsLeaf() { return this->children.empty(); }
    };
    
} // namespace Compiler::Parsing

#endif // !ASTNODE_H
