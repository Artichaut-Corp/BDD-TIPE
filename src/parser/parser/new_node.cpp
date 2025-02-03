#include <iostream>
#include <memory>
#include <queue>

#include "new_node.h"

namespace Compiler::Parsing {
	
template <typename T>

void Node<T>::PrintTreeLarg()
{
    std::queue<std::unique_ptr<Node<T>>> file = std::queue<std::unique_ptr<Node<T>>>();
    file.push(this);
    while (!file.empty()) {
        std::unique_ptr<Node<T>> NoeudPtr = file.pop();
        std::vector<std::unique_ptr<Node<T>>> fils = NoeudPtr.GetChildren();
        for (std::unique_ptr<Node<T>> e : fils)
            file.push(e);
        std::cout << NoeudPtr;
    }
}

template <typename T>

void Node<T>::PrintTreePref()
{	
	std::cout << " ( "; 
    std::cout << this;
    std::vector<std::unique_ptr<Node<T>>> fils = GetChildren();
    for (std::unique_ptr<Node<T>> e : fils)
        e.PrintTreePref();
    std::cout << " ) ";
};

template <typename T>
std::ostream& operator<<(std::ostream& os, const Node<T>& t)
{
    os << t.type;

    return os;
}
}; // namespace Compiler::Parsing
