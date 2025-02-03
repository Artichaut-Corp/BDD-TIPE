#include "../lexer/tokens.h"
#include <memory>
#include <vector>
#ifndef TREE_H
#define TREE_H

namespace Compiler::Parsing{
template <typename T>

class Node {

private:
	T type;

    std::vector<std::unique_ptr<Node>>childrens;

public:
    Node<T>(T ast_type)
    {
        this->type = ast_type;
        this->childrens = std::vector<std::unique_ptr<Node>>();
    }

	std::vector<std::unique_ptr<Node<T>>> GetChildren (){return childrens;}

    void PrintTreePref();

    void PrintTreeLarg();

    void AddNode(Node<T>& parent);

    void Traverse(Node<T>& root);
};
} 
#endif