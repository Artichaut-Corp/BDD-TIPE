#ifndef UNION_FIND_UTILS_H
#define UNION_FIND_UTILS_H
#include "algebrizer/tree.h"

#include <string>
#include <unordered_map>
namespace Database::Utils {

class UnionFind {
private:
    std::unordered_map<std::string, Database::QueryPlanning::Node*> TableToNode;
    std::unordered_map<std::string, std::string> parent;
    std::unordered_map<std::string, int> rang;

public:
    UnionFind()
    {
    }

    Database::QueryPlanning::Node* AddElem(Database::QueryPlanning::Join* join)
    {
        auto parentTableL = trouver(join->GetLTable().GetMainName());

        auto NodeL = TableToNode[parentTableL];

        auto parentTableR = trouver(join->GetRTable().GetMainName());

        auto NodeR = TableToNode[parentTableR];

        auto node = new Database::QueryPlanning::Node(join);

        node->AddChild(true, NodeL);
        node->AddChild(false, NodeR);

        unir(join->GetLTable().GetMainName(), join->GetRTable().GetMainName(), node);

        return node;
    }
    std::string trouver(const std::string table)
    {
        if (parent.contains(table)) {
            auto parent_table = parent[table];
            if (parent_table == table) {
                return table;
            } else {
                auto parent_parent_table = trouver(parent_table);
                parent[table] = parent_parent_table;
                return parent_parent_table;
            }
        } else {
            parent[table] = table;
            rang[table] = 0;
            TableToNode[table] = nullptr;
            return table;
        }
    }
    void unir(std::string tableL, std::string tableR, Database::QueryPlanning::Node* node)
    {
        auto parent_tableL = trouver(tableL);
        auto parent_tableR = trouver(tableR);
        if (parent_tableL == parent_tableR) {
            TableToNode[parent_tableL] = node;
        } else {
            auto rangL = rang[parent_tableL];
            auto rangR = rang[parent_tableR];
            if (rangL > rangR) {
                TableToNode[parent_tableL] = node;
                parent[parent_tableR] = parent_tableL;
            } else {
                TableToNode[parent_tableR] = node;
                parent[parent_tableL] = parent_tableR;
                if (rangL == rangR) {
                    rang[parent_tableR] = rangR + 1;
                }
            }
        }
    }
};

} // namespace Database::Utils

#endif //! UNION_FIND_UTILS_H
