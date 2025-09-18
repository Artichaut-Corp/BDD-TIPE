#include "../data_process_system/table.h"

#include "../operation/join.h"
#include "../operation/proj.h"
#include "../operation/select.h"

#include <memory>
#include <variant>
namespace Database::QueryPlanning {
using NodeType = std::variant<Join*, Proj*, Select*>;


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

    std::unique_ptr<Table> Pronf()
    {
        std::unique_ptr<Table> result;

        std::unique_ptr<Table> tFg;
        std::unique_ptr<Table> tFd;
        bool hasFg = false, hasFd = false;

        if (m_Fg) {
            tFg = m_Fg->Pronf();
            hasFg = true;
        }
        if (m_Fd) {
            tFd = m_Fd->Pronf();
            hasFd = true;
        }

        result = std::visit([&](auto* op) -> std::unique_ptr<Table> {
            using T = std::decay_t<decltype(*op)>;

            if constexpr (std::is_same_v<T, Join>) {
                if (hasFg && hasFd)
                    return op->Exec(tFg, tFd);
                else
                    throw std::runtime_error("Join requires two children tables");
            } else if constexpr (std::is_same_v<T, Proj>) {
                if (hasFg) {
                    return op->Exec(tFg);
                } else
                    throw std::runtime_error("Proj requires one child table");
            } else if constexpr (std::is_same_v<T, Select>) {
                if (hasFg) {
                    return op->Exec(tFg);
                } else
                    throw std::runtime_error("Select requires one child table");
            } else {
                throw std::runtime_error("Unknown node type");
            }
        },
            m_Type);

        return result;
    }
};
}