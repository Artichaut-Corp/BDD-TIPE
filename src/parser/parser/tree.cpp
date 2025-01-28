#include <iostream>
#include <memory>
#include <vector>

#include "node_type.h"
#include "tree.h"

namespace Compiler::Parsing {

Tree::Tree(Lexing::Token start_token)
{
    this->root = std::make_unique<AstNode>(AstNode(NodeType::Start, start_token));
}

AstNode& Tree::GetRoot() const
{
    return *root.get();
}

void Tree::PrintTree(AstNode& root_node)
{
    if (root_node.IsLeaf()) {
        std::cout << &root_node;
        return;
    }

    auto& children = root_node.GetChildren();

    for (int i = 0; i < children.size(); i++) {
        PrintTree(*children[i].get());
    }
}

void Tree::AddNode(AstNode& parent, NodeType type)
{
    parent.GetChildren().push_back(std::make_unique<AstNode>(AstNode(type, Lexing::Token())));
}

void Tree::AddNodeLeaf(AstNode& parent) { }

}; // namespace Compiler::Parsing
