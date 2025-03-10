#include <memory>

#ifndef NODE_H

#define NODE_H

namespace Database::Parsing {

template <typename U>
class Node {
    std::unique_ptr<U> m_Type;

public:
    Node(U* type) { m_Type = std::unique_ptr<U>(type); }
};

} // namespace parsing

#endif // !NODE_H
