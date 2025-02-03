#include "ast_node.h"
#include "node_type.h"

#ifndef TREE_H
#define TREE_H

namespace Compiler::Parsing {

// Class grouping abstract method for Tree creation, and traversing
class Tree {

private:
    std::unique_ptr<AstNode> root;

public:
    Tree(Lexing::Token start_token);

    AstNode& GetRoot() const;

    static void PrintTree(AstNode& root);

    static void AddNode(AstNode& parent, NodeType type);

    static void AddNodeLeaf(AstNode& parent);

    static void Traverse(AstNode& root);
};

} // namespace Compiler::Parsing

#endif // TREE_H
