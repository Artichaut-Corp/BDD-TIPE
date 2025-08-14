#include "../Data Process System/Table.h"
#include "../Operation/Join.h"
#include "../Operation/Proj.h"
#include "../Operation/Select.h"
#include <memory>
#include <variant>

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

    Table Pronf()
    {
        Table result;

        Table tFg, tFd;
        bool hasFg = false, hasFd = false;

        if (m_Fg) {
            tFg = m_Fg->Pronf();
            hasFg = true;
        }
        if (m_Fd) {
            tFd = m_Fd->Pronf();
            hasFd = true;
        }

        result = std::visit([&](auto* op) -> Table {
            using T = std::decay_t<decltype(*op)>;

            if constexpr (std::is_same_v<T, Join>) {
                if (hasFg && hasFd)
                    return op->Exec(tFg, tFd);
                else
                    throw std::runtime_error("Join requires two children tables");
            } else if constexpr (std::is_same_v<T, Proj>) {
                if (hasFg) {
                    op->edit_table(tFg);
                    return op->Exec();
                } else
                    throw std::runtime_error("Proj requires one child table");
            } else if constexpr (std::is_same_v<T, Select>) {
                if (hasFg) {
                    op->edit_table(tFg);
                    return op->Exec();
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
