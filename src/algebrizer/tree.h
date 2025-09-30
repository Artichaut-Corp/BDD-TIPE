#include "../data_process_system/table.h"
#include "ikea.h"

#include "../operation/join.h"
#include "../operation/proj.h"
#include "../operation/select.h"

#include <memory>
#include <variant>
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

    Table* Pronf(Ikea* Tables)
    {
        Table* result = nullptr;

        Table* tFg = nullptr;
        Table* tFd = nullptr;

        if (m_Fg) {
            tFg = m_Fg->Pronf(Tables);
        }
        if (m_Fd) {
            tFd = m_Fd->Pronf(Tables);
        }
        result = std::visit([&](auto* op) -> Table* { // <-- retour explicite Table*
            using T = std::decay_t<decltype(*op)>;
            if constexpr (std::is_same_v<T, Join>) {
                if (tFg && tFd) {
                    return op->Exec(tFg, tFd); // doit retourner Table*
                } else {
                    return op->Exec(Tables->GetTableByName(op->GetLTable()), Tables->GetTableByName(op->GetRTable()));
                }
            } else if constexpr (std::is_same_v<T, Proj>) {
                if (tFg) {
                    return op->Exec(tFg); // doit retourner Table*
                } else {
                    return op->Exec(Tables->GetTableByName(op->GetTableName()));
                }
            } else if constexpr (std::is_same_v<T, Select>) {
                if (tFg) {
                    return op->Exec(tFg); // doit retourner Table*
                } else {
                    return op->Exec(Tables->GetTableByName(op->GetTableName()));
                }
            } else {
                throw std::runtime_error("Unknown node type");
            }
        },
            m_Type);

        return result;
    }

    void SetNodeRootInfo(std::string LeftTableName, std::string RightTableName)
    {
        std::visit([&](auto* op) -> void { // <-- retour explicite Table*
            using T = std::decay_t<decltype(*op)>;
            if constexpr (std::is_same_v<T, Join>) {
                op->SetRootInfo(LeftTableName, RightTableName);
            } else if constexpr (std::is_same_v<T, Proj>) {
                op->SetRootInfo(LeftTableName);
            } else if constexpr (std::is_same_v<T, Select>) {
                op->SetRootInfo(LeftTableName);
            } else {
                throw std::runtime_error("Unknown node type");
            }
        },
            m_Type);
    }

    void AfficheArbreExec(std::ostream& out)
    {
        auto s = Utils::Stack<Node*>();

        s.push(this);

        while (!s.empty()) {
            auto n = s.pop();

            if (n->m_Fg != nullptr) {
                s.push(n->m_Fg);
            }
            if (n->m_Fd != nullptr) {
                s.push(n->m_Fd);
            }
            std::visit([&](auto* op) -> void { // <-- retour explicite Table*
                using T = std::decay_t<decltype(*op)>;
                if constexpr (std::is_same_v<T, Join>) {
                    out << "Jointure entre la table " << op->GetLTable() << " et " << op->GetRTable() << "\n";
                } else if constexpr (std::is_same_v<T, Proj>) {
                    out << "Projection sur " << op->GetTableName() << "\n";
                } else if constexpr (std::is_same_v<T, Select>) {
                    out << "Selection sur " << op->GetTableName() << "\n";
                } else {
                    throw std::runtime_error("Unknown node type");
                }
            },
                n->m_Type);
        }

        out << std::endl;
        ;
    }
};
}