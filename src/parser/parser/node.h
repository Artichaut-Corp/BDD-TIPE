#include <memory>
#include <vector>

#ifndef NODE_H

#define NODE_H

namespace Compiler::Parsing {

template <typename U>
class Node {
    std::unique_ptr<U> m_Type;

    std::vector<Node*> m_Children;

public:
    Node(U* type) { m_Type = std::unique_ptr<U>(type); }
};

} // namespace parsing

#endif // !NODE_H
