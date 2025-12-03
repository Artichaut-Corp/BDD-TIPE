#include "../data_process_system/meta-table.h"
#include "../data_process_system/namingsystem.h"
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

    std::shared_ptr<MetaTable> Pronf(Ikea* Tables, int type_of_join);

    void printBT(const std::string& prefix, const Node* node, bool isLeft, std::ostream& out);

    void printBT(std::ostream& out);

    std::shared_ptr<TableNamesSet> GetTableUsedByCurrL()
    {
        if (std::holds_alternative<Join*>(m_Type)) {
            auto op = std::get<Join*>(m_Type);
            return op->GetLTable();
        } else if (std::holds_alternative<Proj*>(m_Type)) {
            auto op = std::get<Proj*>(m_Type);
            return op->GetTableName();
        } else if (std::holds_alternative<Select*>(m_Type)) {
            auto op = std::get<Select*>(m_Type);
            return op->GetTableName();
        } else {
            throw std::runtime_error("Unknown node type");
        }
    }
    std::shared_ptr<TableNamesSet> GetTableUsedByCurrR()
    {
        if (std::holds_alternative<Join*>(m_Type)) {
            auto op = std::get<Join*>(m_Type);
            return op->GetRTable();
        } else if (std::holds_alternative<Proj*>(m_Type)) {
            auto op = std::get<Proj*>(m_Type);
            return op->GetTableName();
        } else if (std::holds_alternative<Select*>(m_Type)) {
            auto op = std::get<Select*>(m_Type);
            return op->GetTableName();
        } else {
            throw std::runtime_error("Unknown node type");
        }
    }
    std::unordered_set<std::shared_ptr<ColonneNamesSet>>* SelectionDescent(Ikea* Tables, Select* MainSelect);

    void InsertProj(std::unordered_set<std::shared_ptr<ColonneNamesSet>>* ColumnToKeep);

    Node* GetLeftPtr() { return m_Fg; }

    NodeType GetAction() { return m_Type; }
};
}
#endif // ! TREE_H
