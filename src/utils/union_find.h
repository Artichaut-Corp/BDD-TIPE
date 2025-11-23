#ifndef UNION_FIND_UTILS_H
#define UNION_FIND_UTILS_H
#include "../algebrizer/tree.h"

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
        std::unordered_map<std::string, std::string> parent = std::unordered_map<std::string, std::string>();
        std::unordered_map<std::string, Database::QueryPlanning::Node*> TableToNode = std::unordered_map<std::string, Database::QueryPlanning::Node*>();

        std::unordered_map<std::string, int> rang = std::unordered_map<std::string, int>();
    }

    Database::QueryPlanning::Node* AddElem(Database::QueryPlanning::Join* join)
    {
        auto parenttableL = trouver(join->GetLTable()->GetMainName());
        auto NodeL = TableToNode[parenttableL];
        auto parenttableR = trouver(join->GetRTable()->GetMainName());
        auto NodeR = TableToNode[parenttableR];
        auto node = new Database::QueryPlanning::Node(join);
        node->AddChild(true, NodeL);
        node->AddChild(false, NodeR);

        unir(join->GetLTable()->GetMainName(), join->GetRTable()->GetMainName(), node);
        return node;
    }
    std::string trouver(std::string table)
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
        TableToNode[tableL] = node;
        TableToNode[tableR] = node;
        auto parent_tableL = trouver(tableL);
        auto parent_tableR = trouver(tableR);
        if (parent_tableL != parent_tableR) {
            auto rangL = rang[parent_tableL];
            auto rangR = rang[parent_tableR];
            if (rangL > rangR) {
                parent[parent_tableR] = parent_tableL;
            } else {
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
