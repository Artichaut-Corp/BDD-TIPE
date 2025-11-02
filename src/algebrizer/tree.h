#include "../data_process_system/namingsystem.h"
#include "../data_process_system/table.h"
#include "../operation/join.h"
#include "../operation/proj.h"
#include "../operation/select.h"
#include "../parser/expression.h"
#include "ikea.h"
#include <unordered_set>
#include <variant>

#ifndef TREE_H

#define TREE_H

namespace Database::QueryPlanning {
using NodeType = std::variant<Join*, Proj*, Select*>; // le type root est censé être la racine de la query et ne jamais parti de là

class Node {
private:
    NodeType m_Type;
    Node* m_Fg = nullptr;
    Node* m_Fd = nullptr;

public:
    Node(NodeType type)
        : m_Type(type)
    {
    }

    void AddChild(bool left, Node* child)
    {
        if (left)
            m_Fg = child;
        else
            m_Fd = child;
    }

    Table* Pronf(Ikea* Tables);

    void printBT(const std::string& prefix, const Node* node, bool isLeft, std::ostream& out);
    
    void printBT(std::ostream& out);


    std::unordered_set<ColonneNamesSet*>* SelectionDescent(Ikea* Tables, Select* MainSelect);
};
}
#endif // ! TREE_H