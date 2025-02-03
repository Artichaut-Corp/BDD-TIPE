#ifndef UTILS_H
#define UTILS_H

#include <iostream>
#include <optional>
#include <ostream>

namespace Compiler::Utils {

template <typename T>
class ListNode {

public:
    std::optional<ListNode*> next = std::nullopt;

    T data;

    ListNode() = default;
    ListNode(T data)
    {
        this->data = data;
    }

    void set_next(ListNode* data)
    {
        this->next = std::make_optional(data);
    }
};

template <typename U>
class LinkedList {
protected:
    std::optional<ListNode<U>*> HEAD;

private:
    std::optional<ListNode<U>> get_head()
    {
        return HEAD.has_value() ? std::make_optional(*HEAD.value()) : std::nullopt;
    }

public:
    LinkedList()
    {
        HEAD = std::nullopt;
    }

    virtual ~LinkedList() { }

    void append(U data)
    {
        ListNode<U>* new_node = new ListNode<U>(data);

        // If head is null, it means we are adding the first element which point to the HEAD
        if (get_head() == std::nullopt) {
            HEAD = std::make_optional(new_node);
        } else {
            new_node->set_next(*HEAD);
            HEAD = std::make_optional(new_node);
        }

        return;
    }

    void push(U data)
    {
        ListNode<U>* new_node = new ListNode<U>(data);
        ListNode<U> current = get_head().value_or(ListNode<U>());

        while (current.next->has_value()) {
            current = current.next->value();
        }

        current.next = std::make_optional(new_node);

        return;
    }

    void insert_at(U data, int indice)
    {
        ListNode<U>* new_node = new ListNode<U>(data);
        ListNode<U> current = get_head().value_or(ListNode<U>());

        int pos = 0;

        do {
            current = current->next.value();
            if (++pos >= indice) {
                break;
            }
        } while (current.next.has_value());

        current.next = std::make_optional(new_node);

        return;
    }

    std::optional<U> get_at(int indice)
    {
        ListNode<U> current = get_head().value_or(ListNode<U>());

        int pos = 0;
        while (current.next != std::nullopt) {
            current = *current.next.value();
            if (++pos >= indice) {
                break;
            }
        }

        return std::make_optional(current.data);
    }

    std::optional<U> get_first()
    {
        return HEAD.has_value() ? std::make_optional(HEAD.value()->data) : std::nullopt;
    }

    std::optional<U> get_last()
    {
        ListNode<U> current = get_head().value_or(ListNode<U>());
        while (current.next != std::nullopt) {
            current = *current.next.value();
        }
        return std::make_optional(current.data);
    }

    void print_all()
    {
        ListNode<U> current = get_head().value_or(ListNode<U>());
        // Tant que l'on a pas atteint
        while (current.next != std::nullopt) {
            std::cout << current.value().data << "-> ";
            current = *current.next.value();
        }
        std::cout << "NULL\n";
        return;
    }
}; // LinkedList

}; // namespace Compiler::Utils

#endif // !UTILS_H
