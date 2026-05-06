#include "linked_list.h"

#ifndef STACK_H

#define STACK_H

namespace Database::Utils {

template <typename T>
class Stack {
    LinkedList<T> m_Data;

public:
    Stack()
        : m_Data(LinkedList<T>())
    {
    }

    bool empty() const { return m_Data.is_empty(); }

    void push(const T element)
    {
        m_Data.append(element);
    }

    const T& pop()
    {
        const T& value = m_Data.get_first().value();

        m_Data.advance();

        return value;
    }
    const T& first()
    {        
        const T& value = m_Data.get_first().value();

        return value;
    }
};

} // namespace Database::Utils

#endif
