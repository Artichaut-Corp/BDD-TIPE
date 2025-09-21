#include "../data_process_system/table.h"
#include "ikea.h"

#include "../operation/join.h"
#include "../operation/proj.h"
#include "../operation/select.h"

#include <memory>
#include <variant>
namespace Database::QueryPlanning {
using NodeType = std::variant<Join*, Proj*, Select*>; //le type root est censé être la racine de la query et ne jamais parti de là

class Node {
private:
    NodeType m_Type;
    std::unique_ptr<Node> m_Fg = nullptr;
    std::unique_ptr<Node> m_Fd = nullptr;

public:
    Node(NodeType type)
        : m_Type(type)
    {
    }

    void AddChild(bool left, std::unique_ptr<Node> child)
    {
        if (left)
            m_Fg = std::move(child);
        else
            m_Fd = std::move(child);
    }

    Table* Pronf(Ikea Tables)
    {
        Table* result = nullptr;

        Table* tFg = nullptr;
        Table* tFd = nullptr;

        if (m_Fg)
            tFg = m_Fg->Pronf(Tables);
        if (m_Fd)
            tFd = m_Fd->Pronf(Tables);

        result = std::visit([&](auto* op) -> Table* { // <-- retour explicite Table*
            using T = std::decay_t<decltype(*op)>;

            if constexpr (std::is_same_v<T, Join>) {
                if (tFg && tFd)
                    return op->Exec(tFg, tFd); // doit retourner Table*
                else
                    
                    throw std::runtime_error("Join requires two children tables");
            } else if constexpr (std::is_same_v<T, Proj>) {
                if (tFg)
                    return op->Exec(tFg); // doit retourner Table*
                else
                    throw std::runtime_error("Proj requires one child table");
            } else if constexpr (std::is_same_v<T, Select>) {
                if (tFg)
                    return op->Exec(tFg); // doit retourner Table*
                else
                    throw std::runtime_error("Select requires one child table");
            } else{
                throw std::runtime_error("Unknown node type");
            }
        },
            m_Type);

        return result;
    }
};
}