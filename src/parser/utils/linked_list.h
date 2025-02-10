#ifndef UTILS_H
#define UTILS_H

#include <iostream>
#include <optional>
#include <ostream>

namespace Compiler::Utils {

template <typename T>
class ListNode {

public:
    std::optional<ListNode*> m_Next = std::nullopt;

    T m_Data;

    ListNode() = default;
    ListNode(T data)
    {
        this->m_Data = data;
    }

    void set_next(ListNode* data)
    {
        this->m_Next = std::make_optional(data);
    }
};

template <typename U>
std::ostream& operator<<(std::ostream& os, const ListNode<U>& node)
{
    os << node.data;

    return os;
}

template <typename U>
class LinkedList {
protected:
    std::optional<ListNode<U>*> HEAD;

private:
    std::optional<ListNode<U>*> get_head()
    {
        return HEAD.has_value() ? std::make_optional(HEAD.value()) : std::nullopt;
    }

public:
    LinkedList()
    {
        HEAD = std::nullopt;
    }

    ~LinkedList()
    {
    }

    void append(U data)
    {
        ListNode<U>* new_node = new ListNode<U>(data);

        // If head is null, it means we are adding the first element which point to the HEAD
        if (get_head() == std::nullopt) {
            HEAD = std::make_optional(new_node);
        } else {
            new_node->set_next(get_head().value());
            HEAD = std::make_optional(new_node);
        }

        return;
    }

    void push(U data)
    {
        ListNode<U>* new_node = new ListNode<U>(data);
        ListNode<U>* current = get_head().value_or(nullptr);

        while (current->m_Next.has_value()) {
            current = current->m_Next.value();
        }

        current->m_Next = std::make_optional(new_node);

        return;
    }

    void insert_at(U data, int indice)
    {
        ListNode<U>* new_node = new ListNode<U>(data);
        ListNode<U>* current = get_head().value_or(nullptr);

        int pos = 0;

        do {
            current = *current->m_Next.value();
            if (++pos >= indice) {
                break;
            }
        } while (current->m_Next.has_value());

        current->m_Next = std::make_optional(new_node);

        return;
    }

    std::optional<U> get_at(int indice)
    {
        ListNode<U>* current = get_head().value_or(nullptr);

        int pos = 0;
        while (current->m_Next != std::nullopt) {
            current = current->m_Next.value();
            if (++pos >= indice) {
                break;
            }
        }

        return std::make_optional(current->m_Data);
    }

    std::optional<U> get_first() const
    {
        return HEAD.has_value() ? std::make_optional(HEAD.value()->m_Data) : std::nullopt;
    }

    std::optional<U> get_last()
    {
        ListNode<U>* current = get_head().value_or(nullptr);
        while (current->m_Next != std::nullopt) {
            current = current->m_Next.value();
        }
        return std::make_optional(current->m_Data);
    }

    void advance()
    {
        ListNode<U>* current = get_head().value_or(nullptr);

        if (current->m_Next != std::nullopt) {
            HEAD = std::make_optional(current->m_Next.value());
        } else {
            HEAD = std::nullopt;
        }
    }

    bool is_empty()
    {
        return !HEAD.has_value();
    }

    void print_all()
    {
        std::optional<ListNode<U>*> current = get_head().value_or(nullptr);

        if (!HEAD.has_value())
            return;

        // Tant que l'on a pas atteint
        while (current != std::nullopt) {
            std::cout << current.value()->m_Data << "-> ";
            current = current.value()->m_Next;
        }
        std::cout << "NULL\n";
        return;
    }
}; // LinkedList

}; // namespace Compiler::Utils

#endif // !UTILS_H
